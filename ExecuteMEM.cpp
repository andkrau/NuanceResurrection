#include <assert.h>
#include "basetypes.h"
#include "Byteswap.h"
#include "ExecuteMem.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "NuonMemoryMap.h"
#include "NuonEnvironment.h"

#define XTILEMASK (~(0xFFFF0000UL << (16 - BilinearInfo_XTile(control))))
#define YTILEMASK (~(0xFFFF0000UL << (16 - BilinearInfo_YTile(control))))

#define MIP(mip_me) (((uint32)(mip_me)) >> BilinearInfo_XYMipmap(control))
#define SIGNMIP(mip_me) (((int32)(mip_me)) >> BilinearInfo_XYMipmap(control))

#define REG_X (0)
#define REG_Y (1)
#define REG_U (2)
#define REG_V (3)


extern int32 pixel_type_width[];
extern NuonEnvironment *nuonEnv;

inline void SaturateColorComponents(uint32 *Y, uint32 *Cr, uint32 *Cb, bool bChnorm)
{
  static uint32 YLookup[] = {0x00UL,0xFFUL,0x00UL,0x00UL};

  YLookup[0] = (*Y >> (13 - 7)) & 0xFFUL;
  *Y = YLookup[*Y >> 14];

  switch(*Cr >> 14)
  {
    case 0:
      //If chnorm bit is set, clamp to 0x7F
      if(bChnorm && (*Cr > 0x1FFF))
      {
        *Cr = 0x7F;
      }
      else
      {
        *Cr = (*Cr >> (13 - 7)) & 0xFFUL;
      }
      break;
    case 1:
      //clamp to 0x7F or 0xFF
      if(bChnorm)
      {
        *Cr = 0x7F;
      }
      else
      {
        *Cr = 0xFF;
      }
      break;
    case 2:
      //clamp to 0x80 or 0x00
      if(bChnorm)
      {
        *Cr = 0x80;
      }
      else
      {
        *Cr = 0x00;
      }
      break;
    case 3:
      //Clamp to 0x80 or 0x00
      if(bChnorm && (*Cr < 0xE000))
      {
        *Cr = 0x80;
      }
      else
      {
        *Cr = (*Cr >> (13 - 7)) & 0xFFUL;
        if(!bChnorm)
        {
          *Cr = 0x00;
        }
      }
      break;
  }

  switch(*Cb >> 14)
  {
    case 0:
      //If chnorm bit is set, clamp to 0x7F
      if(bChnorm && (*Cb > 0x1FFF))
      {
        *Cb = 0x7F;
      }
      else
      {
        *Cb = (*Cb >> (13 - 7)) & 0xFFUL;
      }
      break;
    case 1:
      //clamp to 0x7F or 0xFF
      if(bChnorm)
      {
        *Cb = 0x7F;
      }
      else
      {
        *Cb = 0xFF;
      }
      break;
    case 2:
      //clamp to 0x80 or 0x00
      if(bChnorm)
      {
        *Cb = 0x80;
      }
      else
      {
        *Cb = 0x00;
      }
      break;
    case 3:
      //Clamp to 0x80 or 0x00
      if(bChnorm && (*Cb < 0xE000))
      {
        *Cb = 0x80;
      }
      else
      {
        *Cb = (*Cb >> (13 - 7)) & 0xFFUL;
        if(!bChnorm)
        {
          *Cb = 0x00;
        }
      }
      break;
  }

  if(bChnorm)
  {
    *Cr = (*Cr + 0x80) & 0xFFUL;
    *Cb = (*Cb + 0x80) & 0xFFUL;
  }
}

uint32 mipped_xoffset = 0;

inline void CalculateBilinearAddress(MPE &mpe, uint32 *pOffsetAddress, uint32 control, uint32 x, uint32 y)
{
  if(BilinearInfo_XRev(control))
  {
    *((uint8 *)&x) = mpe.mirrorLookup[*((uint8 *)&x)];
    *(((uint8 *)&x) + 1) = mpe.mirrorLookup[*(((uint8 *)&x) + 1)];
    SwapWordBytes((uint16 *)&x);
  }

  if(BilinearInfo_YRev(control))
  {
    *((uint8 *)&y) = mpe.mirrorLookup[*((uint8 *)&y)];
    *(((uint8 *)&y) + 1) = mpe.mirrorLookup[*(((uint8 *)&y) + 1)];
    SwapWordBytes((uint16 *)&y);
  }

  //*pOffsetAddress = (((MIP(y) & SIGNMIP(YTILEMASK)) >> 16) * MIP(bi->xy_width) + ((MIP(x) & SIGNMIP(XTILEMASK)) >> 16));
  //mipped_xoffset = ((MIP(x) & SIGNMIP(XTILEMASK)) >> 16);
  //*pOffsetAddress = (((MIP(y) & SIGNMIP(YTILEMASK)) >> 16) * MIP(BilinearInfo_XYWidth(control)) + mipped_xoffset);
  mipped_xoffset = (MIP((x)) & SIGNMIP(XTILEMASK)) >> 16;
  *pOffsetAddress = (((MIP((y)) & SIGNMIP(YTILEMASK)) >> 16) * MIP(BilinearInfo_XYWidth(control)) + mipped_xoffset);
}

structBilinearAddressInfo bilinearAddressInfo;

void GetBilinearAddress(void)
{
  uint32 control = bilinearAddressInfo.control;
  int32 pixwidth = BilinearInfo_PixelWidth(pixel_type_width,control);
  uint32 address;

  if(BilinearInfo_XRev(control))
  {
    *((uint8 *)&(bilinearAddressInfo.x)) = MPE::mirrorLookup[*((uint8 *)(&bilinearAddressInfo.x))];
    *(((uint8 *)&(bilinearAddressInfo.x) + 1)) = MPE::mirrorLookup[*(((uint8 *)(&bilinearAddressInfo.x)) + 1)];
    SwapWordBytes((uint16 *)&bilinearAddressInfo.x);
  }

  if(BilinearInfo_YRev(control))
  {
    *((uint8 *)&bilinearAddressInfo.y) = MPE::mirrorLookup[*((uint8 *)(&bilinearAddressInfo.y))];
    *(((uint8 *)&bilinearAddressInfo.y) + 1) = MPE::mirrorLookup[*(((uint8 *)(&bilinearAddressInfo.y)) + 1)];
    SwapWordBytes((uint16 *)&bilinearAddressInfo.y);
  }

  //*pOffsetAddress = (((MIP(y) & SIGNMIP(YTILEMASK)) >> 16) * MIP(bi->xy_width) + ((MIP(x) & SIGNMIP(XTILEMASK)) >> 16));
  //mipped_xoffset = ((MIP(x) & SIGNMIP(XTILEMASK)) >> 16);
  //*pOffsetAddress = (((MIP(y) & SIGNMIP(YTILEMASK)) >> 16) * MIP(BilinearInfo_XYWidth(control)) + mipped_xoffset);
  mipped_xoffset = (MIP((bilinearAddressInfo.x)) & SIGNMIP(XTILEMASK)) >> 16;
  address = (((MIP((bilinearAddressInfo.y)) & SIGNMIP(YTILEMASK)) >> 16) * MIP(BilinearInfo_XYWidth(control)) + mipped_xoffset);

  if(pixwidth >= 0)
  {
    //Everything but 4-bit pixels and MPEG
    address <<= pixel_type_width[(control >> 20) & 0x0FUL];
  }
  else
  {
    //4-bit pixels
    address >>= 1;
  }

  bilinearAddressInfo.offset_address = (bilinearAddressInfo.base & 0xFFFFFFFC) + address;
  bilinearAddressInfo.mipped_xoffset = mipped_xoffset;
}

