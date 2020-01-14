#include "Basetypes.h"
#include "Handlers.h"
#include "InstructionDependencies.h"
#include "InstructionCache.h"
#include "PropagateConstants.h"
#include "SuperBlockConstants.h"

#define ALLOW_PROPAGATE_ECU true

void PropagateConstants_ECU_NOP(SuperBlockConstants &constants)
{
  constants.ClearAllMiscInputDependencies();
  constants.ClearAllMiscOutputDependencies();
  constants.ClearAllScalarInputDependencies();
  constants.ClearAllScalarOutputDependencies();

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_ECU_NOP;
  constants.ClearInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_Halt(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_HALT;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_BRAAlways(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_ALWAYS_DIRECT;
  constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
  //number of delay slots to execute
  constants.status.info[1] = 2;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_BRAAlways_NOP(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_ALWAYS_DIRECT;
  constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
  //number of delay slots to execute
  constants.status.info[1] = 0;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_BRAConditional(SuperBlockConstants &constants)
{
  bool branchResult;

  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_BRAAlways;
      PropagateConstants_BRAAlways(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_CONDITIONAL_DIRECT;
    //number of delay slots to execute
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    constants.status.info[1] = 2;
  }
}

void PropagateConstants_BRAConditional_NOP(SuperBlockConstants &constants)
{
  bool branchResult;

  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_BRAAlways_NOP;
      PropagateConstants_BRAAlways_NOP(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_CONDITIONAL_DIRECT;
    //number of delay slots to execute
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    constants.status.info[1] = 0;
  }
}

void PropagateConstants_JMPAlwaysIndirect(SuperBlockConstants &constants)
{
  uint32 regIndex = constants.nuance->fields[FIELD_ECU_ADDRESS];
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.IsScalarRegisterConstant(regIndex))
  {
    constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_BRAAlways;
    constants.nuance->fields[FIELD_ECU_ADDRESS] = constants.GetScalarRegisterConstant(regIndex);
    constants.ClearScalarInputDependency(regIndex);
    PropagateConstants_BRAAlways(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_ALWAYS_INDIRECT;
    //number of delay slots to execute
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    constants.status.info[1] = 2;
  }
}

void PropagateConstants_JMPAlwaysIndirect_NOP(SuperBlockConstants &constants)
{
  uint32 regIndex = constants.nuance->fields[FIELD_ECU_ADDRESS];
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.IsScalarRegisterConstant(regIndex))
  {
    constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_BRAAlways_NOP;
    constants.nuance->fields[FIELD_ECU_ADDRESS] = constants.GetScalarRegisterConstant(regIndex);
    constants.ClearScalarInputDependency(regIndex);
    PropagateConstants_BRAAlways_NOP(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_ALWAYS_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_JMPConditionalIndirect(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JMPAlwaysIndirect;
      PropagateConstants_JMPAlwaysIndirect(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_CONDITIONAL_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_JMPConditionalIndirect_NOP(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JMPAlwaysIndirect_NOP;
      PropagateConstants_JMPAlwaysIndirect_NOP(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_BRANCH_CONDITIONAL_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_JSRAlways(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_ALWAYS_DIRECT;
  constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
  //number of delay slots to execute
  constants.status.info[1] = 2;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}

void PropagateConstants_JSRAlways_NOP(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_ALWAYS_DIRECT;
  constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
  //number of delay slots to execute
  constants.status.info[1] = 0;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_JSRConditional(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JSRAlways;
      PropagateConstants_JSRAlways(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_CONDITIONAL_DIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_JSRConditional_NOP(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JSRAlways_NOP;
      PropagateConstants_JSRAlways_NOP(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_CONDITIONAL_DIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_JSRAlwaysIndirect(SuperBlockConstants &constants)
{
  uint32 regIndex = constants.nuance->fields[FIELD_ECU_ADDRESS];
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.IsScalarRegisterConstant(regIndex))
  {
    constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JSRAlways;
    constants.nuance->fields[FIELD_ECU_ADDRESS] = constants.GetScalarRegisterConstant(regIndex);
    constants.ClearScalarInputDependency(regIndex);
    PropagateConstants_JSRAlways(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_ALWAYS_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_JSRAlwaysIndirect_NOP(SuperBlockConstants &constants)
{
  uint32 regIndex = constants.nuance->fields[FIELD_ECU_ADDRESS];
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);


  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.IsScalarRegisterConstant(regIndex))
  {
    constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JSRAlways_NOP;
    constants.nuance->fields[FIELD_ECU_ADDRESS] = constants.GetScalarRegisterConstant(regIndex);
    constants.ClearScalarInputDependency(regIndex);
    PropagateConstants_JSRAlways_NOP(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_ALWAYS_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_JSRConditionalIndirect(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);


  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JSRAlwaysIndirect;
      PropagateConstants_JSRAlwaysIndirect(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_CONDITIONAL_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_JSRConditionalIndirect_NOP(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_JSRAlwaysIndirect_NOP;
      PropagateConstants_JSRAlwaysIndirect_NOP(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_JSR_CONDITIONAL_INDIRECT;
    constants.status.info[0] = constants.nuance->fields[FIELD_ECU_ADDRESS];
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_RTSAlways(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTS_ALWAYS;
  //number of delay slots to execute
  constants.status.info[1] = 2;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_RTSAlways_NOP(SuperBlockConstants &constants)
{
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTS_ALWAYS;
  //number of delay slots to execute
  constants.status.info[1] = 0;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);
}
void PropagateConstants_RTSConditional(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_RTSAlways;
      PropagateConstants_RTSAlways(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTS_CONDITIONAL;
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_RTSConditional_NOP(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_RTSAlways_NOP;
      PropagateConstants_RTSAlways_NOP(constants);
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTS_CONDITIONAL;
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_RTI1Conditional(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI1_ALWAYS;
      //number of delay slots to execute
      constants.status.info[1] = 2;
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI1_CONDITIONAL;
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_RTI1Conditional_NOP(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI1_ALWAYS;
      //number of delay slots to execute
      constants.status.info[1] = 0;
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI1_CONDITIONAL;
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
void PropagateConstants_RTI2Conditional(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI2_ALWAYS;
      //number of delay slots to execute
      constants.status.info[1] = 2;
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI2_CONDITIONAL;
    //number of delay slots to execute
    constants.status.info[1] = 2;
  }
}
void PropagateConstants_RTI2Conditional_NOP(SuperBlockConstants &constants)
{
  bool branchResult;
  constants.SetInstructionFlags(SUPERBLOCKINFO_LOCKED);

  if(!ALLOW_PROPAGATE_ECU)
  {
    return;
  }

  if(constants.EvaluateBranchCondition(constants.nuance->fields[FIELD_ECU_CONDITION], &branchResult))
  {
    if(branchResult)
    {
      constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI2_ALWAYS;
      //number of delay slots to execute
      constants.status.info[1] = 0;
    }
    else
    {
      constants.nuance->fields[FIELD_ECU_HANDLER] = Handler_ECU_NOP;
      PropagateConstants_ECU_NOP(constants);
    }
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_RTI2_CONDITIONAL;
    //number of delay slots to execute
    constants.status.info[1] = 0;
  }
}
