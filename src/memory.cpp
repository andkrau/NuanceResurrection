#include "basetypes.h"
#include "byteswap.h"
#include "memory.h"
#include "mpe.h"
#include "NuonEnvironment.h"

extern NuonEnvironment nuonEnv;

void MemLocalScratch(MPE &mpe)
{
  const uint32 pSize = mpe.regs[0];

  mpe.regs[0] = 0x20100C80;

  //pointer to size buffer may be zero
  if(pSize != 0)
  {
    //MML2D will corrupt memory in most commercial games if a size greater than 3968 is returned.
    //The address and size returned in this implementation are identical to the values returned by
    //the VMLabs BIOS
    uint32* const pMem = (uint32 *)nuonEnv.GetPointerToMemory(mpe.mpeIndex, pSize);
    *pMem = SwapBytes(512u);
  }
}

void MemAlloc(MPE &mpe)
{
  const uint32 requestedBytes = mpe.regs[0];
  const uint32 requestedAlignment = mpe.regs[1];
  const uint32 flags = mpe.regs[2];
  const uint32 result = nuonEnv.nuonMemoryManager.Alloc(requestedBytes, requestedAlignment, flags);
  mpe.regs[0] = result;
  if(!result) // allocation failed
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    MessageBox(NULL,"WARNING WILL ROBINSON", "WARNING WILL ROBINSON", MB_OK);
#endif
  }
}

void MemFree(MPE &mpe)
{
  const uint32 address = mpe.regs[0];

  nuonEnv.nuonMemoryManager.Free(address);
}

void MemInit(MPE &mpe)
{
  nuonEnv.nuonMemoryManager.Init();
}

// Internal COFF loader shared by _MemLoadCoffX, _DownloadCoff, and
// _StreamLoadCoff. Parses a COFF image already in memory at coffbase
// and copies each section into its paddr; returns the entry point.
//
// `updateMcpState` mirrors the side-effect of _MemLoadCoffX in the real
// VMLabs BIOS: it bumps mem[0x800395A4] and pokes mem[0x80039980] so
// Iron Soldier 3's module-load helper at 0x80031280 takes its slow
// path. _DownloadCoff / _StreamLoadCoff are post-init "load this
// extra COFF I already buffered" services and don't touch that state.
//
// IS3's main loader at MPE3 reads programs.dat sectors into system
// DRAM (0x80600000) then calls _MemLoadCoffX to relocate the embedded
// mcp.run module to 0x80018000 and read its entry point. With the
// original NullBiosHandler the loader compares r0 against the expected
// sentinel, never matches, and reopens programs.dat indefinitely.
// See nuance-stuck-loading.md.
//
// LOADFLAGS_RUN (0x01) requests starting the loaded program — we don't
// implement that here; the caller is responsible for jumping to the
// returned entry point itself, which IS3 does.
static void MemLoadCoffImpl(MPE &mpe, bool updateMcpState)
{
  const int32  mpeIdx   = (int32)mpe.regs[0];
  const uint32 coffbase = mpe.regs[1];

  uint8* const buf = (uint8*)nuonEnv.GetPointerToMemory(mpe.mpeIndex, coffbase);
  if (!buf) { mpe.regs[0] = 0; return; }

  const uint16 magic   = (buf[0] << 8) | buf[1];
  const uint16 nscns   = (buf[2] << 8) | buf[3];
  const uint16 opthdr  = (buf[16] << 8) | buf[17];
  if (magic != 0x0120) { mpe.regs[0] = 0; return; }

  const uint32 entryPoint =
      ((uint32)buf[20] << 24) | ((uint32)buf[21] << 16) |
      ((uint32)buf[22] <<  8) | ((uint32)buf[23]);

  const uint32 sectionsOffset = 20 + opthdr;
  for (int i = 0; i < nscns; i++) {
    const uint8* h = buf + sectionsOffset + i * 44;
    auto rd32 = [](const uint8* p) {
      return ((uint32)p[0] << 24) | ((uint32)p[1] << 16) |
             ((uint32)p[2] <<  8) | ((uint32)p[3]);
    };
    const uint32 paddr  = rd32(h + 8);
    const uint32 size   = rd32(h + 16);
    const uint32 scnptr = rd32(h + 20);
    const uint32 flags  = rd32(h + 40);

    if (flags & 0x282) continue;
    if (size == 0)     continue;

    uint8* dst = (uint8*)nuonEnv.GetPointerToMemory(
        (mpeIdx >= 0 && mpeIdx < 4) ? (uint32)mpeIdx : mpe.mpeIndex, paddr);
    if (!dst) continue;

    memcpy(dst, buf + scnptr, size);

    // Invalidate the JIT cache on every MPE for this region. fmv.run
    // and other late-loaded modules typically overwrite memory that
    // already had compiled JIT blocks (IS3's main loader at 0x80031xxx
    // is replaced by fmv.run); without this, the JIT keeps executing
    // stale compiled blocks pointing at the OLD bytes and the function
    // epilogues restore the wrong saved registers. UpdateInvalidateRegion
    // alone isn't enough because its consumer at the FDE site only fires
    // when pcexec is inside MPE-local IRAM range; for system-DRAM loads
    // we have to flush the native and interpreter caches directly.
    const uint32 endAddr = paddr + size - 1;
    for (int m = 0; m < 4; m++) {
      nuonEnv.mpe[m].nativeCodeCache.FlushRegion(paddr, endAddr);
      nuonEnv.mpe[m].InvalidateICacheRegion(paddr, endAddr);
    }
  }

  // IS3-specific state mirror — only for _MemLoadCoffX. The real BIOS
  // bumps these somewhere along the *initial* COFF load path; mirroring
  // it here unblocks the module-load helper at 0x80031280.
  if (updateMcpState) {
    if (uint32* p = (uint32*)nuonEnv.GetPointerToMemory(mpe.mpeIndex, 0x800395A4)) {
      const uint32 cur = SwapBytes(*p);
      *p = SwapBytes(cur + 1u);
    }
    if (uint32* p = (uint32*)nuonEnv.GetPointerToMemory(mpe.mpeIndex, 0x80039980)) {
      *p = SwapBytes(0x01000000u);
    }
  }

  mpe.regs[0] = entryPoint;
}

void MemLoadCoff(MPE &mpe)        { MemLoadCoffImpl(mpe, true);  }
void DownloadCoff(MPE &mpe)       { MemLoadCoffImpl(mpe, false); }
void StreamLoadCoff(MPE &mpe)     { MemLoadCoffImpl(mpe, false); }
