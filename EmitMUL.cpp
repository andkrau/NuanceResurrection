#include "basetypes.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "EmitMisc.h"
#include "NativeCodeCache.h"
#include "PatchManager.h"
#include "SuperBlock.h"
#include "mpe.h"

const uint32 shiftTable[4] = {16, 8, 0, 2};

void Emit_ADDM(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
    vars->codeCache->X86Emit_ADDMR(x86Reg_eax, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}
void Emit_ADDMImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
    vars->codeCache->X86Emit_ADDIR(nuance.fields[FIELD_MUL_SRC1],x86Reg_eax);
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}

void Emit_SUBM(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    if(src1RegIndex == src2RegIndex)
    {
      vars->codeCache->X86Emit_MOVIM(0, x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    }
    else
    {
      vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
      vars->codeCache->X86Emit_SUBMR(x86Reg_eax, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
      vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    }
  }
}

void Emit_SUBMImmediateReverse(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    vars->codeCache->X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg_eax);
    vars->codeCache->X86Emit_SUBMR(x86Reg_eax, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}

void Emit_MULScalarShiftAcshift(EmitterVariables *vars, Nuance &nuance)
{
  const uint32 l_pos = 0;
  const uint32 l_neg = 1;
  const uint32 l_poslt32 = 2;
  const uint32 l_posge32 = 3;
  const uint32 l_exit = 4;

  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg acshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_ACSHIFT);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  int32 acshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_ACSHIFT);


