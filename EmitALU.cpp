#include "basetypes.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "EmitMisc.h"
#include "NativeCodeCache.h"
#include "PatchManager.h"
#include "mpe.h"
#include "SuperBlock.h"

static const uint32 sub_sv_mask[] = {0xFFFF0000UL, 0xFFFF0000UL};

void Emit_ABS(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  //eax = src1
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  //ebx = src1
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  //ebp = src1
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebp, x86Reg_eax);
  // ebx = 0 if src1 is positive and -1 if src1 is negative
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 31);
  // eax = ~src1 if src1 is negative and src1 if src1 is positve
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_ebx);
  // eax = ~src1 + 1 if src1 is negative and src1 if src1 is positive
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_eax, x86Reg_ebx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ebx, 30);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);
  }

  if(vars->miscRegOutDep & (DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V))
  {
    if((vars->miscRegOutDep & (DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V)) == DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ebx, x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_NEGATIVE, x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_CMPIR(0x80000000, x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);
    }
    else if((vars->miscRegOutDep & (DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V)) == DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ebx, x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_OVERFLOW, x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_CMPIR(0x80000000, x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);
    }
    else
    {
      vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ebx, x86Reg_ebx);
      vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_NEGATIVE|CC_ALU_OVERFLOW, x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_CMPIR(0x80000000, x86Reg_ebp);
      vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
      vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);
    }
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebp, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_BITSScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src1Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x1F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_eax);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_BITSImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ebx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(src1Imm, x86Reg_ebx);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_BTST(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 mask = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(mask, x86Reg_ebx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_BUTT(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);

  if(vars->scalarRegOutDep & (1 << (destRegIndex+1)))
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_eax, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  }

  vars->mpe->nativeCodeCache.X86Emit_ADDMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep & (1 << destRegIndex))
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_COPY(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_MSB(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  //n = (n ^ (n >> 31))
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 31);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_ebx);
  
  //fold n into itself to get a new value where all bits below the
  //most significant one bit have also been set to one.

  //n = n | (n >> 1)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 1);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);

  //n = n | (n >> 2)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 2);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);

  //n = n | (n >> 4)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 4);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);

  //n = n | (n >> 8)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 8);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);

  //n = n | (n >> 16)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 16);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebx);

  //get the ones count

  //n = n - ((n >> 1) & 0x55555555)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 1);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x55555555, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_eax, x86Reg_ebx);
  
  //n = (((n >> 2) & 0x33333333) + (n & 0x33333333))
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 2);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x33333333, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x33333333, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);

  //n = (((n >> 4) + n) & 0x0f0f0f0f)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 4);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x0F0F0F0F, x86Reg_eax);

  //n = n + (n >> 8)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, 8);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);

  //n = n + (n >> 16)
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_eax, 16);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ebx, x86Reg_eax);

  //sigbits = n & 0x1F
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x1F, x86Reg_ebx);

  //dest = sigbits
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~CC_ALU_ZERO, x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SAT(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const int32 mask = (0x01UL << nuance.fields[FIELD_ALU_SRC2]) - 1;
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  //const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(mask, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMPRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNLERR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_NOTR(x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMPRR(x86Reg_ebx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_CMOVLRR(x86Reg_ebx, x86Reg_eax);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADD_SV(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if( ((src1RegReadBaseReg_0 == src1RegReadBaseReg_1) && (src1RegReadBaseReg_2 == src1RegReadBaseReg_3)) &&
      ((src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3)) )
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm4, (uint32)sub_sv_mask);
    vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm0, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm1, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm2, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm3, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm0, x86Reg_mm4);
    vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm1, x86Reg_mm4);
    vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm2, x86Reg_mm4);
    vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm3, x86Reg_mm4);
    vars->mpe->nativeCodeCache.X86Emit_PADDRR(x86Reg_mm0, x86Reg_mm1);
    vars->mpe->nativeCodeCache.X86Emit_PADDRR(x86Reg_mm2, x86Reg_mm3);
    vars->mpe->nativeCodeCache.X86Emit_MOVQRM(x86Reg_mm0, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVQRM(x86Reg_mm2, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
    vars->bUsesMMX = true;
  }
  else
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(0xFFFF0000, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_eax, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ecx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src1RegDisp+12);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_eax, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ecx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ebx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_eax, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
  }
}

