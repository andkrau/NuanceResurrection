#include "Basetypes.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "mpe.h"
#include "NuonMemoryMap.h"
#include "NuonEnvironment.h"
#include "PropagateConstants.h"
#include "SuperBlockConstants.h"

extern NuonEnvironment *nuonEnv;
#define ALLOW_MEM_PROPAGATION true

void PropagateConstants_UpdateFlagConstants(SuperBlockConstants &constants)
{
  uint32 flagValues;
  uint32 flagMask = constants.nuance->fields[FIELD_CONSTANT_FLAGMASK];

  if(flagMask)
  {
    flagValues = constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES];
    //Lock instruction in place if it sets flag constants
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
    if(flagMask & CC_COPROCESSOR0)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_CP0, (flagValues & CC_COPROCESSOR0) ? 1 : 0);
    }
    if(flagMask & CC_COPROCESSOR1)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_CP1, (flagValues & CC_COPROCESSOR1) ? 1 : 0);
    }
    if(flagMask & CC_MODMI)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_MODMI, (flagValues & CC_MODMI) ? 1 : 0);
    }
    if(flagMask & CC_MODGE)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_MODGE, (flagValues & CC_MODGE) ? 1 : 0);
    }
    if(flagMask & CC_COUNTER0_ZERO)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_C0Z, (flagValues & CC_COUNTER0_ZERO) ? 1 : 0);
    }
    if(flagMask & CC_COUNTER1_ZERO)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_C1Z, (flagValues & CC_COUNTER1_ZERO) ? 1 : 0);
    }
    if(flagMask & CC_MUL_OVERFLOW)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_MV, (flagValues & CC_MUL_OVERFLOW) ? 1 : 0);
    }
    if(flagMask & CC_ALU_NEGATIVE)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_N, (flagValues & CC_ALU_NEGATIVE) ? 1 : 0);
    }
    if(flagMask & CC_ALU_OVERFLOW)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_V, (flagValues & CC_ALU_OVERFLOW) ? 1 : 0);
    }
    if(flagMask & CC_ALU_CARRY)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_C, (flagValues & CC_ALU_CARRY) ? 1 : 0);
    }
    if(flagMask & CC_ALU_ZERO)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_Z, (flagValues & CC_ALU_ZERO) ? 1 : 0);
    }
  }
}

void PropagateConstants_StoreScalarRegisterConstant(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STORE_SCALAR_REGISTER_CONSTANT;
  constants.status.info[0] = constants.nuance->fields[FIELD_CONSTANT_ADDRESS];
  constants.SetScalarRegisterConstant(constants.nuance->fields[FIELD_CONSTANT_ADDRESS],constants.nuance->fields[FIELD_CONSTANT_VALUE]);
  constants.bConstantPropagated = true;
  PropagateConstants_UpdateFlagConstants(constants);
}

