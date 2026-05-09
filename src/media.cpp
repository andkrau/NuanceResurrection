#include "basetypes.h"
#include <windows.h>
#include <string>
#include "byteswap.h"
#include "Bios.h"
#include "mpe.h"
#include "media.h"
#include "mpe_alloc.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "iso9660.h"

extern NuonEnvironment nuonEnv;
extern uint32 mpeFlags[];

bool bCallingMediaCallback = false; // globally used

uint32 media_mpe_allocated = 0;
uint32 media_mpe = 0;

static MediaDevInfo DeviceInfo[] = {
  {0,0,0,0,0,0},
  {0,0,0,0,0,0},
  {0,0,0,0,0,0},
  {(int32)eMedia::MEDIA_BOOT_DEVICE,0,BLOCK_SIZE_DVD,0,0,DATA_RATE_DVD},
  {(int32)eMedia::MEDIA_BOOT_DEVICE,0,BLOCK_SIZE_DVD,0,0,DATA_RATE_DVD}};

void MediaShutdownMPE(MPE &mpe)
{
  if(media_mpe_allocated)
  {
    //Halt the media processor
    nuonEnv.mpe[media_mpe].Halt();
    //Disable commrecv interrupts
    nuonEnv.mpe[media_mpe].inten1 &= ~(INT_COMMRECV);
    //Mark as free and clear Mini BIOS flag
    mpeFlags[media_mpe] &= ~(MPE_ALLOC_USER | MPE_ALLOC_BIOS | MPE_HAS_MINI_BIOS);
    nuonEnv.bProcessorStartStopChange = true;
  }

  media_mpe_allocated = 0;
}

void MediaInitMPE(const uint32 i)
{
  bool loadStatus;

  mpeFlags[i] |= (MPE_ALLOC_BIOS | MPE_HAS_MINI_BIOS);

  //nuonEnv.mpe[i]->inten1 = INT_COMMRECV;

  //Load the minibios code
  if(i == 0)
  {
    loadStatus = nuonEnv.mpe[i].LoadCoffFile("minibios.cof",false);

    if(!loadStatus)
    {
      char tmp[1024];
      GetModuleFileName(NULL, tmp, 1024);
      string tmps(tmp);
      size_t idx = tmps.find_last_of("/\\");
      if (idx != string::npos)
        tmps = tmps.substr(0, idx+1);
      loadStatus = nuonEnv.mpe[i].LoadCoffFile((tmps+"minibios.cof").c_str(),false);
      if(!loadStatus)
        MessageBox(NULL,"Missing File!","Could not load minibios.cof",MB_OK);
    }

    nuonEnv.mpe[i].intvec1 = MINIBIOS_INTVEC1_HANDLER_ADDRESS;
    nuonEnv.mpe[i].intvec2 = MINIBIOS_INTVEC2_HANDLER_ADDRESS;
  }
  else
  {
    loadStatus = nuonEnv.mpe[i].LoadCoffFile("minibiosX.cof",false);

    if(!loadStatus)
    {
      char tmp[1024];
      GetModuleFileName(NULL, tmp, 1024);
      string tmps(tmp);
      size_t idx = tmps.find_last_of('\\');
      if (idx != string::npos)
        tmps = tmps.substr(0, idx+1);
      loadStatus = nuonEnv.mpe[i].LoadCoffFile((tmps+"minibiosX.cof").c_str(),false);
      if(!loadStatus)
        MessageBox(NULL,"Missing File!","Could not load minibiosX.cof",MB_OK);
    }

    nuonEnv.mpe[i].intvec1 = MINIBIOSX_INTVEC1_HANDLER_ADDRESS;
    nuonEnv.mpe[i].intvec2 = MINIBIOSX_INTVEC2_HANDLER_ADDRESS;
  }

  //Mask interrupts
  nuonEnv.mpe[i].intctl = 0xAA;
  nuonEnv.mpe[i].UpdateInvalidateRegion(MPE_IRAM_BASE, MPE::overlayLengths[i]);

  //Set pcexec to the minibios spinwait routine.
  if(i == 0)
  {
    nuonEnv.mpe[i].sp = 0x20101BE0;
    nuonEnv.mpe[i].pcexec = MINIBIOS_ENTRY_POINT;
  }
  else
  {
    nuonEnv.mpe[i].pcexec = MINIBIOSX_SPINWAIT_ADDRESS;
  }

  media_mpe = i;
  media_mpe_allocated = 1;

  if(!(nuonEnv.mpe[i].mpectl & MPECTRL_MPEGO))
  {
    nuonEnv.mpe[i].mpectl |= MPECTRL_MPEGO;
    nuonEnv.bProcessorStartStopChange = true;
  }
}

