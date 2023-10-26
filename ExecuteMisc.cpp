#include "basetypes.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "SuperBlockConstants.h"

void Execute_CheckECUSkipCounter(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  if(mpe.ecuSkipCounter)
  {
    mpe.ecuSkipCounter--;
    if(!mpe.ecuSkipCounter)
    {
      mpe.bInterpretedBranchTaken = true;
    }
  }
}

void Execute_SaveFlags(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.tempCC = mpe.cc;
  mpe.nuances_use_tempreg_union = false;
}

void Execute_SaveRegs(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  memcpy(mpe.tempreg_union,mpe.reg_union,sizeof(uint32)*48);
  mpe.nuances_use_tempreg_union = true;
}

void Execute_StoreScalarRegisterConstant(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_CONSTANT_ADDRESS]] = nuance.fields[FIELD_CONSTANT_VALUE];
  const uint32 flagMask = nuance.fields[FIELD_CONSTANT_FLAGMASK];

  if(flagMask)
  {
    mpe.cc &= (~flagMask);
    mpe.cc |= nuance.fields[FIELD_CONSTANT_FLAGVALUES];
  }
}

void Execute_StoreMiscRegisterConstant(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 miscRegIndex = nuance.fields[FIELD_CONSTANT_ADDRESS];
  const uint32 flagMask = nuance.fields[FIELD_CONSTANT_FLAGMASK];

  if(miscRegIndex != CONSTANT_REG_DISCARD)
  {
    const uint32 c = nuance.fields[FIELD_CONSTANT_VALUE];
    switch(miscRegIndex)
    {
      case CONSTANT_REG_RX:
        mpe.rx = c;
        break;
      case CONSTANT_REG_RY:
        mpe.ry = c;
        break;
      case CONSTANT_REG_RU:
        mpe.ru = c;
        break;
      case CONSTANT_REG_RV:
        mpe.rv = c;
        break;
      case CONSTANT_REG_RC0:
        mpe.rc0 = c;
        break;
      case CONSTANT_REG_RC1:
        mpe.rc1 = c;
        break;
      case CONSTANT_REG_RZ:
        mpe.rz = c;
        break;
      case CONSTANT_REG_XYCTL:
        mpe.xyctl = c;
        break;
      case CONSTANT_REG_UVCTL:
        mpe.uvctl = c;
        break;
      case CONSTANT_REG_XYRANGE:
        mpe.xyrange = c;
        break;
      case CONSTANT_REG_UVRANGE:
        mpe.uvrange = c;
        break;
      case CONSTANT_REG_ACSHIFT:
        mpe.acshift = c;
        break;
      case CONSTANT_REG_SVSHIFT:
        mpe.svshift = c;
        break;
    }
  }

  if(flagMask)
  {
    mpe.cc &= (~flagMask);
    mpe.cc |= nuance.fields[FIELD_CONSTANT_FLAGVALUES];
  }
}
