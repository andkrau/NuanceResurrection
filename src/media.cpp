#include "basetypes.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <deque>
#include <vector>
#include <cstring>
#include "byteswap.h"
#include "Bios.h"
#include "mpe.h"
#include "media.h"
#include "mpe_alloc.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "iso9660.h"

// MEDIA_ASYNC_CALLBACKS makes MediaRead/MediaWrite defer the callback dispatch
// roughly the way the vmlabs_bios2.dis would - read the disc into an emulator-side
// intermediate buffer immediately, then trickle the data into the NUON memory one
// block at a time and (optionally) fire user callbacks on the same schedule
// (every-block and/or end, dependent on the mode bits the caller set).
// Each block-delivery / callback is queued in a per-MPE FIFO that MPE::FetchDecodeExecute
// drains via TickMediaCallbacks(), once s_tickCounter passes the entry's fire time
//
// If not defined, fall back to the previous tail-call shim: the callback fires
// synchronously inside the BIOS handler, and the shim restores the caller's
// reg0/reg1 afterwards. That older path also can't fire multiple callbacks per call,
// so it ignores MCB_EVERY. This would make (at least) T3K's audio act up again, incl. hangs
#define MEDIA_ASYNC_CALLBACKS // stay with the more precise behavior

// Ticks (= MPE::FetchDecodeExecute steps for the calling MPE) between
// successive per-block deliveries / callback notifications. "Real" DVD at
// 1.5 MiB/s may load a 2 KiB block in ~1.3 ms, but the matching tick count
// also depends on how fast the host runs the emulator's main loop through this
// MPE. Empirically, 5000 worked across various tests and setups - small enough that
// streaming-audio titles' refill callbacks could fire well within one audio
// frame (the mixer seems to underrun silently if the next chunk is late, which
// at the previous (estimated) 20000 caused T3K music to play only briefly before stalling
// and some other games hanging completely).
// Cranking this higher may lead to "more accurate" pacing/load times, but may starve
// any consumer that doesn't double-buffer; cranking it lower, flattens the per-block
// granularity toward just "all at once"
#define MEDIA_CYCLES_PER_BLOCK (1000ULL)// ~5000 barely works with T3K, so use something much smaller than that for now to be safe (btw: 1 also works)

extern NuonEnvironment nuonEnv;
extern uint32 mpeFlags[];
extern NuonBiosHandler BiosJumpTable[];

bool bCallingMediaCallback = false; // globally used

uint32 media_mpe_allocated = 0;
uint32 media_mpe = 0;

// Post-callback shim for MediaRead and MediaWrite:
// The original(?) BIOS routines (see vmlabs_bios2.dis 8078c4fc / 8078c618) enqueue
// a request and return reg0/reg1 to the caller, while the user's media callback
// is dispatched later by the driver ISR with its own reg0/reg1 values. Because
// Nuance currently either redirects pcexec to the callback synchronously (legacy
// path) or interrupts user code from the FIFO drain (async path), the
// callback would otherwise overwrite/lose the surrounding context. The shim
// recovers whichever flavor of saved state is pending, once the callback rts
// lands in a designated BIOS slot
#define MEDIA_CB_SHIM_INDEX   (151U)
#define MEDIA_CB_SHIM_ADDRESS (ROM_BIOS_BASE + (MEDIA_CB_SHIM_INDEX << 1))

// Synchronous (legacy) shim state
static uint32 s_cbSavedRz = 0;
static uint32 s_cbReturnR0 = 0;
static uint32 s_cbReturnR1 = 0;
static bool   s_cbReturnPending = false;

#ifdef MEDIA_ASYNC_CALLBACKS
// Asynchronous (delayed-FIFO) state
struct PendingMediaCB
{
  uint32 callback;           // 0 = "delivery only", no user callback fires
  uint32 cbR0;               // reg0 handed to the callback (MCB_END/MCB_EVERY/MCB_ERROR)
  uint32 cbR1;               // reg1 handed to the callback (block index / -1 / count)
  uint64 fireAtCycle;        // s_tickCounter threshold at which to act
  uint32 destAddr;           // NUON RAM address to copy data to (0 = no copy)
  std::vector<uint8_t> data; // intermediate buffer slice; empty = no copy
};

