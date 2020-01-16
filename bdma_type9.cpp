#include "basetypes.h"
#include "byteswap.h"
#include "dma.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment *nuonEnv;
extern VidChannel structMainChannel;
extern VidChannel structOverlayChannel;

void BDMA_Type9_Write_0(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  uint32 *pSrc32;
  uint16 *pDest16, *pSrc16;
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

  //pixel+Z write (16 + 16Z)
  bCompareZ = true;
  bUpdatePixel = true;
  bUpdateZ = true;

  if(pixtype == 15 || pixtype == 12)
  {
    map = 2;
  }

  if(pixtype > 12)
  {
    map = pixtype - 13;
    zmap = 2;
  }
  else
  {
    map = pixtype - 9;
    zmap = 3;
  }

  //The formula (framebuffer width * framebuffer size * (zmap - map)) specifies the pixel
  //offset that is added to the pixel address to obtain the address of the associated z-value

  destZOffset = xsize * structMainChannel.src_height * zmap;
  mapOffset = xsize * structMainChannel.src_height * map;

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

  pSrc32 = ((uint32 *)pSrc) + srcOffset;
  pSrc16 = ((uint16 *)pSrc) + srcOffset;
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
      bZTestResult = false;

      if(bCompareZ && (zcompare != 0))
      {
        bool result;
        int16 ztarget, ztransfer;

        ztarget = pDest16[destA + destB + destZOffset];
        ztransfer = ((uint16 *)(&pSrc32[srcA + srcB]))[1];
        SwapWordBytes((uint16 *)&ztarget);
        SwapWordBytes((uint16 *)&ztransfer);

        switch(zcompare)
        {
          case 0x1:
            result = (ztarget < ztransfer);
            break;
          case 0x2:
            result = (ztarget == ztransfer);
            break;
          case 0x3:
            result = (ztarget <= ztransfer);
            break;
          case 0x4:
            result = (ztarget > ztransfer);
            break;
          case 0x5:
            result = (ztarget != ztransfer);
            break;
          case 0x6:
            result = (ztarget >= ztransfer);
            break;
          case 0x7:
            result = false;
            break;
        }

        bZTestResult = result;
      }

      if(!bZTestResult)
      {
        if(zcompare == 7)
        {
          pDest16[destA + destB + mapOffset] = ((uint16 *)(&pSrc16[srcA + srcB]))[0];        
        }
        else
        {
          pDest16[destA + destB + mapOffset] = ((uint16 *)(&pSrc32[srcA + srcB]))[0];
          pDest16[destA + destB + destZOffset] = ((uint16 *)(&pSrc32[srcA + srcB]))[1];
        }
      }

      srcA += srcAStep;
      destA += 1;
    }

    srcB += srcBStep;
    destB += xsize;
  }
}

void BDMA_Type9_Write_1(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Write_2(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Write_3(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Write_4(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Write_5(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Write_6(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Write_7(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_0(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  the_mpe->regs[0] = 0;
}

void BDMA_Type9_Read_1(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_2(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_3(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_4(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_5(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_6(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type9_Read_7(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}
