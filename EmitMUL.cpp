#include "basetypes.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "EmitMisc.h"
#include "NativeCodeCache.h"
#include "PatchManager.h"
#include "mpe.h"

extern bool SSE41_supported;

static constexpr uint32 shiftTable[4] = {16, 8, 0, 2};

void Emit_ADDM(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_ADDMR(x86Reg::x86Reg_eax, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_ADDMImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_ADDIR(nuance.fields[FIELD_MUL_SRC1],x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_SUBM(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    if(src1RegIndex == src2RegIndex)
    {
      vars->mpe->nativeCodeCache.X86Emit_MOVIM(0, x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
    }
    else
    {
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
      vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg::x86Reg_eax, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
      vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
    }
  }
}

void Emit_SUBMImmediateReverse(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg::x86Reg_eax, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_MULScalarShiftAcshift(EmitterVariables * const vars, const Nuance &nuance)
{
  constexpr uint32 l_pos = 0;
  constexpr uint32 l_neg = 1;
  constexpr uint32 l_poslt32 = 2;
  constexpr uint32 l_posge32 = 3;
  constexpr uint32 l_exit = 4;

  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg acshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_ACSHIFT);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  const int32 acshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_ACSHIFT);

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

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, acshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, acshiftDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }

  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x7F, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x40,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z,l_pos);
  vars->mpe->nativeCodeCache.X86Emit_NEGR(x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(128,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_neg);
  //l_pos:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_pos);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_poslt32);
  vars->mpe->nativeCodeCache.X86Emit_SHRDRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_posge32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_posge32);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_neg:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_neg);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLDRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_exit);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_MULScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

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

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] < 32)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHRDIRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx, (uint8)nuance.fields[FIELD_MUL_INFO]);
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, (uint8)nuance.fields[FIELD_MUL_INFO]);
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
      vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else if(nuance.fields[FIELD_MUL_INFO] == 32)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, (uint8)(nuance.fields[FIELD_MUL_INFO] - 32));
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_MULScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

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

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] > 31)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIM(0, x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
  }
  else
  {
    if(nuance.fields[FIELD_MUL_INFO] != 0)
    {
      vars->mpe->nativeCodeCache.X86Emit_SHLDIRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax, (uint8)nuance.fields[FIELD_MUL_INFO]);
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, (uint8)nuance.fields[FIELD_MUL_INFO]);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
      vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_MULImmediateShiftAcshift(EmitterVariables * const vars, const Nuance &nuance)
{
  constexpr uint32 l_pos = 0;
  constexpr uint32 l_neg = 1;
  constexpr uint32 l_poslt32 = 2;
  constexpr uint32 l_posge32 = 3;
  constexpr uint32 l_exit = 4;

  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg acshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_ACSHIFT);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  const int32 acshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_ACSHIFT);

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

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, acshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, acshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);

  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x7F, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x40,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z,l_pos);
  vars->mpe->nativeCodeCache.X86Emit_NEGR(x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(128,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_neg);
  //l_pos:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_pos);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_poslt32);
  vars->mpe->nativeCodeCache.X86Emit_SHRDRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_posge32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_posge32);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMPIR(0, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_neg:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_neg);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLDRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_exit);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_MULScalarShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  constexpr uint32 l_pos = 0;
  constexpr uint32 l_neg = 1;
  constexpr uint32 l_poslt32 = 2;
  constexpr uint32 l_posge32 = 3;
  constexpr uint32 l_exit = 4;

  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const uint32 shiftRegIndex = nuance.fields[FIELD_MUL_INFO];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg shiftRegReadBaseReg = GetScalarRegReadBaseReg(vars,shiftRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 shiftRegDisp = GetScalarRegEmitDisp(vars,shiftRegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

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

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, shiftRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, shiftRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);

  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x7F, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x40,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z,l_pos);
  vars->mpe->nativeCodeCache.X86Emit_NEGR(x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(128,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_neg);
  //l_pos:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_pos);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_poslt32);
  vars->mpe->nativeCodeCache.X86Emit_SHRDRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_posge32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_posge32);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMPIR(0, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_neg:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_neg);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLDRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_exit);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_MULImmediateShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  constexpr uint32 l_pos = 0;
  constexpr uint32 l_neg = 1;
  constexpr uint32 l_poslt32 = 2;
  constexpr uint32 l_posge32 = 3;
  constexpr uint32 l_exit = 4;

  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  //const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const uint32 shiftRegIndex = nuance.fields[FIELD_MUL_INFO];
  //const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg shiftRegReadBaseReg = GetScalarRegReadBaseReg(vars,shiftRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  //const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 shiftRegDisp = GetScalarRegEmitDisp(vars,shiftRegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

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

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, shiftRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, shiftRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);

  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x7F, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x40,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_Z,l_pos);
  vars->mpe->nativeCodeCache.X86Emit_NEGR(x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(128,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_neg);
  //l_pos:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_pos);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32,x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NL,l_posge32);
  //l_poslt32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_poslt32);
  vars->mpe->nativeCodeCache.X86Emit_SHRDRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_posge32:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_posge32);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMPIR(0, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  //l_neg:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_neg);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLDRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  //l_exit:
  vars->mpe->nativeCodeCache.SetLabelPointer(l_exit);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  
  //Patch forward branch offsets
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_MULImmediateShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  //const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

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

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg::x86Reg_eax);
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] < 32)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHRDIRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx, (uint8)nuance.fields[FIELD_MUL_INFO]);
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, (uint8)nuance.fields[FIELD_MUL_INFO]);
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
      vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else if(nuance.fields[FIELD_MUL_INFO] == 32)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, (uint8)(nuance.fields[FIELD_MUL_INFO] - 32));
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_MULImmediateShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

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

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(nuance.fields[FIELD_MUL_SRC1], x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->pInstructionEntry->miscOutputDependencies)), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  vars->mpe->nativeCodeCache.X86Emit_IMULMR(x86MemPtr::x86MemPtr_dword, src2RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  if(nuance.fields[FIELD_MUL_INFO] > 31)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIM(0, x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
    if(vars->miscRegDep & DEPENDENCY_FLAG_MV)
    {
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
  }
  else
  {
    if(nuance.fields[FIELD_MUL_INFO] != 0)
    {
      vars->mpe->nativeCodeCache.X86Emit_SHLDIRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_eax, (uint8)nuance.fields[FIELD_MUL_INFO]);
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, (uint8)nuance.fields[FIELD_MUL_INFO]);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_MV)
    {
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x80000000, x86Reg::x86Reg_eax);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(1, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_MUL_OVERFLOW, x86Reg::x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_edx);
      vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ebp, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_MUL_SVImmediateShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  const uint32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

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

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(scalar, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  if(shift > 0)
    vars->mpe->nativeCodeCache.X86Emit_PSLDIR(x86Reg::x86Reg_xmm0, shift);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_eax, scalar, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_ebx, scalar, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_ecx, scalar, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_edx, scalar, x86Reg::x86Reg_edx);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_edx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  }
}