void MediaInitMPE(MPE &mpe)
{
  //Check to see if the media code is already running on an MPE
  //in which case don't reallocate it and simply return the index
  //of the MPE that it is running on

  if(media_mpe_allocated)
  {
    mpe.regs[0] = media_mpe;
    return;
  }

  int32 which = -1;

  for(uint32 i = 0; i < 4; i++)
  {
    if((mpeFlags[i] & (MPE_ALLOC_BIOS | MPE_ALLOC_USER)) == 0)
    {
        which = i;
        MediaInitMPE(i);
        break;
    }
  }

  mpe.regs[0] = which;
}

static std::string fileNameArray[] = {"stdin","stdout","stderr","","","","","","","","","","","","","","","","",""};
// Per-handle flag: skip .mpx (NUON-MOVIELIB) reads so games like
// Iron Soldier 3 / Freefall 3050 / Crayon Shin-chan don't hang waiting
// for an MPEG decoder we don't have. MediaRead returns 0 blocks read
// for these handles; the game treats it as end-of-stream and proceeds.
// See andkrau/NuanceResurrection#36.
static bool mpxSkipArray[20] = {false};
static uint32 fileModeArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#define FIRST_DVD_FD (3)
#define LAST_DVD_FD (19)

// libavcodec decoder for .mpx (NUON-MOVIELIB) cutscenes. When MediaOpen
// sees an .mpx file we spin up a background MPEG-2 decoder; the render
// path in video.cpp pulls decoded frames for direct on-screen display.
// MediaRead still feeds the real bytes to the game so its NUON-MOVIELIB
// parser makes forward progress, even though the FMV hardware acks
// fmv.run polls for never come (no real FMV emulation). The end result:
// the cutscene is visible to the player. See nuance-stuck-loading.md.
#include "mpx_decoder.h"
static MpxDecoder* g_mpxDecoder[LAST_DVD_FD + 1] = {};

// Decoder spun up via NUANCE_FORCE_MPX (or `./nuance file.mpx` standalone
// path). When set, the probe / pull APIs find this decoder first; the
// per-handle slots above still serve in-game playback.
static MpxDecoder* g_mpxStandaloneDecoder = nullptr;
extern "C" void RegisterStandaloneMpxDecoder(MpxDecoder* d) { g_mpxStandaloneDecoder = d; }

static bool IsMpxFilename(const std::string& path) {
  if (path.size() < 4) return false;
  const char* ext = path.c_str() + path.size() - 4;
  return (ext[0] == '.') &&
         (ext[1] == 'm' || ext[1] == 'M') &&
         (ext[2] == 'p' || ext[2] == 'P') &&
         (ext[3] == 'x' || ext[3] == 'X');
}

static MpxDecoder* FindActiveDecoder() {
  if (g_mpxStandaloneDecoder && g_mpxStandaloneDecoder->IsOpen())
    return g_mpxStandaloneDecoder;
  for (int i = FIRST_DVD_FD; i <= LAST_DVD_FD; i++)
    if (g_mpxDecoder[i] && g_mpxDecoder[i]->IsOpen())
      return g_mpxDecoder[i];
  return nullptr;
}

bool MpxDecoderActive_Probe(uint32* outSrcWidth, uint32* outSrcHeight)
{
  MpxDecoder* d = FindActiveDecoder();
  if (!d) return false;
  if (outSrcWidth)  *outSrcWidth  = d->Width();
  if (outSrcHeight) *outSrcHeight = d->Height();
  return true;
}

