#include "basetypes.h"
#include "EmitMisc.h"
#include "EmitRCU.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "mpe.h"
#include "X86EmitTypes.h"

void Emit_DECRc0(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rc0ReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RC0);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg rc0WriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RC0);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 rc0Disp = GetMiscRegEmitDisp(vars,REGINDEX_RC0);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  //eax = rc0WriteBack, ebx = rc0M1, ecx = flagSetMask, ebp = 0

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C0Z)
  {
    //cc &= ~CC_COUNTER0_ZERO
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~CC_COUNTER0_ZERO, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  //rc0WriteBack = rc0
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rc0ReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc0Disp);
  //flagSetMask = CC_COUNTER0_ZERO
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_COUNTER0_ZERO, x86Reg::x86Reg_ecx);
  //rc0M1 = rc0
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_eax);
  //ebp = 0
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  //rc0M1 = rc0 - 1
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(1, x86Reg::x86Reg_ebx);
  //if(rc0M1 >= 0), rc0WriteBack = rc0 (otherwise rc0M1)
  vars->mpe->nativeCodeCache.X86Emit_CMOVNBRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  //if(rc0M1) > 0), flagSetMask = 0 (otherwise CC_COUNTER0_ZERO)
  vars->mpe->nativeCodeCache.X86Emit_CMOVNBERR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  if(vars->miscRegOutDep & DEPENDENCY_MASK_RC0)
  {
    //rc0 = rc0WriteBack
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, rc0WriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc0Disp);
  }
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C0Z)
  {
    //cc = cc | flagSetMask
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ecx, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
}

void Emit_DECRc1(EmitterVariables * const vars, const Nuance &nuance)
{
  const x86BaseReg rc1ReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RC1);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg rc1WriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RC1);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  const int32 rc1Disp = GetMiscRegEmitDisp(vars,REGINDEX_RC1);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C1Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~CC_COUNTER1_ZERO, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rc1ReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc1Disp);
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_COUNTER1_ZERO, x86Reg::x86Reg_ecx);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  vars->mpe->nativeCodeCache.X86Emit_SUBIR(1, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNBRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_CMOVNBERR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  if(vars->miscRegOutDep & DEPENDENCY_MASK_RC1)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, rc1WriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc1Disp);
  }
  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C1Z)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ecx, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
}

void Emit_DECBoth(EmitterVariables * const vars, const Nuance &nuance)
{
  //const x86BaseReg rc0ReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RC0);
  //const x86BaseReg rc1ReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_RC1);
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  //const x86BaseReg rc0WriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RC0);
  //const x86BaseReg rc1WriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_RC1);
  //const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  //const int32 rc0Disp = GetMiscRegEmitDisp(vars,REGINDEX_RC0);
  //const int32 rc1Disp = GetMiscRegEmitDisp(vars,REGINDEX_RC1);
  //const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  Emit_DECRc0(vars,nuance);
  Emit_DECRc1(vars,nuance);
/*
  //eax = rc0WriteBack, ebx = rc0M1, ecx = flagSetMask, ebp = 0

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_C0Z)
  {
    //cc &= ~(CC_COUNTER0_ZERO | CC_COUNTER1_ZERO)
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(CC_COUNTER0_ZERO | CC_COUNTER1_ZERO), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }
  //rc0WriteBack = rc0
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rc0ReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc0Disp);
  //flagSetMask = CC_COUNTER0_ZERO
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_COUNTER0_ZERO, x86Reg::x86Reg_ecx);
  //rc0M1 = rc0
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_eax);
  //ebp = 0
  vars->mpe->nativeCodeCache.X86Emit_XORRR(x86Reg::x86Reg_ebp, x86Reg::x86Reg_ebp);
  //rc0M1 = rc0 - 1
  vars->mpe->nativeCodeCache.X86Emit_DECR(x86Reg::x86Reg_ebx);
  //if(rc0M1 >= 0), rc0WriteBack = rc0M1 (otherwise rc0)
  vars->mpe->nativeCodeCache.X86Emit_CMOVNLRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  //if(rc0M1 > 0), flagSetMask = 0 (otherwise CC_COUNTER0_ZERO)
  vars->mpe->nativeCodeCache.X86Emit_CMOVNLERR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  //rc0 = rc0WriteBack
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, rc0WriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc0Disp);
  //cc = cc | flagSetMask
  vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ecx, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);

  //eax = rc1WriteBack, ebx = rc1M1, ecx = flagSetMask, ebp = 0

  //rc1WriteBack = rc1
  vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, rc1ReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc1Disp);
  //flagSetMask = CC_COUNTER1_ZERO
  vars->mpe->nativeCodeCache.X86Emit_MOVIR(CC_COUNTER1_ZERO, x86Reg::x86Reg_ecx);
  //rc1M1 = rc1
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_eax);
  //rc1M1 = rc1 - 1
  vars->mpe->nativeCodeCache.X86Emit_DECR(x86Reg::x86Reg_ebx);
  //if(rc1M1 >= 0), rc1WriteBack = rc1M1 (otherwise rc1)
  vars->mpe->nativeCodeCache.X86Emit_CMOVNLRR(x86Reg::x86Reg_eax, x86Reg::x86Reg_ebx);
  //if(rc1M1 > 0), flagSetMask = 0 (otherwise CC_COUNTER1_ZERO)
  vars->mpe->nativeCodeCache.X86Emit_CMOVNLERR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebp);
  //rc1 = rc1WriteBack
  vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, rc1WriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, rc1Disp);
  //cc = cc | flagSetMask
  vars->mpe->nativeCodeCache.X86Emit_ORRM(x86Reg::x86Reg_ecx, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
*/
}

