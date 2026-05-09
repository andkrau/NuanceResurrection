#pragma once

#include "basetypes.h"
#include <atomic>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" {
struct AVCodecContext;
struct AVCodecParserContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
}

// Decoder for NUON-MOVIELIB (.mpx) cutscene files.
//
// .mpx is a thin wrapper: 0x81000 bytes of header / zero padding, then a
// raw MPEG-2 video elementary stream (no PES, no audio). Iron Soldier 3,
// Freefall 3050 and a few others use it for intros / level briefings.
//
// This decoder:
// - opens the file, seeks past the 0x81000 header
// - feeds bytes in chunks through libavcodec's MPEG-2 parser+decoder
// - exposes the most recently decoded frame as a contiguous BGRA / YCrCbA32
//   buffer that the rest of the emulator can blit into the NUON main video
//   channel
//
// All public methods are safe to call from the main MPE thread; decode runs
// on a background thread.
class MpxDecoder {
public:
  MpxDecoder();
  ~MpxDecoder();

  // Returns true on success. Detects the NUON-MOVIELIB magic, opens the
  // libavcodec MPEG-2 decoder and starts the background decode thread.
  bool Open(const char* path);

  // Stop background thread, close decoder, free buffers.
  void Close();

  bool IsOpen() const { return mRunning.load(std::memory_order_acquire); }

  // Output frame width / height in pixels (typical NUON cutscene: 704x480).
  uint32 Width()  const { return mWidth; }
  uint32 Height() const { return mHeight; }

  // Pull the most recently decoded frame as a NUON YCrCbA32 packed buffer
  // (4 bytes per pixel, layout matches what the YCrCbA→RGB shader in
  // RenderVideo expects). Width / height are returned via Width() / Height().
  // Returns true if a fresh frame was copied; false if no new frame yet
  // (caller may keep displaying the previous frame).
  bool CopyLatestYCrCbA32(uint8* dst, uint32 dstPitchBytes, uint32 dstWidth, uint32 dstHeight);

  // Has decoding finished (EOF reached, no more frames will be produced)?
  bool IsAtEnd() const { return mEndOfStream.load(std::memory_order_acquire); }

private:
  void DecodeThread();
  bool InitDecoder();
  void ConvertAndPublish(AVFrame* frame);

  std::string mPath;
  FILE* mFile = nullptr;

  AVCodecContext*       mAvCtx    = nullptr;
  AVCodecParserContext* mParser   = nullptr;
  AVPacket*             mPkt      = nullptr;
  AVFrame*              mFrame    = nullptr;
  SwsContext*           mSws      = nullptr;

  uint32 mWidth  = 0;
  uint32 mHeight = 0;

  // Latest decoded frame in NUON YCrCbA32 layout.
  std::mutex mFrameMutex;
  std::vector<uint8> mLatestFrame;
  uint64 mFrameCounter = 0;
  uint64 mLastCopiedFrame = 0;

  std::thread mThread;
  std::atomic<bool> mRunning{false};
  std::atomic<bool> mStopRequested{false};
  std::atomic<bool> mEndOfStream{false};
};