static std::deque<PendingMediaCB> s_cbFifo[4]; // queue per MPE

// Per-MPE step counter for callback scheduling. Using mpe.cycleCounter
// is not feasible at the moment, as FetchDecodeExecute resets
// it to 0 at the start of every call. s_tickCounter is bumped
// once per TickMediaCallbacks for the MPE in question and is the
// reference clock both MediaRead/MediaWrite (when scheduling) and
// the tick (when checking due times) read
static uint64 s_tickCounter[4] = {0, 0, 0, 0};

// User context is saved while an async-fired callback is in flight (so the shim
// can resume the interrupted code exactly where it left off). There is
// no ISR mechanism - so snapshot the whole reg_union (r0..r31, cc,
// rc0/1, rx/ry/ru/rv, rz, rzi1/2, xyctl/uvctl, xyrange/uvrange, acshift/
// svshift) plus pcexec and sp on entry, and restore it in the shim
static uint32 s_asyncSavedRegUnion[32 + 3 + 13];
static_assert(sizeof(s_asyncSavedRegUnion) == sizeof(MPE::reg_union), "s_asyncSavedRegUnion size mismatch");
static uint32 s_asyncSavedPc = 0;
static uint32 s_asyncSavedSp = 0;
static bool   s_asyncCbActive = false;
#endif // MEDIA_ASYNC_CALLBACKS

static void MediaCallbackReturnShim(MPE &mpe)
{
#ifdef MEDIA_ASYNC_CALLBACKS
  // Async path: the saved context includes pcexec and must
  // be restored before the dispatcher's implicit pcexec=rz would clobber it.
  // Restore the full scalar register file so the interrupted user code sees
  // exactly what it had before we yanked it for the callback
  if(s_asyncCbActive)
  {
    memcpy(mpe.reg_union, s_asyncSavedRegUnion, sizeof(s_asyncSavedRegUnion));
    mpe.pcexec = s_asyncSavedPc;
    mpe.sp     = s_asyncSavedSp;
    bCallingMediaCallback = true; // prevent dispatcher's pcexec=rz
    s_asyncCbActive = false;
    return;
  }
#endif

  if(s_cbReturnPending)
  {
    // Legacy sync path: BIOS-call return values need to be restored;
    // Let the dispatcher jump to rz to resume the caller of MediaRead/MediaWrite
    mpe.regs[0] = s_cbReturnR0;
    mpe.regs[1] = s_cbReturnR1;
    mpe.rz      = s_cbSavedRz;
    s_cbReturnPending = false;
  }
}

static void InvokeMediaCallback(MPE &mpe, const uint32 callback, const uint32 cbR0, const uint32 cbR1, const uint32 retR0, const uint32 retR1)
{
  s_cbSavedRz = mpe.rz;
  s_cbReturnR0 = retR0;
  s_cbReturnR1 = retR1;
  s_cbReturnPending = true;

  mpe.rz = MEDIA_CB_SHIM_ADDRESS;
  mpe.regs[0] = cbR0;
  mpe.regs[1] = cbR1;
  mpe.pcexec = callback;
  bCallingMediaCallback = true;
}