void Emit_DEC(EmitterVariables * const vars, const Nuance &nuance)
{
  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    Emit_DECRc0(vars, nuance);
  }

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
  {
    Emit_DECRc1(vars, nuance);
  }
}

void Emit_ADDRImmediateOnly(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 indexRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_DEST];

  const x86BaseReg indexRegReadBaseReg = GetMiscRegReadBaseReg(vars,indexRegIndex);
  const x86BaseReg indexRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,indexRegIndex);
  const int32 indexRegDisp = GetMiscRegEmitDisp(vars,indexRegIndex);

  if(vars->miscRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax,indexRegReadBaseReg,x86IndexReg::x86IndexReg_none,x86ScaleVal::x86Scale_1,indexRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_ADDIR(nuance.fields[FIELD_RCU_SRC], x86Reg::x86Reg_eax);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax,indexRegWriteBaseReg,x86IndexReg::x86IndexReg_none,x86ScaleVal::x86Scale_1,indexRegDisp);
  }
}

void Emit_ADDRImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  Emit_ADDRImmediateOnly(vars, nuance);
  Emit_DEC(vars, nuance);
}

void Emit_ADDRScalarOnly(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 indexRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_DEST];
  const uint32 scalarRegIndex = (uint32_t)nuance.fields[FIELD_RCU_SRC];

  const x86BaseReg indexRegReadBaseReg = GetMiscRegReadBaseReg(vars,indexRegIndex);
  const x86BaseReg indexRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,indexRegIndex);
  const x86BaseReg scalarRegReadBaseReg = GetScalarRegReadBaseReg(vars,scalarRegIndex);
  const int32 indexRegDisp = GetMiscRegEmitDisp(vars,indexRegIndex);
  const int32 scalarRegDisp = GetScalarRegEmitDisp(vars,scalarRegIndex);

  if(vars->miscRegOutDep)
  {
    //eax = rcu_dest
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, scalarRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, scalarRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_ADDMR(x86Reg::x86Reg_eax, indexRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, indexRegDisp);
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, indexRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, indexRegDisp);
  }
}

void Emit_ADDRScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  Emit_ADDRScalarOnly(vars, nuance);
  Emit_DEC(vars, nuance);
}

void Emit_MVRImmediateOnly(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 indexRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_DEST];
  const x86BaseReg indexRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,indexRegIndex);
  const int32 indexRegDisp = GetMiscRegEmitDisp(vars,indexRegIndex);

  if(vars->miscRegOutDep)
  {
    vars->mpe->nativeCodeCache.X86Emit_MOVIM((uint32_t)nuance.fields[FIELD_RCU_SRC], x86MemPtr::x86MemPtr_dword, indexRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, indexRegDisp);
  }
}