bool MpxDecoderActive_GetLatestFrame(uint8* dst, uint32 dstPitchBytes,
                                     uint32 dstWidth, uint32 dstHeight)
{
  MpxDecoder* d = FindActiveDecoder();
  if (!d) return false;
  return d->CopyLatestYCrCbA32(dst, dstPitchBytes, dstWidth, dstHeight);
}

bool MpxDecoderActive_IsAtEnd()
{
  MpxDecoder* d = FindActiveDecoder();
  return d && d->IsAtEnd();
}

// Best-effort "skip cutscene" hotkey: when an .mpx is being played and
// fmv.run is stuck waiting on FMV-hardware acks we can't deliver, the
// player can press F12 to forcibly halt the FMV state. We tear down the
// libavcodec decoder, mark per-handle slots inert (close them), and
// (separately, in NuanceMain) halt MPE3 by clearing MPECTRL_MPEGO so the
// game's containing function — usually mcp.run on its own MPE — sees
// the FMV pipeline stop and may advance. This is a hack: many games
// will simply hang in a different place after the halt. Best for
// IS3-style "play logo, then continue" cutscenes where the game's only
// FMV interaction is "wait for the cutscene to end."
void MpxSkipCutscene()
{
  for (int i = FIRST_DVD_FD; i <= LAST_DVD_FD; i++) {
    if (g_mpxDecoder[i]) {
      fprintf(stderr, "[MPX-SKIP] tearing down decoder for handle %d (%s)\n",
              i, fileNameArray[i].c_str());
      delete g_mpxDecoder[i];
      g_mpxDecoder[i] = nullptr;
    }
  }
}

#define NUON_FD_BOOT_DEVICE (3)
#define NUON_FD_DVD (4)
#define NUON_FD_REMOTE (5)
#define NUON_FD_FLASH (6)
#define NUON_FD_SBMEM (7)

static int FindFirstFreeHandle()
{
  for(int i = FIRST_DVD_FD; i <= LAST_DVD_FD; i++)
    if(fileNameArray[i].empty())
      return i;

  return 0;
}

void MediaOpen(MPE &mpe)
{
  const uint32 device = mpe.regs[0];
  const uint32 mode = mpe.regs[2];
  int32 handle = 0;

  if(((eMedia)device == eMedia::MEDIA_BOOT_DEVICE) || ((eMedia)device == eMedia::MEDIA_DVD))
  {
    handle = FindFirstFreeHandle();

    if(handle)
    {    
      uint32 * const pBlockSize = (uint32 *)nuonEnv.GetPointerToMemory(mpe.mpeIndex, mpe.regs[3]);
      if(pBlockSize)
      {
        *pBlockSize = SwapBytes((uint32)BLOCK_SIZE_DVD);
      }

      const char * const baseString = nuonEnv.GetDVDBase();

      //Treat iso9660 device reads as DVD device reads
      const char * name = (char *)nuonEnv.GetPointerToMemory(mpe.mpeIndex, mpe.regs[1]);
      if(!strncmp("/iso9660/",name,9))
      {
        name = &name[9];
      }
      else if(!strncmp("/udf/",name,5))
      {
        name = &name[5];
      }

      fileNameArray[handle] = baseString;
      fileNameArray[handle] += name;
      fileModeArray[handle] = mode;

      // Flag .mpx (NUON-MOVIELIB containers) so MediaRead can short-
      // circuit them — see mpxSkipArray comment.
      mpxSkipArray[handle] = false;
      const size_t nlen = strlen(name);
      if (nlen >= 4) {
        const char* ext = name + nlen - 4;
        if ((ext[0]=='.' || ext[0]==0) &&
            (ext[1]=='m'||ext[1]=='M') &&
            (ext[2]=='p'||ext[2]=='P') &&
            (ext[3]=='x'||ext[3]=='X'))
        {
          mpxSkipArray[handle] = true;
        }
      }

#ifndef _WIN32
      // Case-insensitive file open: if file not found, try lowercase name
      {
        FILE* testf = fopen(fileNameArray[handle].c_str(), "rb");
        if (!testf) {
          // Try lowercasing the filename part
          std::string dir = baseString;
          std::string lname = name;
          for (auto& c : lname) c = tolower(c);
          std::string tryPath = dir + lname;
          testf = fopen(tryPath.c_str(), "rb");
          if (testf) {
            fileNameArray[handle] = tryPath;
          } else {
            // Try uppercasing
            std::string uname = name;
            for (auto& c : uname) c = toupper(c);
            tryPath = dir + uname;
            testf = fopen(tryPath.c_str(), "rb");
            if (testf) fileNameArray[handle] = tryPath;
          }
        }
        if (testf) fclose(testf);
      }
#endif

      // .mpx → spin up a libavcodec MPEG-2 decoder for host-side
      // visual playback. fmv.run still polls for FMV-hardware acks
      // we can't deliver, so the game's cutscene state machine won't
      // advance past the cutscene — but the player at least sees the
      // decoded frames on screen instead of a black window.
      // fileNameArray[handle] is now resolved (case-fixed if needed).
      if (mpxSkipArray[handle] && IsMpxFilename(fileNameArray[handle])) {
        if (g_mpxDecoder[handle]) { delete g_mpxDecoder[handle]; g_mpxDecoder[handle] = nullptr; }
        MpxDecoder* dec = new MpxDecoder();
        if (dec->Open(fileNameArray[handle].c_str())) {
          g_mpxDecoder[handle] = dec;
          fprintf(stderr, "[MPX] decoding %s\n", fileNameArray[handle].c_str());
        } else {
          delete dec;
          fprintf(stderr, "[MPX] failed to open %s — falling back to EOF skip\n",
                  fileNameArray[handle].c_str());
        }
      }
    }
  }

  mpe.regs[0] = handle;
}

