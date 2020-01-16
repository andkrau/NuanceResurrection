#include "basetypes.h"
#include "byteswap.h"
#include "dma.h"
#include "NuonEnvironment.h"
#include "video.h"

extern VidChannel structMainChannel;
extern NuonEnvironment *nuonEnv;

void BDMA_Type12_Write_0(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  uint16 *pSrcColor, *pDestColor, *pSrcZ, *pDestZ;
  void *intMemory, *baseMemory;
  uint16 directColor, directZ;
  uint32 type, pixtype, directValue, map, zmap;
  uint32 aCount, bCount;
  uint32 srcA, srcB, destA, destB, srcOffset, destOffset;
  int32 srcAStep, srcBStep, destAStep, destBStep, xsize;
  uint32 xlen, xpos, ylen, ypos, zcompare, bva;
  uint32 mode, sdramBase, mpeBase, srcStrideShift;

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

  map = 0;
  zmap = 1;

  if(pixtype >= 13)
  {
    map = pixtype - 13;
    zmap = 2;
  }
  else if(pixtype >= 9)
  {
    map = pixtype - 9;
    zmap = 3;
  }


  if(zcompare != 7)
  {
    bCompareZ = (zcompare ? true : false);
    //Z write, no pixel (16Z)
    bUpdatePixel = false;
    bUpdateZ = true;
  }
  else
  {
    //Z write, no pixel (16Z, no compare?)
    bCompareZ = false;
    bUpdatePixel = false;
    bUpdateZ = true;
  }

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

  pSrcColor = ((uint16 *)intMemory) + 1;
  pDestColor = ((uint16 *)baseMemory) + (xsize * structMainChannel.src_height * zmap);

  if(bDirect & !bDup)
  {
    bDirect = true;
  }
  if(bDup)
  {
    if(bDirect)
    {
      //Direct and Dup: intaddr is data.
      directColor = intaddr >> 16;
      //swap back to big endian format
      SwapWordBytes(&directColor);
    }
    else
    {
      //Dup but not Direct: read scalar from memory, no need to swap
      directColor = *((uint16 *)intMemory);
    }

    pSrcColor = &directColor;
    srcAStep = 0;
    srcBStep = 0;
  }
  else
  {
    srcAStep = 1;
    srcBStep = xsize;
  }

  srcOffset = 0;
  destOffset = ((ypos * (uint32)xsize)) + xpos;

  //BVA = 000 (horizontal DMA, x increment, y increment)
  destAStep = 1;
  destBStep = xsize;
  aCount = xlen;
  bCount = ylen;

  pDestColor += destOffset;
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
        uint16 ztarget, ztransfer;

        ztarget = pDestColor[destA + destB];
        ztransfer = pSrcColor[srcA + srcB];
        SwapWordBytes((uint16 *)&ztarget);
        SwapWordBytes((uint16 *)&ztransfer);

        switch(zcompare)
        {
          case 0x0:
            result = false;
            break;
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
        pDestColor[destA + destB] = pSrcColor[srcA + srcB];
      }

      srcA += srcAStep;
      destA += 1;
    }

    srcB += srcBStep;
    destB += xsize;
  }
}