void Execute_Mirror(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 x = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];

  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	
  mpe.regs[nuance.fields[FIELD_MEM_TO]] = ((x >> 16) | (x << 16));
}
void Execute_MV_SImmediate(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MEM_TO]] = nuance.fields[FIELD_MEM_FROM];
}
void Execute_MV_SScalar(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MEM_TO]] = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
}
void Execute_MV_V(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 src = nuance.fields[FIELD_MEM_FROM];
  uint32 dest = nuance.fields[FIELD_MEM_TO];

  mpe.regs[dest] = entry.pScalarRegs[src];
  mpe.regs[dest + 1] = entry.pScalarRegs[src + 1];
  mpe.regs[dest + 2] = entry.pScalarRegs[src + 2];
  mpe.regs[dest + 3] = entry.pScalarRegs[src + 3];
}
void Execute_PopVector(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 *srcPtr;
  uint32 *destPtr;
  uint32 dest_vector[4];

  srcPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  dest_vector[0] = srcPtr[0];
  dest_vector[1] = srcPtr[1];
  dest_vector[2] = srcPtr[2];
  dest_vector[3] = srcPtr[3];

  destPtr = (uint32 *)&(mpe.regs[nuance.fields[FIELD_MEM_TO]]);

  SwapVectorBytes(dest_vector);

  destPtr[0] = dest_vector[0];
  destPtr[1] = dest_vector[1];
  destPtr[2] = dest_vector[2];
  destPtr[3] = dest_vector[3];

  mpe.sp += 16;
}
void Execute_PopVectorRz(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 dest_vector[4];
  uint32  *srcPtr;
  uint32 *destPtr;

  srcPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  dest_vector[0] = srcPtr[0];
  dest_vector[1] = srcPtr[1];
  dest_vector[2] = srcPtr[2];
  dest_vector[3] = srcPtr[3];

  destPtr = (uint32 *)&mpe.regs[nuance.fields[FIELD_MEM_TO]];

  SwapVectorBytes(dest_vector);

  destPtr[0] = dest_vector[0];
  destPtr[1] = dest_vector[1];
  destPtr[2] = dest_vector[2];
  mpe.rz = dest_vector[3];

  mpe.sp += 16;
}
void Execute_PopScalarRzi1(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 dest_vector[4];
  uint32  *srcPtr;

  srcPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  dest_vector[0] = srcPtr[0];
  dest_vector[1] = srcPtr[1];
  dest_vector[2] = srcPtr[2];
  dest_vector[3] = srcPtr[3];

  SwapVectorBytes(dest_vector);

  mpe.regs[nuance.fields[FIELD_MEM_TO]] = dest_vector[0];
  mpe.cc = dest_vector[1];
  mpe.rzi1 = dest_vector[2];
  mpe.rz = dest_vector[3];

  mpe.sp += 16;
}
void Execute_PopScalarRzi2(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 dest_vector[4];
  uint32  *srcPtr;

  srcPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  dest_vector[0] = srcPtr[0];
  dest_vector[1] = srcPtr[1];
  dest_vector[2] = srcPtr[2];
  dest_vector[3] = srcPtr[3];

  SwapVectorBytes(dest_vector);

  mpe.regs[nuance.fields[FIELD_MEM_TO]] = dest_vector[0];
  mpe.cc = dest_vector[1];
  mpe.rzi2 = dest_vector[2];
  mpe.rz = dest_vector[3];

  mpe.sp += 16;
}
void Execute_PushVector(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 src_vector[4];
  uint32 *pVal;
  uint32 *destPtr;

  mpe.sp -= 16;

  pVal = &(entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]]);

  src_vector[0] = pVal[0];
  src_vector[1] = pVal[1];
  src_vector[2] = pVal[2];
  src_vector[3] = pVal[3];

  destPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  SwapVectorBytes(src_vector);

  destPtr[0] = src_vector[0];
  destPtr[1] = src_vector[1];
  destPtr[2] = src_vector[2];
  destPtr[3] = src_vector[3];
}
void Execute_PushVectorRz(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 src_vector[4];
  uint32 *pVal;
  uint32 *destPtr;

  mpe.sp -= 16;

  pVal = &(entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]]);

  src_vector[0] = pVal[0];
  src_vector[1] = pVal[1];
  src_vector[2] = pVal[2];
  src_vector[3] = entry.pRzRegs[0];

  destPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  SwapVectorBytes(src_vector);

  destPtr[0] = src_vector[0];
  destPtr[1] = src_vector[1];
  destPtr[2] = src_vector[2];
  destPtr[3] = src_vector[3];
}
void Execute_PushScalarRzi1(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 src_vector[4];
  uint32 *destPtr;

  mpe.sp -= 16;

  src_vector[0] = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  src_vector[1] = mpe.tempCC;
  src_vector[2] = entry.pRzRegs[1];
  src_vector[3] = entry.pRzRegs[0];

  destPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  SwapVectorBytes(src_vector);

  destPtr[0] = src_vector[0];
  destPtr[1] = src_vector[1];
  destPtr[2] = src_vector[2];
  destPtr[3] = src_vector[3];
}
void Execute_PushScalarRzi2(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 src_vector[4];
  uint32 *destPtr;

  mpe.sp -= 16;

  src_vector[0] = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  src_vector[1] = mpe.tempCC;
  src_vector[2] = entry.pRzRegs[2];
  src_vector[3] = entry.pRzRegs[0];

  destPtr = (uint32 *)&(mpe.dtrom[mpe.sp & MPE_VALID_MEMORY_MASK]);

  SwapVectorBytes(src_vector);

  destPtr[0] = src_vector[0];
  destPtr[1] = src_vector[1];
  destPtr[2] = src_vector[2];
  destPtr[3] = src_vector[3];
}
void Execute_LoadScalarControlRegisterAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MEM_TO]] = mpe.ReadControlRegister(nuance.fields[FIELD_MEM_FROM] - MPE_CTRL_BASE, &entry);
}
void Execute_LoadByteAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 data;
  data = *((uint8 *)nuance.fields[FIELD_MEM_POINTER]);
  data <<= 24;

  mpe.regs[nuance.fields[FIELD_MEM_TO]] = data;
}
void Execute_LoadWordAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 data;
  data = ((uint32)(*((uint8 *)nuance.fields[FIELD_MEM_POINTER]))) << 24;
  data =  data | (((uint32)( *((uint8 *)(nuance.fields[FIELD_MEM_POINTER]+1)) )) << 16);

  mpe.regs[nuance.fields[FIELD_MEM_TO]] = data;
}
void Execute_LoadScalarAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 data;

  data = *((uint32 *)nuance.fields[FIELD_MEM_POINTER]);
  SwapScalarBytes(&data);
  mpe.regs[nuance.fields[FIELD_MEM_TO]] = data;
}
void Execute_LoadScalarLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 dest, address, data;
  dest = nuance.fields[FIELD_MEM_TO];
  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];

  if((address < MPE_CTRL_BASE) || (address >= MPE_RESV_BASE))
  {
    //The reserved area from $00000000 to $1FFFFFFF might always return zero
    //Freefall has a routine which will purposely read from $0 instead of skipping directly to
    //the epilogue code
    if(address >= MPE_ADDR_SPACE_BASE)
    {
      data = *((uint32 *)(nuonEnv->GetPointerToMemory(&mpe,address & 0xFFFFFFFC)));
      SwapScalarBytes(&data);
    }
    else
    {
      data = 0;
    }

    mpe.regs[dest] = data;
  }
  else
  {
    mpe.regs[dest] = mpe.ReadControlRegister(address - MPE_CTRL_BASE, &entry);
  }
}
void Execute_LoadVectorAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 *destPtr;
  uint32 *srcPtr = (uint32 *)nuance.fields[FIELD_MEM_POINTER];
  
  destPtr = &mpe.regs[nuance.fields[FIELD_MEM_TO]];

  destPtr[0] = srcPtr[0];
  destPtr[1] = srcPtr[1];
  destPtr[2] = srcPtr[2];
  destPtr[3] = srcPtr[3];

  SwapVectorBytes(destPtr);
}
void Execute_LoadVectorControlRegisterAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 dest = nuance.fields[FIELD_MEM_TO];
  uint32 address = nuance.fields[FIELD_MEM_FROM];

  mpe.regs[dest] = mpe.ReadControlRegister(address - MPE_CTRL_BASE, &entry);
  mpe.regs[dest + 1] = mpe.ReadControlRegister(address + 4 - MPE_CTRL_BASE, &entry);
  mpe.regs[dest + 2] = mpe.ReadControlRegister(address + 8 - MPE_CTRL_BASE, &entry);
  mpe.regs[dest + 3] = mpe.ReadControlRegister(address + 12 - MPE_CTRL_BASE, &entry);
}

