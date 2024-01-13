#include "basetypes.h"
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment nuonEnv;

void BDMA_Type8_Write_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  const bool bRemote = flags & (1UL << 28);
  const bool bDirect = flags & (1UL << 27);
  const bool bDup = flags & (3UL << 26); //bDup = dup | direct
  //const bool bTrigger = flags & (1UL << 25);
  //const bool bRead = flags & (1UL << 13);
  const int32 xsize = (flags >> 13) & 0x7F8UL;
  //const uint32 type = (flags >> 14) & 0x03UL;
  //const uint32 mode = flags & 0xFFFUL;
  //const uint32 zcompare = (flags >> 1) & 0x07UL;
  //const uint32 pixtype = (flags >> 4) & 0x0FUL;
  //const uint32 bva = ((flags >> 7) & 0x06UL) | (flags & 0x01UL);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEUL;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCUL;
  const uint32 xlen = (xinfo >> 16) & 0x3FFUL;
  const uint32 xpos = xinfo & 0x7FFUL;
        uint32 ylen = (yinfo >> 16) & 0x3FFUL;
  const uint32 ypos = yinfo & 0x7FFUL;

  void* intMemory;
  if(bRemote)
  {
    //internal address is system address (but still in MPE memory)
    assert(((mpeBase >> 23) & 0x1FUL) < 4);
    intMemory = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(mpeBase >> 23) & 0x1FUL], mpeBase & 0x207FFFFF, false);
  }
  else
  {
    //internal address is local to MPE
    intMemory = nuonEnv.GetPointerToMemory(mpe, mpeBase & 0x207FFFFF, false);
  }

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1FUL) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(sdramBase >> 23) & 0x1FUL], sdramBase, false);

  const void* pSrc = intMemory;
  void* const pDest = baseMemory;

  uint32 directValue;
  int32 srcAStep, srcBStep;
  if(bDup)
  {
    if(bDirect)
    {
      //Direct and Dup: intaddr is data.
      directValue = SwapBytes(intaddr);
    }
    else
    {
      //Dup but not Direct: read scalar from memory, no need to swap
      directValue = *(uint32 *)intMemory;
    }

    pSrc = (void *)&directValue;
    srcAStep = 0;
    srcBStep = 0;
  }
  else
  {
    srcAStep = 1;
    srcBStep = xlen;
  }

  constexpr uint32 srcOffset = 0;
  const uint32 destOffset = ypos * (uint32)xsize + xpos;

  /*if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv.mainChannelUpperLimit) ||
      (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv.mainChannelUpperLimit))
  {
    nuonEnv.bMainBufferModified = true;
  }
  else if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv.overlayChannelUpperLimit) ||
      (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv.overlayChannelUpperLimit))
  {
    nuonEnv.bOverlayBufferModified = true;
  }*/

  //BVA = 000 (horizontal DMA, x increment, y increment)
  constexpr int32 destAStep = 1;
  const int32 destBStep = xsize;

  const uint32* pSrc32 = ((uint32 *)pSrc) + srcOffset;
  uint16* pDest16 = ((uint16 *)pDest) + destOffset;

  while(ylen--)
  {
    uint32 srcA = 0;

    for(uint32 destA = 0; destA < xlen; ++destA) // as destAStep==1
    {
      const uint32 pix32 = SwapBytes(pSrc32[srcA]);
      pDest16[destA] = SwapBytes((uint16)(((pix32 >> 16) & 0xFC00UL) | ((pix32 >> 14) & 0x03E0UL) | ((pix32 >> 11) & 0x001FUL)));

      srcA += srcAStep;
    }

    pSrc32 += srcBStep;
    pDest16 += destBStep;
  }
}

void BDMA_Type8_Write_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Write_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Write_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Write_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Write_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Write_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Write_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type8_Read_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}
