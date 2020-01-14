#include "InstructionCache.h"
#include "mpe.h"
#include "SuperBlockConstants.h"

void Execute_CheckECUSkipCounter(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
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

void Execute_SaveFlags(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  mpe.tempCC = mpe.cc;
  mpe.pICacheEntry = &(mpe.ICacheEntry_SaveFlags);
}

void Execute_SaveRegs(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  uint32 i;

  mpe.tempCC = mpe.cc;
  for(i = 0; i < 32; i++)
  {
    mpe.tempScalarRegs[i] = mpe.regs[i];
  }

  mpe.tempRx = mpe.rx;
  mpe.tempRy = mpe.ry;
  mpe.tempRu = mpe.ru;
  mpe.tempRv = mpe.rv;
  mpe.tempRc0 = mpe.rc0;
  mpe.tempRc1 = mpe.rc1;
  mpe.tempRz = mpe.rz;
  mpe.tempRzi1 = mpe.rzi1;
  mpe.tempRzi2 = mpe.rzi2;
  mpe.tempXyctl = mpe.xyctl;
  mpe.tempUvctl = mpe.uvctl;
  mpe.tempXyrange = mpe.xyrange;
  mpe.tempUvrange = mpe.uvrange;
  mpe.tempAcshift = mpe.acshift;
  mpe.tempSvshift = mpe.svshift;

  mpe.pICacheEntry = &(mpe.ICacheEntry_SaveRegs);
}

void Execute_StoreScalarRegisterConstant(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  mpe.regs[nuance.fields[FIELD_CONSTANT_ADDRESS]] = nuance.fields[FIELD_CONSTANT_VALUE];
  uint32 flagMask = nuance.fields[FIELD_CONSTANT_FLAGMASK];

  if(flagMask)
  {
    mpe.cc &= (~flagMask);
    mpe.cc |= nuance.fields[FIELD_CONSTANT_FLAGVALUES];
  }
}

void Execute_StoreMiscRegisterConstant(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  uint32 miscRegIndex = nuance.fields[FIELD_CONSTANT_ADDRESS];
  uint32 flagMask = nuance.fields[FIELD_CONSTANT_FLAGMASK];

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