void LoadPixelAbsolute(void)
{
  uint32 control;
  uint32 pixelData32;
  uint32 *regs;
  uint32 pixType;
  uint32 bChnorm;
  
  void *memPtr;
  
  control = bilinearAddressInfo.control;
  memPtr = bilinearAddressInfo.pPixelData;
  regs = bilinearAddressInfo.pRegs;
  pixType = BilinearInfo_XYType(control);
  bChnorm = BilinearInfo_XYChnorm(control);
  
  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      //The initial xoffset is guaranteed to start at the first pixel of a group of four.  
      //This means that for even values of X, the pixel bits to be extracted are always [7:4]
      //and for odd values of X, the pixel bits to be extracted are [3:0]

      pixelData32 = (*((uint8 *)memPtr) >> (4 - ((bilinearAddressInfo.mipped_xoffset & 1) << 2))) & 0x0FUL;
      regs[0] = (bilinearAddressInfo.clutBase & 0xFFFFFFC0UL) | (pixelData32 << 2);
      regs[1] = 0;
      regs[2] = 0;
      return;
    case 0x2:
    case 0x5:
    {
      //16
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      regs[0] = (pixelData32 >> 2) & (0xFCUL << 22);
      regs[1] = (pixelData32 << 4) & (0xF8UL << 22);
      regs[2] = (pixelData32 << 9) & (0xF8UL << 22);

      if(bChnorm)
      {
        regs[1] = (regs[1] - 0x20000000UL) & 0xFE000000UL;
        regs[2] = (regs[2] - 0x20000000UL) & 0xFE000000UL;
      }

      return;
    }
    case 0x3:
      //8 bit
      pixelData32 = *((uint8 *)memPtr);
      regs[0] = (bilinearAddressInfo.clutBase & 0xFFFFFC00UL) | (pixelData32 << 2);
      regs[1] = 0;
      regs[2] = 0;
      return;
    case 0x4:
    case 0x6:
    {
      //32 bit or 32+32Z (both behave the same for LD_P)
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      regs[0] = ((pixelData32 >> 2)) & (0xFFUL << 22);
      regs[1] = ((pixelData32 << 6)) & (0xFFUL << 22);
      regs[2] = ((pixelData32 << 14)) & (0xFFUL << 22);

      if(bChnorm)
      {
        regs[1] = (regs[1] - 0x20000000UL) & 0xFFC00000UL;
        regs[2] = (regs[2] - 0x20000000UL) & 0xFFC00000UL;
      }

      return;
    }
  }
}

