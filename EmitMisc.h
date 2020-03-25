#ifndef EMITMISC_H
#define EMITMISC_H

#include "basetypes.h"
#include "PatchManager.h"
#include "X86EmitTypes.h"
#include "InstructionDependencies.h"

class MPE;
struct InstructionEntry;
struct Nuance;
class NativeCodeCache;

struct EmitterVariables
{
  MPE *mpe;

  InstructionEntry *pInstructionEntry;
  uint32 scalarRegDep;
  uint32 miscRegDep;
  uint32 scalarRegOutDep;
  uint32 miscRegOutDep;

  bool bCheckECUSkipCounter;
};

inline x86BaseReg GetScalarRegReadBaseReg(const EmitterVariables* const vars, const uint32 regIndex)
{
  return (vars->scalarRegDep & SCALAR_REG_DEPENDENCY_MASK(regIndex)) ? x86BaseReg::x86BaseReg_edi : x86BaseReg::x86BaseReg_esi;
}

inline x86BaseReg GetScalarRegWriteBaseReg(const EmitterVariables* const vars, const uint32 regIndex)
{
  return x86BaseReg::x86BaseReg_esi;
}

inline x86BaseReg GetMiscRegReadBaseReg(const EmitterVariables * const vars, const uint32 regIndex)
{
  const uint32 mask = !regIndex ? DEPENDENCY_FLAG_ALLFLAGS : MISC_REG_DEPENDENCY_MASK(regIndex - 1);

  return (vars->miscRegDep & mask) ? x86BaseReg::x86BaseReg_edi : x86BaseReg::x86BaseReg_esi;
}

inline x86BaseReg GetMiscRegWriteBaseReg(const EmitterVariables * const vars, const uint32 regIndex)
{
  return x86BaseReg::x86BaseReg_esi;
}

inline int32 GetMiscRegEmitDisp(const EmitterVariables * const vars, const uint32 regIndex)
{
  return regIndex*sizeof(uint32);
}

inline int32 GetScalarRegEmitDisp(const EmitterVariables * const vars, const uint32 regIndex)
{
  return -((32 - (int32)regIndex) * ((int32)sizeof(uint32)));
}

void Emit_ExitBlock(EmitterVariables * const vars);
void Emit_NOP(EmitterVariables * const vars, const Nuance &nuance);
void Emit_SaveRegs(EmitterVariables * const vars, const Nuance &nuance);
void Emit_StoreMiscRegisterConstant(EmitterVariables * const vars, const Nuance &nuance);
void Emit_StoreScalarRegisterConstant(EmitterVariables * const vars, const Nuance &nuance);

#endif