/*
  mov eax, [src1]
  and [cc], ~CC_MV
  imul [src2]
  mov ecx, [acshift]
  cmp ecx, 0
  jnl mulscalarshiftacshift_pos
  neg ecx
  jmp mulscalarshiftacshift_neg
mulscalarshiftacshift_pos:
  cmp ecx, 32
  jnl mulscalarshiftacshift_posge32
mulscalarshiftacshift_poslt32:
  shrd eax, edx, cl
  if(mv flag required)
  {
    sar edx, cl
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_posge32:
  mov eax, edx
  sub ecx, 32
  sar eax, ecx
  if(mv flag required)
  {
    mov ebp, CC_MV
    cmp eax, 0
    cmovz ebp, eax
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_neg:
  xor ebp, ebp
  cmp ecx, 32
  cmovz eax, ebp
  cmovz ecx, ebp
  shld edx, eax, cl
  shl eax, cl
  if(mv flag required)
  {
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
mulscalarshiftacshift_exit:
  mov [dest], eax
*/

  vars->patchMgr->Reset();

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, acshiftReadBaseReg, x86IndexReg_none, x86Scale_1, acshiftDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  
  vars->codeCache->X86Emit_IMULRR(x86Reg_ebx);
  vars->codeCache->X86Emit_TESTIR(0x40,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_Z,l_pos);
  vars->codeCache->X86Emit_NEGR(x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(128,x86Reg_ecx);
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_neg);
  //l_pos:
  vars->patchMgr->SetLabelPointer(l_pos,vars->GetEmitLoc());
  vars->codeCache->X86Emit_CMPIR(32,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->patchMgr->SetLabelPointer(l_poslt32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_SHRDRRR(x86Reg_eax, x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_SARRR(x86Reg_edx);
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_posge32:
  vars->patchMgr->SetLabelPointer(l_posge32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRR(x86Reg_eax, x86Reg_edx);
  vars->codeCache->X86Emit_SUBIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_XORRR(x86Reg_ebp, x86Reg_ebp);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_neg:
  vars->patchMgr->SetLabelPointer(l_neg,vars->GetEmitLoc());
  vars->codeCache->X86Emit_XORRR(x86Reg_ebp, x86Reg_ebp);
  vars->codeCache->X86Emit_CMPIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLDRRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->patchMgr->SetLabelPointer(l_exit,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->patchMgr->ApplyPatches();
}

void Emit_MULScalarShiftRightImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  /*
  eax = src1
  and [cc], ~CC_MV
  imul [src2]
  if(shiftCount == 32)
  {
    mov [dest], edx
    //overflow cant occur
  }
  else
  {
    shrd eax, edx, shiftCount
    if(mv flag required)
    {
      sar edx, shiftCount
      xor ecx, ecx
      test eax, 0x80000000
      mov ebx, 1
      mov ebp, CC_MV
      cmovz ebx, ecx
      add edx, ebx
      cmovz ebp, edx
      or [cc], ebp
    }
    mov [dest], eax
  }
*/

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] < 32)
  {
    vars->codeCache->X86Emit_SHRDIRR(x86Reg_eax, x86Reg_edx, nuance.fields[FIELD_MUL_INFO]);
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->codeCache->X86Emit_SARIR(x86Reg_edx, nuance.fields[FIELD_MUL_INFO]);
      vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
      vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
      vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
      vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
      vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
  else if(nuance.fields[FIELD_MUL_INFO] == 32)
  {
    vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
  else
  {
    vars->codeCache->X86Emit_MOVRR(x86Reg_eax, x86Reg_edx);
    vars->codeCache->X86Emit_SARIR(x86Reg_eax, nuance.fields[FIELD_MUL_INFO] - 32);
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}

void Emit_MULScalarShiftLeftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

/*
  mov eax, [src1]
  and [cc], ~CC_MV
  imul [src2]
  if(shiftCount > 31)
  {     
    mov [dest], 0
    if(mv flag required)
    {
      shl eax, shiftCount - 32
      mov ebp, CC_MV
      cmovz ebp, eax
      or [cc], ebp
    }
  }
  else
  {
    if(shiftCount != 0)
    {
      shld edx, eax, shiftCount
      shl eax, shiftCount
    }

    if(mv flag required)
    {
      xor ecx, ecx
      test eax, 0x80000000
      mov ebx, 1
      mov ebp, CC_MV
      cmovz ebx, ecx
      add edx, ebx
      cmovz ebp, edx
      or [cc], ebp
    }
    mov [dest], eax
  }
*/

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] > 31)
  {
    vars->codeCache->X86Emit_MOVIM(0, x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
      vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_eax);
      vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
  }
  else
  {
    if(nuance.fields[FIELD_MUL_INFO] != 0)
    {
      vars->codeCache->X86Emit_SHLDIRR(x86Reg_edx, x86Reg_eax, nuance.fields[FIELD_MUL_INFO]);
      vars->codeCache->X86Emit_SHLIR(x86Reg_eax, nuance.fields[FIELD_MUL_INFO]);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
      vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
      vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
      vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
      vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}

void Emit_MULImmediateShiftAcshift(EmitterVariables *vars, Nuance &nuance)
{
  const uint32 l_pos = 0;
  const uint32 l_neg = 1;
  const uint32 l_poslt32 = 2;
  const uint32 l_posge32 = 3;
  const uint32 l_exit = 4;

  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg acshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_ACSHIFT);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  int32 acshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_ACSHIFT);


/*
  mov eax, src1
  and [cc], ~CC_MV
  imul [src2]
  mov ecx, [acshift]
  cmp ecx, 0
  jnl mulscalarshiftacshift_pos
  neg ecx
  jmp mulscalarshiftacshift_neg
mulscalarshiftacshift_pos:
  cmp ecx, 32
  jnl mulscalarshiftacshift_posge32
mulscalarshiftacshift_poslt32:
  shrd eax, edx, cl
  if(mv flag required)
  {
    sar edx, cl
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_posge32:
  mov eax, edx
  sub ecx, 32
  sar eax, ecx
  if(mv flag required)
  {
    mov ebp, CC_MV
    cmp eax, 0
    cmovz ebp, eax
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_neg:
  xor ebp, ebp
  cmp ecx, 32
  cmovz eax, edx
  cmovz ecx, ebp
  shld edx, eax, cl
  shl eax, cl
  if(mv flag required)
  {
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
mulscalarshiftacshift_exit:
  mov [dest], eax
*/

  vars->patchMgr->Reset();

  vars->codeCache->X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg_eax);
  vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, acshiftReadBaseReg, x86IndexReg_none, x86Scale_1, acshiftDisp);
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->codeCache->X86Emit_TESTIR(0x40,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_Z,l_pos);
  vars->codeCache->X86Emit_NEGR(x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(128,x86Reg_ecx);
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_neg);
  //l_pos:
  vars->patchMgr->SetLabelPointer(l_pos,vars->GetEmitLoc());
  vars->codeCache->X86Emit_CMPIR(32,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->patchMgr->SetLabelPointer(l_poslt32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_SHRDRRR(x86Reg_eax, x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_SARRR(x86Reg_edx);
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_posge32:
  vars->patchMgr->SetLabelPointer(l_posge32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRR(x86Reg_eax, x86Reg_edx);
  vars->codeCache->X86Emit_SUBIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMPIR(0, x86Reg_eax);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_eax);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_neg:
  vars->patchMgr->SetLabelPointer(l_neg,vars->GetEmitLoc());
  vars->codeCache->X86Emit_XORRR(x86Reg_ebp, x86Reg_ebp);
  vars->codeCache->X86Emit_CMPIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLDRRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->patchMgr->SetLabelPointer(l_exit,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->patchMgr->ApplyPatches();
}

void Emit_MULScalarShiftScalar(EmitterVariables *vars, Nuance &nuance)
{
  const uint32 l_pos = 0;
  const uint32 l_neg = 1;
  const uint32 l_poslt32 = 2;
  const uint32 l_posge32 = 3;
  const uint32 l_exit = 4;

  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  uint32 shiftRegIndex = nuance.fields[FIELD_MUL_INFO];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg shiftRegReadBaseReg = GetScalarRegReadBaseReg(vars,shiftRegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 shiftRegDisp = GetScalarRegEmitDisp(vars,shiftRegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);


/*
  mov eax, [src1]
  and [cc], ~CC_MV
  imul [src2]
  mov ecx, [shiftReg]
  test ecx, 0x40
  jz mulscalarshiftacshift_pos
  neg ecx
  add ecx, 128
  jmp mulscalarshiftacshift_neg
mulscalarshiftacshift_pos:
  cmp ecx, 32
  jnl mulscalarshiftacshift_posge32
mulscalarshiftacshift_poslt32:
  shrd eax, edx, cl
  if(mv flag required)
  {
    sar edx, cl
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_posge32:
  mov eax, edx
  sub ecx, 32
  sar eax, ecx
  if(mv flag required)
  {
    mov ebp, CC_MV
    cmp eax, 0
    cmovz ebp, eax
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_neg:
  xor ebp, ebp
  cmp ecx, 32
  cmovz eax, edx
  cmovz ecx, ebp
  shld edx, eax, cl
  shl eax, cl
  if(mv flag required)
  {
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
mulscalarshiftacshift_exit:
  mov [dest], eax
*/

  vars->patchMgr->Reset();

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, shiftRegReadBaseReg, x86IndexReg_none, x86Scale_1, shiftRegDisp);
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->codeCache->X86Emit_TESTIR(0x40,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_Z,l_pos);
  vars->codeCache->X86Emit_NEGR(x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(128,x86Reg_ecx);
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_neg);
  //l_pos:
  vars->patchMgr->SetLabelPointer(l_pos,vars->GetEmitLoc());
  vars->codeCache->X86Emit_CMPIR(32,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->patchMgr->SetLabelPointer(l_poslt32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_SHRDRRR(x86Reg_eax, x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_SARRR(x86Reg_edx);
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_posge32:
  vars->patchMgr->SetLabelPointer(l_posge32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRR(x86Reg_eax, x86Reg_edx);
  vars->codeCache->X86Emit_SUBIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMPIR(0, x86Reg_eax);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_eax);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_neg:
  vars->patchMgr->SetLabelPointer(l_neg,vars->GetEmitLoc());
  vars->codeCache->X86Emit_XORRR(x86Reg_ebp, x86Reg_ebp);
  vars->codeCache->X86Emit_CMPIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLDRRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->patchMgr->SetLabelPointer(l_exit,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->patchMgr->ApplyPatches();
}

void Emit_MULImmediateShiftScalar(EmitterVariables *vars, Nuance &nuance)
{
  const uint32 l_pos = 0;
  const uint32 l_neg = 1;
  const uint32 l_poslt32 = 2;
  const uint32 l_posge32 = 3;
  const uint32 l_exit = 4;

  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  uint32 shiftRegIndex = nuance.fields[FIELD_MUL_INFO];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg shiftRegReadBaseReg = GetScalarRegReadBaseReg(vars,shiftRegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 shiftRegDisp = GetScalarRegEmitDisp(vars,shiftRegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);


/*
  mov eax, src1
  and [cc], ~CC_MV
  imul [src2]
  mov ecx, [shiftReg]
  test ecx, 0x40
  jz mulscalarshiftacshift_pos
  neg ecx
  add ecx, 128
  jmp mulscalarshiftacshift_neg
mulscalarshiftacshift_pos:
  cmp ecx, 32
  jnl mulscalarshiftacshift_posge32
mulscalarshiftacshift_poslt32:
  shrd eax, edx, cl
  if(mv flag required)
  {
    sar edx, cl
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_posge32:
  mov eax, edx
  sub ecx, 32
  sar eax, ecx
  if(mv flag required)
  {
    mov ebp, CC_MV
    cmp eax, 0
    cmovz ebp, eax
    or [cc], ebp
  }
  jmp mulscalarshiftacshift_exit
mulscalarshiftacshift_neg:
  xor ebp, ebp
  cmp ecx, 32
  cmovz eax, edx
  cmovz ecx, ebp
  shld edx, eax, cl
  shl eax, cl
  if(mv flag required)
  {
    xor ecx, ecx
    test eax, 0x80000000
    mov ebx, 1
    mov ebp, CC_MV
    cmovz ebx, ecx
    add edx, ebx
    cmovz ebp, edx
    or [cc], ebp
  }
mulscalarshiftacshift_exit:
  mov [dest], eax
*/

  vars->patchMgr->Reset();

  vars->codeCache->X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg_eax);
  vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, shiftRegReadBaseReg, x86IndexReg_none, x86Scale_1, shiftRegDisp);
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->codeCache->X86Emit_TESTIR(0x40,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_Z,l_pos);
  vars->codeCache->X86Emit_NEGR(x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(128,x86Reg_ecx);
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_neg);
  //l_pos:
  vars->patchMgr->SetLabelPointer(l_pos,vars->GetEmitLoc());
  vars->codeCache->X86Emit_CMPIR(32,x86Reg_ecx);
  vars->codeCache->X86Emit_JCC_Label(vars->patchMgr,X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->patchMgr->SetLabelPointer(l_poslt32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_SHRDRRR(x86Reg_eax, x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_SARRR(x86Reg_edx);
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_posge32:
  vars->patchMgr->SetLabelPointer(l_posge32,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRR(x86Reg_eax, x86Reg_edx);
  vars->codeCache->X86Emit_SUBIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMPIR(0, x86Reg_eax);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_eax);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_JMPI_Label(vars->patchMgr,l_exit);
  //l_neg:
  vars->patchMgr->SetLabelPointer(l_neg,vars->GetEmitLoc());
  vars->codeCache->X86Emit_XORRR(x86Reg_ebp, x86Reg_ebp);
  vars->codeCache->X86Emit_CMPIR(32, x86Reg_ecx);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLDRRR(x86Reg_edx, x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
    vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
    vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
    vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
    vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->patchMgr->SetLabelPointer(l_exit,vars->GetEmitLoc());
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->patchMgr->ApplyPatches();
}

void Emit_MULImmediateShiftRightImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  /*
  eax = src1
  and [cc], ~CC_MV
  imul [src2]
  if(shiftCount == 32)
  {
    mov [dest], edx
    //overflow cant occur
  }
  else
  {
    shrd eax, edx, shiftCount
    if(mv flag required)
    {
      sar edx, shiftCount
      xor ecx, ecx
      test eax, 0x80000000
      mov ebx, 1
      mov ebp, CC_MV
      cmovz ebx, ecx
      add edx, ebx
      cmovz ebp, edx
      or [cc], ebp
    }
    mov [dest], eax
  }
*/

  vars->codeCache->X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  }
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] < 32)
  {
    vars->codeCache->X86Emit_SHRDIRR(x86Reg_eax, x86Reg_edx, nuance.fields[FIELD_MUL_INFO]);
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->codeCache->X86Emit_SARIR(x86Reg_edx, nuance.fields[FIELD_MUL_INFO]);
      vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
      vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
      vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
      vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
      vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
  else if(nuance.fields[FIELD_MUL_INFO] == 32)
  {
    vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
  else
  {
    vars->codeCache->X86Emit_MOVRR(x86Reg_eax, x86Reg_edx);
    vars->codeCache->X86Emit_SARIR(x86Reg_eax, nuance.fields[FIELD_MUL_INFO] - 32);
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}

void Emit_MULImmediateShiftLeftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

/*
  mov eax, src1
  and [cc], ~CC_MV
  imul [src2]
  if(shiftCount > 31)
  {     
    mov [dest], 0
    if(mv flag required)
    {
      shl eax, shiftCount - 32
      mov ebp, CC_MV
      cmovz ebp, eax
      or [cc], ebp
    }
  }
  else
  {
    if(shiftCount != 0)
    {
      shld edx, eax, shiftCount
      shl eax, shiftCount
    }

    if(mv flag required)
    {
      xor ecx, ecx
      test eax, 0x80000000
      mov ebx, 1
      mov ebp, CC_MV
      cmovz ebx, ecx
      add edx, ebx
      cmovz ebp, edx
      or [cc], ebp
    }
    mov [dest], eax
  }
*/

  vars->codeCache->X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg_eax);
  vars->codeCache->X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
  vars->codeCache->X86Emit_IMULMR(x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] > 31)
  {
    vars->codeCache->X86Emit_MOVIM(0, x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    if(vars->miscRegDep & DEPENDENCY_FLAG_MV)
    {
      vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
      vars->codeCache->X86Emit_TESTRR(x86Reg_eax, x86Reg_eax);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_eax);
      vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
  }
  else
  {
    if(nuance.fields[FIELD_MUL_INFO] != 0)
    {
      vars->codeCache->X86Emit_SHLDIRR(x86Reg_edx, x86Reg_eax, nuance.fields[FIELD_MUL_INFO]);
      vars->codeCache->X86Emit_SHLIR(x86Reg_eax, nuance.fields[FIELD_MUL_INFO]);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->codeCache->X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
      vars->codeCache->X86Emit_TESTIR(0x80000000, x86Reg_eax);
      vars->codeCache->X86Emit_MOVIR(1, x86Reg_ebx);
      vars->codeCache->X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg_ebp);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->codeCache->X86Emit_ADDRR(x86Reg_edx, x86Reg_ebx);
      vars->codeCache->X86Emit_CMOVZRR(x86Reg_ebp, x86Reg_edx);
      vars->codeCache->X86Emit_ORRM(x86Reg_ebp, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }
}

void Emit_MUL_SVImmediateShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];


/*
  mov ebp, scalar
  mov eax, [src2+0]
  mov ebx, [src2+4]
  mov ecx, [src2+8]
  mov edx, [src2+12]
  sar eax, 16
  sar ebx, 16
  sar ecx, 16
  sar edx, 16
  imul eax, eax, scalar
  imul ebx, ebx, scalar
  imul ecx, ecx, scalar
  imul edx, edx, scalar
  if(shift > 0)
  {
    shl eax, shift
    shl ebx, shift
    shl ecx, shift
    shl edx, shift
  }
  mov [src2+0], eax
  mov [src2+4], ebx
  mov [src2+8], ecx
  mov [src2+12], edx
*/

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_eax, scalar, x86Reg_eax);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_ebx, scalar, x86Reg_ebx);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_ecx, scalar, x86Reg_ecx);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_edx, scalar, x86Reg_edx);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_edx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
}

void Emit_MUL_SVScalarShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_edx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
}

void Emit_MUL_SVScalarShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebp, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_esi);
  vars->codeCache->X86Emit_SHRRR(x86Reg_esi);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_esi);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_esi);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebp, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
  vars->codeCache->X86Emit_MOVIR(vars->regBase, x86Reg_esi);
}
void Emit_MUL_SVRuShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

 //scalar = (int32)(entry.pIndexRegs[2] >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;

  //ebp = ((int32)ru) >> (2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, ruRegReadBaseReg, x86IndexReg_none, x86Scale_1, ruDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_edx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
}
void Emit_MUL_SVRuShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = (int32)ru >> ((2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, ruRegReadBaseReg, x86IndexReg_none, x86Scale_1, ruDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebp, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_esi);
  vars->codeCache->X86Emit_SHRRR(x86Reg_esi);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_esi);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_esi);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebp, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
  vars->codeCache->X86Emit_MOVIR(vars->regBase, x86Reg_esi);
}
void Emit_MUL_SVRvShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  //ebp = (int32)rv >> ((2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, rvRegReadBaseReg, x86IndexReg_none, x86Scale_1, rvDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_edx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
}

