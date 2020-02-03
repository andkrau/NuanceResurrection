#include "basetypes.h"
#include "EmitMisc.h"
#include "InstructionDependencies.h"
#include "SuperBlockConstants.h"
#include "X86EmitTypes.h"
#include "mpe.h"

void Emit_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  //Nothing to emit, so just return
  return;
}

void Emit_ExitBlock(EmitterVariables * const vars)
{
  if(vars->bSaveRegs)
  {
    vars->codeCache->X86Emit_EMMS();
  }

  vars->codeCache->X86Emit_POPAD();
  vars->codeCache->X86Emit_RETN();
}

void Emit_SaveRegs(EmitterVariables * const vars, const Nuance &nuance)
{
  uint32 testMask = 0xF;

  int32 offset = -128;

  for(int32 i = 8; i > 0; i--)
  {
    if(testMask & nuance.fields[1])
    {
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm0, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, offset);
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm1, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, offset+8);

      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm0, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, offset);
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm1, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, offset+8);
    }
    offset += 16;
    testMask <<= 4;
  }

  if(nuance.fields[2] & DEPENDENCY_FLAG_ALLFLAGS)
  {
    vars->codeCache->X86Emit_MOVMR(x86Reg_eax, (uint32)&(vars->mpe->cc));
    vars->codeCache->X86Emit_MOVRM(x86Reg_eax, (uint32)&(vars->mpe->tempCC));
  }

  if(nuance.fields[2])
  {
    if(nuance.fields[2] & ~DEPENDENCY_FLAG_ALLFLAGS)
    {
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm0, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 0*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm1, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 1*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm2, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 2*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm3, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 3*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm4, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 4*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm5, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 5*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm6, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 6*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQMR(x86Reg_mm7, x86BaseReg_esi, x86IndexReg_none, x86Scale_1, 7*sizeof(uint64));

      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm0, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 0*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm1, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 1*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm2, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 2*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm3, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 3*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm4, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 4*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm5, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 5*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm6, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 6*sizeof(uint64));
      vars->codeCache->X86Emit_MOVQRM(x86Reg_mm7, x86BaseReg_edi, x86IndexReg_none, x86Scale_1, 7*sizeof(uint64));
    }
  }
}

void Emit_StoreScalarRegisterConstant(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_CONSTANT_ADDRESS];
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  const uint32 flagMask = nuance.fields[FIELD_CONSTANT_FLAGMASK];
  const uint32 flagValues = (nuance.fields[FIELD_CONSTANT_FLAGVALUES] & flagMask);

  vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_CONSTANT_VALUE], x86MemPtr_dword, destRegWriteBaseReg,x86IndexReg_none, x86Scale_1, destRegDisp);
  if(flagMask)
  {
    //If any of the flags represented by the mask are to be set to 0, clear all of the flags first
    if(~flagValues)
    {
      vars->codeCache->X86Emit_ANDIM(~flagMask, x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }

    //If any of the flags represented by the mask are to be set to 1, set the flags
    if(flagValues)
    {
      vars->codeCache->X86Emit_ORIM(flagValues, x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
  }
}

void Emit_StoreMiscRegisterConstant(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = nuance.fields[FIELD_CONSTANT_ADDRESS];
  const x86BaseReg destRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 destRegDisp = GetMiscRegEmitDisp(vars,destRegIndex+1);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  const uint32 flagMask = nuance.fields[FIELD_CONSTANT_FLAGMASK];
  const uint32 flagValues = (nuance.fields[FIELD_CONSTANT_FLAGVALUES] & flagMask);

  if(destRegIndex != CONSTANT_REG_DISCARD)
  {
    switch(destRegIndex)
    {
      case CONSTANT_REG_RC0:
        //if(nuance.fields[FIELD_CONSTANT_VALUE])
        {
        }
      case CONSTANT_REG_RX:
      case CONSTANT_REG_RY:
      case CONSTANT_REG_RU:
      case CONSTANT_REG_RV:
      case CONSTANT_REG_RC1:
      case CONSTANT_REG_RZ:
      case CONSTANT_REG_XYCTL:
      case CONSTANT_REG_UVCTL:
      case CONSTANT_REG_XYRANGE:
      case CONSTANT_REG_UVRANGE:
      case CONSTANT_REG_ACSHIFT:
      case CONSTANT_REG_SVSHIFT:
        vars->codeCache->X86Emit_MOVIM(nuance.fields[FIELD_CONSTANT_VALUE], x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg_none, x86Scale_1, destRegDisp);
        break;
    }
  }
  
  if(flagMask)
  {
    //If any of the flags represented by the mask are to be set to 0, clear all of the flags first
    if(~flagValues)
    {
      vars->codeCache->X86Emit_ANDIM(~flagMask, x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }

    //If any of the flags represented by the mask are to be set to 1, set the flags
    if(flagValues)
    {
      vars->codeCache->X86Emit_ORIM(flagValues, x86MemPtr_dword, ccWriteBaseReg, x86IndexReg_none, x86Scale_1, ccDisp);
    }
  }
}