void Execute_LoadPixelAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 pixelData32;
  uint32 dest;
  uint32 pixType;
  uint32 bChnorm;
  
  void *memPtr;
  
  memPtr = (void *)((uint32 *)nuance.fields[FIELD_MEM_POINTER]);
  dest = nuance.fields[FIELD_MEM_TO];
  
  if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_UV)
  {
    pixType = BilinearInfo_XYType(*entry.pUvctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pUvctl);
  }
  else if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_XY)
  {
    pixType = BilinearInfo_XYType(*entry.pXyctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pXyctl);
  }
  else
  {
    pixType = BilinearInfo_XYType(mpe.linpixctl);
    bChnorm = BilinearInfo_XYChnorm(mpe.linpixctl);
  }

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      //The initial xoffset is guaranteed to start at the first pixel of a group of four.  
      //This means that for even values of X, the pixel bits to be extracted are always [7:4]
      //and for odd values of X, the pixel bits to be extracted are [3:0]

      pixelData32 = (*((uint8 *)memPtr) >> (4 - ((mipped_xoffset & 1) << 2))) & 0x0FUL;
      mpe.regs[dest] = (mpe.clutbase & 0xFFFFFFC0UL) | (pixelData32 << 2);
      mpe.regs[dest+1] = 0;
      mpe.regs[dest+2] = 0;
      return;
    case 0x2:
    case 0x5:
    {
      //16
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      mpe.regs[dest] = (pixelData32 >> 2) & (0xFCUL << 22);
      mpe.regs[dest+1] = (pixelData32 << 4) & (0xF8UL << 22);
      mpe.regs[dest+2] = (pixelData32 << 9) & (0xF8UL << 22);

      if(bChnorm)
      {
        mpe.regs[dest+1] = (mpe.regs[dest+1] - 0x20000000UL) & 0xFE000000UL;
        mpe.regs[dest+2] = (mpe.regs[dest+2] - 0x20000000UL) & 0xFE000000UL;
      }

      return;
    }
    case 0x3:
      //8 bit
      pixelData32 = *((uint8 *)memPtr);
      mpe.regs[dest] = (mpe.clutbase & 0xFFFFFC00UL) | (pixelData32 << 2);
      mpe.regs[dest+1] = 0;
      mpe.regs[dest+2] = 0;
      return;
    case 0x4:
    case 0x6:
    {
      //32 bit or 32+32Z (both behave the same for LD_P)
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      mpe.regs[dest] = ((pixelData32 >> 2)) & (0xFFUL << 22);
      mpe.regs[dest+1] = ((pixelData32 << 6)) & (0xFFUL << 22);
      mpe.regs[dest+2] = ((pixelData32 << 14)) & (0xFFUL << 22);

      if(bChnorm)
      {
        mpe.regs[dest+1] = (mpe.regs[dest+1] - 0x20000000UL) & 0xFFC00000UL;
        mpe.regs[dest+2] = (mpe.regs[dest+2] - 0x20000000UL) & 0xFFC00000UL;
      }

      return;
    }
  }
}

void LoadPixelZAbsolute(void)
{
  uint32 control;
  uint32 pixelData32, zData32;
  uint32 *regs;
  uint32 pixType;
  uint32 bChnorm;
  
  void *memPtr;
  
  control = bilinearAddressInfo.control;
  memPtr = bilinearAddressInfo.pPixelData;
  regs = bilinearAddressInfo.pRegs;
  pixType = BilinearInfo_XYType(control);
  bChnorm = BilinearInfo_XYChnorm(control);

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      return;
    case 0x2:
    {
      //16
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      regs[0] = (pixelData32 >> 2) & (0xFCUL << 22);
      regs[1] = (pixelData32 << 4) & (0xF8UL << 22);
      regs[2] = (pixelData32 << 9) & (0xF8UL << 22);

      if(bChnorm)
      {
        regs[1] = (regs[1] - 0x20000000UL) & 0xFE000000UL;
        regs[2] = (regs[2] - 0x20000000UL) & 0xFE000000UL;
      }

      return;
    }
    case 0x5:
    {
      //16+16Z
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      regs[0] = (pixelData32 >> 2) & (0xFCUL << 22);
      regs[1] = (pixelData32 << 4) & (0xF8UL << 22);
      regs[2] = (pixelData32 << 9) & (0xF8UL << 22);
      regs[3] = (pixelData32 << 16);

      if(bChnorm)
      {
        regs[1] = (regs[1] - 0x20000000UL) & 0xFE000000UL;
        regs[2] = (regs[2] - 0x20000000UL) & 0xFE000000UL;
      }

      return;
    }
    case 0x3:
      //8 bit
      return;
    case 0x4:
    {
      //32 bit

      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);

      regs[0] = ((pixelData32 >> 2)) & (0xFFUL << 22);
      regs[1] = ((pixelData32 << 6)) & (0xFFUL << 22);
      regs[2] = ((pixelData32 << 14)) & (0xFFUL << 22);
      regs[3] = (pixelData32 << 24);

      if(bChnorm)
      {
        regs[1] = (regs[1] - 0x20000000UL) & 0xFFC00000UL;
        regs[2] = (regs[2] - 0x20000000UL) & 0xFFC00000UL;
      }

      return;
    }
    case 0x6:
    {
      pixelData32 = *((uint32 *)memPtr);
      zData32 = *(((uint32 *)memPtr) + 1);
      SwapScalarBytes(&pixelData32);
      SwapScalarBytes(&zData32);
      regs[0] = ((pixelData32 >> 2)) & (0xFFUL << 22);
      regs[1] = (pixelData32 << 6) & (0xFFUL << 22);
      regs[2] = (pixelData32 << 14) & (0xFFUL << 22);
      regs[3] = zData32;

      if(bChnorm)
      {
        regs[1] = (regs[1] - 0x20000000UL) & 0xFFC00000UL;
        regs[2] = (regs[2] - 0x20000000UL) & 0xFFC00000UL;
      }
      return;
    }
  }
}