#ifdef MEDIA_ASYNC_CALLBACKS
// Called by MPE::FetchDecodeExecute once per emulator step. If the head of
// this MPE's FIFO is due, copy the pending block data into NUON RAM and
// (if the entry had a user callback assigned) interrupt user code to run it.
// Multiple data-only deliveries can be processed in a single tick; only one callback
// fires per tick (the rest stays queued until that callback's shim returns)
void TickMediaCallbacks(MPE &mpe)
{  
  const uint64 nowTick = ++s_tickCounter[mpe.mpeIndex]; // Advance the per-MPE tick counter

  if(s_asyncCbActive || s_cbReturnPending) return; // Re-entry guard: if either kind of callback is mid-flight we mustn't destroy the saved context.

  // Don't interrupt mid-delay-slot. ecuSkipCounter > 0 equals a previously-taken branch
  // that hasn't finished yet; redirecting pcexec now would either be undone by the
  // dispatcher's pcexec = pcfetchnext when the counter expires, or replace the delay-slot
  // instruction with the callback's first packet - both break the interrupted user code
  if(mpe.ecuSkipCounter != 0) return;

  std::deque<PendingMediaCB>& q = s_cbFifo[mpe.mpeIndex];

  while(!q.empty())
  {
    PendingMediaCB& top = q.front();
    if(nowTick < top.fireAtCycle)
      return; // wait for next time

    // Copy the block into NUON RAM: "intermediate buffer -> real destination" that real HW/DVD would do as the data streams in
    if(top.destAddr != 0 && !top.data.empty())
    {
      void* const dest = nuonEnv.GetPointerToMemory(mpe.mpeIndex, top.destAddr);
      if(dest)
        memcpy(dest, top.data.data(), top.data.size());
    }

    if(top.callback != 0)
    {
      // Interrupt user code for the callback. Snapshot the entire register file (incl. rz) plus pcexec and sp
      // so the shim can restore the user state untouched after the callback rts's
      memcpy(s_asyncSavedRegUnion, mpe.reg_union, sizeof(s_asyncSavedRegUnion));
      s_asyncSavedPc = mpe.pcexec;
      s_asyncSavedSp = mpe.sp;
      s_asyncCbActive = true;

      mpe.regs[0] = top.cbR0;
      mpe.regs[1] = top.cbR1;
      mpe.rz      = MEDIA_CB_SHIM_ADDRESS;
      mpe.pcexec  = top.callback;

      q.pop_front();
      return; // wait for the shim before processing the next entry
    }

    // Delivery-only entry (no callback to fire): pop and check the next one //!! unclear if this could/should be optimized (so far)
    q.pop_front();
  }
}

#else // !MEDIA_ASYNC_CALLBACKS

void TickMediaCallbacks(MPE& /*mpe*/) {} // provide, so mpe.cpp links

#endif // MEDIA_ASYNC_CALLBACKS

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

#ifdef MEDIA_ASYNC_CALLBACKS
  // Discard any pending block deliveries / callbacks targeted for the now-shut-down media MPE - they would dispatch into a halted processor otherwise
  s_cbFifo[media_mpe].clear();
  s_asyncCbActive = false;
  s_cbReturnPending = false;
  s_tickCounter[media_mpe] = 0; // Reset monotonic clock so fresh MediaInitMPE starts from 0
#endif

  media_mpe_allocated = 0;
}

void MediaInitMPE(const uint32 i)
{
  bool loadStatus;

  // Register the post-callback shim into the BIOS jump table here, once.
  // (Both the sync InvokeMediaCallback- and the async TickMediaCallbacks-
  // paths land their callback's rts in BiosJumpTable[MEDIA_CB_SHIM_INDEX])
  BiosJumpTable[MEDIA_CB_SHIM_INDEX] = MediaCallbackReturnShim;

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
  //Check to see if media code is already running on an MPE
  //in which case do not reallocate, and simply return the running MPE-index
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
static uint32 fileModeArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#define FIRST_DVD_FD (3)
#define LAST_DVD_FD (19)

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
    }
  }

  mpe.regs[0] = handle;
}