void Emit_MVRImmediate(EmitterVariables * const vars, const Nuance &nuance)
{
  Emit_MVRImmediateOnly(vars, nuance);
  Emit_DEC(vars, nuance);
}

void Emit_MVRScalarOnly(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 indexRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_DEST];
  const uint32 scalarRegIndex = (uint32_t)nuance.fields[FIELD_RCU_SRC];
  const x86BaseReg indexRegWriteBaseReg = GetMiscRegWriteBaseReg(vars,indexRegIndex);
  const x86BaseReg scalarRegReadBaseReg = GetScalarRegReadBaseReg(vars,scalarRegIndex);
  const int32 indexRegDisp = GetMiscRegEmitDisp(vars,indexRegIndex);
  const int32 scalarRegDisp = GetScalarRegEmitDisp(vars,scalarRegIndex);

  if(vars->miscRegOutDep)
  {
    //eax = rcu_src
    vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, scalarRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, scalarRegDisp);
    //rcu_dest = eax
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_eax, indexRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, indexRegDisp);
  }
}

void Emit_MVRScalar(EmitterVariables * const vars, const Nuance &nuance)
{
  Emit_MVRScalarOnly(vars,nuance);
  Emit_DEC(vars,nuance);
}

void Emit_RangeOnly(EmitterVariables * const vars, const Nuance &nuance)
{
  constexpr uint32 l_testmodmi = 0;
  constexpr uint32 l_exit = 1;

  const uint32 srcRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_SRC];
  //const uint32 destRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_DEST];
  //const x86BaseReg ccReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_CC);
  const x86BaseReg xyRangeReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_XYRANGE);
  const x86BaseReg uvRangeReadBaseReg = GetMiscRegReadBaseReg(vars,REGINDEX_UVRANGE);
  const x86BaseReg srcRegReadBaseReg = GetMiscRegReadBaseReg(vars, srcRegIndex);
  //const x86BaseReg destRegReadBaseReg = GetMiscRegReadBaseReg(vars, destRegIndex);
  const x86BaseReg ccWriteBaseReg = GetMiscRegWriteBaseReg(vars,REGINDEX_CC);
  //const x86BaseReg destRegWriteBaseReg = GetMiscRegWriteBaseReg(vars, destRegIndex);
  const int32 xyRangeDisp = GetMiscRegEmitDisp(vars,REGINDEX_XYRANGE);
  const int32 uvRangeDisp = GetMiscRegEmitDisp(vars,REGINDEX_UVRANGE);
  const int32 srcRegDisp = GetMiscRegEmitDisp(vars,srcRegIndex);
  //const int32 destRegDisp = GetMiscRegEmitDisp(vars,destRegIndex);
  const int32 ccDisp = GetMiscRegEmitDisp(vars,REGINDEX_CC);

  vars->mpe->nativeCodeCache.patchMgr.Reset();

  switch(nuance.fields[FIELD_RCU_SRC])
  {
    case 0:
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, xyRangeReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, xyRangeDisp);
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, srcRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, srcRegDisp);
      break;
    case 1:
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, xyRangeReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, xyRangeDisp);
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, srcRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, srcRegDisp);
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, 16);
      break;
    case 2:
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, uvRangeReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvRangeDisp);
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, srcRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, srcRegDisp);
      break;
    case 3:
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_eax, uvRangeReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, uvRangeDisp);
      vars->mpe->nativeCodeCache.X86Emit_MOVMR(x86Reg::x86Reg_ebx, srcRegReadBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, srcRegDisp);
      vars->mpe->nativeCodeCache.X86Emit_SHLIR(x86Reg::x86Reg_eax, 16);
      break;
    default:
      assert(!"missing Emit_RangeOnly");
      break;
  }

  // Conditional clear gated on miscRegOutDep, vs Execute_RangeOnly which clears
  // both MODGE and MODMI unconditionally. This is the standard JIT pattern (see
  // EmitALU.cpp for the same idiom on N/Z/C/V): we only touch a flag bit when
  // dead-code-elimination says a later-on consumer reads it. The clear-mask
  // uses the SAME (miscRegOutDep & (MODMI|MODGE)) subset that drives the
  // conditional ORIMs, so the cleared bits and
  // the bits that may be re-set always match. DecodeRCU.cpp pairs
  // MODGE+MODMI in the output dep, so under correct DCE either both bits flow
  // through here or both are pruned together. If DCE ever incorrectly drops a
  // bit that is actually read downstream, the JIT will leave a stale flag -
  // but that hazard applies to every flag-producing emitter, not just this one
  if(vars->miscRegOutDep & (DEPENDENCY_FLAG_MODGE | DEPENDENCY_FLAG_MODMI))
  {
    vars->mpe->nativeCodeCache.X86Emit_ANDIM(~(FLAG_DEPENDENCIES(vars->miscRegOutDep & (DEPENDENCY_FLAG_MODMI | DEPENDENCY_FLAG_MODGE))), x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }

  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0x03FF0000, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_MOVRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_ANDIR(0xFFFF0000, x86Reg::x86Reg_ecx);

  //eax = rcu_range, ebx = rcu_src, ecx = rcu_src & 0xFFFF0000
  
  vars->mpe->nativeCodeCache.X86Emit_CMPRR(x86Reg::x86Reg_ecx, x86Reg::x86Reg_eax);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_L,l_testmodmi);

  vars->mpe->nativeCodeCache.X86Emit_SUBRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_eax);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MODGE)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORIM(CC_MODGE, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }

  vars->mpe->nativeCodeCache.X86Emit_JMPI_Label(l_exit);
  vars->mpe->nativeCodeCache.SetLabelPointer(l_testmodmi);
  //compare rcu_src to zero
  vars->mpe->nativeCodeCache.X86Emit_CMPIR(0,x86Reg::x86Reg_ebx);
  vars->mpe->nativeCodeCache.X86Emit_JCC_Label(X86_CC_NL,l_exit);

  vars->mpe->nativeCodeCache.X86Emit_ADDRR(x86Reg::x86Reg_ebx, x86Reg::x86Reg_eax);

  if(vars->miscRegOutDep & DEPENDENCY_FLAG_MODMI)
  {
    vars->mpe->nativeCodeCache.X86Emit_ORIM(CC_MODMI, x86MemPtr::x86MemPtr_dword, ccWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, ccDisp);
  }

  vars->mpe->nativeCodeCache.SetLabelPointer(l_exit);
  vars->mpe->nativeCodeCache.patchMgr.ApplyPatches();

  //ebx = modulo_writeback
}

