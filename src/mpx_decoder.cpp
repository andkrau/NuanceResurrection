#include "mpx_decoder.h"

#include <chrono>
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace {
constexpr size_t kHeaderSize       = 0x81000;
constexpr size_t kReadChunk        = 64 * 1024;
constexpr const char* kMagic       = "NUON-MOVIELIB";
constexpr size_t kMagicLen         = 13;

// Convert one BGRA pixel (libswscale output) into a NUON YCrCbA32 pixel.
// NUON pixType 4 layout (matches what the video.cpp shader expects when
// `pixType == 4`): bytes are [Y, Cb, Cr, A] little-endian — same packing
// that VidConfig produces for normal NUON-rendered frames.
inline uint32 BgraToYCrCbA(uint8 b, uint8 g, uint8 r) {
  // BT.601 RGB→YCbCr (limited range to match NUON content):
  const int y  = ( 66 * r + 129 * g +  25 * b + 128) >> 8;
  const int cb = (-38 * r -  74 * g + 112 * b + 128) >> 8;
  const int cr = (112 * r -  94 * g -  18 * b + 128) >> 8;
  const uint8 Y  = (uint8)(y  + 16);
  const uint8 Cb = (uint8)(cb + 128);
  const uint8 Cr = (uint8)(cr + 128);
  // Packing observed in NUON content: [Y, Cb, Cr, A] in memory order.
  // The shader samples this as RGBA texture, then uses .r/.g/.b as Y/Cb/Cr.
  return ((uint32)0xFF << 24) | ((uint32)Cr << 16) | ((uint32)Cb << 8) | Y;
}
} // namespace

MpxDecoder::MpxDecoder() = default;

MpxDecoder::~MpxDecoder() {
  Close();
}

bool MpxDecoder::Open(const char* path) {
  Close();
  mPath = path ? path : "";
  mFile = fopen(mPath.c_str(), "rb");
  if (!mFile) {
    fprintf(stderr, "[MPX] open failed: %s\n", mPath.c_str());
    return false;
  }

  // Verify NUON-MOVIELIB magic.
  char magic[16] = {};
  if (fread(magic, 1, kMagicLen, mFile) != kMagicLen ||
      memcmp(magic, kMagic, kMagicLen) != 0) {
    fprintf(stderr, "[MPX] not a NUON-MOVIELIB file: %s\n", mPath.c_str());
    fclose(mFile); mFile = nullptr;
    return false;
  }

  if (fseeko(mFile, (off_t)kHeaderSize, SEEK_SET) != 0) {
    fprintf(stderr, "[MPX] seek to MPEG payload failed: %s\n", mPath.c_str());
    fclose(mFile); mFile = nullptr;
    return false;
  }

  if (!InitDecoder()) {
    fclose(mFile); mFile = nullptr;
    return false;
  }

  fprintf(stderr, "[MPX] opened %s\n", mPath.c_str());
  mEndOfStream.store(false, std::memory_order_release);
  mStopRequested.store(false, std::memory_order_release);
  mRunning.store(true, std::memory_order_release);
  mThread = std::thread(&MpxDecoder::DecodeThread, this);
  return true;
}

bool MpxDecoder::InitDecoder() {
  const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);
  if (!codec) {
    fprintf(stderr, "[MPX] avcodec_find_decoder(MPEG2VIDEO) failed\n");
    return false;
  }
  mParser = av_parser_init(codec->id);
  if (!mParser) { fprintf(stderr, "[MPX] av_parser_init failed\n"); return false; }
  mAvCtx = avcodec_alloc_context3(codec);
  if (!mAvCtx) { fprintf(stderr, "[MPX] avcodec_alloc_context3 failed\n"); return false; }
  if (avcodec_open2(mAvCtx, codec, nullptr) < 0) {
    fprintf(stderr, "[MPX] avcodec_open2 failed\n");
    return false;
  }
  mPkt   = av_packet_alloc();
  mFrame = av_frame_alloc();
  if (!mPkt || !mFrame) { fprintf(stderr, "[MPX] alloc packet/frame failed\n"); return false; }
  return true;
}

void MpxDecoder::Close() {
  mStopRequested.store(true, std::memory_order_release);
  if (mThread.joinable()) mThread.join();
  mRunning.store(false, std::memory_order_release);

  if (mPkt)   { av_packet_free(&mPkt);   mPkt   = nullptr; }
  if (mFrame) { av_frame_free(&mFrame);  mFrame = nullptr; }
  if (mAvCtx) { avcodec_free_context(&mAvCtx); mAvCtx = nullptr; }
  if (mParser){ av_parser_close(mParser); mParser = nullptr; }
  if (mSws)   { sws_freeContext(mSws);    mSws    = nullptr; }
  if (mFile)  { fclose(mFile);            mFile   = nullptr; }

  std::lock_guard<std::mutex> lg(mFrameMutex);
  mLatestFrame.clear();
  mFrameCounter = 0;
  mLastCopiedFrame = 0;
  mWidth = mHeight = 0;
  mEndOfStream.store(false, std::memory_order_release);
}