void Execute_LoadPixelZAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 pixelData32, zData32;
  uint32 dest;
  uint32 pixType;
  uint32 bChnorm;
  uint16 pixelData16, zData16;
  void *memPtr;

  memPtr = (void *)((uint32 *)nuance.fields[FIELD_MEM_POINTER]);
  dest = nuance.fields[FIELD_MEM_TO];
  
  if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_UV)
  {
    pixType = BilinearInfo_XYType(*entry.pUvctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pUvctl);
  }
  else if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_XY)
  {
    pixType = BilinearInfo_XYType(*entry.pXyctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pXyctl);
  }
  else
  {
    pixType = BilinearInfo_XYType(mpe.linpixctl);
    bChnorm = BilinearInfo_XYChnorm(mpe.linpixctl);
  }

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      return;
    case 0x2:
    {
      //16
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      mpe.regs[dest] = (pixelData32 >> 2) & (0xFCUL << 22);
      mpe.regs[dest+1] = (pixelData32 << 4) & (0xF8UL << 22);
      mpe.regs[dest+2] = (pixelData32 << 9) & (0xF8UL << 22);

      if(bChnorm)
      {
        mpe.regs[dest+1] = (mpe.regs[dest+1] - 0x20000000UL) & 0xFE000000UL;
        mpe.regs[dest+2] = (mpe.regs[dest+2] - 0x20000000UL) & 0xFE000000UL;
      }

      return;
    }
    case 0x5:
    {
      //16+16Z
      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);
      mpe.regs[dest] = (pixelData32 >> 2) & (0xFCUL << 22);
      mpe.regs[dest+1] = (pixelData32 << 4) & (0xF8UL << 22);
      mpe.regs[dest+2] = (pixelData32 << 9) & (0xF8UL << 22);
      mpe.regs[dest+3] = (pixelData32 << 16);

      if(bChnorm)
      {
        mpe.regs[dest+1] = (mpe.regs[dest+1] - 0x20000000UL) & 0xFE000000UL;
        mpe.regs[dest+2] = (mpe.regs[dest+2] - 0x20000000UL) & 0xFE000000UL;
      }

      return;
    }
    case 0x3:
      //8 bit
      return;
    case 0x4:
    {
      //32 bit

      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);

      mpe.regs[dest] = ((pixelData32 >> 2)) & (0xFFUL << 22);
      mpe.regs[dest+1] = ((pixelData32 << 6)) & (0xFFUL << 22);
      mpe.regs[dest+2] = ((pixelData32 << 14)) & (0xFFUL << 22);
      mpe.regs[dest+3] = (pixelData32 << 24);

      if(bChnorm)
      {
        mpe.regs[dest+1] = (mpe.regs[dest+1] - 0x20000000UL) & 0xFFC00000UL;
        mpe.regs[dest+2] = (mpe.regs[dest+2] - 0x20000000UL) & 0xFFC00000UL;
      }

      return;
    }
    case 0x6:
    {
      pixelData32 = *((uint32 *)memPtr);
      zData32 = *(((uint32 *)memPtr) + 1);
      SwapScalarBytes(&pixelData32);
      SwapScalarBytes(&zData32);
      mpe.regs[dest] = ((pixelData32 >> 2)) & (0xFFUL << 22);
      mpe.regs[dest+1] = (pixelData32 << 6) & (0xFFUL << 22);
      mpe.regs[dest+2] = (pixelData32 << 14) & (0xFFUL << 22);
      mpe.regs[dest+3] = zData32;

      if(bChnorm)
      {
        mpe.regs[dest+1] = (mpe.regs[dest+1] - 0x20000000UL) & 0xFFC00000UL;
        mpe.regs[dest+2] = (mpe.regs[dest+2] - 0x20000000UL) & 0xFFC00000UL;
      }
      return;
    }
  }
}

void Execute_LoadByteLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 address, offset, data;
  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];

  data = *((uint8 *)(nuonEnv->GetPointerToMemory(&mpe,address)));
  mpe.regs[nuance.fields[FIELD_MEM_TO]] = data << 24;
}

void Execute_LoadByteBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;
  
  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadByteAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadByteAbsolute(mpe,entry,newNuance);
}
void Execute_LoadByteBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadByteAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadByteAbsolute(mpe,entry,newNuance);
}
void Execute_LoadWordLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 dest, address, offset, data;
  uint8* memPtr;
  dest = nuance.fields[FIELD_MEM_TO];
  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];

  memPtr = (uint8 *)(nuonEnv->GetPointerToMemory(&mpe,address & 0xFFFFFFFE));
  data = ((uint32)(*memPtr)) << 24;
  data = data | (((uint32)(*(memPtr + 1))) << 16);

  mpe.regs[dest] = data;
}
void Execute_LoadWordBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadWordAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadWordAbsolute(mpe,entry,newNuance);
}
void Execute_LoadWordBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadWordAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadWordAbsolute(mpe,entry,newNuance);
}
void Execute_LoadScalarBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadScalarAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadScalarAbsolute(mpe,entry,newNuance);
}
void Execute_LoadScalarBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadScalarAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadScalarAbsolute(mpe,entry,newNuance);
}
void Execute_LoadShortVectorAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 data[4];
  uint32 dest = nuance.fields[FIELD_MEM_TO];
  uint32 address = nuance.fields[FIELD_MEM_FROM];
  uint8 *ptr = (uint8 *)nuance.fields[FIELD_MEM_POINTER]; 

  data[0] = *((uint32 *)(ptr + 0));
  data[1] = *((uint32 *)(ptr + 2));
  data[2] = *((uint32 *)(ptr + 4));
  data[3] = *((uint32 *)(ptr + 6));
  SwapVectorBytes(data);
  data[0] &= 0xFFFF0000;
  data[1] &= 0xFFFF0000;
  data[2] &= 0xFFFF0000;
  data[3] &= 0xFFFF0000;

  mpe.regs[dest] = data[0];
  mpe.regs[dest + 1] = data[1];
  mpe.regs[dest + 2] = data[2];
  mpe.regs[dest + 3] = data[3];
}
void Execute_LoadShortVectorLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 data[4];
  uint32 dest = nuance.fields[FIELD_MEM_TO];
  uint8 *ptr = (uint8 *)(nuonEnv->GetPointerToMemory(&mpe,entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]] & 0xFFFFFFF8));
  
  data[0] = *((uint32 *)(ptr + 0));
  data[1] = *((uint32 *)(ptr + 2));
  data[2] = *((uint32 *)(ptr + 4));
  data[3] = *((uint32 *)(ptr + 6));
  SwapVectorBytes(data);
  data[0] &= 0xFFFF0000;
  data[1] &= 0xFFFF0000;
  data[2] &= 0xFFFF0000;
  data[3] &= 0xFFFF0000;

  mpe.regs[dest] = data[0];
  mpe.regs[dest + 1] = data[1];
  mpe.regs[dest + 2] = data[2];
  mpe.regs[dest + 3] = data[3];
}
void Execute_LoadShortVectorBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadShortVectorAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadShortVectorAbsolute(mpe,entry,newNuance);
}
void Execute_LoadShortVectorBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadShortVectorAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(mpe.GetPointerToMemoryBank(address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadShortVectorAbsolute(mpe,entry,newNuance);
}
void Execute_LoadVectorLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 address, offset;
  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  uint32 *destPtr = &mpe.regs[nuance.fields[FIELD_MEM_TO]];
  uint32 *srcPtr = (uint32 *)(nuonEnv->GetPointerToMemory(&mpe,address & 0xFFFFFFF0));

  if((address < MPE_CTRL_BASE) || (address >= MPE_RESV_BASE))
  {
    destPtr[0] = srcPtr[0];
    destPtr[1] = srcPtr[1];
    destPtr[2] = srcPtr[2];
    destPtr[3] = srcPtr[3];
    SwapVectorBytes(destPtr);
  }
  else
  {
    destPtr[0] = mpe.ReadControlRegister(address - MPE_CTRL_BASE, &entry);
    destPtr[1] = mpe.ReadControlRegister(address + 4 - MPE_CTRL_BASE, &entry);
    destPtr[2] = mpe.ReadControlRegister(address + 8 - MPE_CTRL_BASE, &entry);
    destPtr[3] = mpe.ReadControlRegister(address + 12 - MPE_CTRL_BASE, &entry);
  }
}
void Execute_LoadVectorBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadVectorAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadVectorAbsolute(mpe,entry,newNuance);
}
void Execute_LoadVectorBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_HANDLER] = (uint32)Execute_LoadVectorAbsolute;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  Execute_LoadVectorAbsolute(mpe,entry,newNuance);
}
void Execute_LoadPixelLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_LoadPixelAbsolute(mpe,entry,newNuance);
}
void Execute_LoadPixelBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;
  int32 pixwidth = BilinearInfo_PixelWidth(pixel_type_width,*entry.pUvctl);

  pixwidth = pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL];
  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  if(pixwidth >= 0)
  {
    address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixwidth);
  }
  else
  {
    //type1: 4-bit pixels
    address = (mpe.uvbase & 0xFFFFFFFC) + (address >> 1);
  }
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_LoadPixelAbsolute(mpe,entry,newNuance);
}
void Execute_LoadPixelBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;
  int32 pixwidth = BilinearInfo_PixelWidth(pixel_type_width,*entry.pXyctl);

  pixwidth = pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL];
  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  if(pixwidth >= 0)
  {
    address = (mpe.xybase & 0xFFFFFFFC) + (address << pixwidth);
  }
  else
  {
    //type1: 4-bit pixels
    address = (mpe.xybase & 0xFFFFFFFC) + (address >> 1);
  }

  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_LoadPixelAbsolute(mpe,entry,newNuance);
}
void Execute_LoadPixelZLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_LoadPixelZAbsolute(mpe,entry,newNuance);
}
void Execute_LoadPixelZBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << BilinearInfo_PixelWidth(pixel_type_width,*entry.pUvctl));
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_LoadPixelZAbsolute(mpe,entry,newNuance);
}
void Execute_LoadPixelZBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << BilinearInfo_PixelWidth(pixel_type_width,*entry.pXyctl));
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_TO] = nuance.fields[FIELD_MEM_TO];
  newNuance.fields[FIELD_MEM_FROM] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_LoadPixelZAbsolute(mpe,entry,newNuance);
}
void Execute_StoreScalarImmediate(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 *destPtr = (uint32 *)nuance.fields[FIELD_MEM_POINTER];
  *destPtr = nuance.fields[FIELD_MEM_FROM];
  SwapScalarBytes(destPtr);
}
void Execute_StoreScalarAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 *destPtr = (uint32 *)nuance.fields[FIELD_MEM_POINTER];
  *destPtr = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  SwapScalarBytes(destPtr);
}
void Execute_StoreScalarControlRegisterImmediate(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 reg = nuance.fields[FIELD_MEM_TO];

  //normal control register write
  mpe.WriteControlRegister(reg - MPE_CTRL_BASE, nuance.fields[FIELD_MEM_FROM]);
}

