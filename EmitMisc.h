#ifndef EMITMISC_H
#define EMITMISC_H

#include "BaseTypes.h"
#include "PatchManager.h"
#include "X86EmitTypes.h"
#include "InstructionDependencies.h"

class MPE;
struct InstructionEntry;
class Nuance;
class NativeCodeCache;

class EmitterVariables
{
public:
  MPE *mpe;
  NativeCodeCache *codeCache;
  PatchManager *patchMgr;
  InstructionEntry *pInstructionEntry;
  bool bCheckECUSkipCounter;
  bool bSaveRegs;
  bool bUsesMMX;
  int32 regBase;
  int32 tempRegBase;
  uint32 scalarRegDep;
  uint32 miscRegDep;
  uint32 scalarRegOutDep;
  uint32 miscRegOutDep;
  uint8 **ppEmitLoc;
  uint8 *GetEmitLoc() { return *ppEmitLoc; }
};

inline x86BaseReg GetScalarRegReadBaseReg(EmitterVariables *vars, uint32 regIndex)
{
  if(vars->scalarRegDep & SCALAR_REG_DEPENDENCY_MASK(regIndex))
  {
    return x86BaseReg_edi;
  }
  else
  {
    return x86BaseReg_esi;
  }
}

inline x86BaseReg GetScalarRegWriteBaseReg(EmitterVariables *vars, uint32 regIndex)
{
  return x86BaseReg_esi;
}

inline x86BaseReg GetMiscRegReadBaseReg(EmitterVariables *vars, uint32 regIndex)
{
  uint32 mask = regIndex;

  if(!regIndex)
  {
    mask = DEPENDENCY_FLAG_ALLFLAGS;
  }
  else
  {
    mask = MISC_REG_DEPENDENCY_MASK(regIndex - 1);
  }
    

  if(vars->miscRegDep & mask)
  {
    return x86BaseReg_edi;
  }
  else
  {
    return x86BaseReg_esi;
  }
}
inline x86BaseReg GetMiscRegWriteBaseReg(EmitterVariables *vars, uint32 regIndex)
{
  return x86BaseReg_esi;
}

inline int32 GetMiscRegEmitDisp(EmitterVariables *vars, uint32 regIndex)
{
  return regIndex*sizeof(uint32);
}

inline int32 GetScalarRegEmitDisp(EmitterVariables *vars, uint32 regIndex)
{
  return -((32 - (int32)regIndex) * ((int32)sizeof(uint32)));
}

void Emit_ExitBlock(EmitterVariables *vars);
void Emit_NOP(EmitterVariables *vars, Nuance &nuance);
void Emit_SaveRegs(EmitterVariables *vars, Nuance &nuance);
void Emit_StoreMiscRegisterConstant(EmitterVariables *vars, Nuance &nuance);
void Emit_StoreScalarRegisterConstant(EmitterVariables *vars, Nuance &nuance);
#endif