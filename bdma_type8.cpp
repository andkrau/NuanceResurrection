#include "basetypes.h"
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment nuonEnv;

// 32bit -> 16bit RGB conversion
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

  if(bRemote)
    assert(((mpeBase >> 23) & 0x1Fu) < 4);
  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1Fu) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1Fu, sdramBase, false);

  const void* pSrc = intMemory;

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
  uint16* pDest16 = ((uint16 *)baseMemory) + destOffset;

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
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_1", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_2", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_3", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_4", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_5", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_6", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_7", "Error", MB_OK);
#endif
}

// 16bit -> 32bit RGB conversion
void BDMA_Type8_Read_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  //SI: Rem 0 Dir 0 Dup 0 Read 1 xs 256 zc 7 pix 8 bva 0

  const bool bRemote = flags & (1UL << 28);
  //const bool bDirect = flags & (1UL << 27);
  //const bool bDup = flags & (3UL << 26); //bDup = dup | direct
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

  if(bRemote)
    assert(((mpeBase >> 23) & 0x1Fu) < 4);
  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1FUL) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1Fu, sdramBase, false);

  constexpr int32 srcAStep = 1;
  const int32 srcBStep = xsize;

  const uint32 srcOffset = ypos * (uint32)xsize + xpos;
  constexpr uint32 destOffset = 0;

  //BVA = 000 (horizontal DMA, x increment, y increment)
  constexpr int32 destAStep = 1;
  const int32 destBStep = xlen;

  const uint16* pSrc16 = ((uint16 *)baseMemory) + srcOffset;
  uint32* pDest32 = ((uint32 *)intMemory) + destOffset;

  while(ylen--)
  {
    for(uint32 A = 0; A < xlen; ++A) // as srcAStep and destAStep==1
    {
      const uint32 pix16 = SwapBytes(pSrc16[A]);
      pDest32[A] = SwapBytes(((pix16 & 0xFC00U) << 16) | ((pix16 & 0x03E0U) << 14) | ((pix16 & 0x001FU) << 11));
    }

    pSrc16 += srcBStep;
    pDest32 += destBStep;
  }
}

void BDMA_Type8_Read_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_1", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_2", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_3", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_4", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_5", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_6", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_7", "Error", MB_OK);
#endif
}
