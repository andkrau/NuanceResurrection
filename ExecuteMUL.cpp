#include "InstructionCache.h"
#include "mpe.h"

static constexpr uint32 shiftTable[4] = {16, 8, 0, 2};

void Execute_ADDM(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    pRegs[nuance.fields[FIELD_MUL_SRC1]] +
    pRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_ADDMImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    nuance.fields[FIELD_MUL_SRC1] + 
    pRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_SUBM(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    pRegs[nuance.fields[FIELD_MUL_SRC1]] -
    pRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_SUBMImmediateReverse(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    nuance.fields[FIELD_MUL_SRC1] - 
    pRegs[nuance.fields[FIELD_MUL_SRC2]];
}

void Execute_MULScalarShiftAcshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = pRegs[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = pRegs[ACS_REG] & 0x7FUL;

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
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = pRegs[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) >> nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = pRegs[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) << nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftAcshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = pRegs[ACS_REG] & 0x7FUL;

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
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULScalarShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = pRegs[nuance.fields[FIELD_MUL_SRC1]];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = pRegs[nuance.fields[FIELD_MUL_INFO]] & 0x7FUL;

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
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const uint32 mul_shift = pRegs[nuance.fields[FIELD_MUL_INFO]] & 0x7FUL;

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
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) >> nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MULImmediateShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int64 mulop1 = nuance.fields[FIELD_MUL_SRC1];
  const int64 mulop2 = pRegs[nuance.fields[FIELD_MUL_SRC2]];
  const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) << nuance.fields[FIELD_MUL_INFO];

  mpe.cc &= (~CC_MUL_OVERFLOW);
  if((result & 0x0000000080000000LL) == 0LL)
  {
    if((result & 0xFFFFFFFF00000000LL) != 0LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }
  else
  {
    if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
    {
      mpe.cc |= CC_MUL_OVERFLOW;
    }
  }

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] = (uint32)result;
}

void Execute_MUL_SVImmediateShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVScalarShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(pRegs[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVScalarShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(pRegs[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[pRegs[SVS_REG]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRuShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  //? scalar = (int32)(pRegs[INDEX_REG+REG_U] >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL; //!!
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_U])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRuShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_U])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[pRegs[SVS_REG]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRvShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_V])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVRvShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_V])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[pRegs[SVS_REG]];