void MpxDecoder::DecodeThread() {
  std::vector<uint8> buf(kReadChunk);
  while (!mStopRequested.load(std::memory_order_acquire)) {
    const size_t got = fread(buf.data(), 1, buf.size(), mFile);
    if (got == 0) {
      // Drain decoder, then mark EOF.
      avcodec_send_packet(mAvCtx, nullptr);
      while (avcodec_receive_frame(mAvCtx, mFrame) == 0) {
        ConvertAndPublish(mFrame);
      }
      fprintf(stderr, "[MPX] EOF for %s (%llu frames decoded)\n",
              mPath.c_str(), (unsigned long long)mFrameCounter);
      mEndOfStream.store(true, std::memory_order_release);
      break;
    }

    const uint8* in = buf.data();
    int          inLen = (int)got;
    while (inLen > 0 && !mStopRequested.load(std::memory_order_acquire)) {
      uint8* outData = nullptr;
      int    outSize = 0;
      const int used = av_parser_parse2(mParser, mAvCtx,
                                        &outData, &outSize,
                                        in, inLen,
                                        AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
      if (used < 0) break;
      in    += used;
      inLen -= used;
      if (outSize > 0) {
        mPkt->data = outData;
        mPkt->size = outSize;
        if (avcodec_send_packet(mAvCtx, mPkt) == 0) {
          while (avcodec_receive_frame(mAvCtx, mFrame) == 0) {
            ConvertAndPublish(mFrame);
          }
        }
      }
    }
  }
}

void MpxDecoder::ConvertAndPublish(AVFrame* frame) {
  if (frame->width <= 0 || frame->height <= 0) return;
  const uint32 w = (uint32)frame->width;
  const uint32 h = (uint32)frame->height;

  // Pull libswscale to BGRA, then convert per-pixel to YCrCbA32 once. We
  // could go directly via a colour matrix, but the BGRA intermediate keeps
  // the swscale path standard and trivial to debug.
  if (!mSws || mWidth != w || mHeight != h) {
    if (mSws) { sws_freeContext(mSws); mSws = nullptr; }
    mSws = sws_getContext((int)w, (int)h, (AVPixelFormat)frame->format,
                          (int)w, (int)h, AV_PIX_FMT_BGRA,
                          SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!mSws) {
      fprintf(stderr, "[MPX] sws_getContext failed\n");
      return;
    }
    mWidth = w; mHeight = h;
  }

  std::vector<uint8> bgra(w * h * 4);
  uint8* dst[1]   = { bgra.data() };
  int    dstStr[1]= { (int)(w * 4) };
  sws_scale(mSws, frame->data, frame->linesize, 0, (int)h, dst, dstStr);

  std::vector<uint8> ycrcba(w * h * 4);
  for (uint32 y = 0; y < h; y++) {
    const uint8* srcRow = bgra.data() + y * w * 4;
    uint32* dstRow = (uint32*)(ycrcba.data() + y * w * 4);
    for (uint32 x = 0; x < w; x++) {
      dstRow[x] = BgraToYCrCbA(srcRow[x*4 + 0], srcRow[x*4 + 1], srcRow[x*4 + 2]);
    }
  }

  {
    std::lock_guard<std::mutex> lg(mFrameMutex);
    mLatestFrame = std::move(ycrcba);
    mFrameCounter++;
  }
}

bool MpxDecoder::CopyLatestYCrCbA32(uint8* dst, uint32 dstPitchBytes,
                                    uint32 dstWidth, uint32 dstHeight) {
  std::lock_guard<std::mutex> lg(mFrameMutex);
  if (mLatestFrame.empty() || mWidth == 0 || mHeight == 0) return false;
  if (mFrameCounter == mLastCopiedFrame)                  return false;

  const uint32 copyW = (dstWidth  < mWidth)  ? dstWidth  : mWidth;
  const uint32 copyH = (dstHeight < mHeight) ? dstHeight : mHeight;
  const uint32 srcPitch = mWidth * 4;
  for (uint32 y = 0; y < copyH; y++) {
    memcpy(dst + (size_t)y * dstPitchBytes,
           mLatestFrame.data() + (size_t)y * srcPitch,
           (size_t)copyW * 4);
  }
  mLastCopiedFrame = mFrameCounter;
  return true;
}
