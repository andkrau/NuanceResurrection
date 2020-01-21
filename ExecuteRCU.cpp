#include <assert.h>
#include "InstructionCache.h"
#include "mpe.h"

void Execute_DECRc0(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.cc |= CC_COUNTER0_ZERO;
  
  if(mpe.rc0)
  {
    mpe.rc0--;
    if(mpe.rc0)
    {
      mpe.cc &= ~CC_COUNTER0_ZERO;
    }
  }
}

void Execute_DECRc1(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.cc |= CC_COUNTER1_ZERO;
  
  if(mpe.rc1)
  {
    mpe.rc1--;
    if(mpe.rc1)
    {
      mpe.cc &= ~CC_COUNTER1_ZERO;
    }
  }
}

void Execute_DECBoth(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.cc |= (CC_COUNTER1_ZERO | CC_COUNTER0_ZERO);

  if(mpe.rc0)
  {
    mpe.rc0--;

    if(mpe.rc0)
    {
      mpe.cc &= ~CC_COUNTER0_ZERO;
    }
  }

  if(mpe.rc1)
  {
    mpe.rc1--;

    if(mpe.rc1)
    {
      mpe.cc &= ~CC_COUNTER1_ZERO;
    }
  }
}

void Execute_ADDRImmediateOnly(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = entry.pIndexRegs[nuance.fields[FIELD_RCU_DEST]] + nuance.fields[FIELD_RCU_SRC];
}

void Execute_ADDRImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = entry.pIndexRegs[nuance.fields[FIELD_RCU_DEST]] + nuance.fields[FIELD_RCU_SRC];

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    mpe.cc |= CC_COUNTER0_ZERO;

    if(mpe.rc0)
    {
      mpe.rc0--;
      if(mpe.rc0)
      {
        mpe.cc &= ~CC_COUNTER0_ZERO;
      }
    }
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    mpe.cc |= CC_COUNTER1_ZERO;

    if(mpe.rc1)
    {
      mpe.rc1--;
      if(mpe.rc1)
      {
        mpe.cc &= ~CC_COUNTER1_ZERO;
      }
    }
  }
}

void Execute_ADDRScalarOnly(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = entry.pIndexRegs[nuance.fields[FIELD_RCU_DEST]] + entry.pScalarRegs[nuance.fields[FIELD_RCU_SRC]];
}

void Execute_ADDRScalar(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = entry.pIndexRegs[nuance.fields[FIELD_RCU_DEST]] + entry.pScalarRegs[nuance.fields[FIELD_RCU_SRC]];
  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    mpe.cc |= CC_COUNTER0_ZERO;

    if(mpe.rc0)
    {
      mpe.rc0--;
      if(mpe.rc0)
      {
        mpe.cc &= ~CC_COUNTER0_ZERO;
      }
    }
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    mpe.cc |= CC_COUNTER1_ZERO;

    if(mpe.rc1)
    {
      mpe.rc1--;
      if(mpe.rc1)
      {
        mpe.cc &= ~CC_COUNTER1_ZERO;
      }
    }
  }
}

void Execute_MVRImmediateOnly(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = nuance.fields[FIELD_RCU_SRC];
}

void Execute_MVRImmediate(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = nuance.fields[FIELD_RCU_SRC];
  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    mpe.cc |= CC_COUNTER0_ZERO;

    if(mpe.rc0)
    {
      mpe.rc0--;
      if(mpe.rc0)
      {
        mpe.cc &= ~CC_COUNTER0_ZERO;
      }
    }
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    mpe.cc |= CC_COUNTER1_ZERO;

    if(mpe.rc1)
    {
      mpe.rc1--;
      if(mpe.rc1)
      {
        mpe.cc &= ~CC_COUNTER1_ZERO;
      }
    }
  }
}

void Execute_MVRScalarOnly(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = entry.pScalarRegs[nuance.fields[FIELD_RCU_SRC]];
}

