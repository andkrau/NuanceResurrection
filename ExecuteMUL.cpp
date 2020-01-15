#include "InstructionCache.h"
#include "mpe.h"

static const uint32 shiftTable[4] = {16, 8, 0, 2};

void Execute_ADDM(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    entry.pScalarRegs[nuance.fields[FIELD_MUL_SRC1]] +
    entry.pScalarRegs[nuance.fields[FIELD_MUL_SRC2]];
}
void Execute_ADDMImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    nuance.fields[FIELD_MUL_SRC1] + 
    entry.pScalarRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_SUBM(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    entry.pScalarRegs[nuance.fields[FIELD_MUL_SRC1]] -
    entry.pScalarRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_SUBMImmediateReverse(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    nuance.fields[FIELD_MUL_SRC1] - 
    entry.pScalarRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_MULScalarShiftAcshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = *(entry.pAcshift) & 0x7FUL;

  int64 result = ((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32);

  if(mul_shift & 0x40)
  {
    //ASL
    result <<= (128 - mul_shift);
  }
  else
  {
    //ASR
    result >>= mul_shift;
  }

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULScalarShiftRightImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) >> nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULScalarShiftLeftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) << nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftAcshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = *(entry.pAcshift) & 0x7FUL;

  int64 result = ((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32);

  if(mul_shift & 0x40)
  {
    //ASL
    result <<= (128 - mul_shift);
  }
  else
  {
    //ASR
    result >>= mul_shift;
  }

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULScalarShiftScalar(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_INFO]] & 0x7FUL;

  int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32));

  if(mul_shift & 0x40)
  {
    //ASL
    result <<= (128 - mul_shift);
  }
  else
  {
    //ASR
    result >>= mul_shift;
  }

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftScalar(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_INFO]] & 0x7FUL;

  int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32));

  if(mul_shift & 0x40)
  {
    //ASL
    result <<= (128 - mul_shift);
  }
  else
  {
    //ASR
    result >>= mul_shift;
  }

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftRightImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) >> nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftLeftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = (entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) << nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000i64) == 0i64)
  {
    if((result & 0xFFFFFFFF00000000i64) != 0i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000i64) != 0xFFFFFFFF00000000i64)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MUL_SVImmediateShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVScalarShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)((entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVScalarShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)((entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[*(entry.pSvshift)];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRuShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  //? scalar = (int32)(entry.pIndexRegs[2] >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL; //!!
  const int32 scalar = (((int32)(entry.pIndexRegs[2])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRuShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[2])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[*(entry.pSvshift)];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRvShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[3])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRvShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[3])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[*(entry.pSvshift)];

//Execute

  mpe.regs[dest] =
    (scalar * ((int32)((entry.pScalarRegs)[src2 + 0]) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * ((int32)((entry.pScalarRegs)[src2 + 1]) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * ((int32)((entry.pScalarRegs)[src2 + 2]) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * ((int32)((entry.pScalarRegs)[src2 + 3]) >> 16)) << shift;
}

void Execute_MUL_SVVectorShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  int32 src1_vector[4];

  src1_vector[0] = ((int32)(entry.pScalarRegs[src1])) >> 16;
  src1_vector[1] = ((int32)(entry.pScalarRegs[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)(entry.pScalarRegs[src1 + 2])) >> 16;
  src1_vector[3] = ((int32)(entry.pScalarRegs[src1 + 3])) >> 16;

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    (src1_vector[0] * (((int32)(entry.pScalarRegs[src2])) >> 16)) << shift;
  mpe.regs[nuance.fields[FIELD_MUL_DEST] + 1] =
    (src1_vector[1] * (((int32)(entry.pScalarRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[nuance.fields[FIELD_MUL_DEST] + 2] =
    (src1_vector[2] * (((int32)(entry.pScalarRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[nuance.fields[FIELD_MUL_DEST] + 3] =
    (src1_vector[3] * (((int32)(entry.pScalarRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVVectorShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 shift = shiftTable[*entry.pSvshift & 0x03UL];
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const uint32 dest = nuance.fields[FIELD_MUL_DEST];
  int32 src1_vector[4];

  src1_vector[0] = ((int32)(entry.pScalarRegs[src1])) >> 16;
  src1_vector[1] = ((int32)(entry.pScalarRegs[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)(entry.pScalarRegs[src1 + 2])) >> 16;
  src1_vector[3] = ((int32)(entry.pScalarRegs[src1 + 3])) >> 16;

  mpe.regs[dest] =
    (src1_vector[0] * (((int32)(entry.pScalarRegs[src2])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (src1_vector[1] * (((int32)(entry.pScalarRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (src1_vector[2] * (((int32)(entry.pScalarRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (src1_vector[3] * (((int32)(entry.pScalarRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_PImmediateShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PScalarShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)((entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PScalarShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)((entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[*(entry.pSvshift)];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRuShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[2])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRuShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[2])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[*(entry.pSvshift)];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRvShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[3])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  mpe.regs[dest] =
    (scalar * (((int32)(entry.pScalarRegs[src2])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(entry.pScalarRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(entry.pScalarRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRvShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = (((int32)(entry.pIndexRegs[3])) >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[*(entry.pSvshift)];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(entry.pScalarRegs[src2])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(entry.pScalarRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(entry.pScalarRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PVectorShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const uint32 dest = nuance.fields[FIELD_MUL_DEST];
  const uint32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];
  int32 src1_vector[3];

  src1_vector[0] = ((int32)((entry.pScalarRegs)[src1])) >> 16;
  src1_vector[1] = ((int32)((entry.pScalarRegs)[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)((entry.pScalarRegs)[src1 + 2])) >> 16;

//Execute

  mpe.regs[dest] =
    (src1_vector[0] * (((int32)(entry.pScalarRegs[src2])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (src1_vector[1] * (((int32)(entry.pScalarRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (src1_vector[2] * (((int32)(entry.pScalarRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PVectorShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const uint32 dest = nuance.fields[FIELD_MUL_DEST];
  const uint32 shift = shiftTable[*(entry.pSvshift)];;
  int32 src1_vector[3];

  src1_vector[0] = ((int32)((entry.pScalarRegs)[src1])) >> 16;
  src1_vector[1] = ((int32)((entry.pScalarRegs)[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)((entry.pScalarRegs)[src1 + 2])) >> 16;

//Execute

  mpe.regs[dest] =
    (src1_vector[0] * (((int32)(entry.pScalarRegs[src2])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (src1_vector[1] * (((int32)(entry.pScalarRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (src1_vector[2] * (((int32)(entry.pScalarRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_DOTPScalarShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)((entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  int32 products[4];

  products[0] =
    (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16) * scalar;
  products[1] =
    (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16) * scalar;
  products[2] =
    (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16) * scalar;
  products[3] =
    (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16) * scalar;

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    (products[0] + products[1] + products[2] + products[3]) << shiftTable[nuance.fields[FIELD_MUL_INFO]];
}

void Execute_DOTPScalarShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const int32 scalar = ((int32)((entry.pScalarRegs)[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  int32 products[4];

  products[0] =
    (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16) * scalar;
  products[1] =
    (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16) * scalar;
  products[2] =
    (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16) * scalar;
  products[3] =
    (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16) * scalar;

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    (products[0] + products[1] + products[2] + products[3]) << shiftTable[*(entry.pSvshift)];
}

void Execute_DOTPVectorShiftImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const uint32 shiftVal = nuance.fields[FIELD_MUL_INFO];
  const int32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  int32 src1_vector[4];
  int32 products[4];

  src1_vector[0] = ((int32)((entry.pScalarRegs)[src1 + 0])) >> 16;
  src1_vector[1] = ((int32)((entry.pScalarRegs)[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)((entry.pScalarRegs)[src1 + 2])) >> 16;
  src1_vector[3] = ((int32)((entry.pScalarRegs)[src1 + 3])) >> 16;

  products[0] =
    (src1_vector[0] * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16));
  products[1] =
    (src1_vector[1] * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16));
  products[2] =
    (src1_vector[2] * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16));
  products[3] =
    (src1_vector[3] * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16));

  mpe.regs[dest] = (products[0] + products[1] + products[2] + products[3]) << shiftTable[shiftVal];
}

void Execute_DOTPVectorShiftSvshift(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  const uint32 shiftVal = *(entry.pSvshift);
  const int32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  int32 src1_vector[4];
  int32 products[4];

  src1_vector[0] = ((int32)((entry.pScalarRegs)[src1 + 0])) >> 16;
  src1_vector[1] = ((int32)((entry.pScalarRegs)[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)((entry.pScalarRegs)[src1 + 2])) >> 16;
  src1_vector[3] = ((int32)((entry.pScalarRegs)[src1 + 3])) >> 16;

  products[0] =
    (src1_vector[0] * (((int32)((entry.pScalarRegs)[src2 + 0])) >> 16));
  products[1] =
    (src1_vector[1] * (((int32)((entry.pScalarRegs)[src2 + 1])) >> 16));
  products[2] =
    (src1_vector[2] * (((int32)((entry.pScalarRegs)[src2 + 2])) >> 16));
  products[3] =
    (src1_vector[3] * (((int32)((entry.pScalarRegs)[src2 + 3])) >> 16));

  mpe.regs[dest] = (products[0] + products[1] + products[2] + products[3]) << shiftTable[shiftVal];
}