void Emit_Range(EmitterVariables * const vars, const Nuance &nuance)
{
  Emit_RangeOnly(vars,nuance);
  Emit_DEC(vars,nuance);
}

void Emit_ModuloOnly(EmitterVariables * const vars, const Nuance &nuance)
{
  const uint32 destRegIndex = REGINDEX_RX + (uint32_t)nuance.fields[FIELD_RCU_DEST];
  const x86BaseReg destRegWriteBaseReg = GetMiscRegWriteBaseReg(vars, destRegIndex);
  const int32 destRegDisp = GetMiscRegEmitDisp(vars,destRegIndex);

  Emit_RangeOnly(vars,nuance);
  if(vars->miscRegOutDep & (~DEPENDENCY_FLAG_ALLFLAGS))
  {
    // ebx (= modulo result from Emit_RangeOnly) is stored to dest in full,
    // while Execute_ModuloOnly preserves the low 16 bits of the old dest:
    //   dest = (oldDest & 0x0000FFFF) | (moduloResult & 0xFFFF0000)
    // Equivalent here because DecodeRCU.cpp forces RCU_SRC == RCU_DEST for
    // modulo, and Emit_RangeOnly only modifies bits 16-25
    // of ebx (range value is masked to 0x03FF0000, and is added to / subtracted
    // from the source's upper-half via SUB/ADD), leaving the low 16 bits of
    // src == dest untouched. If the decoder ever permits RCU_SRC != RCU_DEST,
    // this write must mask in the original low 16 bits of dest!
    vars->mpe->nativeCodeCache.X86Emit_MOVRM(x86Reg::x86Reg_ebx, destRegWriteBaseReg, x86IndexReg::x86IndexReg_none, x86ScaleVal::x86Scale_1, destRegDisp);
  }
}

void Emit_Modulo(EmitterVariables * const vars, const Nuance &nuance)
{
  Emit_ModuloOnly(vars,nuance);
  Emit_DEC(vars,nuance);
}