void MediaClose(MPE &mpe)
{
  const int32 handle = mpe.regs[0];

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD)) {
    if (g_mpxDecoder[handle]) {
      delete g_mpxDecoder[handle];
      g_mpxDecoder[handle] = nullptr;
    }
    fileNameArray[handle].clear();
    mpxSkipArray[handle] = false;
  }
}

void MediaGetDevicesAvailable(MPE &mpe)
{
  mpe.regs[0] = MEDIA_DEVICES_AVAILABLE;
}

void MediaGetInfo(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const uint32 devInfo = mpe.regs[1];
  int32 result = -1;

  if(devInfo)
  {
    if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
    {
      MediaDevInfo * const pDevInfo = (MediaDevInfo*)nuonEnv.GetPointerToMemory(mpe.mpeIndex, devInfo);

      const uint32 id = SwapBytes((uint32)handle);
      const uint32 datarate = SwapBytes((uint32)DATA_RATE_DVD);
      const uint32 sectorsize = SwapBytes((uint32)BLOCK_SIZE_DVD);

      pDevInfo->sectorsize = BLOCK_SIZE_DVD;
      pDevInfo->datarate = DATA_RATE_DVD;
      pDevInfo->id = handle;
      pDevInfo->type = 0;
      pDevInfo->bus = 0;
      pDevInfo->state = 0;
      result = 0;
    }
  }

  mpe.regs[0] = result;
}

