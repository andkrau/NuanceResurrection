#include "basetypes.h"
#include <stdarg.h>
#include "EmitMisc.h"
#include "EmitECU.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "NativeCodeCache.h"
#include "X86EmitTypes.h"

void EmitConditionCheck(const EmitterVariables * const vars, const uint32 condition, const uint32 conditionFalseLabel, const uint32 conditionTrueLabel)
{
  if(condition == ECU_CONDITION_T)
  {
    //conditon is always true
    return;
  }

  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax,ccRegReadBaseReg,x86IndexReg::x86IndexReg_none,x86ScaleVal::x86Scale_1,ccRegDisp);

  switch(condition)
  {
    case 0:
      //ne (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 1:
      //c0z (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COUNTER0_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 2:
      //c1z (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COUNTER1_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 3:
      //cc (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_CARRY,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 4:
      //eq (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 5:
      //cs (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_CARRY,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 6:
      //vc (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_OVERFLOW,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 7:
      //vs (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_OVERFLOW,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 8:
      //lt (checked)
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE|CC_ALU_OVERFLOW,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      vars->mpe->nativeCodeCache.X86Emit_CMPIR(CC_ALU_NEGATIVE|CC_ALU_OVERFLOW,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 9:
      //mvc (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_MUL_OVERFLOW,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 10:
      //mvs (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_MUL_OVERFLOW,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 11:
      //hi (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_CARRY|CC_ALU_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 12:
      //le (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionTrueLabel);
      vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_OVERFLOW, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg::x86Reg_ebx);
      //shift CC_ALU_OVERFLOW flag into position of CC_ALU_NEGATIVE bit
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax,1);
      //ebx = CC_ALU_OVERFLOW ^ CC_ALU_NEGATIVE
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebx,x86Reg::x86Reg_eax);
      //if result is zero, the condition is false
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 13:
      //ls (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_CARRY|CC_ALU_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 14:
      //pl (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_NEGATIVE,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 15:
      //mi (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_NEGATIVE,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 16:
      //gt (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_ALU_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_OVERFLOW, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg::x86Reg_ebx);
      //shift CC_ALU_OVERFLOW flag into position of CC_ALU_NEGATIVE bit
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax,1);
      //ebx = CC_ALU_OVERFLOW ^ CC_ALU_NEGATIVE
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebx,x86Reg::x86Reg_eax);
      //if result is non-zero, the condition is false
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return; 
    case 17:
      //always (checked)
      return;
    case 18:
      //modmi (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_MODMI,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 19:
      //modpl (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_MODMI,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 20:
      //ge (checked)
      vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_OVERFLOW, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg::x86Reg_ebx);
      //shift CC_ALU_OVERFLOW flag into position of CC_ALU_NEGATIVE bit
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax,1);
      //ebx = CC_ALU_OVERFLOW ^ CC_ALU_NEGATIVE
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebx,x86Reg::x86Reg_eax);
      //if result is non-zero, the condition is false
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 21:
      //modge (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_MODGE,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 22:
      //modlt (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_MODGE,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 23:
      //never (checked)
      vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(conditionFalseLabel);
      return;
    case 24:
      //c0ne (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COUNTER0_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 25:
      //never (checked)
      vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(conditionFalseLabel);
      return;
    case 26: 
      //never (checked)
      vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(conditionFalseLabel);
      return;
    case 27:
      //cf0lo (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COPROCESSOR0,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 28:
      //c1ne (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COUNTER1_ZERO,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 29:
      //cf0hi (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COPROCESSOR0,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
    case 30:
      //cf1lo (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COPROCESSOR1,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, conditionFalseLabel);
      return;
    case 31:
      //cf1hi (checked)
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(CC_COPROCESSOR1,x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z, conditionFalseLabel);
      return;
   }
}

void Emit_BRAAlways(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_BRAAlways_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_BRAConditional(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_BRAConditional_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JMPAlwaysIndirect(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JMPAlwaysIndirect_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JMPConditionalIndirect(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JMPConditionalIndirect_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRAlways(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRAlways_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRConditional(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRConditional_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(address, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRAlwaysIndirect(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  constexpr uint32 l_skip_ecu = 0;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRAlwaysIndirect_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  constexpr uint32 l_skip_ecu = 0;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRConditionalIndirect(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_JSRConditionalIndirect_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  const x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr::x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTSAlways(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTSAlways_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTSConditional(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTSConditional_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  const int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTI1Conditional(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzi1RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI1);
  const int32 rzi1RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI1);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzi1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzi1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(1UL << 1), x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTI1Conditional_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzi1RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI1);
  const int32 rzi1RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI1);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzi1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzi1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(1UL << 1), x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTI2Conditional(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzi2RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI2);
  const int32 rzi2RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI2);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzi2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzi2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(1UL << 5), x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->intctl));

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_RTI2Conditional_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rzi2RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI2);
  const int32 rzi2RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI2);

  //const uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  constexpr uint32 l_skip_ecu = 0;
  constexpr uint32 l_condition_true = 1;

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->mpe->nativeCodeCache.SetLabelPointer(l_condition_true);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rzi2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rzi2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->mpe->nativeCodeCache.X86Emit_MOVIM(1, x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(1UL << 5), x86MemPtr::x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  Emit_ExitBlock(vars);

  vars->mpe->nativeCodeCache.SetLabelPointer(l_skip_ecu);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}