void PropagateConstants_StoreMiscRegisterConstant(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STORE_MISC_REGISTER_CONSTANT;
  constants.status.info[0] = constants.nuance->fields[FIELD_CONSTANT_ADDRESS];

  if(constants.nuance->fields[FIELD_CONSTANT_ADDRESS] != CONSTANT_REG_DISCARD)
  {
    constants.SetMiscRegisterConstant(constants.nuance->fields[FIELD_CONSTANT_ADDRESS],constants.nuance->fields[FIELD_CONSTANT_VALUE]);
    //This lock should be updated with a mask that only locks if a side-effect will occur
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
  constants.bConstantPropagated = true;
  PropagateConstants_UpdateFlagConstants(constants);
}

void PropagateConstants_Mirror(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 destValue;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    destValue = constants.GetScalarRegisterConstant(srcIndex);

    destValue = (((destValue & 0xaaaaaaaa) >> 1) | ((destValue & 0x55555555) << 1));
	  destValue = (((destValue & 0xcccccccc) >> 2) | ((destValue & 0x33333333) << 2));
	  destValue = (((destValue & 0xf0f0f0f0) >> 4) | ((destValue & 0x0f0f0f0f) << 4));
	  destValue = (((destValue & 0xff00ff00) >> 8) | ((destValue & 0x00ff00ff) << 8));
	
    destValue = ((destValue >> 16) | (destValue << 16));
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = 0;
    constants.ClearScalarInputDependency(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;  
    constants.ClearScalarRegisterConstant(destIndex);
  }
}
void PropagateConstants_MV_SImmediate(SuperBlockConstants &constants)
{
  uint32 srcValue = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  constants.SetScalarRegisterConstant(destIndex,srcValue);
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
}

void PropagateConstants_MV_SScalar(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 srcValue;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_MV_SImmediate;
    srcValue = constants.GetScalarRegisterConstant(srcIndex);
    constants.nuance->fields[FIELD_MEM_FROM] = srcValue;
    constants.ClearScalarInputDependency(srcIndex);
    constants.SetScalarRegisterConstant(destIndex,srcValue);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;  
    constants.ClearScalarRegisterConstant(destIndex);
  }
}
void PropagateConstants_MV_V(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;  
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_PopVector(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;  
  constants.ClearVectorRegisterConstant(destIndex);
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PopVectorRz(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;  
  constants.ClearPixelRegisterConstant(destIndex);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RZ);
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PopScalarRzi1(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RZ);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RZI1);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_ALLFLAGS);
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PopScalarRzi2(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RZ);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_RZI2);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_ALLFLAGS);
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PushVector(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PushVectorRz(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PushScalarRzi1(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_PushScalarRzi2(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_LoadScalarControlRegisterAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 regIndex, regValue;
  bool bIsConstant = true;

  regIndex = (constants.nuance->fields[FIELD_MEM_FROM] - MPE_CTRL_BASE) >> 4;
  switch(regIndex)
  {
    case 0x7:
      //PCEXEC: at time of load, pcexec has already been incremented to the next packet address
      //I don't know what the return value would be for a load in delay slot 2 and the only code
      //encountered which loads the value of pcexec are the 3D libraries.  These libraries only
      //load the value of pcexec outside of a delay slot so the value is always pcroute
      bIsConstant = true;
      regValue = constants.pCurrentInstructionEntry->packet->pcroute;
      break;
    case 0x8:
      //RZ
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RZ) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RZ);
      }
      break;
    case 0x1E:
      //RC0
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RC0) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RC0);
      }
      break;
    case 0x1F:
      //RC1
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RC1) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RC1);
      }
      break;
    case 0x20:
      //RX
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RX) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RX);
      }
      break;    
    case 0x21:
      //RY
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RY) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RY);
      }
      break;    
    case 0x25:
      //RU
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RU) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RU);
      }
      break;    
    case 0x26:
      //RV
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_RV) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_RV);
      }
      break;    
    case 0x2C:
      //SVSHIFT
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_SVSHIFT) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
      }
      break;    
    case 0x2D:
      //ACSHIFT
      bIsConstant = false;
      if(constants.IsMiscRegisterConstant(CONSTANT_REG_ACSHIFT) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = ((int32)(constants.GetMiscRegisterConstant(CONSTANT_REG_ACSHIFT) << 25)) >> 25;
      }
      break;
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      bIsConstant = false;
      if(constants.IsScalarRegisterConstant(regIndex - 0x30UL) && ALLOW_MEM_PROPAGATION)
      {
        bIsConstant = true;
        regValue = constants.GetScalarRegisterConstant(regIndex - 0x30UL);
      }
      break;
    default:
      bIsConstant = false;
      break;
  }

  if(bIsConstant && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_MV_SImmediate;
    constants.nuance->fields[FIELD_MEM_FROM] = regValue;
    constants.ClearAllMiscInputDependencies();
    constants.bConstantPropagated = true;
    PropagateConstants_MV_SImmediate(constants);
  }
  else
  {
    constants.ClearScalarRegisterConstant(destIndex);
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;  
}
void PropagateConstants_LoadByteAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadWordAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadScalarAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadScalarLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex) & 0xFFFFFFFCUL;
    constants.bConstantPropagated = true;

    if((address < MPE_CTRL_BASE) || (address >= MPE_RESV_BASE))
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadScalarAbsolute;
      constants.nuance->fields[FIELD_MEM_FROM] = address;
      constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address));
      constants.ClearScalarInputDependency(srcIndex);
      PropagateConstants_LoadScalarAbsolute(constants);    
    }
    else
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadScalarControlRegisterAbsolute;
      constants.nuance->fields[FIELD_MEM_FROM] = address;
      constants.ClearScalarInputDependency(srcIndex);
      PropagateConstants_LoadScalarControlRegisterAbsolute(constants);    
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearScalarRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadVectorAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadVectorControlRegisterAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_LoadPixelAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearPixelRegisterConstant(destIndex);
}
void PropagateConstants_LoadPixelZAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadByteLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex);

    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadByteAbsolute;
    constants.nuance->fields[FIELD_MEM_FROM] = address;
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address));
    constants.ClearScalarInputDependency(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_LoadByteAbsolute(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearScalarRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadByteBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadByteBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadWordLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex) & 0xFFFFFFFEUL;

    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadWordAbsolute;
    constants.nuance->fields[FIELD_MEM_FROM] = address;
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address));
    constants.ClearScalarInputDependency(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_LoadWordAbsolute(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearScalarRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadWordBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadWordBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadScalarBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadScalarBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearScalarRegisterConstant(destIndex);
}
void PropagateConstants_LoadShortVectorAbsolute(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadShortVectorLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex) & 0xFFFFFFF8UL;

    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadShortVectorAbsolute;
    constants.nuance->fields[FIELD_MEM_FROM] = address;
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address));
    constants.ClearScalarInputDependency(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_LoadShortVectorAbsolute(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearVectorRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadShortVectorBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadShortVectorBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadVectorLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex);
    constants.bConstantPropagated = true;

    if((address < MPE_CTRL_BASE) || (address >= MPE_RESV_BASE))
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadVectorAbsolute;
      constants.nuance->fields[FIELD_MEM_FROM] = address;
      constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address & 0xFFFFFFF0));
      constants.ClearScalarInputDependency(srcIndex);
      PropagateConstants_LoadVectorAbsolute(constants);    
    }
    else
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadVectorControlRegisterAbsolute;
      constants.nuance->fields[FIELD_MEM_FROM] = address;
      constants.ClearScalarInputDependency(srcIndex);
      PropagateConstants_LoadVectorControlRegisterAbsolute(constants);    
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearVectorRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadVectorBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadVectorBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadPixelLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex);
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadPixelAbsolute;
    constants.nuance->fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
    constants.nuance->fields[FIELD_MEM_FROM] = address;
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address));
    constants.ClearScalarInputDependency(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_LoadPixelAbsolute(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearPixelRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadPixelBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadPixelBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadPixelZLinear(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];
  uint32 address;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    address = constants.GetScalarRegisterConstant(srcIndex);
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_LoadPixelZAbsolute;
    constants.nuance->fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
    constants.nuance->fields[FIELD_MEM_FROM] = address;
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)(nuonEnv->GetPointerToMemory(constants.mpe,address));
    constants.bConstantPropagated = true;
    PropagateConstants_LoadPixelZAbsolute(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.ClearVectorRegisterConstant(destIndex);
  }
}
void PropagateConstants_LoadPixelZBilinearUV(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}
void PropagateConstants_LoadPixelZBilinearXY(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO]; 
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.ClearVectorRegisterConstant(destIndex);
}

