#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
#include <windows.h>
#endif
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "video.h"

extern VidChannel structMainChannel;
extern NuonEnvironment nuonEnv;

void BDMA_Type5_Write_0(MPE &mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  const bool bRemote = flags & (1UL << 28);
  const bool bDirect = flags & (1UL << 27);
  const bool bDup = flags & (3UL << 26); //bDup = dup | direct
  //const bool bTrigger = flags & (1UL << 25);
  //const bool bRead = flags & (1UL << 13);
  const int32 xsize = (flags >> 13) & 0x7F8UL;
  //const uint32 type = (flags >> 14) & 0x03UL;
  //const uint32 mode = flags & 0xFFFUL;
  const uint32 zcompare = (flags >> 1) & 0x07UL;
  const uint32 pixtype = (flags >> 4) & 0x0FUL;
  //const uint32 bva = ((flags >> 7) & 0x06UL) | (flags & 0x01UL);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEUL;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCUL;
  const uint32 xlen = (xinfo >> 16) & 0x3FFUL;
  const uint32 xpos = xinfo & 0x7FFUL;
  const uint32 ylen = (yinfo >> 16) & 0x3FFUL;
  const uint32 ypos = yinfo & 0x7FFUL;

  uint32 map = 0;
  uint32 zmap = 1;
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

  uint8 srcStrideShift;
  bool bCompareZ; //, bUpdatePixel;
  const bool bUpdateZ = (zcompare != 7);
  if(bUpdateZ)
  {
    //pixel+Z write (16 + 16Z)
    bCompareZ = (zcompare != 0);
    //bUpdatePixel = true;
    srcStrideShift = 1;
  }
  else
  {
    //pixel only write (16 bit)
    bCompareZ = false;
    //bUpdatePixel = true;
    srcStrideShift = 0;
  }

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

  //const uint32 srcOffset = 0;
  const uint32 destOffset = ypos*(uint32)xsize + xpos;

  const uint16* pSrcColor = (uint16 *)intMemory;
  const uint16* pSrcZ = pSrcColor+1;

  uint16* const pDestColor = ((uint16 *)baseMemory) + (xsize * structMainChannel.src_height * map + destOffset);
  uint16* const pDestZ = ((uint16 *)baseMemory) + (xsize * structMainChannel.src_height * zmap + destOffset);

  //if(bDirect && !bDup)
  //  bDirect = true;

  uint16 directZ, directColor;
  int32 srcAStep, srcBStep;
  if(bDup)
  {
    if(bDirect)
    {
      //Direct and Dup: intaddr is data.
      directColor = intaddr >> 16;
      directZ = intaddr & 0xFFFFUL;
      SwapWordBytes(&directColor);
      SwapWordBytes(&directZ);
    }
    else
    {
      //Dup but not Direct: read scalar from memory, no need to swap
      directColor = *((uint16 *)intMemory);
      directZ = *(((uint16 *)intMemory) + 1);
    }

    pSrcColor = &directColor;
    pSrcZ = &directZ;
    srcAStep = 0;
    srcBStep = 0;
  }
  else
  {
    //if zcompare = 7 then then mpe pixel type is 16-bit without Z and the stride shift value is 0
    //if zcompare != 7 then the mpe pixel type is 16+16Z and the stride shift value is 1
    srcAStep = 1u << srcStrideShift;
    srcBStep = xsize << srcStrideShift;
  }

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

  uint32 srcB = 0;
  uint32 destB = 0;
  uint32 bCount = ylen;

  while(bCount--)
  {
    uint32 srcA = srcB;
    uint32 destA = destB;
    uint32 aCount = xlen;

    //BVA = 000 (horizontal DMA, x increment, y increment)
    constexpr int32 destAStep = 1;
    const int32 destBStep = xsize;

    while(aCount--)
    {
      bool bZTestResult = false;

      if(bCompareZ)
      {
        uint16 ztarget = pDestZ[destA];
        uint16 ztransfer = pSrcZ[srcA];
        SwapWordBytes(&ztarget);
        SwapWordBytes(&ztransfer);

        switch(zcompare)
        {
          //case 0x0:
            //bZTestResult = false;
            //break;
          case 0x1:
            bZTestResult = (ztarget < ztransfer);
            break;
          case 0x2:
            bZTestResult = (ztarget == ztransfer);
            break;
          case 0x3:
            bZTestResult = (ztarget <= ztransfer);
            break;
          case 0x4:
            bZTestResult = (ztarget > ztransfer);
            break;
          case 0x5:
            bZTestResult = (ztarget != ztransfer);
            break;
          case 0x6:
            bZTestResult = (ztarget >= ztransfer);
            break;
          //case 0x7:
            //bZTestResult = false;
            //break;
        }
      }

      if(!bZTestResult)
      {
        pDestColor[destA] = pSrcColor[srcA];
        if(bUpdateZ)
          pDestZ[destA] = pSrcZ[srcA];
      }

      srcA += srcAStep;
      destA += destAStep;
    }

    srcB += srcBStep;
    destB += destBStep;
  }
}

