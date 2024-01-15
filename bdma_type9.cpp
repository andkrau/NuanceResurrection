#include "basetypes.h"
#include <cassert>
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment nuonEnv;
extern VidChannel structMainChannel;

void BDMA_Type9_Write_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
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

  //pixel+Z write (16 + 16Z)
  const bool bCompareZ = (zcompare != 0);

  uint32 map;
  //if(pixtype == 12)
  //  map = 2;

  uint32 zmap;
  if(pixtype >= 13)
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

  if(bRemote)
    assert(((mpeBase >> 23) & 0x1Fu) < 4);
  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  assert(((sdramBase >> 23) & 0x1Fu) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1Fu, sdramBase, false);

  const void *pSrc = intMemory;
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

  const uint32 destZOffset = xsize * structMainChannel.src_height * zmap;
  const uint32 mapOffset = xsize * structMainChannel.src_height * map;

  const uint32* pSrc32 = ((uint32*)pSrc) + srcOffset;
  const uint16* pSrc16 = ((uint16*)pSrc) + srcOffset;
  uint16* pDest16 = ((uint16*)pDest) + destOffset;

  const bool bCompareZ2 = bCompareZ && (zcompare > 0) && (zcompare < 7);

  while(ylen--)
  {
    uint32 srcA = 0;

    if(!bCompareZ2)
    {
    for(uint32 destA = 0; destA < xlen; ++destA) // as destAStep == 1
    {
      if(zcompare == 7)
      {
        pDest16[destA + mapOffset] = pSrc16[srcA];
      }
      else
      {
        pDest16[destA + mapOffset] = ((uint16 *)(&pSrc32[srcA]))[0];
        pDest16[destA + destZOffset] = ((uint16 *)(&pSrc32[srcA]))[1];
      }

      srcA += srcAStep;
    }
    }
    else
    for(uint32 destA = 0; destA < xlen; ++destA) // as destAStep == 1
    {
      bool bZTestResult = false;

      //if(bCompareZ)
      {
        const int16 ztarget = SwapBytes(pDest16[destA + destZOffset]);
        const int16 ztransfer = SwapBytes(((uint16 *)(&pSrc32[srcA]))[1]);

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
        //if(zcompare == 7)
        //{
        //  pDest16[destA + mapOffset] = pSrc16[srcA]; // handled above
        //}
        //else
        {
          pDest16[destA + mapOffset] = ((uint16 *)(&pSrc32[srcA]))[0];
          pDest16[destA + destZOffset] = ((uint16 *)(&pSrc32[srcA]))[1];
        }
      }

      srcA += srcAStep;
    }

    pSrc32 += srcBStep;
    pSrc16 += srcBStep;
    pDest16 += destBStep;
  }
}

void BDMA_Type9_Write_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_1", "Error", MB_OK);
#endif
}

void BDMA_Type9_Write_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_2", "Error", MB_OK);
#endif
}

void BDMA_Type9_Write_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_3", "Error", MB_OK);
#endif
}

void BDMA_Type9_Write_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_4", "Error", MB_OK);
#endif
}

void BDMA_Type9_Write_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_5", "Error", MB_OK);
#endif
}

void BDMA_Type9_Write_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_6", "Error", MB_OK);
#endif
}

void BDMA_Type9_Write_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Write_7", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_0", "Error", MB_OK);
#endif
  mpe.regs[0] = 0;
}

void BDMA_Type9_Read_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_1", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_2", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_3", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_4", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_5", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_6", "Error", MB_OK);
#endif
}

void BDMA_Type9_Read_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type9_Read_7", "Error", MB_OK);
#endif
}
