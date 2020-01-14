#include "basetypes.h"
#include "byteswap.h"
#include "dma.h"
#include "NuonEnvironment.h"

extern NuonEnvironment *nuonEnv;

void BDMA_Type5_Write_0(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
  uint32 *pSrc32, *pDest32;
  void *intMemory, *baseMemory, *pSrc, *pDest;
  uint32 directValue, type, pixtype;
  uint32 aCount, bCount;
  uint32 srcA, srcB, destA, destB, srcOffset, destOffset;
  int32 srcAStep, srcBStep, destAStep, destBStep, xsize;
  uint32 xlen, xpos, ylen, ypos, zcompare, bva;
  uint32 mode, sdramBase, mpeBase, skipsize;

  bool bRead, bDirect, bDup, bBatch, bChain, bRemote, bTrigger, bCompareZ, bUpdatePixel, bUpdateZ, bZTestResult;

  bBatch = flags & (1UL << 30);
  bChain = flags & (1UL << 29);
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
  skipsize = 0;

  directValue = intaddr;

  if(bChain)
  {
    //MessageBox(0,"Chained DMA not supported","DMABiLinear Error",MB_OK);
    return;
  }

  if(zcompare != 7)
  {
    //pixel+Z write (16 + 16Z)
    bCompareZ = true;
    bUpdatePixel = true;
    bUpdateZ = true;
  }
  else
  {
    //pixel only write (16 bit)
    bCompareZ = false;
    bUpdatePixel = true;
    bUpdateZ = false;
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

  if((sdramBase < 0x40000000) || (sdramBase > 0x4FFFFFFF))
  {
    //char msgBuf[512];
    //sprintf(msgBuf,"sdramBase is out of range on mpe%d: 0x%lx\n",the_mpe->mpeIndex,sdramBase);
    //::MessageBox(NULL,msgBuf,"DMABiLinear error",MB_OK);
  }
 
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
  pDest32 = (uint32 *)pDest + destOffset;
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

        ztarget = ((uint16 *)&pDest32[destA + destB])[1];
        ztransfer = ((uint16 *)&pSrc32[srcA + srcB])[1];
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
        pDest32[destA + destB] = pSrc32[srcA + srcB];
      }

      srcA += srcAStep;
      destA += 1;
    }

    srcB += srcBStep;
    destB += xsize;
  }
}

void BDMA_Type5_Write_1(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Write_2(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Write_3(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Write_4(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Write_5(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Write_6(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Write_7(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_0(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_1(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_2(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_3(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_4(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_5(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_6(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}

void BDMA_Type5_Read_7(MPE *the_mpe, uint32 baseaddr, uint32 flags, uint32 xinfo, uint32 yinfo, uint32 intaddr)
{
}