void MediaRead(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const int32 mode = mpe.regs[1];
  const uint32 startblock = mpe.regs[2];
  const uint32 blockcount = mpe.regs[3];
  const uint32 buffer = mpe.regs[4];
  const uint32 callback = mpe.regs[5];

  mpe.regs[0] = -1;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    // .mpx → host-side libavcodec decodes the file in parallel for
    // on-screen display. We still need to satisfy the game's own MPX
    // streaming reads though — fmv.run's NUON-MOVIELIB parser feeds
    // bytes from these reads into a ring buffer that drives the FMV
    // hardware. Without real hardware nothing visibly happens with
    // those bytes, but the parser at least makes forward progress and
    // doesn't spin trying to interpret garbage. Returning success
    // with byte count keeps the polling loop happy; reaching real
    // EOF on the file (or our decoder finishing) reports zero blocks
    // so the game moves past the cutscene.
    if (g_mpxDecoder[handle] && !fileNameArray[handle].empty() && buffer) {
      // Serve the real file bytes to the game's NUON-MOVIELIB parser;
      // libavcodec runs in parallel for on-screen display. EOF is
      // signalled the same way as the normal direct-read path:
      // regs[0] stays at -1 (failure) and regs[1] = 0 blocks read.
      void* pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
      uint32 readCount = 0;
      if (pBuf) {
        FILE* f = nullptr;
        if (fopen_s(&f, fileNameArray[handle].c_str(), "rb") == 0 && f) {
          fseek(f, (long)startblock * BLOCK_SIZE_DVD, SEEK_SET);
          readCount = (uint32)fread(pBuf, BLOCK_SIZE_DVD, blockcount, f);
          fclose(f);
        }
      }
      if (readCount >= (blockcount - 1)) {
        mpe.regs[0] = mode;
        mpe.regs[1] = readCount;
      } else {
        mpe.regs[1] = readCount;
      }
      if (callback) {
        mpe.pcexec = callback;
        bCallingMediaCallback = true;
      }
      return;
    }
    // No decoder (open failed) → behave like the old skip path so the
    // game still sees EOF and proceeds.
    if (mpxSkipArray[handle]) {
      mpe.regs[0] = mode;
      mpe.regs[1] = 0;
      if (callback) {
        mpe.pcexec = callback;
        bCallingMediaCallback = true;
      }
      return;
    }

    if(!fileNameArray[handle].empty() && buffer && ((eMedia)fileModeArray[handle] != eMedia::MEDIA_WRITE))
    {
      FILE* inFile = nullptr;
      if (fopen_s(&inFile,fileNameArray[handle].c_str(),"rb") != 0)
        inFile = nullptr;

      // If file not found on disk, try reading from ISO
      if (!inFile) {
        extern std::string g_ISOPath;
        extern std::string g_ISOPrefix;
        if (!g_ISOPath.empty()) {
          // Extract filename from full path
          std::string fname = fileNameArray[handle];
          size_t lastSlash = fname.find_last_of("/\\");
          if (lastSlash != std::string::npos) fname = fname.substr(lastSlash + 1);

          ISO9660Reader isoReader;
          if (isoReader.open(g_ISOPath.c_str())) {
            // Find file in ISO — try various case combinations
            std::string isoPath = g_ISOPrefix + '/' + fname;
            uint32_t lba, fsize;
            if (isoReader.findFile(isoPath.c_str(), lba, fsize)) {
              // Read directly from ISO into NUON memory
              void* pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
              FILE* isoFp = ISO_FOPEN(g_ISOPath.c_str(), "rb");
              if (isoFp) {
                off64_t byteOffset = (off64_t)lba * 2048 + (off64_t)startblock * BLOCK_SIZE_DVD;
                ISO_FSEEK(isoFp, byteOffset, SEEK_SET);
                uint32_t readCount = (uint32)fread(pBuf, BLOCK_SIZE_DVD, blockcount, isoFp);
                fclose(isoFp);
                if (readCount >= (blockcount - 1)) {
                  mpe.regs[0] = mode;
                  mpe.regs[1] = blockcount;
                  if (callback) {
                    mpe.pcexec = callback;
                    bCallingMediaCallback = true;
                  }
                }
                isoReader.close();
                return; // handled via ISO
              }
            }
            isoReader.close();
          }
        }
      }

      if(inFile)
      {
        void* pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
        fseek(inFile,startblock*BLOCK_SIZE_DVD,SEEK_SET);
        const uint32 readCount = (uint32)fread(pBuf,BLOCK_SIZE_DVD,blockcount,inFile);
        if(readCount >= (blockcount - 1))
        {
          mpe.regs[0] = mode;
          mpe.regs[1] = blockcount;
          if(callback)
          {
            mpe.pcexec = callback;
            bCallingMediaCallback = true;
          }
        }
        fclose(inFile);
      }
    }
  }
}