void Execute_StoreScalarLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 address, offset, *destPtr;
  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_TO]] & 0xFFFFFFFC;

  if((address < MPE_ITAGS_BASE) || (address >= MPE_RESV_BASE))
  {
    destPtr = (uint32 *)(nuonEnv->GetPointerToMemory(&mpe,address));
    *destPtr = entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
    SwapScalarBytes(destPtr);
  }
  else
  {
    if((address & 0xFFFF0000) == MPE_ITAGS_BASE)
    {
      mpe.bInvalidateInstructionCaches = true;
    }
    else
    {
      mpe.WriteControlRegister(address - MPE_CTRL_BASE, entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]]);
    }
  }
}
void Execute_StoreScalarBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StoreScalarAbsolute(mpe,entry,newNuance);
}
void Execute_StoreScalarBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StoreScalarAbsolute(mpe,entry,newNuance);
}

void Execute_StoreScalarControlRegisterAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 reg = nuance.fields[FIELD_MEM_TO];

  if(reg != 0x20500FF0)
  {
    mpe.WriteControlRegister(reg - MPE_CTRL_BASE, entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]]);
  }
  else
  {
    //syscall
  }
}
void Execute_StoreVectorControlRegisterAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 address = nuance.fields[FIELD_MEM_TO];
  uint32 *srcPtr = &(entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]]);

  mpe.WriteControlRegister(address - MPE_CTRL_BASE, srcPtr[0]);
  mpe.WriteControlRegister(address + 4 - MPE_CTRL_BASE, srcPtr[1]);
  mpe.WriteControlRegister(address + 8 - MPE_CTRL_BASE, srcPtr[2]);
  mpe.WriteControlRegister(address + 12 - MPE_CTRL_BASE, srcPtr[3]);
}

void StorePixelAbsolute(void)
{
  uint32 pixelData32, y32, cr32, cb32, z32;
  uint16 pixelData16, y16, cr16, cb16, z16;
  uint32 control = bilinearAddressInfo.control;
  uint32 *regs = bilinearAddressInfo.pRegs;
  uint32 pixType;
  bool bChnorm;

  void *memPtr;

  memPtr = (void *)bilinearAddressInfo.pPixelData;

  pixType = BilinearInfo_XYType(control);
  bChnorm = BilinearInfo_XYChnorm(control);

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      return;
    case 0x2:
    case 0x5:
    {
      //16 bit
      y32 = regs[0] >> 16;
      cr32 = regs[1] >> 16;
      cb32 = regs[2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData16 = ((y32 & 0xFC) << (15-7)) | ((cr32 & 0xF8) << (9-7)) | ((cb32 & 0xF8) >> 3);
      SwapWordBytes(&pixelData16);
      *((uint16 *)memPtr) = pixelData16;
      return;
    }
    case 0x3:
      //8 bit
      return;
    case 0x4:
    case 0x6:
    {
      //32 bit

      pixelData32 = *((uint32 *)memPtr);

      SwapScalarBytes(&pixelData32);

      y32 = regs[0] >> 16;
      cr32 = regs[1] >> 16;
      cb32 = regs[2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData32 = (y32 << 24) | (cr32 << 16) | (cb32 << 8) | (pixelData32 & 0xFF);
      SwapScalarBytes(&pixelData32);
      *((uint32 *)memPtr) = pixelData32;
    }
  }
}

void Execute_StorePixelAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 src = nuance.fields[FIELD_MEM_FROM];
  uint32 address = nuance.fields[FIELD_MEM_TO];
  uint32 pixType;
  uint32 pixelData32, y32, cr32, cb32, z32;
  uint16 pixelData16, y16, cr6, cb16, z16;
  bool bChnorm;

  void *memPtr;

  memPtr = (void *)(nuonEnv->GetPointerToMemory(&mpe,address));

  if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_UV)
  {
    pixType = BilinearInfo_XYType(*entry.pUvctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pUvctl);
  }
  else if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_XY)
  {
    pixType = BilinearInfo_XYType(*entry.pXyctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pXyctl);
  }
  else
  {
    pixType = BilinearInfo_XYType(mpe.linpixctl);
    bChnorm = BilinearInfo_XYChnorm(mpe.linpixctl);
  }

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      return;
    case 0x2:
    case 0x5:
    {
      //16 bit
      y32 = entry.pScalarRegs[src] >> 16;
      cr32 = entry.pScalarRegs[src+1] >> 16;
      cb32 = entry.pScalarRegs[src+2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData16 = ((y32 & 0xFC) << (15-7)) | ((cr32 & 0xF8) << (9-7)) | ((cb32 & 0xF8) >> 3);
      SwapWordBytes(&pixelData16);
      *((uint16 *)memPtr) = pixelData16;
      return;
    }
    case 0x3:
      //8 bit
      return;
    case 0x4:
    case 0x6:
    {
      //32 bit

      pixelData32 = *((uint32 *)memPtr);

      SwapScalarBytes(&pixelData32);

      y32 = entry.pScalarRegs[src] >> 16;
      cr32 = entry.pScalarRegs[src+1] >> 16;
      cb32 = entry.pScalarRegs[src+2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData32 = (y32 << 24) | (cr32 << 16) | (cb32 << 8) | (pixelData32 & 0xFF);
      SwapScalarBytes(&pixelData32);
      *((uint32 *)memPtr) = pixelData32;
    }
  }
}

