#include "basetypes.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "SuperBlockConstants.h"
#include "PropagateConstants.h"
#include "PropagateConstants_MEM.h"

#define ALLOW_RCU_PROPAGATION true

void PropagateConstants_DECRc0(SuperBlockConstants &constants)
{
  uint32 counterValue, flagValues;

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_RC0) && ALLOW_RCU_PROPAGATION)
  {
    counterValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RC0);
    flagValues = CC_COUNTER0_ZERO;
    if(counterValue)
    {
      counterValue--;
      if(counterValue)
      {
        flagValues &= ~CC_COUNTER0_ZERO; // = 0
      }
    }

    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_RC0;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = counterValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_COUNTER0_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearMiscInputDependency(CONSTANT_REG_RC0);
    PropagateConstants_StoreMiscRegisterConstant(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}

void PropagateConstants_DECRc1(SuperBlockConstants &constants)
{
  uint32 counterValue, flagValues;

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_RC1) && ALLOW_RCU_PROPAGATION)
  {
    counterValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RC1);
    flagValues = CC_COUNTER1_ZERO;
    if(counterValue)
    {
      counterValue--;
      if(counterValue)
      {
        flagValues &= ~CC_COUNTER1_ZERO; // = 0
      }
    }

    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_RC1;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = counterValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_COUNTER1_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;

    constants.ClearMiscInputDependency(CONSTANT_REG_RC1);
    PropagateConstants_StoreMiscRegisterConstant(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}
void PropagateConstants_DECBoth(SuperBlockConstants &constants)
{
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}

void PropagateConstants_ADDRImmediateOnly(SuperBlockConstants &constants)
{
  uint32 regIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];

  if(constants.IsMiscRegisterConstant(regIndex) && ALLOW_RCU_PROPAGATION)
  {

    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = regIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = constants.GetMiscRegisterConstant(regIndex) + constants.nuance->fields[FIELD_RCU_SRC];
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = 0;

    constants.ClearMiscInputDependency(regIndex);
    PropagateConstants_StoreMiscRegisterConstant(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(regIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}

void PropagateConstants_ADDRImmediate(SuperBlockConstants &constants)
{
  uint32 regIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  constants.ClearMiscRegisterConstant(regIndex);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}

void PropagateConstants_ADDRScalarOnly(SuperBlockConstants &constants)
{
  uint32 regDestIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  uint32 regSrcIndex = constants.nuance->fields[FIELD_RCU_SRC];

  if(constants.IsScalarRegisterConstant(regSrcIndex) && ALLOW_RCU_PROPAGATION)
  {
    constants.ClearScalarInputDependency(regSrcIndex);
    constants.nuance->fields[FIELD_RCU_HANDLER] = Handler_ADDRImmediateOnly;
    constants.nuance->fields[FIELD_RCU_SRC] = constants.GetScalarRegisterConstant(regSrcIndex);
    PropagateConstants_ADDRImmediateOnly(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(regDestIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}
void PropagateConstants_ADDRScalar(SuperBlockConstants &constants)
{
  uint32 regDestIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  uint32 regSrcIndex = constants.nuance->fields[FIELD_RCU_SRC];

  if(constants.IsScalarRegisterConstant(regSrcIndex) && ALLOW_RCU_PROPAGATION)
  {
    constants.ClearScalarInputDependency(regSrcIndex);
    constants.nuance->fields[FIELD_RCU_HANDLER] = Handler_ADDRImmediate;
    constants.nuance->fields[FIELD_RCU_SRC] = constants.GetScalarRegisterConstant(regSrcIndex);
    PropagateConstants_ADDRImmediate(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(regDestIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}
void PropagateConstants_MVRImmediateOnly(SuperBlockConstants &constants)
{
  uint32 regIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  uint32 regValue = constants.nuance->fields[FIELD_RCU_SRC];

  if(ALLOW_RCU_PROPAGATION)
  {
    constants.SetMiscRegisterConstant(regIndex,regValue);
  }
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}
void PropagateConstants_MVRImmediate(SuperBlockConstants &constants)
{
  uint32 regIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  uint32 regValue = constants.nuance->fields[FIELD_RCU_SRC];

  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
  if(ALLOW_RCU_PROPAGATION)
  {
    constants.SetMiscRegisterConstant(regIndex,regValue);
  }
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}
void PropagateConstants_MVRScalarOnly(SuperBlockConstants &constants)
{
  uint32 regDestIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  uint32 regSrcIndex = constants.nuance->fields[FIELD_RCU_SRC];

  if(constants.IsScalarRegisterConstant(regSrcIndex) && ALLOW_RCU_PROPAGATION)
  {
    constants.ClearScalarInputDependency(regSrcIndex);
    constants.nuance->fields[FIELD_RCU_HANDLER] = Handler_MVRImmediateOnly;
    constants.nuance->fields[FIELD_RCU_SRC] = constants.GetScalarRegisterConstant(regSrcIndex);
    PropagateConstants_MVRImmediateOnly(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(regDestIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}
void PropagateConstants_MVRScalar(SuperBlockConstants &constants)
{
  uint32 regDestIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  uint32 regSrcIndex = constants.nuance->fields[FIELD_RCU_SRC];

  if(constants.IsScalarRegisterConstant(regSrcIndex) && ALLOW_RCU_PROPAGATION)
  {
    constants.ClearScalarInputDependency(regSrcIndex);
    constants.nuance->fields[FIELD_RCU_HANDLER] = Handler_MVRImmediate;
    constants.nuance->fields[FIELD_RCU_SRC] = constants.GetScalarRegisterConstant(regSrcIndex);
    PropagateConstants_MVRImmediate(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(regDestIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
  }
}
void PropagateConstants_RangeOnly(SuperBlockConstants &constants)
{
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODMI);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODGE);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}
void PropagateConstants_Range(SuperBlockConstants &constants)
{
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODMI);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODGE);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}

void PropagateConstants_ModuloOnly(SuperBlockConstants &constants)
{
  uint32 regDestIndex;

  regDestIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  constants.ClearMiscRegisterConstant(regDestIndex);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODMI);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODGE);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}
void PropagateConstants_Modulo(SuperBlockConstants &constants)
{
  uint32 regDestIndex;

  regDestIndex = CONSTANT_REG_RX + constants.nuance->fields[FIELD_RCU_DEST];
  constants.ClearMiscRegisterConstant(regDestIndex);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODMI);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_MODGE);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C0Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C1Z);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RCU_OK;
}