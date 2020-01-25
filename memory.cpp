#include "basetypes.h"
#include <stdio.h>
#include "byteswap.h"
#include "Bios.h"
#include "memory.h"
#include "mpe.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"

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
    uint32* const pMem = (uint32 *)nuonEnv.GetPointerToMemory(mpe, pSize);
    *pMem = 512;

    SwapScalarBytes(pMem);
  }
}

void MemAlloc(MPE &mpe)
{
  const uint32 requestedBytes = mpe.regs[0];
  const uint32 requestedAlignment = mpe.regs[1];
  const uint32 flags = mpe.regs[2];
  const uint32 result = nuonEnv.nuonMemoryManager.Alloc(requestedBytes, requestedAlignment, flags);
  mpe.regs[0] = result;
  if(!result)
  {
    printf("WARNING WILL ROBINSON");
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
