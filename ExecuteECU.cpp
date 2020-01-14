#include "InstructionCache.h"
#include "mpe.h"
#include "NuonEnvironment.h"

extern NuonEnvironment *nuonEnv;

void Execute_ECU_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
}

void Execute_Halt(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.excepsrc |= 0x01;
    mpe.invalidateRegionStart = MPE_IRAM_BASE;
    mpe.invalidateRegionEnd = (MPE_IRAM_BASE + OVERLAY_SIZE - 1);

    //If the halt enable bit for the halt exception is not set
    if(!(mpe.excephalten & (1UL << 0)))
    {
      //set exception bit in interrupt source register
      mpe.intsrc |= 0x01;
    }
    else
    {
      mpe.mpectl &= ~MPECTRL_MPEGO;
    }
  }
}

void Execute_BRAAlways(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
    mpe.ecuSkipCounter = 3;
  }
}

void Execute_BRAAlways_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
    mpe.ecuSkipCounter = 1;
  }
}

void Execute_BRAConditional(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
      mpe.ecuSkipCounter = 3;
    }
  }
}

void Execute_BRAConditional_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
      mpe.ecuSkipCounter = 1;
    }
  }
}
void Execute_JMPAlwaysIndirect(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
    mpe.ecuSkipCounter = 3;
  }
}
void Execute_JMPAlwaysIndirect_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
    mpe.ecuSkipCounter = 1;
  }
}
void Execute_JMPConditionalIndirect(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
      mpe.ecuSkipCounter = 3;
    }
  }
}
void Execute_JMPConditionalIndirect_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
      mpe.ecuSkipCounter = 1;
    }
  }
}
void Execute_JSRAlways(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.rz = nuance.fields[FIELD_ECU_PCFETCHNEXT];
    mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
    mpe.ecuSkipCounter = 3;
  }
}
void Execute_JSRAlways_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.rz = nuance.fields[FIELD_ECU_PCROUTE];
    mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
    mpe.ecuSkipCounter = 1;
  }
}
void Execute_JSRConditional(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.rz = nuance.fields[FIELD_ECU_PCFETCHNEXT];
      mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
      mpe.ecuSkipCounter = 3;
    }
  }
}
void Execute_JSRConditional_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.rz = nuance.fields[FIELD_ECU_PCROUTE];
      mpe.pcfetchnext = nuance.fields[FIELD_ECU_ADDRESS];
      mpe.ecuSkipCounter = 1;
    }
  }
}
void Execute_JSRAlwaysIndirect(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.rz = nuance.fields[FIELD_ECU_PCFETCHNEXT];
    mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
    mpe.ecuSkipCounter = 3;
  }
}
void Execute_JSRAlwaysIndirect_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.rz = nuance.fields[FIELD_ECU_PCROUTE];
    mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
    mpe.ecuSkipCounter = 1;
  }
}
void Execute_JSRConditionalIndirect(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.rz = nuance.fields[FIELD_ECU_PCFETCHNEXT];
      mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
      mpe.ecuSkipCounter = 3;
    }
  }
}
void Execute_JSRConditionalIndirect_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.rz = nuance.fields[FIELD_ECU_PCROUTE];
      mpe.pcfetchnext = (entry.pScalarRegs)[nuance.fields[FIELD_ECU_ADDRESS]];
      mpe.ecuSkipCounter = 1;
    }
  }
}
void Execute_RTSAlways(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.pcfetchnext = entry.pRzRegs[0];
    mpe.ecuSkipCounter = 3;
  }
}
void Execute_RTSAlways_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    mpe.pcfetchnext = entry.pRzRegs[0];
    mpe.ecuSkipCounter = 1;
  }
}
void Execute_RTSConditional(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.pcfetchnext = entry.pRzRegs[0];
      mpe.ecuSkipCounter = 3;
    }
  }
}
void Execute_RTSConditional_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.pcfetchnext = entry.pRzRegs[0];
      mpe.ecuSkipCounter = 1;
    }
  }
}
void Execute_RTI1Conditional(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.intctl &= ~(1UL << 1);
      mpe.pcfetchnext = entry.pRzRegs[1];
      mpe.ecuSkipCounter = 3;
    }
  }
}
void Execute_RTI1Conditional_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.intctl &= ~(1UL << 1);
      mpe.pcfetchnext = entry.pRzRegs[1];
      mpe.ecuSkipCounter = 1;
    }
  }
}
void Execute_RTI2Conditional(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.intctl &= ~(1UL << 5);
      mpe.pcfetchnext = entry.pRzRegs[2];
      mpe.ecuSkipCounter = 3;
    }
  }
}
void Execute_RTI2Conditional_NOP(MPE &mpe, InstructionCacheEntry &entry, Nuance &nuance)
{
  if(!mpe.ecuSkipCounter)
  {
    if(mpe.TestConditionCode(nuance.fields[FIELD_ECU_CONDITION]))
    {
      mpe.intctl &= ~(1UL << 5);
      mpe.pcfetchnext = entry.pRzRegs[2];
      mpe.ecuSkipCounter = 1;
    }
  }
}