void BDMA_Type12_Write_1(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Write_2(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Write_3(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Write_4(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Write_5(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Write_6(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Write_7(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_0(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  uint16 *pSrcColor, *pDestColor, *pSrcZ, *pDestZ;
  uint16 directColor, directZ;
  void *intMemory, *baseMemory, *pSrc, *pDest;
  uint16 *pDest16;
  uint32 *pSrc32, *pDest32;
  uint32 directValue, type, pixtype, srcStrideShift;
  uint32 srcAStart, srcBStart, destAStart, destBStart, aCount, bCount, aCountInit;
  uint32 srcA, srcB, destA, destB, srcOffset, destOffset, map, zmap;
  int32 srcAStep, srcBStep, destAStep, destBStep, xsize;
  uint32 xlen, xpos, ylen, ypos, zcompare, bva;
  uint32 mode, wordsize, pixsize, sdramBase, mpeBase, skipsize;
  uint32 lowerLimit, upperLimit, whichRoutine;

  bool bReadZ, bDirect, bDup, bBatch, bChain, bRemote, bTrigger, bCompareZ, bUpdatePixel, bUpdateZ, bZTestResult;

  bBatch = flags & (1UL << 30);
  bChain = flags & (1UL << 29);
  bRemote = flags & (1UL << 28);
  bDirect = flags & (1UL << 27);
  bDup = flags & (3UL << 26); //bDup = dup | direct
  bTrigger = flags & (1UL << 25);
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
  skipsize = 0;
  bUpdatePixel = true;

  directValue = intaddr;

  if(bChain)
  {
    return;
  }

  map = 0;

  if(pixtype >= 13)
  {
    map = pixtype - 13;
    zmap = 2;
  }
  else if(pixtype >= 9)
  {
    map = pixtype - 9;
    zmap = 3;
  }

  if(zcompare != 7)
  {
    //pixel+Z write (16 + 16Z)
    bCompareZ = (zcompare ? true : false);
    bUpdatePixel = true;
    bUpdateZ = true;
    srcStrideShift = 1;
  }
  else
  {
    //pixel only write (16 bit)
    bCompareZ = false;
    bUpdatePixel = true;
    bUpdateZ = false;
    srcStrideShift = 0;
  }

  if(bRemote)
  {
    //internal address is system address (but still in MPE memory)
    intMemory = nuonEnv->GetPointerToMemory(nuonEnv->mpe[(mpeBase >> 23) & 0x1FUL], mpeBase & 0x207FFFFF, false);
  }
  else
  {
    //internal address is local to MPE
    intMemory = nuonEnv->GetPointerToMemory(the_mpe, mpeBase, false);
  }

  //base address is always a system address (absolute)

  baseMemory = nuonEnv->GetPointerToMemory(nuonEnv->mpe[(sdramBase >> 23) & 0x1FUL], sdramBase, false);

  pSrc = (void *)baseMemory;
  pDest = (void *)intMemory;
  srcOffset = ((ypos * (uint32)xsize)) + xpos;
  destOffset = 0;

  destAStart = 0;
  destAStep = 1;
  destBStart = 0;
  destBStep = xlen;

/*
  if(((intaddr & MPE_LOCAL_MEMORY_MASK) >= MPE_IROM_BASE) &&
    ((intaddr & MPE_LOCAL_MEMORY_MASK) < MPE_DTAGS_BASE))
  {
    //Maintain cache coherency!  This assumes that code will not be
    //dynamically created in the dtrom/dtram section, bypassing the need
    //to flush the cache on data writes.
    if(bRemote)
    {
      nuonEnv->mpe[(intaddr >> 23) & 0x1FUL]->InvalidateICache();
      nuonEnv->mpe[(intaddr >> 23) & 0x1FUL]->nativeCodeCache->Flush();
      nuonEnv->mpe[(intaddr >> 23) & 0x1FUL]->UpdateInvalidateRegion(MPE_IRAM_BASE, length << 2);
    }
    else
    {
      the_mpe->InvalidateICache();
      the_mpe->nativeCodeCache->Flush();
      the_mpe->UpdateInvalidateRegion(MPE_IRAM_BASE, length << 2)
    }
  }
*/

  srcAStart = 0;
  srcAStep = 1;
  srcBStep = xsize;

  bCount = ylen;
  aCountInit = xlen;

  pSrc32 = (uint32 *)pSrc;
  pSrc32 += srcOffset;
  pDest16 = (uint16 *)pDest;
  pDest16 += destOffset;
  pDest32 = (uint32 *)pDest;
  pDest32 += destOffset;
  srcB = 0;
  destB = 0;

  if(zcompare == 7)
  {
    while(bCount--)
    {
      srcA = 0;
      destA = 0;
      aCount = xlen;

      while(aCount--)
      {
        pDest16[destA + destB] = ((uint16 *)(&pSrc32[srcA + srcB]))[0];

        srcA += 1;
        destA += 1;
      }

      srcB += xsize;
      destB += xlen;
    }
  }
  else
  {
    while(bCount--)
    {
      srcA = 0;
      destA = 0;
      aCount = xlen;

      while(aCount--)
      {
        pDest32[destA + destB] = pSrc32[srcA + srcB];
        //((uint16 *)(&pDest32[destA + destB]))[0] = ((uint16 *)(&pSrc32[srcA + srcB]))[0];

        srcA += 1;
        destA += 1;
      }

      srcB += xsize;
      destB += xlen;
    }
  }
}

void BDMA_Type12_Read_1(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_2(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_3(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_4(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_5(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_6(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type12_Read_7(MPE* const the_mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}
