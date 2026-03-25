#include "basetypes.h"
#include "Handlers.h"
#include "InstructionDependencies.h"
#include "InstructionCache.h"
#include "PropagateConstants.h"
#include "PropagateConstants_MEM.h"
#include "mpe.h"
#include "SuperBlockConstants.h"

#define ALLOW_MUL_PROPAGATION true

void PropagateConstants_ADDMImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_MUL_SRC1];
    const uint32 destValue = src1 + constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = 0;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants); 
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_ADDM(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index  = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDMImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_ADDMImmediate(constants);
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_ADDMImmediate;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_MUL_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_ADDMImmediate(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_SUBMImmediateReverse(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_MUL_SRC1];
    const uint32 destValue = src1 - constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = 0;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants); 
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_SUBM(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index  = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if((src1Index == src2Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = 0;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = 0;
    constants.ClearScalarInputDependency(src2Index);
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);      
  }
  else if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_SUBMImmediateReverse;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_SUBMImmediateReverse(constants);
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    //Convert <Scalar - Immediate> to <(-Immediate) + Scalar>
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDMImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = -((int32)constants.GetScalarRegisterConstant(src2Index));
    constants.nuance->fields[FIELD_ALU_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    //No ADDMImmediate propogation: just execute the following two lines
    constants.ClearScalarRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULImmediateShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index  = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    const int64 mulop1 = constants.nuance->fields[FIELD_MUL_SRC1];
    const int64 mulop2 = constants.GetScalarRegisterConstant(src2Index);
    const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) >> constants.nuance->fields[FIELD_MUL_INFO];
    uint32 flagValue = 0;
    if((result & 0x0000000080000000LL) == 0LL)
    {
      if((result & 0xFFFFFFFF00000000LL) != 0LL)
      {
        flagValue = CC_MUL_OVERFLOW;
      }
    }
    else
    {
      if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
      {
        flagValue = CC_MUL_OVERFLOW;
      }
    }
    
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = (uint32)result;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_MUL_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValue;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULImmediateShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index  = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    const int64 mulop1 = constants.nuance->fields[FIELD_MUL_SRC1];
    const int64 mulop2 = constants.GetScalarRegisterConstant(src2Index);
    const int64 result = (((mulop1 << 32) >> 32) * ((mulop2 << 32) >> 32)) << constants.nuance->fields[FIELD_MUL_INFO];
    uint32 flagValue = 0;
    if((result & 0x0000000080000000LL) == 0LL)
    {
      if((result & 0xFFFFFFFF00000000LL) != 0LL)
      {
        flagValue = CC_MUL_OVERFLOW;
      }
    }
    else
    {
      if((result & 0xFFFFFFFF00000000LL) != 0xFFFFFFFF00000000LL)
      {
        flagValue = CC_MUL_OVERFLOW;
      }
    }
    
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = (uint32)result;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_MUL_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValue;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftRightImmediate;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_MULImmediateShiftRightImmediate(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftLeftImmediate;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_MULImmediateShiftLeftImmediate(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULImmediateShiftAcshift(SuperBlockConstants &constants)
{
  if(constants.IsMiscRegisterConstant(CONSTANT_REG_ACSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    const uint32 shiftVal = constants.GetMiscRegisterConstant(CONSTANT_REG_ACSHIFT) & 0x7FUL;
    if(shiftVal & 0x40)
    {
      constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftLeftImmediate;
      constants.nuance->fields[FIELD_MUL_INFO] = 128 - shiftVal;
      constants.ClearMiscInputDependency(CONSTANT_REG_ACSHIFT);
      PropagateConstants_MULImmediateShiftLeftImmediate(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftRightImmediate;
      constants.nuance->fields[FIELD_MUL_INFO] = shiftVal;
      constants.ClearMiscInputDependency(CONSTANT_REG_ACSHIFT);
      PropagateConstants_MULImmediateShiftRightImmediate(constants);
    }
  }
  else
  {
    const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULScalarShiftAcshift(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_ACSHIFT))
  {
    const int32 shiftValue = constants.GetMiscRegisterConstant(CONSTANT_REG_ACSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_ACSHIFT);

    if(shiftValue >= 0)
    {
      constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULScalarShiftRightImmediate;
      constants.nuance->fields[FIELD_MUL_INFO] = shiftValue;
      PropagateConstants_MULScalarShiftRightImmediate(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULScalarShiftLeftImmediate;
      constants.nuance->fields[FIELD_MUL_INFO] = -shiftValue;
      PropagateConstants_MULScalarShiftLeftImmediate(constants);
    }
  }
  else if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftAcshift;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_MULImmediateShiftAcshift(constants);
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftAcshift;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_MUL_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_MULImmediateShiftAcshift(constants);
  }
  else
  {
    const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULImmediateShiftScalar(SuperBlockConstants &constants)
{
  const uint32 shiftIndex  = constants.nuance->fields[FIELD_MUL_INFO];

  if(constants.IsScalarRegisterConstant(shiftIndex) && ALLOW_MUL_PROPAGATION)
  {
    const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
    const uint32 shiftVal = constants.GetScalarRegisterConstant(shiftIndex) & 0x7FUL;
    if(shiftVal & 0x40)
    {
      constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftLeftImmediate;
      constants.nuance->fields[FIELD_MUL_INFO] = 128 - shiftVal;
      constants.ClearScalarInputDependency(shiftIndex);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_MULImmediateShiftLeftImmediate(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftRightImmediate;
      constants.nuance->fields[FIELD_MUL_INFO] = shiftVal;
      constants.ClearScalarInputDependency(shiftIndex);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_MULImmediateShiftRightImmediate(constants);
    }
  }
  else
  {
    const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MULScalarShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index  = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index  = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MULImmediateShiftScalar;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_MULImmediateShiftScalar(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_MV);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_SVImmediateShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearVectorRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_SVScalarShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_SVImmediateShiftImmediate;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetShortVectorInputDependency(src2Index);
    PropagateConstants_MUL_SVImmediateShiftImmediate(constants);
  }
  else
  {
    constants.ClearVectorRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_SVScalarShiftSvshift(SuperBlockConstants &constants)
{
  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_SVScalarShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_SVScalarShiftImmediate(constants);
  }
  else
  {
    const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];
    constants.ClearVectorRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_SVRuShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearVectorRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_SVRuShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_SVRuShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_SVRuShiftImmediate(constants);
  }
  else
  {
    constants.ClearVectorRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_SVRvShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearVectorRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_SVRvShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_SVRvShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_SVRvShiftImmediate(constants);
  }
  else
  {
    constants.ClearVectorRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_SVVectorShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearVectorRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_SVVectorShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_SVVectorShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_SVVectorShiftImmediate(constants);
  }
  else
  {
    constants.ClearVectorRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_PImmediateShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearPixelRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_PScalarShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_MUL_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_MUL_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_PImmediateShiftImmediate;
    constants.nuance->fields[FIELD_MUL_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetPixelInputDependency(src2Index);
    PropagateConstants_MUL_PImmediateShiftImmediate(constants);
  }
  else
  {
    constants.ClearPixelRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_PScalarShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_PScalarShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_PScalarShiftImmediate(constants);
  }
  else
  {
    constants.ClearPixelRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_PRuShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearPixelRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_PRuShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_PRuShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_PRuShiftImmediate(constants);
  }
  else
  {
    constants.ClearPixelRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_PRvShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearPixelRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_PRvShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_PRvShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_PRvShiftImmediate(constants);
  }
  else
  {
    constants.ClearPixelRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_MUL_PVectorShiftImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];
  constants.ClearPixelRegisterConstant(destIndex);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_MUL_PVectorShiftSvshift(SuperBlockConstants &constants)
{
  const uint32 destIndex  = constants.nuance->fields[FIELD_MUL_DEST];

  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_MUL_PVectorShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_MUL_PVectorShiftImmediate(constants);
  }
  else
  {
    constants.ClearPixelRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_DOTPScalarShiftImmediate(SuperBlockConstants &constants)
{
  constants.ClearScalarRegisterConstant(constants.nuance->fields[FIELD_MUL_DEST]);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_DOTPScalarShiftSvshift(SuperBlockConstants &constants)
{
  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_DOTPScalarShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);    
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_DOTPScalarShiftImmediate(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(constants.nuance->fields[FIELD_MUL_DEST]);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}

void PropagateConstants_DOTPVectorShiftImmediate(SuperBlockConstants &constants)
{
  constants.ClearScalarRegisterConstant(constants.nuance->fields[FIELD_MUL_DEST]);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
}

void PropagateConstants_DOTPVectorShiftSvshift(SuperBlockConstants &constants)
{
  if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MUL_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MUL_HANDLER] = Handler_DOTPVectorShiftImmediate;
    constants.nuance->fields[FIELD_MUL_INFO] = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT); 
    constants.ClearMiscInputDependency(CONSTANT_REG_SVSHIFT);
    PropagateConstants_DOTPVectorShiftImmediate(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(constants.nuance->fields[FIELD_MUL_DEST]);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MUL_OK;
  }
}
