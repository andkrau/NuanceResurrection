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
    switch(miscRegIndex)
    {
      case CONSTANT_REG_RX:
        mpe.rx = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_RY:
        mpe.ry = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_RU:
        mpe.ru = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_RV:
        mpe.rv = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_RC0:
        mpe.rc0 = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_RC1:
        mpe.rc1 = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_RZ:
        mpe.rz = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_XYCTL:
        mpe.xyctl = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_UVCTL:
        mpe.uvctl = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_XYRANGE:
        mpe.xyrange = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_UVRANGE:
        mpe.uvrange = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_ACSHIFT:
        mpe.acshift = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
      case CONSTANT_REG_SVSHIFT:
        mpe.svshift = nuance.fields[FIELD_CONSTANT_VALUE];
        break;
    }
  }
  
  if(flagMask)
  {
    mpe.cc &= (~flagMask);
    mpe.cc |= nuance.fields[FIELD_CONSTANT_FLAGVALUES];
  }
}
