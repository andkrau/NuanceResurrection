#ifndef EXECUTE_ECU_H
#define EXECUTE_ECU_H

#include "mpe.h"

NuanceHandlerProto Execute_ECU_NOP;
NuanceHandlerProto Execute_Halt;
NuanceHandlerProto Execute_BRAAlways;
NuanceHandlerProto Execute_BRAAlways_NOP;
NuanceHandlerProto Execute_BRAConditional;
NuanceHandlerProto Execute_BRAConditional_NOP;
NuanceHandlerProto Execute_JMPAlwaysIndirect;
NuanceHandlerProto Execute_JMPAlwaysIndirect_NOP;
NuanceHandlerProto Execute_JMPConditionalIndirect;
NuanceHandlerProto Execute_JMPConditionalIndirect_NOP;
NuanceHandlerProto Execute_JSRAlways;
NuanceHandlerProto Execute_JSRAlways_NOP;
NuanceHandlerProto Execute_JSRConditional;
NuanceHandlerProto Execute_JSRConditional_NOP;
NuanceHandlerProto Execute_JSRAlwaysIndirect;
NuanceHandlerProto Execute_JSRAlwaysIndirect_NOP;
NuanceHandlerProto Execute_JSRConditionalIndirect;
NuanceHandlerProto Execute_JSRConditionalIndirect_NOP;
NuanceHandlerProto Execute_RTI1Conditional;
NuanceHandlerProto Execute_RTI1Conditional_NOP;
NuanceHandlerProto Execute_RTI2Conditional;
NuanceHandlerProto Execute_RTI2Conditional_NOP;
NuanceHandlerProto Execute_RTSAlways;
NuanceHandlerProto Execute_RTSAlways_NOP;
NuanceHandlerProto Execute_RTSConditional;
NuanceHandlerProto Execute_RTSConditional_NOP;

#endif