void Execute_MVRScalar(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] = entry.pScalarRegs[nuance.fields[FIELD_RCU_SRC]];
  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    mpe.cc |= CC_COUNTER0_ZERO;

    if(mpe.rc0)
    {
      mpe.rc0--;
      if(mpe.rc0)
      {
        mpe.cc &= ~CC_COUNTER0_ZERO;
      }
    }
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    mpe.cc |= CC_COUNTER1_ZERO;

    if(mpe.rc1)
    {
      mpe.rc1--;
      if(mpe.rc1)
      {
        mpe.cc &= ~CC_COUNTER1_ZERO;
      }
    }
  }
}

void Execute_RangeOnly(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  uint32 rcu_range;
  uint32 rcu_src = nuance.fields[FIELD_RCU_SRC];
  switch(rcu_src)
  {
    case 0:
      //Use x range as integer portion.  Limit to 10 bits.
      rcu_range = *entry.pXyrange & 0x03FF0000UL;
      break;
    case 1:
      //Use y range as integer portion.  Limit to 10 bits.
      rcu_range = (*entry.pXyrange << 16) & 0x003FF0000UL;
      break;
    case 2:
      //Use u range.  Limit to 10 bits.
      rcu_range = *entry.pUvrange & 0x03FF0000UL;
      break;
    case 3:
      //Use v range.  Limit to 10 bits
      rcu_range = (*entry.pUvrange << 16) & 0x03FF0000UL;
      break;
    default:
      assert(false);
      rcu_range = 0;
      break;
  }

  rcu_src = entry.pIndexRegs[rcu_src];

  //clear modge and modmi conditions
  mpe.cc &= ~(CC_MODGE | CC_MODMI);

  //uint32 moduloResult;
  if(((int32)(rcu_src & 0xFFFF0000UL)) >= (int32)rcu_range)
  {
    //moduloResult = rcu_src - rcu_range;
    //set modge condition
    mpe.cc |= CC_MODGE;
  }
  else if((int32)rcu_src < 0)
  {
    //moduloResult = rcu_src + rcu_range;
    //set modmi condition
    mpe.cc |= CC_MODMI;
  }
}

void Execute_Range(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  uint32 rcu_range;
  uint32 rcu_src = nuance.fields[FIELD_RCU_SRC];
  switch(rcu_src)
  {
    case 0:
      //Use x range as integer portion.  Limit to 10 bits.
      rcu_range = *entry.pXyrange & 0x03FF0000UL;
      break;
    case 1:
      //Use y range as integer portion.  Limit to 10 bits.
      rcu_range = (*entry.pXyrange << 16) & 0x03FF0000UL;
      break;
    case 2:
      //Use u range.  Limit to 10 bits.
      rcu_range = *entry.pUvrange & 0x03FF0000UL;
      break;
    case 3:
      //Use v range.  Limit to 10 bits
      rcu_range = (*entry.pUvrange << 16) & 0x03FF0000UL;
      break;
    default:
      assert(false);
      rcu_range = 0;
      break;
  }

  rcu_src = entry.pIndexRegs[rcu_src];

  //clear modge and modmi conditions
  mpe.cc &= ~(CC_MODGE | CC_MODMI);

  //uint32 moduloResult;
  if(((int32)(rcu_src & 0xFFFF0000UL)) >= (int32)rcu_range)
  {
    //moduloResult = rcu_src - rcu_range;
    //set modge condition
    mpe.cc |= CC_MODGE;
  }
  else if((int32)rcu_src < 0)
  {
    //moduloResult = rcu_src + rcu_range;
    //set modmi condition
    mpe.cc |= CC_MODMI;
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    mpe.cc |= CC_COUNTER0_ZERO;

    if(mpe.rc0)
    {
      mpe.rc0--;
      if(mpe.rc0)
      {
        mpe.cc &= ~CC_COUNTER0_ZERO;
      }
    }
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    mpe.cc |= CC_COUNTER1_ZERO;

    if(mpe.rc1)
    {
      mpe.rc1--;
      if(mpe.rc1)
      {
        mpe.cc &= ~CC_COUNTER1_ZERO;
      }
    }
  }
}