void StorePixelZAbsolute(void)
{
  uint32 pixType;
  uint32 pixelData32, y32, cr32, cb32, z32;
  uint16 pixelData16, y16, cr16, cb16, z16;
  uint32 control = bilinearAddressInfo.control;
  uint32 *regs = bilinearAddressInfo.pRegs;
  bool bChnorm;

  void *memPtr;

  memPtr = (void *)bilinearAddressInfo.pPixelData;

  pixType = BilinearInfo_XYType(control);
  bChnorm = BilinearInfo_XYChnorm(control);

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      return;
    case 0x2:
    {
      //16 bit
      y32 = regs[0] >> 16;
      cr32 = regs[1] >> 16;
      cb32 = regs[2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData16 = ((y32 & 0xFC) << (15-7)) | ((cr32 & 0xF8) << (9-7)) | ((cb32 & 0xF8) >> 3);
      SwapWordBytes(&pixelData16);
      *((uint16 *)memPtr) = pixelData16;
      return;
    }
    case 0x5:
    {
      //16 bit
      y32 = regs[0] >> 16;
      cr32 = regs[1] >> 16;
      cb32 = regs[2] >> 16;
      z16 = regs[3] >> 16;
      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData16 = ((y32 & 0xFC) << (15-7)) | ((cr32 & 0xF8) << (9-7)) | ((cb32 & 0xF8) >> 3);
      SwapWordBytes(&pixelData16);
      SwapWordBytes(&z16);
      *((uint16 *)memPtr) = pixelData16;
      *((uint16 *)memPtr + 1) = z16;
      return;
    }
    case 0x3:
      //8 bit
      return;
    case 0x4:
    {
      //32 bit

      pixelData32 = *((uint32 *)memPtr);

      SwapScalarBytes(&pixelData32);

      y32 = regs[0] >> 16;
      cr32 = regs[1] >> 16;
      cb32 = regs[2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData32 = (y32 << 24) | (cr32 << 16) | (cb32 << 8) | (regs[3] >> 24);
      SwapScalarBytes(&pixelData32);
      *((uint32 *)memPtr) = pixelData32;
      return;
    }
    case 0x6:
    {
      //32+32Z

      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);

      y32 = regs[0] >> 16;
      cr32 = regs[1] >> 16;
      cb32 = regs[2] >> 16;
      //z32 = entry.pScalarRegs[src+3];

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData32 = (y32 << 24) | (cr32 << 16) | (cb32 << 8);
      SwapScalarBytes(&pixelData32);
      SwapScalarBytes(&z32);
      *((uint32 *)memPtr) = pixelData32;
      //*(((uint32 *)memPtr) + 1) = z32;
      return;
    }
  }
}

void Execute_StorePixelZAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 pixType;
  uint32 src = nuance.fields[FIELD_MEM_FROM];
  uint32 address = nuance.fields[FIELD_MEM_TO];
  uint32 pixelData32, y32, cr32, cb32, z32;
  uint16 pixelData16, y16, cr6, cb16, z16;
  uint32 *srcPtr;
  uint16 *destPtr;
  bool bChnorm;

  srcPtr = &((entry.pScalarRegs)[src]);
  destPtr = (uint16 *)nuance.fields[FIELD_MEM_POINTER];

  void *memPtr;

  memPtr = (void *)(nuonEnv->GetPointerToMemory(&mpe,address));

  if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_UV)
  {
    pixType = BilinearInfo_XYType(*entry.pUvctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pUvctl);
  }
  else if(nuance.fields[FIELD_MEM_INFO] & MEM_INFO_BILINEAR_XY)
  {
    pixType = BilinearInfo_XYType(*entry.pXyctl);
    bChnorm = BilinearInfo_XYChnorm(*entry.pXyctl);
  }
  else
  {
    pixType = BilinearInfo_XYType(mpe.linpixctl);
    bChnorm = BilinearInfo_XYChnorm(mpe.linpixctl);
  }

  switch(pixType)
  {
    case 0x0:
      //MPEG
      return;
    case 0x1:
      //4 bit
      return;
    case 0x2:
    {
      //16 bit
      y32 = entry.pScalarRegs[src] >> 16;
      cr32 = entry.pScalarRegs[src+1] >> 16;
      cb32 = entry.pScalarRegs[src+2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData16 = ((y32 & 0xFC) << (15-7)) | ((cr32 & 0xF8) << (9-7)) | ((cb32 & 0xF8) >> 3);
      SwapWordBytes(&pixelData16);
      *((uint16 *)memPtr) = pixelData16;
      return;
    }
    case 0x5:
    {
      //16 bit
      y32 = entry.pScalarRegs[src] >> 16;
      cr32 = entry.pScalarRegs[src+1] >> 16;
      cb32 = entry.pScalarRegs[src+2] >> 16;
      z16 = entry.pScalarRegs[src+3] >> 16;
      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData16 = ((y32 & 0xFC) << (15-7)) | ((cr32 & 0xF8) << (9-7)) | ((cb32 & 0xF8) >> 3);
      SwapWordBytes(&pixelData16);
      SwapWordBytes(&z16);
      *((uint16 *)memPtr) = pixelData16;
      *((uint16 *)memPtr + 1) = z16;
      return;
    }
    case 0x3:
      //8 bit
      return;
    case 0x4:
    {
      //32 bit

      pixelData32 = *((uint32 *)memPtr);

      SwapScalarBytes(&pixelData32);

      y32 = entry.pScalarRegs[src] >> 16;
      cr32 = entry.pScalarRegs[src+1] >> 16;
      cb32 = entry.pScalarRegs[src+2] >> 16;

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData32 = (y32 << 24) | (cr32 << 16) | (cb32 << 8) | (entry.pScalarRegs[src+3] >> 24);
      SwapScalarBytes(&pixelData32);
      *((uint32 *)memPtr) = pixelData32;
      return;
    }
    case 0x6:
    {
      //32+32Z

      pixelData32 = *((uint32 *)memPtr);
      SwapScalarBytes(&pixelData32);

      y32 = entry.pScalarRegs[src] >> 16;
      cr32 = entry.pScalarRegs[src+1] >> 16;
      cb32 = entry.pScalarRegs[src+2] >> 16;
      //z32 = entry.pScalarRegs[src+3];

      SaturateColorComponents(&y32, &cr32, &cb32, bChnorm);

      pixelData32 = (y32 << 24) | (cr32 << 16) | (cb32 << 8);
      SwapScalarBytes(&pixelData32);
      SwapScalarBytes(&z32);
      *((uint32 *)memPtr) = pixelData32;
      //*(((uint32 *)memPtr) + 1) = z32;
      return;
    }
  }
}

