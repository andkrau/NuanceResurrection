#include "basetypes.h"
#include "byteswap.h"
#include "dma.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment *nuonEnv;
extern VidChannel structMainChannel;
extern VidChannel structOverlayChannel;

void BDMA_Type8_Write_0(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
  uint32 *pSrc32, pix32;
  uint16 *pDest16, pix16;
  void *intMemory, *baseMemory, *pSrc, *pDest;
  uint32 directValue, type, pixtype;
  uint32 aCount, bCount;
  uint32 srcA, srcB, destA, destB, srcOffset, destOffset;
  int32 srcAStep, srcBStep, destAStep, destBStep, xsize;
  uint32 xlen, xpos, ylen, ypos, zcompare, bva;
  uint32 mode, sdramBase, mpeBase, map, zmap, destZOffset, srcZOffset, mapOffset;

  bool bRead, bDirect, bDup, bRemote, bTrigger, bCompareZ, bUpdatePixel, bUpdateZ, bZTestResult;

  bRemote = flags & (1UL << 28);
  bDirect = flags & (1UL << 27);
  bDup = flags & (3UL << 26); //bDup = dup | direct
  bTrigger = flags & (1UL << 25);
  bRead = flags & (1UL << 13);
  xsize = (flags >> 13) & 0x7F8UL;
  type = (flags >> 14) & 0x03UL;
  mode = flags & 0xFFFUL;
  zcompare = (flags >> 1) & 0x07UL;
  pixtype = (flags >> 4) & 0x0FUL;
  bva = ((flags >> 7) & 0x06UL) | (flags & 0x01UL);
  sdramBase = baseaddr & 0x7FFFFFFEUL;
  mpeBase = intaddr & 0x7FFFFFFCUL;
  xlen = (xinfo >> 16) & 0x3FFUL;
  xpos = xinfo & 0x7FFUL;
  ylen = (yinfo >> 16) & 0x3FFUL;
  ypos = yinfo & 0x7FFUL;

  directValue = intaddr;

   if(bRemote)
  {
    //internal address is system address (but still in MPE memory)
    intMemory = nuonEnv->GetPointerToMemory(nuonEnv->mpe[(mpeBase >> 23) & 0x1FUL], mpeBase & 0x207FFFFF, false);
  }
  else
  {
    //internal address is local to MPE
    intMemory = nuonEnv->GetPointerToMemory(the_mpe, mpeBase & 0x207FFFFF, false);
  }

  //base address is always a system address (absolute)
  baseMemory = nuonEnv->GetPointerToMemory(nuonEnv->mpe[(sdramBase >> 23) & 0x1FUL], sdramBase, false);

  pSrc = intMemory;
  pDest = baseMemory;

  if(bDup)
  {
    if(bDirect)
    {
      //Direct and Dup: intaddr is data.
      directValue = intaddr;
      //swap back to big endian format
      SwapScalarBytes(&directValue);
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

  srcOffset = 0;
  destOffset = ((ypos * (uint32)xsize)) + xpos;

  //BVA = 000 (horizontal DMA, x increment, y increment)
  destAStep = 1;
  destBStep = xsize;
  aCount = xlen;
  bCount = ylen;

  pSrc32 = (uint32 *)pSrc + srcOffset;
  pDest16 = (uint16 *)pDest + destOffset;
  srcB = 0;
  destB = 0;

  if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv->mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv->mainChannelUpperLimit) ||
      (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv->mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv->mainChannelUpperLimit))
  {
    nuonEnv->bMainBufferModified = true;
  }
  else if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv->overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv->overlayChannelUpperLimit) ||
      (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv->overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv->overlayChannelUpperLimit))
  {
    nuonEnv->bOverlayBufferModified = true;
  }

  while(bCount--)
  {
    srcA = 0;
    destA = 0;
    aCount = xlen;

    while(aCount--)
    {
      pix32 = pSrc32[srcA + srcB];
      SwapScalarBytes(&pix32);
      pix16 = ((pix32 >> 16) & 0xFC00UL) | ((pix32 >> 14) & 0x03E0UL) | ((pix32 >> 11) & 0x001FUL);
      SwapWordBytes(&pix16);
      pDest16[destA + destB] = pix16;

      srcA += srcAStep;
      destA += 1;
    }

    srcB += srcBStep;
    destB += xsize;
  }
}

void BDMA_Type8_Write_1(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Write_2(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Write_3(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Write_4(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Write_5(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Write_6(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Write_7(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_0(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_1(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_2(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_3(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_4(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_5(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_6(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type8_Read_7(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}