void Emit_MUL_SVRvShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = (int32)rv >> ((2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, rvRegReadBaseReg, x86IndexReg_none, x86Scale_1, rvDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebp, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_esi);
  vars->codeCache->X86Emit_SHRRR(x86Reg_esi);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_esi);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_esi);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebp, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
  vars->codeCache->X86Emit_MOVIR(vars->regBase, x86Reg_esi);
}

void Emit_MUL_SVVectorShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  uint32 shiftCount = nuance.fields[FIELD_MUL_INFO];

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src1RegDisp+12);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);

  vars->codeCache->X86Emit_MOVIR(16, x86Reg_eax);
  vars->codeCache->X86Emit_MOVIR(nuance.fields[FIELD_MUL_INFO], x86Reg_ecx);
  vars->codeCache->X86Emit_SHRRR(x86Reg_eax);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_eax);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
}

void Emit_MUL_SVVectorShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src1RegDisp+12);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);

  vars->codeCache->X86Emit_MOVIR(16, x86Reg_eax);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_eax);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_eax);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  vars->codeCache->X86Emit_SHLRM(x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
}

void Emit_MUL_PImmediateShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];


/*
  mov ebp, scalar
  mov eax, [src2+0]
  mov ebx, [src2+4]
  mov ecx, [src2+8]
  sar eax, 16
  sar ebx, 16
  sar ecx, 16
  imul eax, eax, scalar
  imul ebx, ebx, scalar
  imul ecx, ecx, scalar
  if(shift > 0)
  {
    shl eax, shift
    shl ebx, shift
    shl ecx, shift
  }
  mov [src2+0], eax
  mov [src2+4], ebx
  mov [src2+8], ecx
*/

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_eax, scalar, x86Reg_eax);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_ebx, scalar, x86Reg_eax);
  vars->codeCache->X86Emit_IMULIRR(x86Reg_ecx, scalar, x86Reg_eax);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
  }
  vars->codeCache->X86Emit_MOVIR(scalar, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PScalarShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_MUL_PScalarShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_MUL_PRuShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

 //scalar = (int32)(entry.pIndexRegs[2] >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;

  //ebp = ((int32)ru >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, ruRegReadBaseReg, x86IndexReg_none, x86Scale_1, ruDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_MUL_PRuShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = ((int32)ru >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, ruRegReadBaseReg, x86IndexReg_none, x86Scale_1, ruDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_MUL_PRvShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  //ebp = ((int32)rv >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, rvRegReadBaseReg, x86IndexReg_none, x86Scale_1, rvDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PRvShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = ((int32)rv >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, rvRegReadBaseReg, x86IndexReg_none, x86Scale_1, rvDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg_none, x86Scale_1, uvctlDisp);
  vars->codeCache->X86Emit_SHRIR(x86Reg_ecx, 24);
  vars->codeCache->X86Emit_ANDIR(0x07, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDIR(2, x86Reg_ecx);
  vars->codeCache->X86Emit_SARRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(0x3FFFUL, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_MUL_PVectorShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ecx, shift);
    vars->codeCache->X86Emit_SHLIR(x86Reg_ebx, shift);
  }
  vars->codeCache->X86Emit_MOVIR(vars->regBase, x86Reg_esi);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_MUL_PVectorShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);

  vars->codeCache->X86Emit_MOVIR(16, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_SHLRR(x86Reg_edx);
  vars->codeCache->X86Emit_SHLRR(x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
  vars->codeCache->X86Emit_MOVRM(x86Reg_edx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->codeCache->X86Emit_MOVRM(x86Reg_ebp, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}
void Emit_DOTPScalarShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3); 
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_ADDRR(x86Reg_ecx, x86Reg_edx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ecx);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
}
void Emit_DOTPScalarShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3); 
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ebp);
  vars->codeCache->X86Emit_ADDRR(x86Reg_ecx, x86Reg_edx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVIR(16, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
}
void Emit_DOTPVectorShiftImmediate(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src1RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_edx, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_edx);
  if(shift > 0)
  {
    vars->codeCache->X86Emit_SHLIR(x86Reg_eax, shift);
  }
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
}
void Emit_DOTPVectorShiftSvshift(EmitterVariables *vars, Nuance &nuance)
{
  uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->codeCache->X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp+0);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->codeCache->X86Emit_MOVMR(x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->codeCache->X86Emit_SARIR(x86Reg_eax, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_edx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_eax, x86Reg_ebx);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ecx, x86Reg_edx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ecx);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
  vars->codeCache->X86Emit_MOVMR(x86Reg_esi, src1RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src1RegDisp+12);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebx, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ebp, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_esi, 16);
  vars->codeCache->X86Emit_SARIR(x86Reg_ecx, 16);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_ebx, x86Reg_ebp);
  vars->codeCache->X86Emit_IMULRRR(x86Reg_esi, x86Reg_ecx);
  vars->codeCache->X86Emit_ADDRR(x86Reg_ebx, x86Reg_esi);
  vars->codeCache->X86Emit_MOVIR(vars->regBase, x86Reg_esi);
  vars->codeCache->X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);

  vars->codeCache->X86Emit_MOVIR(16, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVMR(x86Reg_ecx, svshiftReadBaseReg, x86IndexReg_none, x86Scale_1, svshiftDisp);
  vars->codeCache->X86Emit_SHRRR(x86Reg_ebp);
  vars->codeCache->X86Emit_ANDIR(~0x04, x86Reg_ebp);
  vars->codeCache->X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebp);
  vars->codeCache->X86Emit_SHLRR(x86Reg_eax);
  vars->codeCache->X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+0);
}