void BDMA_Type5_Write_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Write_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Write_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Write_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Write_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Write_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Write_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  //const bool bBatch = flags & (1UL << 30);
  const bool bChain = flags & (1UL << 29);

  if (bChain)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    MessageBox(NULL, "Chained DMA not supported", "DMABiLinear Error", MB_OK);
#endif
    return;
  }

  const bool bRemote = flags & (1UL << 28);
  //const bool bDirect = flags & (1UL << 27);
  //const bool bDup = flags & (3UL << 26); //bDup = dup | direct
  //const bool bTrigger = flags & (1UL << 25);
  const int32 xsize = (flags >> 13) & 0x7F8UL;
  //const uint32 type = (flags >> 14) & 0x03UL;
  //const uint32 mode = flags & 0xFFFUL;
  const uint32 zcompare = (flags >> 1) & 0x07UL;
  const uint32 pixtype = (flags >> 4) & 0x0FUL;
  //const uint32 bva = ((flags >> 7) & 0x06UL) | (flags & 0x01UL);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEUL;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCUL;
  const uint32 xlen = (xinfo >> 16) & 0x3FFUL;
  const uint32 xpos = xinfo & 0x7FFUL;
  const uint32 ylen = (yinfo >> 16) & 0x3FFUL;
  const uint32 ypos = yinfo & 0x7FFUL;

  uint32 map = 0;
  uint32 zmap = 1;

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

  /*
  bool bCompareZ, bUpdatePixel;
  bool bUpdateZ = (zcompare != 7);
  uint8 srcStrideShift;
  if(bUpdateZ)
  {
    //pixel+Z write (16 + 16Z)
    bCompareZ = (zcompare ? true : false);
    bUpdatePixel = true;
    srcStrideShift = 1;
  }
  else
  {
    //pixel only write (16 bit)
    bCompareZ = false;
    bUpdatePixel = true;
    srcStrideShift = 0;
  }*/

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
    intMemory = nuonEnv.GetPointerToMemory(mpe, mpeBase, false);
  }

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1FUL) < 4);
  void * const baseMemory = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(sdramBase >> 23) & 0x1FUL], sdramBase, false);

  const void* const pSrc = (void *)baseMemory;
  void* const pDest = (void *)intMemory;

  if(((intaddr & MPE_LOCAL_MEMORY_MASK) >= MPE_IRAM_BASE) &&
    ((intaddr & MPE_LOCAL_MEMORY_MASK) < MPE_DTAGS_BASE))
  {
    //Maintain cache coherency!  This assumes that code will not be
    //dynamically created in the dtrom/dtram section, bypassing the need
    //to flush the cache on data writes.
    if(bRemote)
    {
      //nuonEnv.mpe[(mpeBase >> 23) & 0x1FUL].InvalidateICache();
      //nuonEnv.mpe[(mpeBase >> 23) & 0x1FUL].nativeCodeCache.Flush();
      nuonEnv.mpe[(mpeBase >> 23) & 0x1FUL].UpdateInvalidateRegion(MPE_IRAM_BASE, MPE::overlayLengths[(mpeBase >> 23) & 0x1FUL]);
    }
    else
    {
      //mpe.InvalidateICache();
      //mpe.nativeCodeCache.Flush();
      mpe.UpdateInvalidateRegion(MPE_IRAM_BASE, MPE::overlayLengths[mpe.mpeIndex]);
    }
  }

  constexpr int32 srcAStep = 1;
  const int32 srcBStep = xsize;
  constexpr int32 destAStep = 1;
  const int32 destBStep = xlen;

  const uint32 srcOffset = ypos * (uint32)xsize + xpos;
  const uint16* pSrcColor = (uint16*)pSrc + (xsize * structMainChannel.src_height * map + srcOffset);
  const uint16* pSrcZ = (uint16 *)pSrc + (xsize * structMainChannel.src_height * zmap + srcOffset);

  //const uint32 destOffset = 0;
  uint16* const pDest16 = (uint16 *)pDest;
  //pDest16 += destOffset;

  uint32 srcB = 0;
  uint32 destB = 0;
  uint32 bCount = ylen;

  if(zcompare == 7)
  {
    while(bCount--)
    {
      uint32 srcA = srcB;
      uint32 destA = destB;
      uint32 aCount = xlen;

      while(aCount--)
      {
        pDest16[destA] = pSrcColor[srcA];

        srcA += srcAStep;
        destA += destAStep;
      }

      srcB += srcBStep;
      destB += destBStep;
    }
  }
  else
  {
    while(bCount--)
    {
      uint32 srcA = srcB;
      uint32 destA = destB;
      uint32 aCount = xlen;

      while(aCount--)
      {
        pDest16[destA] = pSrcColor[srcA];
        pDest16[destA + 1] = pSrcZ[srcA];

        srcA += srcAStep;
        destA += (destAStep << 1);
      }

      srcB += srcBStep;
      destB += destBStep;
    }
  }
}

void BDMA_Type5_Read_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}

void BDMA_Type5_Read_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
}