void PropagateConstants_StoreScalarImmediate(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreScalarControlRegisterImmediate(SuperBlockConstants &constants)
{
  uint32 destValue = constants.nuance->fields[FIELD_MEM_FROM]; 
  uint32 destIndex = (constants.nuance->fields[FIELD_MEM_TO] - MPE_CTRL_BASE) >> 4; 
  bool bIsConstant = true;
  bool bLockInstruction = false;
  bool bScalarReg = false;
  uint32 flagMask = 0;
  uint32 flagValues = 0;
  
  switch(destIndex)
  {
    case 0x4:
      //CC
      destIndex = CONSTANT_REG_DISCARD;
      flagMask = CC_ALLFLAGS;
      //Force CP0 and CP1 to zero
      flagValues = destValue & 0x1FFUL;
      bLockInstruction = true;
      break;
    case 0x8:
      //RZ
      destIndex = CONSTANT_REG_RZ;
      break;
    case 0x1E:
      //RC0
      destIndex = CONSTANT_REG_RC0;
      flagMask = CC_COUNTER0_ZERO;
      if(!destValue)
      {
        flagValues = CC_COUNTER0_ZERO;
      }
      bLockInstruction = true;
      break;
    case 0x1F:
      //RC1
      destIndex = CONSTANT_REG_RC1;
      flagMask = CC_COUNTER1_ZERO;
      if(!destValue)
      {
        flagValues = CC_COUNTER1_ZERO;
      }
      bLockInstruction = true;
      break;
    case 0x20:
      //RX
      destIndex = CONSTANT_REG_RX;
      break;    
    case 0x21:
      //RY
      destIndex = CONSTANT_REG_RY;
      break;    
    case 0x25:
      //RU
      destIndex = CONSTANT_REG_RU;
      break;    
    case 0x26:
      //RV
      destIndex = CONSTANT_REG_RV;
      break;    
    case 0x2C:
      //SVSHIFT
      destIndex = CONSTANT_REG_SVSHIFT;
      break;    
    case 0x2D:
      //ACSHIFT
      destIndex = CONSTANT_REG_ACSHIFT;
      destValue = ((int32)(destValue << 25)) >> 25;
      break;
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      destIndex = destIndex - 0x30UL;
      bScalarReg = true;
      break;
    default:
      bIsConstant = false;
      bLockInstruction = true;
      break;
  }

  if(bIsConstant && ALLOW_MEM_PROPAGATION)
  {
    constants.bConstantPropagated = true;
    if(!bScalarReg)
    {
      constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
      constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
      constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
      constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = flagMask;
      constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
      PropagateConstants_StoreMiscRegisterConstant(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_MV_SImmediate;
      constants.nuance->fields[FIELD_MEM_FROM] = destValue;
      constants.nuance->fields[FIELD_MEM_TO] = destIndex;
      PropagateConstants_MV_SImmediate(constants);
    }
  }
  else
  {
    if(bIsConstant)
    {
      if(!bScalarReg)
      {
        constants.ClearMiscRegisterConstant(destIndex);
      }
      else
      {
        constants.ClearScalarRegisterConstant(destIndex);
      }
    }
  }

  if(bLockInstruction)
  {
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
}

void PropagateConstants_StoreScalarControlRegisterAbsolute(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];
  uint32 destIndex;

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StoreScalarControlRegisterImmediate;
    constants.nuance->fields[FIELD_MEM_FROM] = constants.GetScalarRegisterConstant(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_StoreScalarControlRegisterImmediate(constants);
  }
  else
  {
    destIndex = (constants.nuance->fields[FIELD_MEM_TO] - MPE_CTRL_BASE) >> 4;
    switch(destIndex)
    {
      case 0x4:
        //CC
        constants.ClearMiscRegisterConstant(CONSTANT_REG_ALLFLAGS);
        constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
        break;
      case 0x8:
        //RZ
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RZ);
        break;
      case 0x1E:
        //RC0
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RC0);
        constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
        break;
      case 0x1F:
        //RC1
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RC1);
        constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
        break;
      case 0x20:
        //RX
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RX);
        break;    
      case 0x21:
        //RY
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RY);
        break;    
      case 0x25:
        //RU
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RU);
        break;    
      case 0x26:
        //RV
        constants.ClearMiscRegisterConstant(CONSTANT_REG_RV);
        break;    
      case 0x2C:
        //SVSHIFT
        constants.ClearMiscRegisterConstant(CONSTANT_REG_SVSHIFT);
        break;    
      case 0x2D:
        //ACSHIFT
        constants.ClearMiscRegisterConstant(CONSTANT_REG_ACSHIFT);
        break;
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
      case 0x34:
      case 0x35:
      case 0x36:
      case 0x37:
      case 0x38:
      case 0x39:
      case 0x3A:
      case 0x3B:
      case 0x3C:
      case 0x3D:
      case 0x3E:
      case 0x3F:
      case 0x40:
      case 0x41:
      case 0x42:
      case 0x43:
      case 0x44:
      case 0x45:
      case 0x46:
      case 0x47:
      case 0x48:
      case 0x49:
      case 0x4A:
      case 0x4B:
      case 0x4C:
      case 0x4D:
      case 0x4E:
      case 0x4F:
        constants.ClearScalarRegisterConstant(destIndex - 0x30UL);
        break;
      default:
        constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
        //Sync flags
        constants.SetInstructionFlags(SUPERBLOCKINFO_SYNC);
        break;
    }

    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  }
}