void Emit_ADD_P(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(0xFFFF0000, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_eax, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ecx, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_eax, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ebx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
}

void Emit_SUB_SV(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src1RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src1RegIndex+3);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_3 = GetScalarRegReadBaseReg(vars,src2RegIndex+3);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(src2RegIndex != src1RegIndex)
  {
    if( ((src1RegReadBaseReg_0 == src1RegReadBaseReg_1) && (src1RegReadBaseReg_2 == src1RegReadBaseReg_3)) &&
        ((src2RegReadBaseReg_0 == src2RegReadBaseReg_1) && (src2RegReadBaseReg_2 == src2RegReadBaseReg_3)) )
      {
        vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm4, (uint32)sub_sv_mask);
        vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm0, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp);
        vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm1, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp);
        vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm2, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
        vars->mpe->nativeCodeCache.X86Emit_MOVQMR(x86Reg_mm3, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
        vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm0, x86Reg_mm4);
        vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm1, x86Reg_mm4);
        vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm2, x86Reg_mm4);
        vars->mpe->nativeCodeCache.X86Emit_PANDRR(x86Reg_mm3, x86Reg_mm4);
        vars->mpe->nativeCodeCache.X86Emit_PSUBDRR(x86Reg_mm0, x86Reg_mm1);
        vars->mpe->nativeCodeCache.X86Emit_PSUBDRR(x86Reg_mm2, x86Reg_mm3);
        vars->mpe->nativeCodeCache.X86Emit_MOVQRM(x86Reg_mm0, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
        vars->mpe->nativeCodeCache.X86Emit_MOVQRM(x86Reg_mm2, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
        vars->bUsesMMX = true;
      }
      else
      {
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
        vars->mpe->nativeCodeCache.X86Emit_MOVIR(0xFFFF0000, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_eax, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ecx, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_eax, x86Reg_ebx);
        vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ecx, x86Reg_edx);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
        vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src2RegDisp+12);
        vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
        vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg_3, x86IndexReg_none, x86Scale_1, src1RegDisp+12);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_eax, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ecx, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
        vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ebx, x86Reg_edx);
        vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_eax, x86Reg_ecx);
        vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
        vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
    }
  }
  else
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+12);
  }
}

void Emit_SUB_P(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src1RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src1RegIndex+1);
  const x86BaseReg src1RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src1RegIndex+2);
  const x86BaseReg src2RegReadBaseReg_0 = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg src2RegReadBaseReg_1 = GetScalarRegReadBaseReg(vars,src2RegIndex+1);
  const x86BaseReg src2RegReadBaseReg_2 = GetScalarRegReadBaseReg(vars,src2RegIndex+2);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);

  if(src2RegIndex != src1RegIndex)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, src2RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src2RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg_0, x86IndexReg_none, x86Scale_1, src1RegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src2RegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_1, x86IndexReg_none, x86Scale_1, src1RegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(0xFFFF0000, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_eax, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ecx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_eax, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src2RegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_edx, src1RegReadBaseReg_2, x86IndexReg_none, x86Scale_1, src1RegDisp+8);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ecx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_edx, x86Reg_ebp);
    vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ebx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  }
  else
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+4);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_eax, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp+8);
  }
}

void Emit_ANDScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ANDImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(src1Imm, x86Reg_ebx);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ANDImmediateShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src1Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ANDScalarShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ANDScalarRotateScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_rol = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_rol);

  vars->mpe->nativeCodeCache.X86Emit_RORRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_rol,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ROLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ANDScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ANDScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_FTSTImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(src1Imm, x86Reg_ebx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_FTSTScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_FTSTImmediateShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src1Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_FTSTScalarShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_FTSTScalarRotateScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_rol = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_rol);

  vars->mpe->nativeCodeCache.X86Emit_RORRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_rol,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ROLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  vars->mpe->nativeCodeCache.X86Emit_ANDMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_FTSTScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_FTSTScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ANDRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ORImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ORIR(src1Imm, x86Reg_ebx);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ORScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ORMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ORImmediateShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src1Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ORMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ORScalarShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ORMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ORScalarRotateScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_rol = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_rol);

  vars->mpe->nativeCodeCache.X86Emit_RORRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_rol,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ROLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ORMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ORScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ORScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_EORImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_XORIR(src1Imm, x86Reg_ebx);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_EORScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_XORMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 28);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_NEGATIVE, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ecx, x86Reg_ecx);
    vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_ALU_ZERO, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);
    vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ecx, x86Reg_edx);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ecx);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_EORImmediateShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src1Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_XORMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_EORScalarShiftScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_shl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_shl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_shl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  vars->mpe->nativeCodeCache.X86Emit_XORMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_EORScalarRotateScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_rol = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_rol);

  vars->mpe->nativeCodeCache.X86Emit_RORRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_rol,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ROLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  vars->mpe->nativeCodeCache.X86Emit_XORMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_EORScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_EORScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ADDIR(src1Imm, x86Reg_ebx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
    
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ADDMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(src1Imm, x86Reg_ebx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
    
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBImmediateReverse(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src2Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
    
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(src1Imm, x86Reg_ebx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
    
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPImmediateReverse(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src2Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SUBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
    
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ROT(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_rol = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_rol);

  vars->mpe->nativeCodeCache.X86Emit_RORRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_rol,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_ROLRR(x86Reg_ebx);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ROL(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_ROLIR(x86Reg_ebx, src1Imm);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ROR(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_RORIR(x86Reg_ebx, src1Imm);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_AS(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_asl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebp, x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_asl);

  vars->mpe->nativeCodeCache.X86Emit_SARRR(x86Reg_ebx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ebp, 1);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ebp);
  }

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_asl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_edx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_edx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ebp, 30);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ebp);
  }

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_ASL(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ebx, src1Imm);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ecx, 30);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ecx);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ASR(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ebx, src1Imm);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, 1);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ecx);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_LS(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 l_asl = 0;
  const uint32 l_calc_flags = 1;

  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(64,x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x3F, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg_edx, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ebp, x86Reg_ebx);

  vars->mpe->nativeCodeCache.X86Emit_TESTIR(0x20, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNZRR(x86Reg_ecx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(vars->mpe->nativeCodeCache.patchMgr,X86_CC_NZ,l_asl);

  vars->mpe->nativeCodeCache.X86Emit_SHRRR(x86Reg_ebx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ebp, 1);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ebp);
  }

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(vars->mpe->nativeCodeCache.patchMgr,l_calc_flags);

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_asl,vars->mpe->nativeCodeCache.GetEmitPointer());

  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_edx, x86Reg_edx);
  vars->mpe->nativeCodeCache.X86Emit_SHLRR(x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(32, x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVZRR(x86Reg_ebx, x86Reg_edx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ebp, 30);
    vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ebp);
  }

  vars->mpe->nativeCodeCache.patchMgr.SetLabelPointer(l_calc_flags,vars->mpe->nativeCodeCache.GetEmitPointer());
  
  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_eax, x86Reg_ebp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }

  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();
}