void MediaClose(MPE &mpe)
{
  const int32 handle = mpe.regs[0];

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
    fileNameArray[handle].clear();
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

      pDevInfo->sectorsize = sectorsize;
      pDevInfo->datarate = datarate;
      pDevInfo->id = id;
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
  const uint32 mode = mpe.regs[1];
  const uint32 startblock = mpe.regs[2];
  const uint32 blockcount = mpe.regs[3];
  const uint32 buffer = mpe.regs[4];
  const uint32 callback = mpe.regs[5];

  // vmlabs_bios2.dis at 8078c4fc:
  // - blockcount == 0 invokes the callback with r0 = MCB_END. The 'jsr (r5)' at
  //   8078c50e has no ',nop' operand, so per spec its
  //   2 following packets are delay slots that execute before the callback
  //   target: 'mv_s #$40000000,r0' at 8078c512 sets r0 = MCB_END (the callback's
  //   status arg), and 'mv_s #$ffffffff,r1' at 8078c518 sets r1 = -1 (blocknum).
  // - On success, the asm enqueues the request and returns r0 = 0
  //   (8078c608 mv_s #$00000000,r0); the user callback is dispatched later
  //   by the driver ISR - typically with r0 = MCB_END and r1 = blockcount.
  // - On validation failure (driver fn at struct+8 is NULL, or queue full)
  //   the asm runs 'bra #$8078c60a' at 8078c564 (no ',nop') whose 2 delay-slot
  //   packets execute; 'mv_s #$ffffffff,r0' at 8078c568 sits in that delay
  //   window and sets r0 = -1 before control reaches the epilogue.
  if(blockcount == 0)
  {
    if(callback)
    {
      // Callback enters with r0 = MCB_END (set by jsr DS1 at 8078c512)
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_END, mode,
                          (uint32)eMedia::MCB_END, (uint32)-1);
    }
    else
    {
      mpe.regs[0] = (uint32)eMedia::MCB_END;
      mpe.regs[1] = (uint32)-1;
    }
    return;
  }

  // Validation failure returns r0 = -1 (mv_s #$ffffffff,r0 at 8078c568 runs as DS2 of the unconditional bra at 8078c564)
  mpe.regs[0] = (uint32)-1;

  if((handle < FIRST_DVD_FD) || (handle > LAST_DVD_FD))
    return;
  if(fileNameArray[handle].empty() || !buffer || ((eMedia)fileModeArray[handle] == eMedia::MEDIA_WRITE))
    return;

  // Disc data is loaded into a host-side intermediate buffer directly here, then either:
  // - async path: schedule into the FIFO, so block 'i' of NUON memory is
  //   written at s_tickCounter + '(i+1)*MEDIA_CYCLES_PER_BLOCK', with optional
  //   MCB_EVERY/MCB_END callbacks per the mode bits
  // - sync path (legacy): copied directly to NUON memory and the (always single :/)
  //   callback is fired immediately via the tail-call shim
  // A too-short / failed read is reported via the callback as MCB_ERROR, the way the real driver ISR would with the actual BIOS

  std::vector<uint8_t> intermediate;
  intermediate.resize((size_t)blockcount * BLOCK_SIZE_DVD);
  uint32 readCount = 0;
  bool   ioAttempted = false;

  FILE* inFile = nullptr;
  if(fopen_s(&inFile, fileNameArray[handle].c_str(), "rb") != 0)
    inFile = nullptr;

  if(inFile)
  {
    fseek(inFile, startblock * BLOCK_SIZE_DVD, SEEK_SET);
    readCount = (uint32)fread(intermediate.data(), BLOCK_SIZE_DVD, blockcount, inFile);
    fclose(inFile);
    ioAttempted = true;
  }
  else
  {
    // Fall back to reading directly from the ISO image
    extern std::string g_ISOPath;
    extern std::string g_ISOPrefix;
    if(!g_ISOPath.empty())
    {
      std::string fname = fileNameArray[handle];
      const size_t lastSlash = fname.find_last_of("/\\");
      if(lastSlash != std::string::npos) fname = fname.substr(lastSlash + 1);

      ISO9660Reader isoReader;
      if(isoReader.open(g_ISOPath.c_str()))
      {
        const std::string isoPath = g_ISOPrefix + '/' + fname;
        uint32_t lba, fsize;
        if(isoReader.findFile(isoPath.c_str(), lba, fsize))
        {
          FILE* isoFp = ISO_FOPEN(g_ISOPath.c_str(), "rb");
          if(isoFp)
          {
            const off64_t byteOffset = (off64_t)lba * 2048 + (off64_t)startblock * BLOCK_SIZE_DVD;
            ISO_FSEEK(isoFp, byteOffset, SEEK_SET);
            readCount = (uint32)fread(intermediate.data(), BLOCK_SIZE_DVD, blockcount, isoFp);
            fclose(isoFp);
            ioAttempted = true;
          }
        }
        isoReader.close();
      }
    }
  }

  // Note: the asm only writes r1 in the blockcount == 0 path (8078c518). On
  // every other return path, r1 is left equal to the caller's original mode.
  // Mirror that here - 'mode' is restored to r1 via the shim's retR1
  // (and mpe.regs[1] is not touched on the no-callback paths)

  if(!ioAttempted)
  {
    // Probs with the file - treat like a hard I/O failure: deliver MCB_ERROR via the callback if one was supplied
    if(callback)
    {
#ifdef MEDIA_ASYNC_CALLBACKS
      // Schedule an MCB_ERROR notification, due immediately: No data; the tick just interrupts user code to run the callback
      PendingMediaCB e;
      e.fireAtCycle = s_tickCounter[mpe.mpeIndex];
      e.callback    = callback;
      e.cbR0        = (uint32)eMedia::MCB_ERROR;
      e.cbR1        = 0u;
      s_cbFifo[mpe.mpeIndex].push_back(std::move(e));
      mpe.regs[0]   = (uint32)-1; // visible to caller immediately
#else
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_ERROR, 0u,
                          (uint32)-1, mode);
#endif
    }
    return;
  }

  if(readCount == blockcount)
  {
#ifdef MEDIA_ASYNC_CALLBACKS
    // Mode bits: If neither MCB_EVERY nor MCB_END is set, fall back
    // to a single MCB_END at the end //!! check docs/samples/asm again
    const bool wantsEvery = (mode & (uint32)eMedia::MCB_EVERY) != 0;
    bool       wantsEnd   = (mode & (uint32)eMedia::MCB_END)   != 0;
    if(callback && !wantsEvery && !wantsEnd)
      wantsEnd = true;

    if(callback)
    {
      // Caller-visible return must be set now; we won't touch r0/r1
      // again until the callback fires, which the shim takes over
      mpe.regs[0] = 0;

      // One FIFO entry per block, each carrying its own BLOCK_SIZE_DVD slice of the
      // intermediate buffer. TickMediaCallbacks copies the slice into NUON
      // RAM at the entry's fireAtCycle, mirroring how the real DVD
      // controller may stream a block into host RAM only when it arrives -
      // between MediaRead returning and the entry firing, that part of the
      // destination buffer still holds whatever was there before the read.
      // This may be helpful for apps/games that expect that their buffers are intact until
      // they get the notice
      const uint64 now = s_tickCounter[mpe.mpeIndex];
      for(uint32 i = 0; i < blockcount; i++)
      {
        PendingMediaCB e;
        e.fireAtCycle = now + (uint64)(i + 1) * MEDIA_CYCLES_PER_BLOCK;
        e.destAddr    = buffer + i * BLOCK_SIZE_DVD;
        e.data.assign(intermediate.begin() + (size_t)i * BLOCK_SIZE_DVD,
                      intermediate.begin() + (size_t)(i + 1) * BLOCK_SIZE_DVD);
        if(wantsEvery)
        {
          e.callback  = callback;
          e.cbR0      = (uint32)eMedia::MCB_EVERY;
          e.cbR1      = i + 1; // 1-based block index this delivery covers
        }
        else
        {
          e.callback = 0; // delivery only
        }
        s_cbFifo[mpe.mpeIndex].push_back(std::move(e));
      }
      if(wantsEnd)
      {
        PendingMediaCB e;
        e.fireAtCycle = now + (uint64)blockcount * MEDIA_CYCLES_PER_BLOCK;
        e.destAddr    = 0; // no data; just a notification
        e.callback    = callback;
        e.cbR0        = (uint32)eMedia::MCB_END;
        e.cbR1        = blockcount;
        s_cbFifo[mpe.mpeIndex].push_back(std::move(e));
      }
    }
    else
    {
      // No callback: copy immediately/synchronously so data is visible the moment MediaRead returns
      void* const pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
      if(pBuf)
        memcpy(pBuf, intermediate.data(), intermediate.size());
      mpe.regs[0] = 0;
    }
#else
    // Legacy sync path: copy + tail-call shim
    {
      void* const pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
      if(pBuf)
        memcpy(pBuf, intermediate.data(), intermediate.size());
    }
    if(callback)
    {
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_END, blockcount,
                          0u, mode);
    }
    else
    {
      mpe.regs[0] = 0;
    }