void MediaWrite(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const int32 mode = mpe.regs[1];
  const uint32 startblock = mpe.regs[2];
  const uint32 blockcount = mpe.regs[3];
  const uint32 buffer = mpe.regs[4];
  const uint32 callback = mpe.regs[5];

  mpe.regs[0] = -1;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    if(!fileNameArray[handle].empty() && buffer && ((eMedia)fileModeArray[handle] != eMedia::MEDIA_READ))
    {
      //try to open the existing file for read/write without erasing the contents
      FILE* outFile;
      if(fopen_s(&outFile,fileNameArray[handle].c_str(),"r+b") != 0)
      {
        //try to create the file
        if(fopen_s(&outFile,fileNameArray[handle].c_str(),"w+b") != 0)
          outFile = nullptr;
      }

      if(outFile)
      {
        const void* pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
        fseek(outFile,startblock*BLOCK_SIZE_DVD,SEEK_SET);
        const uint32 writeCount = (uint32)fwrite(pBuf,BLOCK_SIZE_DVD,blockcount,outFile);
        if(writeCount >= (blockcount - 1))
        {
          mpe.regs[0] = mode;
          mpe.regs[1] = blockcount;
          if(callback)
          {
            mpe.pcexec = callback;
            bCallingMediaCallback = true;
          }
        }
        fclose(outFile);
      }
    }
  }
}

void MediaIoctl(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const int32 ctl = mpe.regs[1];
  const uint32 value = mpe.regs[2];
  //char ctlStr[2];

  mpe.regs[0] = -1;
  //ctlStr[0] = '0'+ctl;
  //ctlStr[1] = 0;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    mpe.regs[0] = 0;

    if(!fileNameArray[handle].empty())
    {
      switch((eMedia)ctl)
      {
        case eMedia::MEDIA_IOCTL_SET_MODE:
          break;
        case eMedia::MEDIA_IOCTL_GET_MODE:
          break;
        case eMedia::MEDIA_IOCTL_EJECT:
          break;
        case eMedia::MEDIA_IOCTL_RETRACT:
          break;
        case eMedia::MEDIA_IOCTL_FLUSH:
          break;
        case eMedia::MEDIA_IOCTL_GET_DRIVETYPE:
          //return kTypeDvdSingle for now, but should really allow the user to declare the disk type
          mpe.regs[0] = kTypeDVDSingle;
          break;
        case eMedia::MEDIA_IOCTL_READ_BCA:
          break;
        case eMedia::MEDIA_IOCTL_GET_START:
          break;
        case eMedia::MEDIA_IOCTL_SET_START:
          break;
        case eMedia::MEDIA_IOCTL_SET_END:
          break;
        case eMedia::MEDIA_IOCTL_GET_PHYSICAL:
          if(value)
          {
            uint32* const ptr = (uint32 *)nuonEnv.GetPointerToMemory(mpe.mpeIndex, value);
            //!! For now, return physical sector zero, but in the future there needs to be some sort
            //of TOC to allow for loading from image files in which case the base file sector will
            //be non-zero
            *ptr = SwapBytes(0u);
          }
          //return DVD layer 0 (should this be zero-based or one-based?)
          mpe.regs[0] = 0;
          break;
        case eMedia::MEDIA_IOCTL_OVERWRITE:
          break;
        case eMedia::MEDIA_IOCTL_ERASE:
          break;
        case eMedia::MEDIA_IOCTL_SIZE:
          break;
        case eMedia::MEDIA_IOCTL_CDDATA_OFFSET:
          break;
        default:
          mpe.regs[0] = -1;
          break;
      }
    }
  }
}

void SpinWait(MPE &mpe)
{
  const uint32 status = mpe.regs[0];
  uint32 * const pMediaWaiting = (uint32 *)nuonEnv.GetPointerToMemory(mpe.mpeIndex, MEDIAWAITING_ADDRESS,false);

  uint32 result = 0;
  if((status >> 30) == 0x03)
    result = SwapBytes(status);

  *pMediaWaiting = result;
}