void Execute_StoreShortVectorAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  int32 *srcPtr;
  int16 *destPtr;

  srcPtr = (int32 *)&entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  destPtr = (int16 *)nuance.fields[FIELD_MEM_POINTER];

  destPtr[0] = srcPtr[0] >> 16UL;
  destPtr[1] = srcPtr[1] >> 16UL;
  destPtr[2] = srcPtr[2] >> 16UL;
  destPtr[3] = srcPtr[3] >> 16UL;
  SwapShortVectorBytes((uint16 *)destPtr);
}
void Execute_StoreShortVectorLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 *srcPtr;
  uint16 *destPtr;

  srcPtr = &entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  destPtr = (uint16 *)(nuonEnv->GetPointerToMemory(&mpe,entry.pScalarRegs[nuance.fields[FIELD_MEM_TO]] & 0xFFFFFFF8));

  destPtr[0] = srcPtr[0] >> 16UL;
  destPtr[1] = srcPtr[1] >> 16UL;
  destPtr[2] = srcPtr[2] >> 16UL;
  destPtr[3] = srcPtr[3] >> 16UL;
  SwapShortVectorBytes(destPtr);
}
void Execute_StoreShortVectorBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StoreShortVectorAbsolute(mpe,entry,newNuance);
}
void Execute_StoreShortVectorBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StoreShortVectorAbsolute(mpe,entry,newNuance);
}
void Execute_StoreVectorAbsolute(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 *destPtr = (uint32 *)nuance.fields[FIELD_MEM_POINTER];
  uint32 src = nuance.fields[FIELD_MEM_FROM];
  destPtr[0] = entry.pScalarRegs[src];
  destPtr[1] = entry.pScalarRegs[src+1];
  destPtr[2] = entry.pScalarRegs[src+2];
  destPtr[3] = entry.pScalarRegs[src+3];
  SwapVectorBytes(destPtr);

}
void Execute_StoreVectorLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  uint32 address, offset;
  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_TO]] & 0xFFFFFFF0;
  uint32 *srcPtr = &entry.pScalarRegs[nuance.fields[FIELD_MEM_FROM]];
  uint32 *destPtr = (uint32 *)(nuonEnv->GetPointerToMemory(&mpe,address));

  if((address < MPE_ITAGS_BASE) || (address >= MPE_RESV_BASE))
  {
    destPtr[0] = srcPtr[0];
    destPtr[1] = srcPtr[1];
    destPtr[2] = srcPtr[2];
    destPtr[3] = srcPtr[3];
    SwapVectorBytes(destPtr);
  }
  else
  {
    if((address & 0xFFFF0000) == MPE_ITAGS_BASE)
    {
      mpe.bInvalidateInstructionCaches = true;
    }
    else
    {
      mpe.WriteControlRegister(address - MPE_CTRL_BASE, srcPtr[0]);
      mpe.WriteControlRegister(address + 4 - MPE_CTRL_BASE, srcPtr[1]);
      mpe.WriteControlRegister(address + 8 - MPE_CTRL_BASE, srcPtr[2]);
      mpe.WriteControlRegister(address + 12 - MPE_CTRL_BASE, srcPtr[3]);
    }
  }
}
void Execute_StoreVectorBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pUvctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StoreVectorAbsolute(mpe,entry,newNuance);
}
void Execute_StoreVectorBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << pixel_type_width[(*entry.pXyctl >> 20) & 0x0FUL]);
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StoreVectorAbsolute(mpe,entry,newNuance);
}
void Execute_StorePixelLinear(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_TO]];
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StorePixelAbsolute(mpe,entry,newNuance);
}
void Execute_StorePixelBilinearUV(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << BilinearInfo_PixelWidth(pixel_type_width,*entry.pUvctl));
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StorePixelAbsolute(mpe,entry,newNuance);
}
void Execute_StorePixelBilinearXY(MPE &mpe, InstructionCacheEntry &entry,  Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << BilinearInfo_PixelWidth(pixel_type_width,*entry.pXyctl));
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StorePixelAbsolute(mpe,entry,newNuance);
}
void Execute_StorePixelZLinear(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  address = entry.pScalarRegs[nuance.fields[FIELD_MEM_TO]];
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StorePixelZAbsolute(mpe,entry,newNuance);
}
void Execute_StorePixelZBilinearUV(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pUvctl,entry.pIndexRegs[REG_U],entry.pIndexRegs[REG_V]);
  address = (mpe.uvbase & 0xFFFFFFFC) + (address << BilinearInfo_PixelWidth(pixel_type_width,*entry.pUvctl));
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_UV;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StorePixelZAbsolute(mpe,entry,newNuance);

}
void Execute_StorePixelZBilinearXY(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  Nuance newNuance;
  uint32 address;

  CalculateBilinearAddress(mpe,&address,*entry.pXyctl,entry.pIndexRegs[REG_X],entry.pIndexRegs[REG_Y]);
  address = (mpe.xybase & 0xFFFFFFFC) + (address << BilinearInfo_PixelWidth(pixel_type_width,*entry.pXyctl));
  newNuance.fields[FIELD_MEM_INFO] = MEM_INFO_BILINEAR_XY;
  newNuance.fields[FIELD_MEM_FROM] = nuance.fields[FIELD_MEM_FROM];
  newNuance.fields[FIELD_MEM_TO] = address;
  newNuance.fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(&mpe,address));
  Execute_StorePixelZAbsolute(mpe,entry,newNuance);
}