void Emit_MUL_SVScalarShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const uint32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  if(shift > 0)
    vars->mpe->nativeCodeCache.X86Emit_PSLDIR(x86Reg::x86Reg_xmm0, shift);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_edx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  }
}

void Emit_MUL_SVScalarShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_esi); // shifts by cl
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PSLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebp, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  }
}

void Emit_MUL_SVRuShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  //scalar = (int32)(entry.pIndexRegs[REG_U] >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;

  //ebp = ((int32)ru) >> (2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, ruRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ruDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  if(shift > 0)
    vars->mpe->nativeCodeCache.X86Emit_PSLDIR(x86Reg::x86Reg_xmm0, shift);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_edx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  }
}

void Emit_MUL_SVRuShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = (int32)ru >> ((2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, ruRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ruDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PSLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebp, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  }
}

void Emit_MUL_SVRvShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  //ebp = (int32)rv >> ((2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, rvRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rvDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  if(shift > 0)
    vars->mpe->nativeCodeCache.X86Emit_PSLDIR(x86Reg::x86Reg_xmm0, shift);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_edx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  }
}

void Emit_MUL_SVRvShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = (int32)rv >> ((2 + ((uvctl >> 24) & 0x07UL)) & 0x3FFFUL)

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, rvRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rvDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PSLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebp, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  }
}

