#include "basetypes.h"
#include "EmitMisc.h"
#include "InstructionDependencies.h"
#include "mpe.h"
#include "SuperBlockConstants.h"
#include "X86EmitTypes.h"

void Emit_NOP(EmitterVariables * const vars, const Nuance &nuance)
{
  //Nothing to emit, so just return
  return;
}

void Emit_ExitBlock(const EmitterVariables * const vars)
{
  vars->mpe->nativeCodeCache.X86Emit_POPAD();
  vars->mpe->nativeCodeCache.X86Emit_RETN();
}

void Emit_SaveRegs(EmitterVariables * const vars, const Nuance &nuance)
{
  // tempCC snapshot invariant: the conditional CC->tempCC write
  // below is gated on (nuance.fields[2] & DEPENDENCY_FLAG_ALLFLAGS), while
  // subsequent ECU emitters (EmitConditionCheck in EmitECU.cpp) route their
  // CC read through GetMiscRegReadBaseReg(vars, REGINDEX_CC), which selects
  // edi (=&mpe->tempCC) iff (vars->miscRegDep & DEPENDENCY_FLAG_ALLFLAGS).
  // For the JIT to read fresh tempCC data, those two gating conditions must
  // be the same value. SuperBlock.cpp sets both nuance.fields[2] and
  // vars->miscRegDep from the same pInstruction->miscOpDependencies field,
  // so they're equal today. If a future change makes them diverge, the JIT
  // could route a CC read through edi without tempCC ever being snapshotted,
  // reading stale data from a prior packet!
  assert(vars->miscRegDep == (uint32)nuance.fields[2]
         && "Emit_SaveRegs: nuance.fields[2] must match vars->miscRegDep, "
            "or the tempCC snapshot gate diverges from the EmitConditionCheck "
            "routing gate (see comment above and GetMiscRegReadBaseReg in EmitMisc.h)");

  uint32 testMask = 0xF;

  int32 offset = -128;

  for(int32 i = 8; i > 0; i--)
  {
    if(testMask & nuance.fields[1])
    {
      vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, x86BaseReg::x86BaseReg_esi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, offset);

      vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, x86BaseReg::x86BaseReg_edi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, offset);
    }
    offset += 16;
    testMask <<= 4;
  }

  if(nuance.fields[2] & DEPENDENCY_FLAG_ALLFLAGS)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, (uintptr_t)&(vars->mpe->cc));
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, (uintptr_t)&(vars->mpe->tempCC));
  }

  if(nuance.fields[2])
  {
    if(nuance.fields[2] & ~DEPENDENCY_FLAG_ALLFLAGS)
    {
      vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm0, x86BaseReg::x86BaseReg_esi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 0*sizeof(uint128));
      vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm1, x86BaseReg::x86BaseReg_esi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 1*sizeof(uint128));
      vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm2, x86BaseReg::x86BaseReg_esi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 2*sizeof(uint128));
      vars->mpe->nativeCodeCache.X86Emit_MOVDQUMR(x86Reg::x86Reg_xmm3, x86BaseReg::x86BaseReg_esi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 3*sizeof(uint128));

      vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm0, x86BaseReg::x86BaseReg_edi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 0*sizeof(uint128));
      vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm1, x86BaseReg::x86BaseReg_edi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 1*sizeof(uint128));
      vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm2, x86BaseReg::x86BaseReg_edi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 2*sizeof(uint128));
      vars->mpe->nativeCodeCache.X86Emit_MOVDQURM(x86Reg::x86Reg_xmm3, x86BaseReg::x86BaseReg_edi, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, 3*sizeof(uint128));
    }
  }
}

void Emit_StoreScalarRegisterConstant(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = (uint32_t)nuance.fields[FIELD_CONSTANT_ADDRESS];
  const x86BaseReg destRegWriteBaseReg = GetScalarRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 destRegDisp = GetScalarRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  const uint32 flagMask = (uint32_t)nuance.fields[FIELD_CONSTANT_FLAGMASK];
  const uint32 flagValues = ((uint32_t)nuance.fields[FIELD_CONSTANT_FLAGVALUES] /*& flagMask*/); //see below

  vars->mpe->nativeCodeCache.X86Emit_MOVIM((uint32_t)nuance.fields[FIELD_CONSTANT_VALUE], x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg,x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  if(flagMask)
  {
    //If any of the flags represented by the mask are to be set to 0, clear all of the flags first
    if((~flagMask) != 0xFFFFFFFF) //was (~flagValues)
    {
      vars->mpe->nativeCodeCache.X86Emit_ANDIM(~flagMask, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }

    //If any of the flags represented by the mask are to be set to 1, set the flags
    if(flagValues)
    {
      vars->mpe->nativeCodeCache.X86Emit_ORIM(flagValues, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
  }
}

void Emit_StoreMiscRegisterConstant(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = (uint32_t)nuance.fields[FIELD_CONSTANT_ADDRESS];
  const x86BaseReg destRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,destRegIndex);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 destRegDisp = GetMiscRegEmitDisp(vars,destRegIndex+1);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);
  const uint32 flagMask = (uint32_t)nuance.fields[FIELD_CONSTANT_FLAGMASK];
  const uint32 flagValues = ((uint32_t)nuance.fields[FIELD_CONSTANT_FLAGVALUES] /*& flagMask*/); //see below

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
        vars->mpe->nativeCodeCache.X86Emit_MOVIM((uint32_t)nuance.fields[FIELD_CONSTANT_VALUE], x86MemPtr::x86MemPtr_dword, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
        break;
      default:
        assert(!"missing Emit_StoreMiscRegisterConstant");
        break;
    }
  }

  if(flagMask)
  {
    //If any of the flags represented by the mask are to be set to 0, clear all of the flags first
    if((~flagMask) != 0xFFFFFFFF) //was (~flagValues)
    {
      vars->mpe->nativeCodeCache.X86Emit_ANDIM(~flagMask, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }

    //If any of the flags represented by the mask are to be set to 1, set the flags
    if(flagValues)
    {
      vars->mpe->nativeCodeCache.X86Emit_ORIM(flagValues, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
    }
  }
}