//Execute

  mpe.regs[dest] =
    (scalar * ((int32)(pRegs[src2 + 0]) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * ((int32)(pRegs[src2 + 1]) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * ((int32)(pRegs[src2 + 2]) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (scalar * ((int32)(pRegs[src2 + 3]) >> 16)) << shift;
}

void Execute_MUL_SVVectorShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 src1_vector[4] = {
   ((int32)(pRegs[src1    ])) >> 16,
   ((int32)(pRegs[src1 + 1])) >> 16,
   ((int32)(pRegs[src1 + 2])) >> 16,
   ((int32)(pRegs[src1 + 3])) >> 16};

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    (src1_vector[0] * (((int32)(pRegs[src2    ])) >> 16)) << shift;
  mpe.regs[nuance.fields[FIELD_MUL_DEST] + 1] =
    (src1_vector[1] * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[nuance.fields[FIELD_MUL_DEST] + 2] =
    (src1_vector[2] * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[nuance.fields[FIELD_MUL_DEST] + 3] =
    (src1_vector[3] * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_SVVectorShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 shift = shiftTable[pRegs[SVS_REG] & 0x03UL];
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const uint32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 src1_vector[4] = {
   ((int32)(pRegs[src1    ])) >> 16,
   ((int32)(pRegs[src1 + 1])) >> 16,
   ((int32)(pRegs[src1 + 2])) >> 16,
   ((int32)(pRegs[src1 + 3])) >> 16};

  mpe.regs[dest] =
    (src1_vector[0] * (((int32)(pRegs[src2    ])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (src1_vector[1] * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (src1_vector[2] * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
  mpe.regs[dest + 3] =
    (src1_vector[3] * (((int32)(pRegs[src2 + 3])) >> 16)) << shift;
}

void Execute_MUL_PImmediateShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PScalarShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(pRegs[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PScalarShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(pRegs[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[pRegs[SVS_REG]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRuShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_U])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRuShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_U])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[pRegs[SVS_REG]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2 + 0])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRvShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_V])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2    ])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PRvShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = (((int32)(pRegs[INDEX_REG+REG_V])) >> (2 + BilinearInfo_XYMipmap(pRegs[UVC_REG]))) & 0x3FFFUL;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];
  const int32 shift = shiftTable[pRegs[SVS_REG]];

//Execute

  mpe.regs[dest] =
    (scalar * (((int32)(pRegs[src2    ])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (scalar * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (scalar * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PVectorShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const uint32 dest = nuance.fields[FIELD_MUL_DEST];
  const uint32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];
  int32 src1_vector[3];

  src1_vector[0] = ((int32)(pRegs[src1    ])) >> 16;
  src1_vector[1] = ((int32)(pRegs[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)(pRegs[src1 + 2])) >> 16;

//Execute

  mpe.regs[dest] =
    (src1_vector[0] * (((int32)(pRegs[src2    ])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (src1_vector[1] * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (src1_vector[2] * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_MUL_PVectorShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const uint32 dest = nuance.fields[FIELD_MUL_DEST];
  const uint32 shift = shiftTable[pRegs[SVS_REG]];
  int32 src1_vector[3];

  src1_vector[0] = ((int32)(pRegs[src1    ])) >> 16;
  src1_vector[1] = ((int32)(pRegs[src1 + 1])) >> 16;
  src1_vector[2] = ((int32)(pRegs[src1 + 2])) >> 16;

//Execute

  mpe.regs[dest] =
    (src1_vector[0] * (((int32)(pRegs[src2    ])) >> 16)) << shift;
  mpe.regs[dest + 1] =
    (src1_vector[1] * (((int32)(pRegs[src2 + 1])) >> 16)) << shift;
  mpe.regs[dest + 2] =
    (src1_vector[2] * (((int32)(pRegs[src2 + 2])) >> 16)) << shift;
}

void Execute_DOTPScalarShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(pRegs[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 products[4] = {
   (((int32)(pRegs[src2 + 0])) >> 16) * scalar,
   (((int32)(pRegs[src2 + 1])) >> 16) * scalar,
   (((int32)(pRegs[src2 + 2])) >> 16) * scalar,
   (((int32)(pRegs[src2 + 3])) >> 16) * scalar};

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    (products[0] + products[1] + products[2] + products[3]) << shiftTable[nuance.fields[FIELD_MUL_INFO]];
}

void Execute_DOTPScalarShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const int32 scalar = ((int32)(pRegs[nuance.fields[FIELD_MUL_SRC1]])) >> 16;
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 products[4] = {
   (((int32)(pRegs[src2 + 0])) >> 16) * scalar,
   (((int32)(pRegs[src2 + 1])) >> 16) * scalar,
   (((int32)(pRegs[src2 + 2])) >> 16) * scalar,
   (((int32)(pRegs[src2 + 3])) >> 16) * scalar};

  mpe.regs[nuance.fields[FIELD_MUL_DEST]] =
    (products[0] + products[1] + products[2] + products[3]) << shiftTable[pRegs[SVS_REG]];
}

void Execute_DOTPVectorShiftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 shiftVal = nuance.fields[FIELD_MUL_INFO];
  const int32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];

  const int32 src1_vector[4] = {
    ((int32)(pRegs[src1 + 0])) >> 16,
    ((int32)(pRegs[src1 + 1])) >> 16,
    ((int32)(pRegs[src1 + 2])) >> 16,
    ((int32)(pRegs[src1 + 3])) >> 16};

  const int32 products[4] = {
    (src1_vector[0] * (((int32)(pRegs[src2 + 0])) >> 16)),
    (src1_vector[1] * (((int32)(pRegs[src2 + 1])) >> 16)),
    (src1_vector[2] * (((int32)(pRegs[src2 + 2])) >> 16)),
    (src1_vector[3] * (((int32)(pRegs[src2 + 3])) >> 16))};

  mpe.regs[dest] = (products[0] + products[1] + products[2] + products[3]) << shiftTable[shiftVal];
}

void Execute_DOTPVectorShiftSvshift(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 shiftVal = pRegs[SVS_REG];
  const int32 src1 = nuance.fields[FIELD_MUL_SRC1];
  const int32 src2 = nuance.fields[FIELD_MUL_SRC2];
  const int32 dest = nuance.fields[FIELD_MUL_DEST];

  const int32 src1_vector[4] = {
    ((int32)(pRegs[src1 + 0])) >> 16,
    ((int32)(pRegs[src1 + 1])) >> 16,
    ((int32)(pRegs[src1 + 2])) >> 16,
    ((int32)(pRegs[src1 + 3])) >> 16};

  const int32 products[4] = {
    (src1_vector[0] * (((int32)(pRegs[src2 + 0])) >> 16)),
    (src1_vector[1] * (((int32)(pRegs[src2 + 1])) >> 16)),
    (src1_vector[2] * (((int32)(pRegs[src2 + 2])) >> 16)),
    (src1_vector[3] * (((int32)(pRegs[src2 + 3])) >> 16))};

  mpe.regs[dest] = (products[0] + products[1] + products[2] + products[3]) << shiftTable[shiftVal];
}
