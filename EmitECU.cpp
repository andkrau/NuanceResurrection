#include "BaseTypes.h"
#include "ByteSwap.h"
#include "EmitMisc.h"
#include "EmitECU.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "NativeCodeCache.h"
#include "X86EmitTypes.h"
#include <stdarg.h>

void EmitConditionCheck(EmitterVariables *vars, uint32 condition, uint32 conditionFalseLabel, uint32 conditionTrueLabel)
{
  x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  if(condition == ECU_CONDITION_T)
  {
    //conditon is always true
    return;
  }

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax,ccRegReadBaseReg,x86IndexReg_none,x86Scale_1,ccRegDisp);

  switch(condition)
  {
    case 0:
      //ne (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 1:
      //c0z (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COUNTER0_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 2:
      //c1z (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COUNTER1_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 3:
      //cc (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_CARRY,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 4:
      //eq (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 5:
      //cs (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_CARRY,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 6:
      //vc (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_OVERFLOW,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 7:
      //vs (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_OVERFLOW,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 8:
      //lt (checked)
      vars->codeCache->X86Emit_ANDIR(CC_ALU_NEGATIVE|CC_ALU_OVERFLOW,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      vars->codeCache->X86Emit_CMPIR(CC_ALU_NEGATIVE|CC_ALU_OVERFLOW,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 9:
      //mvc (checked)
      vars->codeCache->X86Emit_TESTIR(CC_MUL_OVERFLOW,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 10:
      //mvs (checked)
      vars->codeCache->X86Emit_TESTIR(CC_MUL_OVERFLOW,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 11:
      //hi (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_CARRY|CC_ALU_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 12:
      //le (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionTrueLabel);
      vars->codeCache->X86Emit_MOVRR(x86Reg_ebx,x86Reg_eax);
      vars->codeCache->X86Emit_ANDIR(CC_ALU_OVERFLOW, x86Reg_eax);
      vars->codeCache->X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ebx);
      //shift CC_ALU_OVERFLOW flag into position of CC_ALU_NEGATIVE bit
      vars->codeCache->X86Emit_SHLIR(x86Reg_eax,1);
      //ebx = CC_ALU_OVERFLOW ^ CC_ALU_NEGATIVE
      vars->codeCache->X86Emit_XORRR(x86Reg_ebx,x86Reg_eax);
      //if result is zero, the condition is false
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 13:
      //ls (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_CARRY|CC_ALU_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 14:
      //pl (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_NEGATIVE,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 15:
      //mi (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_NEGATIVE,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 16:
      //gt (checked)
      vars->codeCache->X86Emit_TESTIR(CC_ALU_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      vars->codeCache->X86Emit_MOVRR(x86Reg_ebx,x86Reg_eax);
      vars->codeCache->X86Emit_ANDIR(CC_ALU_OVERFLOW, x86Reg_eax);
      vars->codeCache->X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ebx);
      //shift CC_ALU_OVERFLOW flag into position of CC_ALU_NEGATIVE bit
      vars->codeCache->X86Emit_SHLIR(x86Reg_eax,1);
      //ebx = CC_ALU_OVERFLOW ^ CC_ALU_NEGATIVE
      vars->codeCache->X86Emit_XORRR(x86Reg_ebx,x86Reg_eax);
      //if result is non-zero, the condition is false
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return; 
    case 17:
      //always (checked)
      return;
    case 18:
      //modmi (checked)
      vars->codeCache->X86Emit_TESTIR(CC_MODMI,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 19:
      //modpl (checked)
      vars->codeCache->X86Emit_TESTIR(CC_MODMI,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 20:
      //ge (checked)
      vars->codeCache->X86Emit_MOVRR(x86Reg_ebx,x86Reg_eax);
      vars->codeCache->X86Emit_ANDIR(CC_ALU_OVERFLOW, x86Reg_eax);
      vars->codeCache->X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ebx);
      //shift CC_ALU_OVERFLOW flag into position of CC_ALU_NEGATIVE bit
      vars->codeCache->X86Emit_SHLIR(x86Reg_eax,1);
      //ebx = CC_ALU_OVERFLOW ^ CC_ALU_NEGATIVE
      vars->codeCache->X86Emit_XORRR(x86Reg_ebx,x86Reg_eax);
      //if result is non-zero, the condition is false
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 21:
      //modge (checked)
      vars->codeCache->X86Emit_TESTIR(CC_MODGE,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 22:
      //modlt (checked)
      vars->codeCache->X86Emit_TESTIR(CC_MODGE,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 23:
      //never (checked)
      vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr, conditionFalseLabel);
      return;
    case 24:
      //c0ne (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COUNTER0_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 25:
      //never (checked)
      vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr, conditionFalseLabel);
      return;
    case 26: 
      //never (checked)
      vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr, conditionFalseLabel);
      return;
    case 27:
      //cf0lo (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COPROCESSOR0,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 28:
      //c1ne (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COUNTER1_ZERO,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 29:
      //cf0hi (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COPROCESSOR0,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
    case 30:
      //cf1lo (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COPROCESSOR1,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, conditionFalseLabel);
      return;
    case 31:
      //cf1hi (checked)
      vars->codeCache->X86Emit_TESTIR(CC_COPROCESSOR1,x86Reg_eax);
      vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_Z, conditionFalseLabel);
      return;
   }
}

void Emit_BRAAlways(EmitterVariables *vars, Nuance &nuance)
{

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_BRAAlways_NOP(EmitterVariables *vars, Nuance &nuance)
{
  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_BRAConditional(EmitterVariables *vars, Nuance &nuance)
{
  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_BRAConditional_NOP(EmitterVariables *vars, Nuance &nuance)
{
  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JMPAlwaysIndirect(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JMPAlwaysIndirect_NOP(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JMPConditionalIndirect(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JMPConditionalIndirect_NOP(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRAlways(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRAlways_NOP(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRConditional(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRConditional_NOP(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVIM(address, x86MemPtr_dword, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRAlwaysIndirect(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  uint32 l_skip_ecu = 0;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRAlwaysIndirect_NOP(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  uint32 l_skip_ecu = 0;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRConditionalIndirect(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCFETCHNEXT], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_JSRConditionalIndirect_NOP(EmitterVariables *vars, Nuance &nuance)
{
  uint32 src1RegIndex = nuance.fields[FIELD_ECU_ADDRESS];
  x86BaseReg rzRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }

  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_ECU_PCROUTE], x86MemPtr_dword, rzRegWriteBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTSAlways(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzRegReadBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTSAlways_NOP(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzRegReadBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTSConditional(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzRegReadBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTSConditional_NOP(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZ);
  int32 rzRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZ);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzRegReadBaseReg, x86IndexReg_none, x86Scale_1, rzRegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTI1Conditional(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzi1RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI1);
  int32 rzi1RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI1);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzi1RegReadBaseReg, x86IndexReg_none, x86Scale_1, rzi1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->codeCache->X86Emit_ANDIM(~(1UL << 1), x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTI1Conditional_NOP(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzi1RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI1);
  int32 rzi1RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI1);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzi1RegReadBaseReg, x86IndexReg_none, x86Scale_1, rzi1RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->codeCache->X86Emit_ANDIM(~(1UL << 1), x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTI2Conditional(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzi2RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI2);
  int32 rzi2RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI2);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzi2RegReadBaseReg, x86IndexReg_none, x86Scale_1, rzi2RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->codeCache->X86Emit_ANDIM(~(1UL << 5), x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}

void Emit_RTI2Conditional_NOP(EmitterVariables *vars, Nuance &nuance)
{
  x86BaseReg rzi2RegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RZI2);
  int32 rzi2RegDisp = GetMiscRegEmitDisp(vars,REGINDEX_RZI2);

  uint32 address = nuance.fields[FIELD_ECU_ADDRESS];
  uint32 l_skip_ecu = 0;
  uint32 l_condition_true = 1;

  vars->patchMgr->Reset();

  if(vars->bCheckECUSkipCounter)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->ecuSkipCounter));
    vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
    vars->codeCache->X86Emit_JCC_Label(vars->patchMgr, X86_CC_NZ, l_skip_ecu);
  }
  if(nuance.fields[FIELD_ECU_CONDITION] != ECU_CONDITION_T)
  {
    EmitConditionCheck(vars, nuance.fields[FIELD_ECU_CONDITION], l_skip_ecu, l_condition_true);
  }
  vars->patchMgr->SetLabelPointer(l_condition_true, vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, rzi2RegReadBaseReg, x86IndexReg_none, x86Scale_1, rzi2RegDisp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->pcfetchnext));
  vars->codeCache->X86Emit_MOVIM(1, x86MemPtr_dword, (uint32)&(vars->mpe->ecuSkipCounter));
  vars->codeCache->X86Emit_ANDIM(~(1UL << 5), x86MemPtr_dword, (uint32)&(vars->mpe->intctl));
  Emit_ExitBlock(vars);

  vars->patchMgr->SetLabelPointer(l_skip_ecu, vars->GetEmitLoc());
  vars->patchMgr->ApplyPatches();
}