#endif
  }
  else
  {
    // Too-short read: copy whatever we got (callers may want to see the partial data), then MCB_ERROR via the callback
    if(readCount > 0)
    {
      void* const pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
      if(pBuf)
        memcpy(pBuf, intermediate.data(), (size_t)readCount * BLOCK_SIZE_DVD);
    }
    if(callback)
    {
#ifdef MEDIA_ASYNC_CALLBACKS
      // Schedule MCB_ERROR, due immediately, carrying the partial count in r1 so the callback knows how much actually loaded
      PendingMediaCB e;
      e.fireAtCycle = s_tickCounter[mpe.mpeIndex];
      e.callback    = callback;
      e.cbR0        = (uint32)eMedia::MCB_ERROR;
      e.cbR1        = readCount;
      s_cbFifo[mpe.mpeIndex].push_back(std::move(e));
      mpe.regs[0]   = (uint32)-1;
#else
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_ERROR, readCount,
                          (uint32)-1, mode);
#endif
    }
    else
    {
      mpe.regs[0] = (uint32)-1;
    }
  }
}

void MediaWrite(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const uint32 mode = mpe.regs[1];
  const uint32 startblock = mpe.regs[2];
  const uint32 blockcount = mpe.regs[3];
  const uint32 buffer = mpe.regs[4];
  const uint32 callback = mpe.regs[5];

  // Per vmlabs_bios2.dis at 8078c618: basically similar to MediaRead asm but
  // - the driver-write fn pointer lives at struct + 0xC (not + 8), and
  // - there is no blockcount == 0 shortcut. With strict-equality success
  //   and MCB_ERROR on too-short writes, handle the blockcount == 0
  //   case as a successful no-op (fwrite returns 0 == blockcount).
  //
  // asm never writes reg1 on any return path - caller's mode is preserved.
  // Mirror this by passing 'mode' as the shim's retR1 and not touching
  // mpe.regs[1] on the no-callback paths.
  //
  // Validation failure returns r0 = -1: 'mv_s #$ffffffff,r0' at 8078c66e runs
  // as DS2 of the unconditional 'bra #$8078c710' at 8078c66a (no ',nop' so delay slots execute, per spec)
  mpe.regs[0] = (uint32)-1;

  if((handle < FIRST_DVD_FD) || (handle > LAST_DVD_FD))
    return;
  if(fileNameArray[handle].empty() || !buffer || ((eMedia)fileModeArray[handle] == eMedia::MEDIA_READ))
    return;

  // Try to open the existing file for read/write without erasing the contents
  FILE* outFile = nullptr;
  if(fopen_s(&outFile, fileNameArray[handle].c_str(), "r+b") != 0)
  {
    // Fall back to creating the file.
    if(fopen_s(&outFile, fileNameArray[handle].c_str(), "w+b") != 0)
      outFile = nullptr;
  }

  if(!outFile)
  {
    // Couldn't reach the underlying file - treat like a hard I/O failure
    if(callback)
    {
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_ERROR, 0u,
                          (uint32)-1, mode);
    }
    return;
  }

  const void* pBuf = nuonEnv.GetPointerToMemory(mpe.mpeIndex, buffer);
  fseek(outFile, startblock * BLOCK_SIZE_DVD, SEEK_SET);
  const uint32 writeCount = (uint32)fwrite(pBuf, BLOCK_SIZE_DVD, blockcount, outFile);
  fclose(outFile);

  if(writeCount == blockcount)
  {
    if(callback)
    {
      // Full write: callback receives (MCB_END, blockcount); caller sees (0, mode) via the shim
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_END, blockcount,
                          0u, mode);
    }
    else
    {
      mpe.regs[0] = 0;
    }
  }
  else
  {
    // Short write: callback receives (MCB_ERROR, writeCount); caller sees (-1, mode) via the shim
    if(callback)
    {
      InvokeMediaCallback(mpe, callback,
                          (uint32)eMedia::MCB_ERROR, writeCount,
                          (uint32)-1, mode);
    }
    else
    {
      mpe.regs[0] = (uint32)-1;
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
        case eMedia::MEDIA_IOCTL_GET_PHYSICAL: //!! Ballistic and Tetris trigger this
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
