#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
#include <windows.h>
#endif
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "video.h"

extern VidChannel structMainChannel;
extern NuonEnvironment nuonEnv;

void BDMA_Type12_Write_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
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
        uint32 ylen = (yinfo >> 16) & 0x3FFUL;
  const uint32 ypos = yinfo & 0x7FFUL;

  //uint32 map = 0;
  uint32 zmap = 1;

  if(pixtype >= 13)
  {
    //map = pixtype - 13;
    zmap = 2;
  }
  else if(pixtype >= 9)
  {
    //map = pixtype - 9;
    zmap = 3;
  }

  bool bCompareZ/*, bUpdatePixel, bUpdateZ*/;
  if(zcompare != 7)
    //Z write, no pixel (16Z)
    bCompareZ = (zcompare != 0);
  else
    //Z write, no pixel (16Z, no compare?)
    bCompareZ = false;
  //bUpdatePixel = false;
  //bUpdateZ = true;

  if(bRemote)
    assert(((mpeBase >> 23) & 0x1Fu) < 4);
  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1Fu) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1Fu, sdramBase, false);

  constexpr uint32 srcOffset = 0;
  const uint32 destOffset = ypos * (uint32)xsize + xpos;
  const uint16* pSrcColor = (uint16 *)intMemory + (1 + srcOffset);
  uint16* pDestColor = (uint16 *)baseMemory + (xsize * structMainChannel.src_height * zmap + destOffset);

  int32 srcAStep, srcBStep;
  uint16 directColor;
  if(bDup)
  {
    if(bDirect)
    {
      //Direct and Dup: intaddr is data.
      directColor = SwapBytes((uint16)(intaddr >> 16));
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

  const bool bCompareZ2 = bCompareZ && (zcompare > 0) && (zcompare < 7);

  while(ylen--)
  {
    uint32 srcA = 0;

    if(!bCompareZ2)
    {
    for(uint32 destA = 0; destA < xlen; ++destA) // as destAStep == 1
    {
      pDestColor[destA] = pSrcColor[srcA];

      srcA += srcAStep;
    }
    }
    else
    for(uint32 destA = 0; destA < xlen; ++destA) // as destAStep == 1
    {
      bool bZTestResult = false;

      //if(bCompareZ)
      {
        const uint16 ztarget = SwapBytes(pDestColor[destA]);
        const uint16 ztransfer = SwapBytes(pSrcColor[srcA]);

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
        pDestColor[destA] = pSrcColor[srcA];

      srcA += srcAStep;
    }

    pSrcColor += srcBStep;
    pDestColor += destBStep;
  }
}

void BDMA_Type12_Write_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_1", "Error", MB_OK);
#endif
}

void BDMA_Type12_Write_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_2", "Error", MB_OK);
#endif
}

void BDMA_Type12_Write_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_3", "Error", MB_OK);
#endif
}

void BDMA_Type12_Write_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_4", "Error", MB_OK);
#endif
}

void BDMA_Type12_Write_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_5", "Error", MB_OK);
#endif
}

void BDMA_Type12_Write_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_6", "Error", MB_OK);
#endif
}

void BDMA_Type12_Write_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Write_7", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  //const bool bBatch = flags & (1UL << 30);
  const bool bChain = flags & (1UL << 29);

  if (bChain)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    MessageBox(NULL,"Chained DMA not supported","DMABiLinear Error",MB_OK);
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
  //const uint32 pixtype = (flags >> 4) & 0x0FUL;
  //const uint32 bva = ((flags >> 7) & 0x06UL) | (flags & 0x01UL);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEUL;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCUL;
  const uint32 xlen = (xinfo >> 16) & 0x3FFUL;
  const uint32 xpos = xinfo & 0x7FFUL;
        uint32 ylen = (yinfo >> 16) & 0x3FFUL;
  const uint32 ypos = yinfo & 0x7FFUL;

  /*uint32 map = 0;
  uint32 zmap;

  if(pixtype >= 13)
  {
    map = pixtype - 13;
    zmap = 2;
  }
  else if(pixtype >= 9)
  {
    map = pixtype - 9;
    zmap = 3;
  }*/

  /*
  bool bCompareZ;//, bUpdatePixel;
  bool bUpdateZ = (zcompare != 7);
  uint32 srcStrideShift;
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
  }*/

  if(bRemote)
    assert(((mpeBase >> 23) & 0x1Fu) < 4);
  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1Fu) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1Fu, sdramBase, false);

  if(((intaddr & MPE_LOCAL_MEMORY_MASK) >= MPE_IRAM_BASE) &&
    ((intaddr & MPE_LOCAL_MEMORY_MASK) < MPE_DTAGS_BASE))
  {
    //Maintain cache coherency!  This assumes that code will not be
    //dynamically created in the dtrom/dtram section, bypassing the need
    //to flush the cache on data writes.
    MPE& m = bRemote ? nuonEnv.mpe[(mpeBase >> 23) & 0x1Fu] : mpe;
    //m.InvalidateICache();
    //m.nativeCodeCache.Flush();
    m.UpdateInvalidateRegion(MPE_IRAM_BASE, MPE::overlayLengths[bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex]);
  }

  constexpr int32 destAStep = 1;
  const int32 destBStep = xlen;

  constexpr int32 srcAStep = 1;
  const int32 srcBStep = xsize;

  const uint32 srcOffset = ypos * (uint32)xsize + xpos;
  constexpr uint32 destOffset = 0;

  if(zcompare == 7)
  {
    const uint16* pSrc16 = (uint16 *)baseMemory + srcOffset;
    uint16* pDest16 = (uint16 *)intMemory + destOffset;

    while(ylen--)
    {
      for(uint32 A = 0; A < xlen; ++A) // as both destAStep and srcAStep == 1
        pDest16[A] = pSrc16[A];

      pSrc16 += srcBStep;
      pDest16 += destBStep;
    }
  }
  else
  {
    const uint32* pSrc32 = (uint32 *)baseMemory + srcOffset;
    uint32* pDest32 = (uint32 *)intMemory + destOffset;

    while(ylen--)
    {
      for (uint32 A = 0; A < xlen; ++A) // as both destAStep and srcAStep == 1
      {
        pDest32[A] = pSrc32[A];
        //((uint16 *)(&pDest32[A]))[0] = ((uint16 *)(&pSrc32[A]))[0];
      }

      pSrc32 += srcBStep;
      pDest32 += destBStep;
    }
  }
}

void BDMA_Type12_Read_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_1", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_2", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_3", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_4", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_5", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_6", "Error", MB_OK);
#endif
}

void BDMA_Type12_Read_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type12_Read_7", "Error", MB_OK);
#endif
}