void PropagateConstants_StoreScalarAbsolute(SuperBlockConstants &constants)
{
  uint32 srcIndex = constants.nuance->fields[FIELD_MEM_FROM];

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StoreScalarImmediate;
    constants.nuance->fields[FIELD_MEM_FROM] = constants.GetScalarRegisterConstant(srcIndex);
    constants.bConstantPropagated = true;
    PropagateConstants_StoreScalarImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
}

void PropagateConstants_StoreScalarLinear(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];

  if(constants.IsScalarRegisterConstant(destIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_TO] = constants.GetScalarRegisterConstant(destIndex);
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)nuonEnv->GetPointerToMemory(constants.mpe, constants.GetScalarRegisterConstant(destIndex));
    constants.ClearScalarInputDependency(destIndex);
    constants.SetScalarInputDependency(constants.nuance->fields[FIELD_MEM_FROM]);
    constants.bConstantPropagated = true;
    if((constants.GetScalarRegisterConstant(destIndex) & MPE_CTRL_BASE) == MPE_CTRL_BASE)
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StoreScalarControlRegisterAbsolute;
      PropagateConstants_StoreScalarControlRegisterAbsolute(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StoreScalarAbsolute;
      PropagateConstants_StoreScalarAbsolute(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
}

void PropagateConstants_StoreScalarBilinearUV(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreScalarBilinearXY(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreVectorControlRegisterAbsolute(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StorePixelAbsolute(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StorePixelZAbsolute(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreShortVectorAbsolute(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreShortVectorLinear(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];

  if(constants.IsScalarRegisterConstant(destIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StoreShortVectorAbsolute;
    constants.nuance->fields[FIELD_MEM_TO] = constants.GetScalarRegisterConstant(destIndex);
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)nuonEnv->GetPointerToMemory(constants.mpe, constants.GetScalarRegisterConstant(destIndex));
    constants.bConstantPropagated = true;
    PropagateConstants_StoreShortVectorAbsolute(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
}
void PropagateConstants_StoreShortVectorBilinearUV(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreShortVectorBilinearXY(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreVectorAbsolute(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreVectorLinear(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];

  if(constants.IsScalarRegisterConstant(destIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StoreVectorAbsolute;
    constants.nuance->fields[FIELD_MEM_TO] = constants.GetScalarRegisterConstant(destIndex);
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)nuonEnv->GetPointerToMemory(constants.mpe, constants.GetScalarRegisterConstant(destIndex));
    constants.bConstantPropagated = true;
    PropagateConstants_StoreVectorAbsolute(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
}
void PropagateConstants_StoreVectorBilinearUV(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StoreVectorBilinearXY(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_StorePixelLinear(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];

  if(constants.IsScalarRegisterConstant(destIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StorePixelAbsolute;
    constants.nuance->fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
    constants.nuance->fields[FIELD_MEM_TO] = constants.GetScalarRegisterConstant(destIndex);
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)nuonEnv->GetPointerToMemory(constants.mpe, constants.GetScalarRegisterConstant(destIndex));
    constants.bConstantPropagated = true;
    PropagateConstants_StorePixelAbsolute(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
}
void PropagateConstants_StorePixelBilinearUV(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StorePixelBilinearXY(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StorePixelZLinear(SuperBlockConstants &constants)
{
  uint32 destIndex = constants.nuance->fields[FIELD_MEM_TO];

  if(constants.IsScalarRegisterConstant(destIndex) && ALLOW_MEM_PROPAGATION)
  {
    constants.nuance->fields[FIELD_MEM_HANDLER] = Handler_StorePixelZAbsolute;
    constants.nuance->fields[FIELD_MEM_INFO] = MEM_INFO_LINEAR_INDIRECT;
    constants.nuance->fields[FIELD_MEM_TO] = constants.GetScalarRegisterConstant(destIndex);
    constants.nuance->fields[FIELD_MEM_POINTER] = (uint32)nuonEnv->GetPointerToMemory(constants.mpe, constants.GetScalarRegisterConstant(destIndex));
    constants.bConstantPropagated = true;
    PropagateConstants_StorePixelZAbsolute(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
    constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
  }
}
void PropagateConstants_StorePixelZBilinearUV(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_StorePixelZBilinearXY(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_MEM_OK;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}