void Emit_MUL_SVVectorShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  //const uint32 shiftCount = nuance.fields[FIELD_MUL_INFO];

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(nuance.fields[FIELD_MUL_INFO], x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
}

void Emit_MUL_SVVectorShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SHLRM(x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+12);
}

void Emit_MUL_PImmediateShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 scalar = ((int32)(nuance.fields[FIELD_MUL_SRC1])) >> 16;
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

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

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_eax, scalar, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_ebx, scalar, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_IMULIRR(x86Reg::x86Reg_ecx, scalar, x86Reg::x86Reg_eax);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(scalar, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PScalarShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PScalarShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PRuShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  //scalar = (int32)(entry.pIndexRegs[REG_U] >> (2 + BilinearInfo_XYMipmap(*entry.pUvctl))) & 0x3FFFUL;

  //ebp = ((int32)ru >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, ruRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ruDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PRuShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg ruRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RU);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 ruDisp = GetMiscRegEmitDisp(vars,REGINDEX_RU);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = ((int32)ru >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, ruRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ruDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PRvShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  //ebp = ((int32)rv >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, rvRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rvDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PRvShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  //const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg rvRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RV);
  const x86BaseReg uvctlRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVCTL);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 rvDisp = GetMiscRegEmitDisp(vars,REGINDEX_RV);
  const int32 uvctlDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVCTL);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  //ebp = ((int32)rv >> (2 + ((uvctl >> 24) & 0x07UL))) & 0x3FFFUL
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, rvRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rvDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, uvctlRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvctlDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg::x86Reg_ecx, 24);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x07, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(2, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3FFFUL, x86Reg::x86Reg_ebp);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PVectorShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ecx, shift);
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_ebx, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ecx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_MUL_PVectorShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_edx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebp, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+8);
}

void Emit_DOTPScalarShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3); 
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  const uint32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_PHADDDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm0);
  vars->mpe->nativeCodeCache.X86Emit_PHADDDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm0);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR2(x86Reg::x86Reg_eax, x86Reg::x86Reg_xmm0);
  if(shift > 0)
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
  }
}

void Emit_DOTPScalarShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3); 
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src1RegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp);

  if (SSE41_supported && (src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3) && (src2RegReadBaseReg_0 == src2RegReadBaseReg_2))
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_PSRADIR(x86Reg::x86Reg_xmm0, 16);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHUFIR(x86Reg::x86Reg_xmm1, x86Reg::x86Reg_xmm1, 0);
  vars->mpe->nativeCodeCache.X86Emit_PMULLDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm1);
  vars->mpe->nativeCodeCache.X86Emit_PHADDDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm0);
  vars->mpe->nativeCodeCache.X86Emit_PHADDDRR(x86Reg::x86Reg_xmm0, x86Reg::x86Reg_xmm0);
  vars->mpe->nativeCodeCache.X86Emit_MOVDRR2(x86Reg::x86Reg_eax, x86Reg::x86Reg_xmm0);
  }
  else
  {
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  }

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
}

void Emit_DOTPVectorShiftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 shift = shiftTable[nuance.fields[FIELD_MUL_INFO]];

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src1RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_edx, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_edx);
  if(shift > 0)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, shift);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
}

void Emit_DOTPVectorShiftSvshift(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_MUL_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_MUL_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_MUL_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg svshiftReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_SVSHIFT);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 svshiftDisp = GetMiscRegEmitDisp(vars,REGINDEX_SVSHIFT);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, src1RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src2RegReadBaseReg_0, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+0);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src1RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_edx, src2RegReadBaseReg_1, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_edx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, src1RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebp, src2RegReadBaseReg_2, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, src2RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src2RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_esi, src1RegReadBaseReg_3, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, src1RegDisp+12);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ebp, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_esi, 16);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg::x86Reg_ecx, 16);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_IMULRRR(x86Reg::x86Reg_esi, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR((uint32)&(vars->mpe->cc), x86Reg::x86Reg_esi);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(16, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ecx, svshiftReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, svshiftDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(~0x04, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp+0);
}