void Emit_LSR(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  //const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_SHRIR(x86Reg_ebx, src1Imm);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_TESTRR(x86Reg_ebx, x86Reg_ebx);

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, 1);
      vars->mpe->nativeCodeCache.X86Emit_ANDIR(CC_ALU_CARRY, x86Reg_ecx);
    }

    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDWCImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_ADCIR(src1Imm, x86Reg_ebx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDWCScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_ADCMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDWCScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_edx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_edx, 2);
  vars->mpe->nativeCodeCache.X86Emit_ADCRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_ADDWCScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_edx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_edx, 2);
  vars->mpe->nativeCodeCache.X86Emit_ADCRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBWCImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBIR(src1Imm, x86Reg_ebx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBWCImmediateReverse(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src2Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBWCScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBWCScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_edx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_edx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_SUBWCScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_edx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_edx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->scalarRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg_ebx, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPWCImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1Imm = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBIR(src1Imm, x86Reg_ebx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPWCImmediateReverse(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(src2Imm, x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPWCScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  //const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2RegIndex = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg src2RegReadBaseReg = GetScalarRegReadBaseReg(vars,src2RegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 src2RegDisp = GetScalarRegEmitDisp(vars,src2RegIndex);
  //const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, src2RegReadBaseReg, x86IndexReg_none, x86Scale_1, src2RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_ecx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_ecx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBMR(x86Reg_ebx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPWCScalarShiftLeftImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_edx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_edx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}

void Emit_CMPWCScalarShiftRightImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_ALU_DEST];
  const uint32 src1RegIndex = nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2Imm = nuance.fields[FIELD_ALU_SRC2];
  const x86BaseReg src1RegReadBaseReg = GetScalarRegReadBaseReg(vars,src1RegIndex);
  const x86BaseReg destRegReadBaseReg = GetScalarRegReadBaseReg(vars,destRegIndex);
  //const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccRegReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg ccRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 src1RegDisp = GetScalarRegEmitDisp(vars,src1RegIndex);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccRegDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_eax, ccRegReadBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ebx, destRegReadBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg_ecx, src1RegReadBaseReg, x86IndexReg_none, x86Scale_1, src1RegDisp);
  vars->mpe->nativeCodeCache.X86Emit_SARIR(x86Reg_ecx, src2Imm);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg_edx, x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_RCRIR(x86Reg_edx, 2);
  vars->mpe->nativeCodeCache.X86Emit_SBBRR(x86Reg_ebx, x86Reg_ecx);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETSR(x86Reg_ch);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETOR(x86Reg_cl);
    }
    
    if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETBR(x86Reg_dh);
    }

    if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
    {
      vars->mpe->nativeCodeCache.X86Emit_SETZR(x86Reg_dl);
    }
  
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)), x86MemPtr_dword, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg_eax, x86Reg_eax);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_N)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_ch, 3);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_ch);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_V)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_cl, 2);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_cl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C)
  {
    vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg_dh, 1);
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dh);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRR(x86Reg_al, x86Reg_dl);
  }

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_NVCZ)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg_eax, ccRegWriteBaseReg, x86IndexReg_none, x86Scale_1, ccRegDisp);
  }
}