void Execute_ModuloOnly(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  uint32 rcu_range;
  uint32 rcu_src = nuance.fields[FIELD_RCU_SRC];
  switch(rcu_src)
  {
    case 0:
      //Use x range as integer portion.  Limit to 10 bits.
      rcu_range = *entry.pXyrange & 0x03FF0000UL;
      break;
    case 1:
      //Use y range as integer portion.  Limit to 10 bits.
      rcu_range = (*entry.pXyrange << 16) & 0x003FF0000UL;
      break;
    case 2:
      //Use u range.  Limit to 10 bits.
      rcu_range = *entry.pUvrange & 0x03FF0000UL;
      break;
    case 3:
      //Use v range.  Limit to 10 bits
      rcu_range = (*entry.pUvrange << 16) & 0x03FF0000UL;
      break;
    default:
      assert(false);
      rcu_range = 0;
      break;
  }

  rcu_src = entry.pIndexRegs[rcu_src];

  //clear modge and modmi conditions
  mpe.cc &= ~(CC_MODGE | CC_MODMI);

  uint32 moduloResult = rcu_src;
  if(((int32)(rcu_src & 0xFFFF0000UL)) >= (int32)rcu_range)
  {
    moduloResult = rcu_src - rcu_range;
    //set modge condition
    mpe.cc |= CC_MODGE;
  }
  else if((int32)rcu_src < 0)
  {
    moduloResult = rcu_src + rcu_range;
    //set modmi condition
    mpe.cc |= CC_MODMI;
  }

  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] =
    (entry.pIndexRegs[nuance.fields[FIELD_RCU_DEST]] & 0x0000FFFFUL) |
    (moduloResult & 0xFFFF0000UL);
}

void Execute_Modulo(MPE &mpe, const InstructionCacheEntry &entry, const Nuance &nuance)
{
  uint32 rcu_range;
  uint32 rcu_src = nuance.fields[FIELD_RCU_SRC];
  switch(rcu_src)
  {
    case 0:
      //Use x range as integer portion.  Limit to 10 bits.
      rcu_range = *entry.pXyrange & 0x03FF0000UL;
      break;
    case 1:
      //Use y range as integer portion.  Limit to 10 bits.
      rcu_range = (*entry.pXyrange << 16) & 0x003FF0000UL;
      break;
    case 2:
      //Use u range.  Limit to 10 bits.
      rcu_range = *entry.pUvrange & 0x03FF0000UL;
      break;
    case 3:
      //Use v range.  Limit to 10 bits
      rcu_range = (*entry.pUvrange << 16) & 0x03FF0000UL;
      break;
    default:
      assert(false);
      rcu_range = 0;
      break;
  }

  rcu_src = entry.pIndexRegs[rcu_src];

  //clear modge and modmi conditions
  mpe.cc &= ~(CC_MODGE | CC_MODMI);

  uint32 moduloResult = rcu_src;
  if(((int32)(rcu_src & 0xFFFF0000UL)) >= (int32)rcu_range)
  {
    moduloResult = rcu_src - rcu_range;
    //set modge condition
    mpe.cc |= CC_MODGE;
  }
  else if((int32)rcu_src < 0)
  {
    moduloResult = rcu_src + rcu_range;
    //set modmi condition
    mpe.cc |= CC_MODMI;
  }

  mpe.rcu_union[nuance.fields[FIELD_RCU_DEST]] =
    (entry.pIndexRegs[nuance.fields[FIELD_RCU_DEST]] & 0x0000FFFFUL) |
    (moduloResult & 0xFFFF0000UL);

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    mpe.cc |= CC_COUNTER0_ZERO;

    if(mpe.rc0)
    {
      mpe.rc0--;
      if(mpe.rc0)
      {
        mpe.cc &= ~CC_COUNTER0_ZERO;
      }
    }
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    mpe.cc |= CC_COUNTER1_ZERO;

    if(mpe.rc1)
    {
      mpe.rc1--;
      if(mpe.rc1)
      {
        mpe.cc &= ~CC_COUNTER1_ZERO;
      }
    }
  }
}
