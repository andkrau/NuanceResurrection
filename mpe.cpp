#include "basetypes.h"
#include <fcntl.h>
#include <intrin.h>
#include <cstdio>

#include "Bios.h"
#include "byteswap.h"
#include "EmitMisc.h"
#include "EmitECU.h"
#include "EmitALU.h"
#include "EmitMEM.h"
#include "EmitMUL.h"
#include "EmitRCU.h"
#include "ExecuteECU.h"
#include "ExecuteRCU.h"
#include "ExecuteALU.h"
#include "ExecuteMUL.h"
#include "ExecuteMEM.h"
#include "ExecuteMisc.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "mpe.h"
#include "nativecodecache.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "OverlayManager.h"
#include "PresentationEngine.h"
#include "PrintECU.h"
#include "PrintRCU.h"
#include "PrintALU.h"
#include "PrintMUL.h"
#include "PrintMEM.h"
#include "PrintMisc.h"
#include "SuperBlock.h"
#include "X86EmitTypes.h"
#include "Utility.h"

#define LOG_MPE_INDEX (1)

#if defined(LOG_PROGRAM_FLOW) || defined(LOG_BIOS_CALLS)
 #define LOG_STUFF
#endif

extern NuonEnvironment nuonEnv;
extern NuonBiosHandler BiosJumpTable[];
extern const char *BiosRoutineNames[];
extern void NullBiosHandler(MPE& mpe);
extern void WillNotImplement(MPE& mpe);
extern void AssemblyBiosHandler(MPE& mpe);
extern void UnimplementedCacheHandler(MPE& mpe);
extern void UnimplementedMediaHandler(MPE& mpe);
extern bool bCallingMediaCallback;
#ifdef LOG_COMM
extern FILE *commLogFile;
#endif

const NuanceHandler nuanceHandlers[] =
{
  //ECU Executes
  Execute_ECU_NOP,
  Execute_Halt,
  Execute_BRAAlways,
  Execute_BRAAlways_NOP,
  Execute_BRAConditional,
  Execute_BRAConditional_NOP,
  Execute_JMPAlwaysIndirect,
  Execute_JMPAlwaysIndirect_NOP,
  Execute_JMPConditionalIndirect,
  Execute_JMPConditionalIndirect_NOP,
  Execute_JSRAlways,
  Execute_JSRAlways_NOP,
  Execute_JSRConditional,
  Execute_JSRConditional_NOP,
  Execute_JSRAlwaysIndirect,
  Execute_JSRAlwaysIndirect_NOP,
  Execute_JSRConditionalIndirect,
  Execute_JSRConditionalIndirect_NOP,
  Execute_RTI1Conditional,
  Execute_RTI1Conditional_NOP,
  Execute_RTI2Conditional,
  Execute_RTI2Conditional_NOP,
  Execute_RTSAlways,
  Execute_RTSAlways_NOP,
  Execute_RTSConditional,
  Execute_RTSConditional_NOP,
  //RCU Executes
  Execute_DECRc1,
  Execute_DECRc0,
  Execute_DECBoth,
  Execute_ADDRImmediateOnly,
  Execute_ADDRImmediate,
  Execute_ADDRScalarOnly,
  Execute_ADDRScalar,
  Execute_MVRImmediateOnly,
  Execute_MVRImmediate,
  Execute_MVRScalarOnly,
  Execute_MVRScalar,
  Execute_RangeOnly,
  Execute_Range,
  Execute_ModuloOnly,
  Execute_Modulo,
  //ALU Executes
  Execute_ABS,
  Execute_BITSScalar,
  Execute_BITSImmediate,
  Execute_BTST,
  Execute_BUTT,
  Execute_COPY,
  Execute_MSB,
  Execute_SAT,
  Execute_AS,
  Execute_ASL,
  Execute_ASR,
  Execute_LS,
  Execute_LSR,
  Execute_ROT,
  Execute_ROL,
  Execute_ROR,
  Execute_ADD_P,
  Execute_SUB_P,
  Execute_ADD_SV,
  Execute_SUB_SV,
  Execute_ADDImmediate,
  Execute_ADDScalar,
  Execute_ADDScalarShiftRightImmediate,
  Execute_ADDScalarShiftLeftImmediate,
  Execute_SUBImmediate,
  Execute_SUBImmediateReverse,
  Execute_SUBScalar,
  Execute_SUBScalarShiftRightImmediate,
  Execute_SUBScalarShiftLeftImmediate,
  Execute_CMPImmediate,
  Execute_CMPImmediateReverse,
  Execute_CMPScalar,
  Execute_CMPScalarShiftRightImmediate,
  Execute_CMPScalarShiftLeftImmediate,
  Execute_ANDImmediate,
  Execute_ANDScalar,
  Execute_ANDImmediateShiftScalar,
  Execute_ANDScalarShiftRightImmediate,
  Execute_ANDScalarShiftLeftImmediate,
  Execute_ANDScalarShiftScalar,
  Execute_ANDScalarRotateScalar,
  Execute_FTSTImmediate,
  Execute_FTSTScalar,
  Execute_FTSTImmediateShiftScalar,
  Execute_FTSTScalarShiftRightImmediate,
  Execute_FTSTScalarShiftLeftImmediate,
  Execute_FTSTScalarShiftScalar,
  Execute_FTSTScalarRotateScalar,
  Execute_ORImmediate,
  Execute_ORScalar,
  Execute_ORImmediateShiftScalar,
  Execute_ORScalarShiftRightImmediate,
  Execute_ORScalarShiftLeftImmediate,
  Execute_ORScalarShiftScalar,
  Execute_ORScalarRotateScalar,
  Execute_EORImmediate,
  Execute_EORScalar,
  Execute_EORImmediateShiftScalar,
  Execute_EORScalarShiftRightImmediate,
  Execute_EORScalarShiftLeftImmediate,
  Execute_EORScalarShiftScalar,
  Execute_EORScalarRotateScalar,
  Execute_ADDWCScalar,
  Execute_ADDWCImmediate,
  Execute_ADDWCScalarShiftRightImmediate,
  Execute_ADDWCScalarShiftLeftImmediate,
  Execute_SUBWCScalar,
  Execute_SUBWCImmediate,
  Execute_SUBWCImmediateReverse,
  Execute_SUBWCScalarShiftRightImmediate,
  Execute_SUBWCScalarShiftLeftImmediate,
  Execute_CMPWCScalar,
  Execute_CMPWCImmediate,
  Execute_CMPWCImmediateReverse,
  Execute_CMPWCScalarShiftRightImmediate,
  Execute_CMPWCScalarShiftLeftImmediate,
  //MUL Executes
  Execute_ADDM,
  Execute_ADDMImmediate,
  Execute_SUBM,
  Execute_SUBMImmediateReverse,
  Execute_MULScalarShiftAcshift,
  Execute_MULScalarShiftRightImmediate,
  Execute_MULScalarShiftLeftImmediate,
  Execute_MULImmediateShiftAcshift,
  Execute_MULScalarShiftScalar,
  Execute_MULImmediateShiftScalar,
  Execute_MULImmediateShiftRightImmediate,
  Execute_MULImmediateShiftLeftImmediate,
  Execute_MUL_SVImmediateShiftImmediate,
  Execute_MUL_SVScalarShiftImmediate,
  Execute_MUL_SVScalarShiftSvshift,
  Execute_MUL_SVRuShiftImmediate,
  Execute_MUL_SVRuShiftSvshift,
  Execute_MUL_SVRvShiftImmediate,
  Execute_MUL_SVRvShiftSvshift,
  Execute_MUL_SVVectorShiftImmediate,
  Execute_MUL_SVVectorShiftSvshift,
  Execute_MUL_PImmediateShiftImmediate,
  Execute_MUL_PScalarShiftImmediate,
  Execute_MUL_PScalarShiftSvshift,
  Execute_MUL_PRuShiftImmediate,
  Execute_MUL_PRuShiftSvshift,
  Execute_MUL_PRvShiftImmediate,
  Execute_MUL_PRvShiftSvshift,
  Execute_MUL_PVectorShiftImmediate,
  Execute_MUL_PVectorShiftSvshift,
  Execute_DOTPScalarShiftImmediate,
  Execute_DOTPScalarShiftSvshift,
  Execute_DOTPVectorShiftImmediate,
  Execute_DOTPVectorShiftSvshift,
  //MEM Executes
  Execute_Mirror,
  Execute_MV_SImmediate,
  Execute_MV_SScalar,
  Execute_MV_V,
  Execute_PopVector,
  Execute_PopVectorRz,
  Execute_PopScalarRzi1,
  Execute_PopScalarRzi2,
  Execute_PushVector,
  Execute_PushVectorRz,
  Execute_PushScalarRzi1,
  Execute_PushScalarRzi2,
  Execute_LoadScalarLinear,
  Execute_LoadScalarControlRegisterAbsolute,
  Execute_LoadByteAbsolute,
  Execute_LoadWordAbsolute,
  Execute_LoadScalarAbsolute,
  Execute_LoadShortVectorAbsolute,
  Execute_LoadVectorAbsolute,
  Execute_LoadVectorControlRegisterAbsolute,
  Execute_LoadPixelAbsolute,
  Execute_LoadPixelZAbsolute,
  Execute_LoadByteLinear,
  Execute_LoadByteBilinearUV,
  Execute_LoadByteBilinearXY,
  Execute_LoadWordLinear,
  Execute_LoadWordBilinearUV,
  Execute_LoadWordBilinearXY,
  Execute_LoadScalarBilinearUV,
  Execute_LoadScalarBilinearXY,
  Execute_LoadShortVectorLinear,
  Execute_LoadShortVectorBilinearUV,
  Execute_LoadShortVectorBilinearXY,
  Execute_LoadVectorLinear,
  Execute_LoadVectorBilinearUV,
  Execute_LoadVectorBilinearXY,
  Execute_LoadPixelLinear,
  Execute_LoadPixelBilinearUV,
  Execute_LoadPixelBilinearXY,
  Execute_LoadPixelZLinear,
  Execute_LoadPixelZBilinearUV,
  Execute_LoadPixelZBilinearXY,
  Execute_StoreScalarAbsolute,
  Execute_StoreScalarControlRegisterAbsolute,
  Execute_StoreShortVectorAbsolute,
  Execute_StoreVectorAbsolute,
  Execute_StoreVectorControlRegisterAbsolute,
  Execute_StorePixelAbsolute,
  Execute_StorePixelZAbsolute,
  Execute_StoreScalarLinear,
  Execute_StoreScalarBilinearUV,
  Execute_StoreScalarBilinearXY,
  Execute_StoreShortVectorLinear,
  Execute_StoreShortVectorBilinearUV,
  Execute_StoreShortVectorBilinearXY,
  Execute_StoreVectorLinear,
  Execute_StoreVectorBilinearUV,
  Execute_StoreVectorBilinearXY,
  Execute_StorePixelLinear,
  Execute_StorePixelBilinearUV,
  Execute_StorePixelBilinearXY,
  Execute_StorePixelZLinear,
  Execute_StorePixelZBilinearUV,
  Execute_StorePixelZBilinearXY,
  Execute_StoreScalarControlRegisterImmediate,
  Execute_StoreScalarImmediate,
  //MISC Executes
  Execute_StoreScalarRegisterConstant,
  Execute_StoreMiscRegisterConstant,
  Execute_ECU_NOP,//Packet_Start
  Execute_ECU_NOP,//Packet_End
  Execute_CheckECUSkipCounter, //Packet_End for packets with branches or packets in delay slots (IL block only)
  Execute_SaveFlags,
  Execute_SaveRegs,
};

NuancePrintHandler printHandlers[] =
{
  //ECU HANDLERS
  Print_ECU_NOP,
  Print_Halt,
  Print_BRAAlways,
  Print_BRAAlways_NOP,
  Print_BRAConditional,
  Print_BRAConditional_NOP,
  Print_JMPAlwaysIndirect,
  Print_JMPAlwaysIndirect_NOP,
  Print_JMPConditionalIndirect,
  Print_JMPConditionalIndirect_NOP,
  Print_JSRAlways,
  Print_JSRAlways_NOP,
  Print_JSRConditional,
  Print_JSRConditional_NOP,
  Print_JSRAlwaysIndirect,
  Print_JSRAlwaysIndirect_NOP,
  Print_JSRConditionalIndirect,
  Print_JSRConditionalIndirect_NOP,
  Print_RTI1Conditional,
  Print_RTI1Conditional_NOP,
  Print_RTI2Conditional,
  Print_RTI2Conditional_NOP,
  Print_RTSAlways,
  Print_RTSAlways_NOP,
  Print_RTSConditional,
  Print_RTSConditional_NOP,
  //RCU HANDLERS
  Print_DEC,
  Print_DEC,
  Print_DEC,
  Print_ADDRImmediate,
  Print_ADDRImmediate,
  Print_ADDRScalar,
  Print_ADDRScalar,
  Print_MVRImmediate,
  Print_MVRImmediate,
  Print_MVRScalar,
  Print_MVRScalar,
  Print_Range,
  Print_Range,
  Print_Modulo,
  Print_Modulo,
  //ALU HANDLERS
  Print_ABS,
  Print_BITSScalar,
  Print_BITSImmediate,
  Print_BTST,
  Print_BUTT,
  Print_COPY,
  Print_MSB,
  Print_SAT,
  Print_AS,
  Print_ASL,
  Print_ASR,
  Print_LS,
  Print_LSR,
  Print_ROT,
  Print_ROL,
  Print_ROR,
  Print_ADD_P,
  Print_SUB_P,
  Print_ADD_SV,
  Print_SUB_SV,
  Print_ADDImmediate,
  Print_ADDScalar,
  Print_ADDScalarShiftRightImmediate,
  Print_ADDScalarShiftLeftImmediate,
  Print_SUBImmediate,
  Print_SUBImmediateReverse,
  Print_SUBScalar,
  Print_SUBScalarShiftRightImmediate,
  Print_SUBScalarShiftLeftImmediate,
  Print_CMPImmediate,
  Print_CMPImmediateReverse,
  Print_CMPScalar,
  Print_CMPScalarShiftRightImmediate,
  Print_CMPScalarShiftLeftImmediate,
  Print_ANDImmediate,
  Print_ANDScalar,
  Print_ANDImmediateShiftScalar,
  Print_ANDScalarShiftRightImmediate,
  Print_ANDScalarShiftLeftImmediate,
  Print_ANDScalarShiftScalar,
  Print_ANDScalarRotateScalar,
  Print_FTSTImmediate,
  Print_FTSTScalar,
  Print_FTSTImmediateShiftScalar,
  Print_FTSTScalarShiftRightImmediate,
  Print_FTSTScalarShiftLeftImmediate,
  Print_FTSTScalarShiftScalar,
  Print_FTSTScalarRotateScalar,
  Print_ORImmediate,
  Print_ORScalar,
  Print_ORImmediateShiftScalar,
  Print_ORScalarShiftRightImmediate,
  Print_ORScalarShiftLeftImmediate,
  Print_ORScalarShiftScalar,
  Print_ORScalarRotateScalar,
  Print_EORImmediate,
  Print_EORScalar,
  Print_EORImmediateShiftScalar,
  Print_EORScalarShiftRightImmediate,
  Print_EORScalarShiftLeftImmediate,
  Print_EORScalarShiftScalar,
  Print_EORScalarRotateScalar,
  Print_ADDWCScalar,
  Print_ADDWCImmediate,
  Print_ADDWCScalarShiftRightImmediate,
  Print_ADDWCScalarShiftLeftImmediate,
  Print_SUBWCScalar,
  Print_SUBWCImmediate,
  Print_SUBWCImmediateReverse,
  Print_SUBWCScalarShiftRightImmediate,
  Print_SUBWCScalarShiftLeftImmediate,
  Print_CMPWCScalar,
  Print_CMPWCImmediate,
  Print_CMPWCImmediateReverse,
  Print_CMPWCScalarShiftRightImmediate,
  Print_CMPWCScalarShiftLeftImmediate,
  //MUL HANDLERS
  Print_ADDM,
  Print_ADDMImmediate,
  Print_SUBM,
  Print_SUBMImmediateReverse,
  Print_MULScalarShiftAcshift,
  Print_MULScalarShiftRightImmediate,
  Print_MULScalarShiftLeftImmediate,
  Print_MULImmediateShiftAcshift,
  Print_MULScalarShiftScalar,
  Print_MULImmediateShiftScalar,
  Print_MULImmediateShiftRightImmediate,
  Print_MULImmediateShiftLeftImmediate,
  Print_MUL_SVImmediateShiftImmediate,
  Print_MUL_SVScalarShiftImmediate,
  Print_MUL_SVScalarShiftSvshift,
  Print_MUL_SVRuShiftImmediate,
  Print_MUL_SVRuShiftSvshift,
  Print_MUL_SVRvShiftImmediate,
  Print_MUL_SVRvShiftSvshift,
  Print_MUL_SVVectorShiftImmediate,
  Print_MUL_SVVectorShiftSvshift,
  Print_MUL_PImmediateShiftImmediate,
  Print_MUL_PScalarShiftImmediate,
  Print_MUL_PScalarShiftSvshift,
  Print_MUL_PRuShiftImmediate,
  Print_MUL_PRuShiftSvshift,
  Print_MUL_PRvShiftImmediate,
  Print_MUL_PRvShiftSvshift,
  Print_MUL_PVectorShiftImmediate,
  Print_MUL_PVectorShiftSvshift,
  Print_DOTPScalarShiftImmediate,
  Print_DOTPScalarShiftSvshift,
  Print_DOTPVectorShiftImmediate,
  Print_DOTPVectorShiftSvshift,
  //MEM HANDLERS
  Print_Mirror,
  Print_MV_SImmediate,
  Print_MV_SScalar,
  Print_MV_V,
  Print_PopVector,
  Print_PopVectorRz,
  Print_PopScalarRzi1,
  Print_PopScalarRzi2,
  Print_PushVector,
  Print_PushVectorRz,
  Print_PushScalarRzi1,
  Print_PushScalarRzi2,
  Print_LoadScalarLinear,
  Print_LoadScalarControlRegisterAbsolute,
  Print_LoadByteAbsolute,
  Print_LoadWordAbsolute,
  Print_LoadScalarAbsolute,
  Print_LoadShortVectorAbsolute,
  Print_LoadVectorAbsolute,
  Print_LoadVectorControlRegisterAbsolute,
  Print_LoadPixelAbsolute,
  Print_LoadPixelZAbsolute,
  Print_LoadByteLinear,
  Print_LoadByteBilinearUV,
  Print_LoadByteBilinearXY,
  Print_LoadWordLinear,
  Print_LoadWordBilinearUV,
  Print_LoadWordBilinearXY,
  Print_LoadScalarBilinearUV,
  Print_LoadScalarBilinearXY,
  Print_LoadShortVectorLinear,
  Print_LoadShortVectorBilinearUV,
  Print_LoadShortVectorBilinearXY,
  Print_LoadVectorLinear,
  Print_LoadVectorBilinearUV,
  Print_LoadVectorBilinearXY,
  Print_LoadPixelLinear,
  Print_LoadPixelBilinearUV,
  Print_LoadPixelBilinearXY,
  Print_LoadPixelZLinear,
  Print_LoadPixelZBilinearUV,
  Print_LoadPixelZBilinearXY,
  Print_StoreScalarAbsolute,
  Print_StoreScalarControlRegisterAbsolute,
  Print_StoreShortVectorAbsolute,
  Print_StoreVectorAbsolute,
  Print_StoreVectorControlRegisterAbsolute,
  Print_StorePixelAbsolute,
  Print_StorePixelZAbsolute,
  Print_StoreScalarLinear,
  Print_StoreScalarBilinearUV,
  Print_StoreScalarBilinearXY,
  Print_StoreShortVectorLinear,
  Print_StoreShortVectorBilinearUV,
  Print_StoreShortVectorBilinearXY,
  Print_StoreVectorLinear,
  Print_StoreVectorBilinearUV,
  Print_StoreVectorBilinearXY,
  Print_StorePixelLinear,
  Print_StorePixelBilinearUV,
  Print_StorePixelBilinearXY,
  Print_StorePixelZLinear,
  Print_StorePixelZBilinearUV,
  Print_StorePixelZBilinearXY,
  Print_StoreScalarControlRegisterImmediate,
  Print_StoreScalarImmediate,
  //Misc
  Print_StoreScalarRegisterConstant,
  Print_StoreMiscRegisterConstant,
  Print_PacketStart,
  Print_PacketEnd,
  Print_CheckECUSkipCounter,
  Print_SaveFlags,
  Print_SaveRegs,
};

NativeEmitHandler emitHandlers[] =
{
  //ECU Executes
  Emit_NOP,           //Execute_ECU_NOP
  0,                  //Execute_Halt
  Emit_BRAAlways,
  Emit_BRAAlways_NOP,
  Emit_BRAConditional,
  Emit_BRAConditional_NOP,
  Emit_JMPAlwaysIndirect,
  Emit_JMPAlwaysIndirect_NOP,
  Emit_JMPConditionalIndirect,
  Emit_JMPConditionalIndirect_NOP,
  Emit_JSRAlways,
  Emit_JSRAlways_NOP,
  Emit_JSRConditional,
  Emit_JSRConditional_NOP,
  Emit_JSRAlwaysIndirect,
  Emit_JSRAlwaysIndirect_NOP,
  Emit_JSRConditionalIndirect,
  Emit_JSRConditionalIndirect_NOP,
  Emit_RTI1Conditional,
  Emit_RTI1Conditional_NOP,
  Emit_RTI2Conditional,
  Emit_RTI2Conditional_NOP,
  Emit_RTSAlways,
  Emit_RTSAlways_NOP,
  Emit_RTSConditional,
  Emit_RTSConditional_NOP,
  //RCU Executes
  Emit_DECRc1,
  Emit_DECRc0,
  Emit_DECBoth,
  Emit_ADDRImmediateOnly,
  Emit_ADDRImmediate,    
  Emit_ADDRScalarOnly,   
  Emit_ADDRScalar,       
  Emit_MVRImmediateOnly, 
  Emit_MVRImmediate,     
  Emit_MVRScalarOnly,    
  Emit_MVRScalar,        
  Emit_RangeOnly,        
  Emit_Range,            
  Emit_ModuloOnly,       
  Emit_Modulo,           
  //ALU Executes
  Emit_ABS,
  Emit_BITSScalar,
  Emit_BITSImmediate,
  Emit_BTST,
  Emit_BUTT,
  Emit_COPY,
  Emit_MSB,
  Emit_SAT,
  Emit_AS,
  Emit_ASL,
  Emit_ASR,
  Emit_LS,
  Emit_LSR,
  Emit_ROT,
  Emit_ROL,
  Emit_ROR,
  Emit_ADD_P,
  Emit_SUB_P,
  Emit_ADD_SV,
  Emit_SUB_SV,
  Emit_ADDImmediate,
  Emit_ADDScalar,
  Emit_ADDScalarShiftRightImmediate,
  Emit_ADDScalarShiftLeftImmediate,
  Emit_SUBImmediate,
  Emit_SUBImmediateReverse,
  Emit_SUBScalar,
  Emit_SUBScalarShiftRightImmediate,
  Emit_SUBScalarShiftLeftImmediate,
  Emit_CMPImmediate,
  Emit_CMPImmediateReverse,
  Emit_CMPScalar,
  Emit_CMPScalarShiftRightImmediate,
  Emit_CMPScalarShiftLeftImmediate,
  Emit_ANDImmediate,
  Emit_ANDScalar,
  Emit_ANDImmediateShiftScalar,
  Emit_ANDScalarShiftRightImmediate,
  Emit_ANDScalarShiftLeftImmediate,
  Emit_ANDScalarShiftScalar,
  Emit_ANDScalarRotateScalar,
  Emit_FTSTImmediate,
  Emit_FTSTScalar,
  Emit_FTSTImmediateShiftScalar,
  Emit_FTSTScalarShiftRightImmediate,
  Emit_FTSTScalarShiftLeftImmediate,
  Emit_FTSTScalarShiftScalar,
  Emit_FTSTScalarRotateScalar,
  Emit_ORImmediate,
  Emit_ORScalar,
  Emit_ORImmediateShiftScalar,
  Emit_ORScalarShiftRightImmediate,
  Emit_ORScalarShiftLeftImmediate,
  Emit_ORScalarShiftScalar,
  Emit_ORScalarRotateScalar,
  Emit_EORImmediate,
  Emit_EORScalar,
  Emit_EORImmediateShiftScalar,
  Emit_EORScalarShiftRightImmediate,
  Emit_EORScalarShiftLeftImmediate,
  Emit_EORScalarShiftScalar,
  Emit_EORScalarRotateScalar,
  Emit_ADDWCScalar,
  Emit_ADDWCImmediate,
  Emit_ADDWCScalarShiftRightImmediate,
  Emit_ADDWCScalarShiftLeftImmediate,
  Emit_SUBWCScalar,
  Emit_SUBWCImmediate,
  Emit_SUBWCImmediateReverse,
  Emit_SUBWCScalarShiftRightImmediate,
  Emit_SUBWCScalarShiftLeftImmediate,
  Emit_CMPWCScalar,
  Emit_CMPWCImmediate,
  Emit_CMPWCImmediateReverse,
  Emit_CMPWCScalarShiftRightImmediate,
  Emit_CMPWCScalarShiftLeftImmediate,
  //MUL Executes
  Emit_ADDM,                    
  Emit_ADDMImmediate,                    
  Emit_SUBM,                    
  Emit_SUBMImmediateReverse,                    
  
  Emit_MULScalarShiftAcshift,                   
  Emit_MULScalarShiftRightImmediate,
  Emit_MULScalarShiftLeftImmediate,
  Emit_MULImmediateShiftAcshift,
  Emit_MULScalarShiftScalar,
  Emit_MULImmediateShiftScalar,
  Emit_MULImmediateShiftRightImmediate,
  Emit_MULImmediateShiftLeftImmediate,

  Emit_MUL_SVImmediateShiftImmediate,
  Emit_MUL_SVScalarShiftImmediate,
  Emit_MUL_SVScalarShiftSvshift,
  Emit_MUL_SVRuShiftImmediate,
  Emit_MUL_SVRuShiftSvshift,
  Emit_MUL_SVRvShiftImmediate,
  Emit_MUL_SVRvShiftSvshift,
  Emit_MUL_SVVectorShiftImmediate,
  Emit_MUL_SVVectorShiftSvshift,
  Emit_MUL_PImmediateShiftImmediate,
  Emit_MUL_PScalarShiftImmediate,
  Emit_MUL_PScalarShiftSvshift,
  Emit_MUL_PRuShiftImmediate,
  Emit_MUL_PRuShiftSvshift,
  Emit_MUL_PRvShiftImmediate,
  Emit_MUL_PRvShiftSvshift,
  Emit_MUL_PVectorShiftImmediate,
  Emit_MUL_PVectorShiftSvshift,
  Emit_DOTPScalarShiftImmediate,
  Emit_DOTPScalarShiftSvshift,
  Emit_DOTPVectorShiftImmediate,
  Emit_DOTPVectorShiftSvshift,
  //MEM Executes
  Emit_Mirror,
  Emit_MV_SImmediate,
  Emit_MV_SScalar,
  Emit_MV_V,
  Emit_PopVector,
  Emit_PopVectorRz,
  Emit_PopScalarRzi1,
  Emit_PopScalarRzi2,
  Emit_PushVector,
  Emit_PushVectorRz,
  Emit_PushScalarRzi1,
  Emit_PushScalarRzi2,
  Emit_LoadScalarLinear,
  Emit_LoadScalarControlRegisterAbsolute,
  Emit_LoadByteAbsolute,
  Emit_LoadWordAbsolute,
  Emit_LoadScalarAbsolute,
  Emit_LoadShortVectorAbsolute,
  Emit_LoadVectorAbsolute,
  0,                    //Execute_LoadVectorControlRegisterAbsolute,
  Emit_LoadPixelAbsolute,
  Emit_LoadPixelZAbsolute,
  Emit_LoadByteLinear,
  Emit_LoadByteBilinearUV,
  Emit_LoadByteBilinearXY,
  Emit_LoadWordLinear,
  Emit_LoadWordBilinearUV,
  Emit_LoadWordBilinearXY,
  Emit_LoadScalarBilinearUV,
  Emit_LoadScalarBilinearXY,
  Emit_LoadShortVectorLinear,
  Emit_LoadShortVectorBilinearUV,
  Emit_LoadShortVectorBilinearXY,
  Emit_LoadVectorLinear,
  Emit_LoadVectorBilinearUV,
  Emit_LoadVectorBilinearXY,
  Emit_LoadPixelLinear,
  Emit_LoadPixelBilinearUV,
  Emit_LoadPixelBilinearXY,
  Emit_LoadPixelZLinear,
  Emit_LoadPixelZBilinearUV,
  Emit_LoadPixelZBilinearXY,
  Emit_StoreScalarAbsolute,
  Emit_StoreScalarControlRegisterAbsolute,
  Emit_StoreShortVectorAbsolute,
  Emit_StoreVectorAbsolute,
  0,                    //Execute_StoreVectorControlRegisterAbsolute,
  Emit_StorePixelAbsolute,
  Emit_StorePixelZAbsolute,
  Emit_StoreScalarLinear,
  Emit_StoreScalarBilinearUV,
  Emit_StoreScalarBilinearXY,
  Emit_StoreShortVectorLinear,
  Emit_StoreShortVectorBilinearUV,
  Emit_StoreShortVectorBilinearXY,
  Emit_StoreVectorLinear,
  Emit_StoreVectorBilinearUV,
  Emit_StoreVectorBilinearXY,
  Emit_StorePixelLinear,
  Emit_StorePixelBilinearUV,
  Emit_StorePixelBilinearXY,
  Emit_StorePixelZLinear,
  Emit_StorePixelZBilinearUV,
  Emit_StorePixelZBilinearXY,
  Emit_StoreScalarControlRegisterImmediate,
  Emit_StoreScalarImmediate,
  //MISC Executes
  Emit_StoreScalarRegisterConstant,
  Emit_StoreMiscRegisterConstant,
  Emit_NOP,            //Packet_Start
  Emit_NOP,            //Packet_End
  Emit_NOP,            //Packet_End (CheckECUSkipCounter)
  Emit_SaveRegs,
  Emit_SaveRegs,
};

#ifdef LOG_STUFF
static FILE *logfile = NULL;
#endif

void MPE::Init(const uint32 index, uint8* mainBusPtr, uint8* systemBusPtr, uint8* flashEEPROMPtr)
{
  constexpr uint32 numCacheEntries[] = {4096,2048,2048,262144};
  //constexpr uint32 numTLBEntries[] = {4096,2048,2048,98304};
  constexpr uint32 overlayLengths[] = {8192,4096,4096,4096};

  numInterpreterCacheFlushes = 0;
  numNativeCodeCacheFlushes = 0;
  numNonCompilablePackets = 0;
  assert(index < 4);
  mpeIndex = index;
  InitMPELocalMemory();
  //nativeCodeCache = new NativeCodeCache(5UL*1024UL*1024UL/*, numTLBEntries[mpeIndex]*/);
  instructionCache = new InstructionCache(numCacheEntries[mpeIndex]);
  overlayManager.SetOverlayLength(overlayLengths[mpeIndex]);
  bInvalidateInstructionCaches = false;
  bInvalidateInterpreterCache = false;
  overlayMask = 0;

  interpretNextPacket = 0;

  nativeCodeCache.SetEmitVars(this);

  if(mpeIndex == LOG_MPE_INDEX)
  {      
#ifdef LOG_STUFF
    if(!logfile)
      logfile = fopen("logfile","w");
#endif

#ifdef LOG_COMM
    if(!commLogFile)
      commLogFile = fopen("commlog","w");
#endif
  }
  Reset();

//Initialize the bank pointer lookup table for use with indirect
//loads and stores.  This table assumes that the only banks present
//are local MPE memory, system bus memory and main bus memory.  All
//other banks map to main bus memory in the hope that a bad memory address
//write will simply cause main bus graphics corruption

//This table cannot distinguish between the ROM, reserved and other bus IO
//banks in the $F0000000/$F1000000/$FE000000 range but programs should
//never read or write in this range anyways.  If these banks are emulated
//at some future point, this table will need to expand to 256 entries in
//order to factor in the entire upper byte of the 32 bit address range

  bankPtrTable[0x0] = dtrom;
  bankPtrTable[0x1] = dtrom;
  bankPtrTable[0x2] = dtrom;
  bankPtrTable[0x3] = mainBusPtr;
  bankPtrTable[0x4] = mainBusPtr;
  bankPtrTable[0x5] = mainBusPtr;
  bankPtrTable[0x6] = mainBusPtr;
  bankPtrTable[0x7] = mainBusPtr;
  bankPtrTable[0x8] = systemBusPtr;
  bankPtrTable[0x9] = systemBusPtr;
  bankPtrTable[0xA] = systemBusPtr;
  bankPtrTable[0xB] = systemBusPtr;
  bankPtrTable[0xC] = systemBusPtr;
  bankPtrTable[0xD] = systemBusPtr;
  bankPtrTable[0xE] = systemBusPtr;
  bankPtrTable[0xF] = flashEEPROMPtr;
}

MPE::~MPE()
{
  //delete nativeCodeCache;
  delete instructionCache;

  if(mpeIndex == LOG_MPE_INDEX)
  {
#ifdef LOG_STUFF
    if(logfile)
      fclose(logfile);
#endif
#ifdef LOG_COMM
    if(commLogFile)
      fclose(commLogFile);
#endif
  }
}

void MPE::InitMPELocalMemory()
{
  init_nuon_mem(dtrom, MPE_LOCAL_MEMORY_SIZE);
}

void MPE::Reset()
{
  //Explicitly clear CSTATE bits to zero so that code works in debug mode
  dcachectl &= ~0xF0000000;
  icachectl &= ~0xF0000000;

  instructionCache->Invalidate();
  nativeCodeCache.Flush();

  invalidateRegionStart = MPE_IRAM_BASE;
  invalidateRegionEnd = MPE_IRAM_BASE + OVERLAY_SIZE - 1;
  interpreterInvalidateRegionStart = 0;
  interpreterInvalidateRegionEnd = 0;

  //overlayIndex = 0;

  //Interpretation of Nuances require the use of the cc composite flags register
  ecuSkipCounter = 0;
  pcfetchnext = 0x20300000;
  pcfetch = 0x20300000;
  pcroute = 0x20300000;
  pcexec = 0x20300000;
  sp = 0x20001000;
  //mpectl is halted on reset and has the was-reset bit set
  mpectl = MPECTRL_MPEWASRESET;
  //all exceptions halt the processor
  excephalten = 0x1FFF;
  //reset all exceptions
  excepsrc = 0;
  //intctl is set to 0 on reset according to SDK docs
  intctl = 0;
  //reset all interrupt sources
  intsrc = 0;
  //reset interupt enable registers
  inten1 = 0;
  //set level 2 selector to software
  inten2sel = 1;
  //enable other bus dma, no commands pending, no other bus activity
  odmactl = 1UL << 5;
  //set bus priority for MDMA transfers to 3 and clear all other bits
  mdmactl = 3UL << 5;
  commxmit[0] = 0;
  commxmit[1] = 0;
  commxmit[2] = 0;
  commxmit[3] = 0;
  commrecv[0] = 0;
  commrecv[1] = 0;
  commrecv[2] = 0;
  commrecv[3] = 0;
  acshift = 0;
  svshift = 0;
  commctl = 0;
  rc0 = 0;
  rc1 = 0;
  cc = (CC_COUNTER0_ZERO | CC_COUNTER1_ZERO);
  cycleCounter = 0;
  //Nuon = aries 2, MPE release = aries 2, mpe identifier, halted on reset
  configa = (3UL << 24) | (3UL << 16) | (mpeIndex << 8) | 0;
  configb = 0;
}

#if 0
bool MPE::LoadBinaryFile(uchar *filename, bool bIRAM)
{
  const int handle = _open((char *)filename,_O_RDONLY|_O_BINARY,0);
  if(handle >= 0)
  {
    const int byteLength = _filelength(handle);
    uint8* const buffer = &dtrom[bIRAM ? MPE_IRAM_OFFSET : MPE_IROM_OFFSET];

    _read(handle, buffer, byteLength);
    _close(handle);
    return true;
  }
  else
  {
    return false;
  }
}
#endif

inline uint32 MPE::GetPacketDelta(const uint8 *iPtr, uint32 numLevels)
{
  bool bTerminating;

  uint8 packetBytes = 0;
  uint8 deltaBytes = 0;

  while(numLevels != 0)
  {
    const uint8 opcode = *iPtr;
    if(opcode <= 0x3F)
    {
      bTerminating = true;
      deltaBytes += 2;
      packetBytes += 2;
      iPtr += 2;
    }
    else
    {
      //Instruction is not a 16 bit ALU instruction

      //If 16 bit non-terminating non-ALU instruction, control instruction
      //or 32 bit immediate extension
      if(opcode < 0x88 || opcode > 0x9F)
      {
        //If not a control instruction (PAD, NOP or BREAKPOINT)
        if((opcode & 0xFC) != 0x80)
        {
          //if bit 15 is set, the instruction is a packet terminator
          bTerminating = (opcode & 0x80);
        }
        else
        {
          //if bit 8 is set, the instruction is a terminating breakpoint or NOP
          bTerminating = (opcode & 0x01);
        }

        deltaBytes += 2;
        packetBytes += 2;
        iPtr += 2;
      }
      else if(opcode >= 0x90)
      {
        //32 bit instruction: if bit 12 is set, instruction is a packet terminator
        bTerminating = *(iPtr + 2) & 0x10;
        deltaBytes += 4;
        packetBytes += 4;
        iPtr += 4;
      }
      else
      {
        //32 bit extension (preceeding 48/64 bit instruction)
        bTerminating = false;
        deltaBytes += 4;
        packetBytes += 4;
        iPtr += 4;
      }
    }

    if(bTerminating || (packetBytes >= 128))
    {
      numLevels--;
      packetBytes = 0;
    }
  }

  return deltaBytes;
}

void MPE::DecompressPacket(const uint8 *iBuffer, InstructionCacheEntry &pICacheEntry, const uint32 options) //!! pICacheEntry is already class member, looks like this is intended here though
{
  InstructionCacheEntry pStruct;

  pStruct.pcexec = pICacheEntry.pcexec;
  pICacheEntry.pcroute = pICacheEntry.pcexec;
  pICacheEntry.nuanceCount = 0;
  pStruct.packetInfo = 0;

  pStruct.ClearDependencies();
  pICacheEntry.ClearDependencies();

  uint32 immExt = 0; // do not pull this into the loop!
  uint32 packetByteCount = 0;
  bool bTerminating = false;

  do
  {
    const uint8 iLength = DecodeSingleInstruction(iBuffer,&pStruct,&immExt,bTerminating);

    pICacheEntry.pcroute += iLength;

    if(packetByteCount >= 128)
    {
      pICacheEntry.pcroute = pICacheEntry.pcexec + 128;
      break;
    }

    iBuffer += iLength;
  }
  while(!bTerminating);

  pStruct.nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_PCROUTE)] = pICacheEntry.pcroute;

  if(pStruct.packetInfo & PACKETINFO_NEEDS_PCFETCHNEXT)
  {
    pICacheEntry.pcfetchnext = pICacheEntry.pcroute + GetPacketDelta(iBuffer, 2);
    pStruct.nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_PCFETCHNEXT)] = pICacheEntry.pcfetchnext;
  }
  else
    pICacheEntry.pcfetchnext = 0; // just to avoid uninited mem

  if(options & DECOMPRESS_OPTIONS_INHIBIT_ECU)
  {
    pStruct.packetInfo &= ~(PACKETINFO_ECU | PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
  }

  const uint32 executionUnits = GETPACKETEXECUTIONUNITS(pStruct.packetInfo);
 
  switch(executionUnits)
  {
    case 0:
      pICacheEntry.nuanceCount = 0;
      break;
    case (PACKETINFO_ECU >> 2):
      pICacheEntry.nuanceCount = 1;
      pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
      break;
    case (PACKETINFO_RCU >> 2):
      pICacheEntry.nuanceCount = 1;
      pICacheEntry.CopyInstructionData(0,pStruct,SLOT_RCU);
      break;
    case ((PACKETINFO_RCU | PACKETINFO_ECU) >> 2):
      pICacheEntry.nuanceCount = 2;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_RCU);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
      }
      else
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
      }    
      break;
    case (PACKETINFO_MEM >> 2):
      pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
      pICacheEntry.nuanceCount = 1;
      break;
    case ((PACKETINFO_MEM | PACKETINFO_ECU) >> 2):
    {
      uint32 ecuIndex, memIndex;
      pICacheEntry.nuanceCount = 2;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_ECU,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
      {
        memIndex = 0;
        ecuIndex = 1;
      }
      else
      {
        memIndex = 1;
        ecuIndex = 0;
      }       
      pICacheEntry.CopyInstructionData(ecuIndex,pStruct,SLOT_ECU);
      pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
      break;
    }
    case ((PACKETINFO_MEM | PACKETINFO_RCU) >> 2):
    {
      uint32 rcuIndex, memIndex;
      pICacheEntry.nuanceCount = 2;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_RCU,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
      {
        memIndex = 0;
        rcuIndex = 1;
      }
      else
      {
        memIndex = 1;
        rcuIndex = 0;
      }       
      pICacheEntry.CopyInstructionData(rcuIndex,pStruct,SLOT_RCU);
      pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
      break;
    }
    case ((PACKETINFO_MEM | PACKETINFO_ECU | PACKETINFO_RCU) >> 2):
    {
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        uint32 rcuIndex, memIndex;
        if(ChooseInstructionPairOrdering(pStruct,SLOT_RCU,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
        {
          memIndex = 0;
          rcuIndex = 1;
        }
        else
        {
          memIndex = 1;
          rcuIndex = 0;
        }        
        pICacheEntry.CopyInstructionData(rcuIndex,pStruct,SLOT_RCU);
        pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
        }
        else
        {
          ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_MEM,SLOT_ECU,SLOT_RCU);
        }
      }
      break;
    }
    case (PACKETINFO_MUL >> 2):
      pICacheEntry.nuanceCount = 1;
      pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MUL);
      break;
    case ((PACKETINFO_MUL | PACKETINFO_ECU) >> 2):
      pICacheEntry.nuanceCount = 2;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MUL);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
      }
      else
      {
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MUL);
      }
      break;
    case ((PACKETINFO_MUL | PACKETINFO_RCU) >> 2):
    {
      uint32 rcuIndex, mulIndex;
      pICacheEntry.nuanceCount = 2;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_RCU,SLOT_MUL))
      {
        mulIndex = 0;
        rcuIndex = 1;
      }
      else
      {
        mulIndex = 1;
        rcuIndex = 0;
      }
      pICacheEntry.CopyInstructionData(rcuIndex,pStruct,SLOT_RCU);
      pICacheEntry.CopyInstructionData(mulIndex,pStruct,SLOT_MUL);
      break;
    }
    case ((PACKETINFO_MUL | PACKETINFO_RCU | PACKETINFO_ECU) >> 2):
    {
      uint32 ecuIndex, baseIndex;
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        baseIndex = 0;
        ecuIndex = 2;
      }
      else
      {
        baseIndex = 1;
        ecuIndex = 0;
      }

      uint32 rcuIndex, mulIndex;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_RCU,SLOT_MUL))
      {
        mulIndex = baseIndex + 0;
        rcuIndex = baseIndex + 1;
      }
      else
      {
        mulIndex = baseIndex + 1;
        rcuIndex = baseIndex + 0;
      }
      
      pICacheEntry.CopyInstructionData(ecuIndex,pStruct,SLOT_ECU);
      pICacheEntry.CopyInstructionData(rcuIndex,pStruct,SLOT_RCU);
      pICacheEntry.CopyInstructionData(mulIndex,pStruct,SLOT_MUL);
      break;
    }
    case ((PACKETINFO_MUL | PACKETINFO_MEM) >> 2):
    {
      uint32 mulIndex, memIndex;
      pICacheEntry.nuanceCount = 2;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_MUL,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
      {
        memIndex = 0;
        mulIndex = 1;
      }
      else
      {
        memIndex = 1;
        mulIndex = 0;
      }       
      pICacheEntry.CopyInstructionData(mulIndex,pStruct,SLOT_MUL);
      pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
      break;
    }
    case ((PACKETINFO_MUL | PACKETINFO_MEM | PACKETINFO_ECU) >> 2):
    {
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        uint32 mulIndex, memIndex;
        if(ChooseInstructionPairOrdering(pStruct,SLOT_MUL,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
        {
          memIndex = 0;
          mulIndex = 1;
        }
        else
        {
          memIndex = 1;
          mulIndex = 0;
        }       
        
        pICacheEntry.CopyInstructionData(mulIndex,pStruct,SLOT_MUL);
        pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_MUL);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
        }
        else
        {
          ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_MUL,SLOT_MEM,SLOT_ECU);
        }
      }
      break;
    }
    case ((PACKETINFO_MUL | PACKETINFO_MEM | PACKETINFO_RCU) >> 2): 
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_MUL);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_RCU);
      }
      else
      {
        ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_MUL,SLOT_MEM,SLOT_RCU);
      }
      break;
    case ((PACKETINFO_MUL | PACKETINFO_MEM | PACKETINFO_ECU | PACKETINFO_RCU) >> 2): 
      pICacheEntry.nuanceCount = 4;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_MUL);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        }
        else
        {
          ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_MUL,SLOT_MEM,SLOT_RCU);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        }
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_MUL);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        }
        else
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
          ScheduleInstructionTriplet(pICacheEntry,1,pStruct,SLOT_MUL,SLOT_MEM,SLOT_RCU);
        }
      }
      break;
    case (PACKETINFO_ALU >> 2):
      pICacheEntry.nuanceCount = 1;
      pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ALU);
      break;
    case ((PACKETINFO_ALU | PACKETINFO_ECU) >> 2):
      pICacheEntry.nuanceCount = 2;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ALU);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
      }
      else
      {
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ALU);
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_RCU) >> 2):
      pICacheEntry.nuanceCount = 2;
      pICacheEntry.CopyInstructionData(0,pStruct,SLOT_RCU);
      pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ALU);
      break;
    case ((PACKETINFO_ALU | PACKETINFO_RCU | PACKETINFO_ECU) >> 2):
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_RCU);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ALU);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
      }
      else
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ALU);
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MEM) >> 2):
    {
      uint32 aluIndex, memIndex;
      pICacheEntry.nuanceCount = 2;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_ALU,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
      {
        memIndex = 0;
        aluIndex = 1;
      }
      else
      {
        memIndex = 1;
        aluIndex = 0;
      }       
      pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
      pICacheEntry.CopyInstructionData(aluIndex,pStruct,SLOT_ALU);
      break;
    }
    case ((PACKETINFO_ALU | PACKETINFO_MEM | PACKETINFO_ECU) >> 2): 
    {
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        uint32 aluIndex, memIndex;
        if(ChooseInstructionPairOrdering(pStruct,SLOT_ALU,SLOT_MEM) || (options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST))
        {
          memIndex = 0;
          aluIndex = 1;
        }
        else
        {
          memIndex = 1;
          aluIndex = 0;
        }       
        pICacheEntry.CopyInstructionData(memIndex,pStruct,SLOT_MEM);
        pICacheEntry.CopyInstructionData(aluIndex,pStruct,SLOT_ALU);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ALU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ECU);
        }
        else
        {
          ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MEM,SLOT_ECU);
        }
      }
      break;
    }
    case ((PACKETINFO_ALU | PACKETINFO_MEM | PACKETINFO_RCU) >> 2): 
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ALU);
      }
      else
      {
        ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MEM,SLOT_RCU);
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MEM | PACKETINFO_ECU | PACKETINFO_RCU) >> 2): 
      pICacheEntry.nuanceCount = 4;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ALU);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        }
        else
        {
          ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MEM,SLOT_RCU);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        }
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {    
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_RCU);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ALU);
        }
        else
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
          ScheduleInstructionTriplet(pICacheEntry,1,pStruct,SLOT_ALU,SLOT_MEM,SLOT_RCU);
        }
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MUL) >> 2):
    {
      uint32 aluIndex, mulIndex;
      pICacheEntry.nuanceCount = 2;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_ALU,SLOT_MUL))
      {
        mulIndex = 0;
        aluIndex = 1;
      }
      else
      {
        mulIndex = 1;
        aluIndex = 0;
      }
      pICacheEntry.CopyInstructionData(mulIndex,pStruct,SLOT_MUL);
      pICacheEntry.CopyInstructionData(aluIndex,pStruct,SLOT_ALU);
      break;
    }
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_ECU) >> 2):
    {
      uint32 ecuIndex, baseIndex;
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        ecuIndex = 2;
        baseIndex = 0;
      }
      else
      {
        ecuIndex = 0;
        baseIndex = 1;
      }
      
      uint32 aluIndex, mulIndex;
      if(ChooseInstructionPairOrdering(pStruct,SLOT_ALU,SLOT_MUL))
      {
        mulIndex = baseIndex + 0;
        aluIndex = baseIndex + 1;
      }
      else
      {
        mulIndex = baseIndex + 1;
        aluIndex = baseIndex + 0;
      }
      pICacheEntry.CopyInstructionData(ecuIndex,pStruct,SLOT_ECU);
      pICacheEntry.CopyInstructionData(mulIndex,pStruct,SLOT_MUL);
      pICacheEntry.CopyInstructionData(aluIndex,pStruct,SLOT_ALU);
      break;
    }
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_RCU) >> 2):
      pICacheEntry.nuanceCount = 3;
      ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MUL,SLOT_RCU);
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_RCU | PACKETINFO_ECU) >> 2):
      pICacheEntry.nuanceCount = 4;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MUL,SLOT_RCU);
      }
      else
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
        ScheduleInstructionTriplet(pICacheEntry,1,pStruct,SLOT_ALU,SLOT_MUL,SLOT_RCU);
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_MEM) >> 2):
      pICacheEntry.nuanceCount = 3;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
        pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ALU);
        pICacheEntry.CopyInstructionData(2,pStruct,SLOT_MUL);
      }
      else
      {
        ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MUL,SLOT_MEM);
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_MEM | PACKETINFO_ECU) >> 2): 
      pICacheEntry.nuanceCount = 4;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ALU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_MUL);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
        }
        else
        {
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_ECU);
          ScheduleInstructionTriplet(pICacheEntry,0,pStruct,SLOT_ALU,SLOT_MUL,SLOT_MEM);
        }
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ALU);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_MUL);
        }
        else
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
          ScheduleInstructionTriplet(pICacheEntry,1,pStruct,SLOT_ALU,SLOT_MUL,SLOT_MEM);
        }
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_MEM | PACKETINFO_RCU) >> 2): 
      pICacheEntry.nuanceCount = 4;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
      {
        pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
        ScheduleInstructionTriplet(pICacheEntry,1,pStruct,SLOT_ALU,SLOT_MUL,SLOT_RCU);
      }
      else
      {
        ScheduleInstructionQuartet(pICacheEntry,0,pStruct);
      }
      break;
    case ((PACKETINFO_ALU | PACKETINFO_MUL | PACKETINFO_MEM | PACKETINFO_ECU | PACKETINFO_RCU) >> 2): 
      pICacheEntry.nuanceCount = 5;
      if(options & DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST)
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_RCU);
          pICacheEntry.CopyInstructionData(2,pStruct,SLOT_ALU);
          pICacheEntry.CopyInstructionData(3,pStruct,SLOT_MUL);
          pICacheEntry.CopyInstructionData(4,pStruct,SLOT_ECU);
        }
        else
        {
          pICacheEntry.CopyInstructionData(4,pStruct,SLOT_ECU);
          ScheduleInstructionQuartet(pICacheEntry,0,pStruct);
        }
      }
      else
      {
        if(options & DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST)
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_MEM);
          pICacheEntry.CopyInstructionData(1,pStruct,SLOT_ECU);
          ScheduleInstructionTriplet(pICacheEntry,2,pStruct,SLOT_RCU,SLOT_ALU,SLOT_MUL);
        }
        else
        {
          pICacheEntry.CopyInstructionData(0,pStruct,SLOT_ECU);
          ScheduleInstructionQuartet(pICacheEntry,1,pStruct);
        }
      }
      break;
    default:
      assert(false);
      break;
  }

  uint32 comboScalarInDep = 0;
  uint32 comboMiscInDep = 0;
  uint32 comboScalarOutDep = pICacheEntry.nuanceCount > 0 ? pICacheEntry.scalarOutputDependencies[0] : 0;
  uint32 comboMiscOutDep = pICacheEntry.nuanceCount > 0 ? pICacheEntry.miscOutputDependencies[0] : 0;

  for(uint32 i = 1; i < pICacheEntry.nuanceCount; i++)
  {
    comboScalarInDep |= (pICacheEntry.scalarInputDependencies[i] & comboScalarOutDep);
    comboMiscInDep |= (pICacheEntry.miscInputDependencies[i] & comboMiscOutDep);
    comboScalarOutDep |= pICacheEntry.scalarOutputDependencies[i];
    comboMiscOutDep |= pICacheEntry.miscOutputDependencies[i];
  }

  pICacheEntry.packetInfo = pStruct.packetInfo;
  pICacheEntry.ecuConditionCode = pStruct.ecuConditionCode;

  if(!nuonEnv.compilerOptions.bAllowCompile)
  {
    pICacheEntry.packetInfo |= PACKETINFO_NEVERCOMPILE;
  }

  if(comboScalarInDep | comboMiscInDep)
  {
    pICacheEntry.packetInfo |= PACKETINFO_DEPENDENCY_PRESENT;

    pICacheEntry.pRegs = tempreg_union;
  }
  else
    pICacheEntry.pRegs = reg_union;

  for(uint32 i = 0; i < pICacheEntry.nuanceCount; ++i)
    pICacheEntry.handlers[i] = pICacheEntry.nuances[i*5];
}

inline bool MPE::ChooseInstructionPairOrdering(const InstructionCacheEntry &entry, const uint32 slot1, const uint32 slot2)
{
  const uint32 xScalarInDep = entry.scalarInputDependencies[slot1];
  const uint32 yScalarInDep = entry.scalarInputDependencies[slot2];
  const uint32 xMiscInDep = entry.miscInputDependencies[slot1];
  const uint32 yMiscInDep = entry.miscInputDependencies[slot2];
  const uint32 xScalarOutDep = entry.scalarOutputDependencies[slot1];
  const uint32 yScalarOutDep = entry.scalarOutputDependencies[slot2];
  const uint32 xMiscOutDep = entry.miscOutputDependencies[slot1];
  const uint32 yMiscOutDep = entry.miscOutputDependencies[slot2];
  return (OnesCount(yScalarInDep & xScalarOutDep) + OnesCount(yMiscInDep & xMiscOutDep)) 
       > (OnesCount(xScalarInDep & yScalarOutDep) + OnesCount(xMiscInDep & yMiscOutDep));
}

void MPE::ScheduleInstructionTriplet(InstructionCacheEntry &destEntry, const uint32 baseSlot, const InstructionCacheEntry &srcEntry, const uint32 slot1, const uint32 slot2, const uint32 slot3)
{
  static constexpr uint32 destSlot1[6] = {0,0,1,1,2,2};
  static constexpr uint32 destSlot2[6] = {1,2,2,0,0,1};
  static constexpr uint32 destSlot3[6] = {2,1,0,2,1,0};

/*
  After completion, scores will contain the dependency count for the following permuations:
  {[1,2,3],[1,3,2],[2,3,1],[2,1,3],[3,1,2],[3,2,1]}.  Choose the permutation with the lowest
  score.
*/

  const uint32 comboScalarOutDep12 = srcEntry.scalarOutputDependencies[slot1] | srcEntry.scalarOutputDependencies[slot2];
  const uint32 comboMiscOutDep12 = srcEntry.miscOutputDependencies[slot1] | srcEntry.miscOutputDependencies[slot2];

  uint32 tempScalarInDep = srcEntry.scalarInputDependencies[slot3] & comboScalarOutDep12;
  uint32 tempMiscInDep = srcEntry.miscInputDependencies[slot3] & comboMiscOutDep12;

  uint32 scores[6];
  scores[0] =
    OnesCount((srcEntry.scalarInputDependencies[slot2] & srcEntry.scalarOutputDependencies[slot1]) |
              (tempScalarInDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot2] & srcEntry.miscOutputDependencies[slot1]) |
              (tempMiscInDep));

  scores[3] = 
    OnesCount((srcEntry.scalarInputDependencies[slot1] & srcEntry.scalarOutputDependencies[slot2]) |
              (tempScalarInDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot1] & srcEntry.miscOutputDependencies[slot2]) |
              (tempMiscInDep));

  const uint32 comboScalarOutDep13 = srcEntry.scalarOutputDependencies[slot1] | srcEntry.scalarOutputDependencies[slot3];
  const uint32 comboMiscOutDep13 = srcEntry.miscOutputDependencies[slot1] | srcEntry.miscOutputDependencies[slot3];

  tempScalarInDep = srcEntry.scalarInputDependencies[slot2] & comboScalarOutDep13;
  tempMiscInDep = srcEntry.miscInputDependencies[slot2] & comboMiscOutDep13;

  scores[1] = 
    OnesCount((srcEntry.scalarInputDependencies[slot3] & srcEntry.scalarOutputDependencies[slot1]) |
              (tempScalarInDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot3] & srcEntry.miscOutputDependencies[slot1]) |
              (tempMiscInDep));

  scores[4] = 
    OnesCount((srcEntry.scalarInputDependencies[slot1] & srcEntry.scalarOutputDependencies[slot3]) |
              (tempScalarInDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot1] & srcEntry.miscOutputDependencies[slot3]) |
              (tempMiscInDep));

  const uint32 comboScalarOutDep23 = srcEntry.scalarOutputDependencies[slot2] | srcEntry.scalarOutputDependencies[slot3];
  const uint32 comboMiscOutDep23 = srcEntry.miscOutputDependencies[slot2] | srcEntry.miscOutputDependencies[slot3];

  tempScalarInDep = srcEntry.scalarInputDependencies[slot1] & comboScalarOutDep23;
  tempMiscInDep = srcEntry.miscInputDependencies[slot1] & comboMiscOutDep23;

  scores[2] = 
    OnesCount((srcEntry.scalarInputDependencies[slot3] & srcEntry.scalarOutputDependencies[slot2]) |
              (tempScalarInDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot3] & srcEntry.miscOutputDependencies[slot2]) |
              (tempMiscInDep));

  scores[5] = 
    OnesCount((srcEntry.scalarInputDependencies[slot2] & srcEntry.scalarOutputDependencies[slot3]) |
              (tempScalarInDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot2] & srcEntry.miscOutputDependencies[slot3]) |
              (tempMiscInDep));

  uint32 minVal = scores[0];
  uint32 minIndex = 0;

  for(uint32 i = 1; i < 6; i++)
  {
    if(scores[i] <= minVal)
    {
      minIndex = i;
      minVal = scores[i];
    }
  }

  destEntry.CopyInstructionData(baseSlot + destSlot1[minIndex], srcEntry, slot1);
  destEntry.CopyInstructionData(baseSlot + destSlot2[minIndex], srcEntry, slot2);
  destEntry.CopyInstructionData(baseSlot + destSlot3[minIndex], srcEntry, slot3);
}

#if 0
uint32 MPE::ScoreInstructionTriplet(const InstructionCacheEntry &srcEntry, const uint32 slot1, const uint32 slot2, const uint32 slot3)
{
  const uint32 comboMiscOutDep = srcEntry.miscOutputDependencies[slot1] | srcEntry.miscOutputDependencies[slot2];
  const uint32 comboScalarOutDep = srcEntry.scalarOutputDependencies[slot1] | srcEntry.scalarOutputDependencies[slot2];

  return OnesCount((srcEntry.scalarInputDependencies[slot2] & srcEntry.scalarOutputDependencies[slot1]) |
    (srcEntry.scalarInputDependencies[slot3] & comboScalarOutDep)) +
    OnesCount((srcEntry.miscInputDependencies[slot2] & srcEntry.miscOutputDependencies[slot1]) |
    (srcEntry.miscInputDependencies[slot3] & comboMiscOutDep));
}
#endif

void MPE::GetInstructionTripletDependencies(uint32& comboScalarDep, uint32& comboMiscDep, const InstructionCacheEntry &srcEntry, const uint32 slot1, const uint32 slot2, const uint32 slot3)
{
  const uint32 comboMiscOutDep = srcEntry.miscOutputDependencies[slot1] | srcEntry.miscOutputDependencies[slot2];
  const uint32 comboScalarOutDep = srcEntry.scalarOutputDependencies[slot1] | srcEntry.scalarOutputDependencies[slot2];

  comboMiscDep = (srcEntry.miscInputDependencies[slot2] & srcEntry.miscOutputDependencies[slot1]) |
    (srcEntry.miscInputDependencies[slot3] & comboMiscOutDep);
  comboScalarDep = (srcEntry.scalarInputDependencies[slot2] & srcEntry.scalarOutputDependencies[slot1]) |
    (srcEntry.scalarInputDependencies[slot3] & comboScalarOutDep);
}

void MPE::ScheduleInstructionQuartet(InstructionCacheEntry &destEntry, const uint32 baseSlot, const InstructionCacheEntry &srcEntry)
{
  static constexpr uint32 destSlotRCU[6] = {0,0,1,1,2,0};
  static constexpr uint32 destSlotALU[6] = {1,2,2,3,3,3};
  static constexpr uint32 destSlotMUL[6] = {2,1,0,2,1,2};
  static constexpr uint32 destSlotMEM[6] = {3,3,3,0,0,1};

  uint32 tempScalarDep1 = srcEntry.scalarOutputDependencies[SLOT_RCU] | srcEntry.scalarOutputDependencies[SLOT_MUL];
  uint32 tempMiscDep1 = srcEntry.miscOutputDependencies[SLOT_RCU] | srcEntry.miscOutputDependencies[SLOT_MUL];
  
  const uint32 comboScalarOutDep1 = srcEntry.scalarOutputDependencies[SLOT_ALU] | tempScalarDep1;
  const uint32 comboMiscOutDep1 = srcEntry.miscOutputDependencies[SLOT_ALU] | tempMiscDep1;

  const uint32 comboScalarOutDep2 = srcEntry.scalarOutputDependencies[SLOT_MEM] | tempScalarDep1;
  const uint32 comboMiscOutDep2 = srcEntry.miscOutputDependencies[SLOT_MEM] | tempMiscDep1;

  uint32 tempScalarDep2 = srcEntry.scalarInputDependencies[SLOT_MEM] & comboScalarOutDep1;
  uint32 tempMiscDep2 = srcEntry.miscInputDependencies[SLOT_MEM] & comboMiscOutDep1;

  GetInstructionTripletDependencies(tempScalarDep1,tempMiscDep1,srcEntry,SLOT_RCU,SLOT_ALU,SLOT_MUL);
  uint32 scores[6];
  scores[0] = OnesCount(tempScalarDep1 | tempScalarDep2) + OnesCount(tempMiscDep1 | tempMiscDep2);

  GetInstructionTripletDependencies(tempScalarDep1,tempMiscDep1,srcEntry,SLOT_RCU,SLOT_MUL,SLOT_ALU);
  scores[1] = OnesCount(tempScalarDep1 | tempScalarDep2) + OnesCount(tempMiscDep1 | tempMiscDep2);

  GetInstructionTripletDependencies(tempScalarDep1,tempMiscDep1,srcEntry,SLOT_MUL,SLOT_RCU,SLOT_ALU);
  scores[2] = OnesCount(tempScalarDep1 | tempScalarDep2) + OnesCount(tempMiscDep1 | tempMiscDep2);

  tempScalarDep2 = srcEntry.scalarInputDependencies[SLOT_ALU] & comboScalarOutDep2;
  tempMiscDep2 = srcEntry.miscInputDependencies[SLOT_ALU] & comboMiscOutDep2;

  GetInstructionTripletDependencies(tempScalarDep1,tempMiscDep1,srcEntry,SLOT_MEM,SLOT_RCU,SLOT_MUL);
  scores[3] = OnesCount(tempScalarDep1 | tempScalarDep2) + OnesCount(tempMiscDep1 | tempMiscDep2);

  GetInstructionTripletDependencies(tempScalarDep1,tempMiscDep1,srcEntry,SLOT_MEM,SLOT_MUL,SLOT_RCU);
  scores[4] = OnesCount(tempScalarDep1 | tempScalarDep2) + OnesCount(tempMiscDep1 | tempMiscDep2);
 
  GetInstructionTripletDependencies(tempScalarDep1,tempMiscDep1,srcEntry,SLOT_RCU,SLOT_MEM,SLOT_MUL);
  scores[5] = OnesCount(tempScalarDep1 | tempScalarDep2) + OnesCount(tempMiscDep1 | tempMiscDep2);

  uint32 minVal = scores[0];
  uint32 minIndex = 0;

  for(uint32 i = 1; i < 6; i++)
  {
    if(scores[i] <= minVal)
    {
      minIndex = i;
      minVal = scores[i];
    }
  }

  destEntry.CopyInstructionData(baseSlot + destSlotRCU[minIndex], srcEntry, SLOT_RCU);
  destEntry.CopyInstructionData(baseSlot + destSlotALU[minIndex], srcEntry, SLOT_ALU);
  destEntry.CopyInstructionData(baseSlot + destSlotMUL[minIndex], srcEntry, SLOT_MUL);
  destEntry.CopyInstructionData(baseSlot + destSlotMEM[minIndex], srcEntry, SLOT_MEM);
}

void LogMemoryLocation(FILE *outFile, const char * const varname, const uint32 address, const MPE &mpe)
{
  uint32 value = *((uint32 *)nuonEnv.GetPointerToMemory(mpe,address));
  SwapScalarBytes(&value);
  fprintf(outFile,"%s = $%8.8lx\n",varname,value);
}

void MPE::UpdateInvalidateRegion(const uint32 start, const uint32 length)
{
  const uint32 end = (start + length - 1);

  if(start < invalidateRegionStart) invalidateRegionStart = start;
  if(end > invalidateRegionEnd) invalidateRegionEnd = end;

  if(start < interpreterInvalidateRegionStart) interpreterInvalidateRegionStart = start;
  if(end > interpreterInvalidateRegionEnd) interpreterInvalidateRegionEnd = end;
}

bool MPE::FetchDecodeExecute()
{
  cycleCounter = 0;

  if(mpectl & MPECTRL_MPEGO)
  {
    //StartPerformanceTimer();

    //uint32 blockExecuteCount = 100;

    /* Force 16 bit alignment of pcexec */
    pcexec &= ~0x01UL;

    /* Check for interrupts and update pcexec, rzi1 and rzi2 if an interrupt is to be serviced */

    if(intsrc && (ecuSkipCounter == 0))
    {
      //Test imaskHw2 bit
      if((intctl & (1UL << 5)) == 0)
      {
        //imaskHw2 not set
        //Test to see if the level 2 interrupt has occurred
        if(intsrc & (1UL << inten2sel))
        {
          //Test imaskSw2 mask
          if((intctl & (1UL << 7)) == 0)
          {
            //imaskSw2 not set so jump to the level 2 interrupt vector
            rzi2 = pcexec;
            pcexec = intvec2;
            //set imaskHw2 flag
            intctl |= (1UL << 5);
          }
        }
        else if((intctl & ((1UL << 3) | (1UL << 1))) == 0)
        {
          //imaskHw2 not set
          //neither imaskSw1 nor imaskHw1 set
          //Test to see if an enabled level 1 interrupt has occurred
          if(intsrc & inten1)
          {
            //Jump to the level 1 interrupt vector
            rzi1 = pcexec;
            pcexec = intvec1;
            //set imaskHw1 flag
            intctl |= (1UL << 1);
          }
        }
      }
    }

    uint32 pcexecLookupValue = pcexec;

    /* Now check to see if the MPE is executing out of local MPE memory.  If it is, then check to see if the MPE is executing */
    /* code within a region that has been marked for invalidation.  If executing with an invalidated region, the overlay manager */
    /* needs to hash the local MPE memory and see if the hash matches that of any existing compiled overlays in its list.  If it */
    /* can't find a match, a new overlay index is allocated or an existing set of compiled overlay code is replaced.  It a match is */
    /* made then that index is used.  Once an index is assigned, the index is combined with the current pcexec value to map the overlay */
    /* to an unused region of the Nuon address space to allow multiple sets of compiled overlays to co-exist even when the overlays */
    /* correspond to the same physical memory addresses.  As an example, the first set of code to be executed from MPE memory will be */
    /* assigned overlay index 0 and the compiled code will be assigned to the code cache range $20300000-$20307FFF.  If new overlay code */
    /* is loaded into the MPE local memory and executed, the hash will not match that of overlay index 0 and so overlay index 1 will be */
    /* assigned with a code cache range of $20308000-$2030FFFF.  This is an important optimization as games like Tempest 3000 load */
    /* multiple overlays into the MPEs several times per frame.  If compiled overlay code was not allowed to exist, the overlay code would */
    /* require compiliation every time new overlay code was loaded into an MPE even when ping-ponging between two sets of overlay code. */
    /* The code cache entries would also require invalidation each time this happened. */

    if((pcexec < (MPE_IRAM_BASE + OVERLAY_SIZE)) && (pcexec >= MPE_IRAM_BASE))
    {
      //pcexec is within local MPE IRAM address space
      if((pcexec <= invalidateRegionEnd) && (pcexec >= invalidateRegionStart))
      {
        //pcexec is within MPE IRAM region that has been modified since the last time it was hashed

        bool bInvalidateOverlayRegion;
        /*overlayIndex =*/ overlayManager.FindOverlay((uint32 *)&dtrom[MPE_IRAM_OFFSET], bInvalidateOverlayRegion);

        //Get the new overlay mask
        overlayMask = overlayManager.GetOverlayMask();

        //Invalidate the interpreter cache because these entries are not mapped to unique address ranges
        
        //numInterpreterCacheFlushes++;
        //instructionCache->InvalidateRegion(invalidateRegionStart, invalidateRegionEnd);
        bInvalidateInterpreterCache = true;

        if(bInvalidateOverlayRegion)
        {
          //The overlay manager assigned a previously used overlay ID so invalidate the code cache entries
          //associated with the overlay address range
          numNativeCodeCacheFlushes++;
          nativeCodeCache.FlushRegion(invalidateRegionStart | overlayMask, invalidateRegionEnd | overlayMask);
        }

        //Reset the IRAM invalidation indicators
        invalidateRegionStart = 0xFFFFFFFFUL;
        invalidateRegionEnd = 0x00000000UL;
      }

      //Modify the pcexec lookup value.  If pcexec is not within MPE IRAM, the lookup value will remain equal to
      //pcexec otherwise it will be equal to pcexec ORed with the overlay mask

      pcexecLookupValue |= overlayMask;
    }
    else
    {
      if(bInvalidateInstructionCaches)
      {
        bInvalidateInstructionCaches = false;
        InvalidateICache();
        if((mpeIndex == 0) || (mpeIndex == 3))
        {
          numNativeCodeCacheFlushes++;
          nativeCodeCache.FlushRegion(MAIN_BUS_BASE, MAIN_BUS_BASE + MAIN_BUS_SIZE - 1);
          nativeCodeCache.FlushRegion(SYSTEM_BUS_BASE, SYSTEM_BUS_BASE + SYSTEM_BUS_SIZE - 1);
        }
      }
    }

    NativeCodeCacheEntry* pNativeCodeCacheEntry = 0;
    NativeCodeCacheEntryPoint nativeCodeCacheEntryPoint = 0;

    bool skip_to_execute_block = false;

    const bool only_find_icache_entry = (ecuSkipCounter | interpretNextPacket) != 0;
    if(!only_find_icache_entry)
    {
      pNativeCodeCacheEntry = nativeCodeCache.pageMap.FindEntry(pcexecLookupValue);
      if(pNativeCodeCacheEntry && (pNativeCodeCacheEntry->virtualAddress == pcexecLookupValue))
      {
        nativeCodeCacheEntryPoint = pNativeCodeCacheEntry->entryPoint;
        skip_to_execute_block = true;
      }
      else if(bInvalidateInterpreterCache)
      {
        numInterpreterCacheFlushes++;
        instructionCache->InvalidateRegion(interpreterInvalidateRegionStart, interpreterInvalidateRegionEnd);
        bInvalidateInterpreterCache = false;
        interpreterInvalidateRegionStart = 0xFFFFFFFFUL;
        interpreterInvalidateRegionEnd = 0x00000000;
      }
    }

    bool bCacheEntryValid;
    InstructionCacheEntry* const pInstructionCacheEntry = !skip_to_execute_block ? instructionCache->FindInstructionCacheEntry(pcexec,bCacheEntryValid) : 0;
    if(!skip_to_execute_block && bCacheEntryValid && (pcexec == pInstructionCacheEntry->pcexec))
    {
      if (!only_find_icache_entry)
      {
//check_compile_threshhold:
        if(!(pInstructionCacheEntry->packetInfo & (PACKETINFO_COMPILED | PACKETINFO_NEVERCOMPILE)) && (pInstructionCacheEntry->frequencyCount >= COMPILE_THRESHOLD))
        {
          if(nativeCodeCache.IsBeyondThreshold())
          {
            numNativeCodeCacheFlushes++;
            nativeCodeCache.Flush();
            instructionCache->ClearCompiledStates();
          }

          bool bError;
          nativeCodeCacheEntryPoint = CompileNativeCodeBlock(pcexecLookupValue, COMPILE_TYPE, bError);
          if(!bError)
          {
            pNativeCodeCacheEntry = nativeCodeCache.pageMap.FindEntry(pcexecLookupValue);
            assert(pNativeCodeCacheEntry && pNativeCodeCacheEntry->virtualAddress == pcexecLookupValue);

#ifdef ENABLE_EMULATION_MESSAGEBOXES
            if(nuonEnv.compilerOptions.bDumpBlocks)
            {
              //if(pNativeCodeCacheEntry->compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK)
              //if(pNativeCodeCacheEntry->compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_BLOCK)
              {
                superBlock.PrintBlockToFile(pNativeCodeCacheEntry->compileType, pNativeCodeCacheEntry->codeSize);
              }
            }
#endif
            pInstructionCacheEntry->packetInfo |= PACKETINFO_COMPILED;
          }
          else
          {
            if(nativeCodeCacheEntryPoint == (NativeCodeCacheEntryPoint)-1)
            {
              pInstructionCacheEntry->packetInfo |= PACKETINFO_NEVERCOMPILE;
              numNonCompilablePackets++;
            }
            else if(nativeCodeCacheEntryPoint == 0)
            {
              numNativeCodeCacheFlushes++;
              nativeCodeCache.Flush();
              instructionCache->ClearCompiledStates();
            }

            nativeCodeCacheEntryPoint = 0;
          }
        }
        else
        {
          pInstructionCacheEntry->frequencyCount++;
          if(pInstructionCacheEntry->frequencyCount == 0) // overflow?
            pInstructionCacheEntry->frequencyCount = ~0u;
        }
      }

      skip_to_execute_block = true;
    }

    if(!skip_to_execute_block)
    {
      pInstructionCacheEntry->pcexec = pcexec;
      pInstructionCacheEntry->frequencyCount = 1;
      if(pcexec < ROM_BIOS_BASE)
      {
        DecompressPacket(GetPointerToMemoryBank(pcexec),*pInstructionCacheEntry,DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST);
        if(((pcexec >= BIOS_JUMPTABLE_START) && (pcexec <= BIOS_JUMPTABLE_END)) || (pInstructionCacheEntry->packetInfo & PACKETINFO_BREAKPOINT))
        {
          pInstructionCacheEntry->packetInfo |= PACKETINFO_NEVERCOMPILE;
        }
      }
      else
      {
        pInstructionCacheEntry->nuanceCount = 0;
        pInstructionCacheEntry->pcroute = 0;
        pInstructionCacheEntry->packetInfo |= PACKETINFO_NEVERCOMPILE;
      }
      instructionCache->SetEntryValid(pcexec);
    }

    interpretNextPacket = 0;

    //StopPerformanceTimer();
    //double timeDelta = GetTimeDeltaMs();

#ifdef LOG_PROGRAM_FLOW
    if(LOG_MPE_INDEX == mpeIndex)
    {
      fprintf(logfile,"pcexec: %8.8x\n",pcexec);
    }
#endif

    bool skip_to_halt_block = false;
    if(nativeCodeCacheEntryPoint)
    {
      assert(pNativeCodeCacheEntry);

      cycleCounter += pNativeCodeCacheEntry->numPackets;

      //pNativeCodeCacheEntry->accessCount++;
      //if(pNativeCodeCacheEntry->accessCount == 0) // overflow?
      //  pNativeCodeCacheEntry->accessCount = ~0;

      //prevPcexec = pcexec;
      if((pNativeCodeCacheEntry->compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_BLOCK) || (pNativeCodeCacheEntry->compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE))
      {
        const uint32 nInstructions = pNativeCodeCacheEntry->numInstructions;
        const Nuance* pNuance = (Nuance *)nativeCodeCacheEntryPoint;
        bInterpretedBranchTaken = false;

        for(uint32 i = 0; i < nInstructions; i++)
        {
          (nuanceHandlers[pNuance->fields[0]])(*this,nuances_use_tempreg_union ? tempreg_union : reg_union,*pNuance);
          pNuance++;

          if(bInterpretedBranchTaken) // Execute_CheckECUSkipCounter can set this
          {
            pcexec = pcfetchnext;
            skip_to_halt_block = true;
            break;
          }
        }

        if(!skip_to_halt_block)
        {
          pcexec = pNativeCodeCacheEntry->nextVirtualAddress;
          skip_to_halt_block = true;
        }
      }
      else
      {
        //do
        {
          nativeCodeCacheEntryPoint();
          //blockExecuteCount--;
        } 
        //while((pcexec == prevPcexec) && (mpectl & MPECTRL_MPEGO) && !ecuSkipCounter && blockExecuteCount);       
      }
    }
    else
    {
      if(pcexec < ROM_BIOS_BASE)
      {
        cycleCounter++;

        //prevPcexec = pcexec;
        pcroute = pInstructionCacheEntry->pcroute;
        pcexec = pcroute;
        ExecuteNuances(*pInstructionCacheEntry);
      }
      else if(pcexec < ROM_PE_BASE)
      {
        cycleCounter++;
        //Execute BIOS function: force to one of 256 entries
        BiosJumpTable[(pcexec >> 1) & 0xFF](*this);

        if(!bCallingMediaCallback)
        {
          //Perform an implicit RTS, nop
          pcexec = rz;
        }
      }
      else
      {
        //Execute PE function
        cycleCounter++;
        CallPEHandler(*this, pcexec);
        if(!bCallingMediaCallback)
        {
          //Perform an implicit RTS, nop
          pcexec = rz;
        }
      }
        
      bCallingMediaCallback = false;
    }

    if(!skip_to_halt_block && ecuSkipCounter != 0)
    {
      ecuSkipCounter--;
      if(ecuSkipCounter == 0)
      {
#ifdef LOG_BIOS_CALLS
          if(((pcfetchnext >= BIOS_JUMPTABLE_START) && (pcfetchnext <= 0x8000FFFF)) && (mpeIndex == LOG_MPE_INDEX)) //!! rather DVD_JUMPTABLE_END?
          {
            if(logfile)
            {
              if(pcfetchnext >= DVD_JUMPTABLE_START) // Presentation Engine
              {
#ifdef LOG_ADDRESS_ONLY
                fprintf(logfile,"PE CALL: $%8.8lX\n",pcfetchnext);
                fflush(logfile);
#else
                fprintf(logfile,"PE CALL: %s ($%8.8lX): V0 = [$%lX,$%lX,$%lX,%lX], R4 = %lX, R5 = %lX\n",BiosRoutineNames[(pcfetchnext >> 3) & 0x3FFF],pcfetchnext,regs[0],regs[1],regs[2],regs[3],regs[4],regs[5]);
                //fprintf(logfile,"PE CALL: %s ($%8.8lX)\n",BiosRoutineNames[(pcfetchnext >> 3) & 0x3FFF],pcfetchnext,regs[0],regs[1],regs[2],regs[3],regs[4],regs[5]);
                fflush(logfile);
#endif
              }
              else
              {
#ifdef LOG_ADDRESS_ONLY
                fprintf(logfile,"BIOS CALL: %s ($%8.8lX)\n",BiosRoutineNames[(pcfetchnext >> 3) & 0x3FFF],pcfetchnext);
                fflush(logfile);
#else
                fprintf(logfile,"BIOS CALL: %s ($%8.8lX): V0 = [$%lX,$%lX,$%lX,%lX], R4 = %lX, R5 = %lX\n",BiosRoutineNames[(pcfetchnext >> 3) & 0x3FFF],pcfetchnext,regs[0],regs[1],regs[2],regs[3],regs[4],regs[5]);
                fflush(logfile);
#endif
              }
            }
          }
#endif
        pcexec = pcfetchnext;
      }
    }

    if((excephalten & excepsrc) || (pcexec == breakpointAddress))
      Halt();

    //StopPerformanceTimer();
    //timeDelta = GetTimeDeltaMs();
    return true;
  }

  //StopPerformanceTimer();
  //timeDelta = GetTimeDeltaMs();
  return false;
}

uint8 MPE::DecodeSingleInstruction(const uint8 *const iPtr, InstructionCacheEntry *const entry, uint32 * const immExt, bool &bTerminating)
{
  //if 16 bit ALU instruction
  const uint8 opcode = *iPtr;
  if(opcode <= 0x3F)
  {
    DecodeInstruction_ALU16(iPtr,entry,immExt);
    bTerminating = true;
    *immExt = 0;
    return 2;
  }
  else
  {
    //Instruction is not a 16 bit ALU instruction

    //If 16 bit non-terminating non-ALU instruction, control instruction
    //or 32 bit immediate extension
    if(opcode < 0x88 || opcode > 0x9F)
    {
      //If not a control instruction (PAD, NOP or BREAKPOINT)
      if((opcode & 0xFC) != 0x80)
      {
        if((opcode & 0x7F) <= 0x67)
        {
          if((opcode & 0x7F) >= 0x48)
          {
            DecodeInstruction_MEM16(iPtr,entry,immExt);
          }
          else
          {
            DecodeInstruction_MUL16(iPtr,entry,immExt);
          }
        }
        else
        {
          if((opcode & 0x7F) <= 0x73)
          {
            DecodeInstruction_ECU16(iPtr,entry,immExt);
          }
          else
          {
            DecodeInstruction_RCU16(iPtr,entry,immExt);
          }
        }

        //if bit 15 is set, the instruction is a packet terminator
        bTerminating = (opcode & 0x80);
      }
      else
      {
        //PAD, NOP, or BREAKPOINT.
        entry->packetInfo |= (opcode & 0x03);

        //if bit 8 is set, the instruction is a terminating breakpoint or NOP
        bTerminating = (opcode & 0x01);
      }

      *immExt = 0;
      return 2;
    }
    else if(opcode >= 0x90)
    {
      switch((opcode & 0x0C) >> 2)
      {
        case 0:
          DecodeInstruction_ECU32(iPtr,entry,immExt);
          break;
        case 1:
          DecodeInstruction_MEM32(iPtr,entry,immExt);
          break;
        case 2:
          DecodeInstruction_ALU32(iPtr,entry,immExt);
          break;
        case 3:
          DecodeInstruction_MUL32(iPtr,entry,immExt);
          break;
      }

      //32 bit instruction: if bit 12 is set, instruction is a packet terminator
      bTerminating = *(iPtr + 2) & 0x10;
      *immExt = 0;
      return 4;
    }
    else
    {
      //32 bit extension (preceeding 48/64 bit instruction)
      *immExt = *((uint32 *)iPtr);
      SwapScalarBytes(immExt);
      bTerminating = false;
      return 4;
    }
  }
  return 0;
}

NativeCodeCacheEntryPoint MPE::CompileNativeCodeBlock(const uint32 _pcexec, const SuperBlockCompileType compileType, bool &bError, const bool bSinglePacket)
{
  return superBlock.CompileBlock(_pcexec, nativeCodeCache, compileType, bSinglePacket, bError);
}

void MPE::PrintInstructionCachePacket(char *buffer, size_t bufSize, const InstructionCacheEntry &entry)
{
  for(uint32 i = 0; i < entry.nuanceCount; i++)
  {
    const uint32 outputLen = (printHandlers[entry.handlers[i]])(buffer, bufSize, *((Nuance *)(&entry.nuances[FIXED_FIELD(i,0)])), true);
    buffer += outputLen;
    bufSize -= outputLen;
  }

  if(entry.packetInfo & PACKETINFO_BREAKPOINT)
  {
    sprintf_s(buffer, bufSize, "breakpoint\n");
  }
  else if(entry.packetInfo & PACKETINFO_NOP)
  {
    sprintf_s(buffer, bufSize, "nop\n");
  }
  else if(entry.pcexec >= ROM_BIOS_BASE && pcexec < ROM_PE_BASE)
  {
    const unsigned char idx = (entry.pcexec >> 1) & 0xFF;
    sprintf_s(buffer, bufSize, "BIOS: %s (%s)\n", BiosRoutineNames[idx], BiosJumpTable[idx] == AssemblyBiosHandler ? "ASM" : (BiosJumpTable[idx] == UnimplementedCacheHandler ? "Unimpl. Cache Handler" : (BiosJumpTable[idx] == UnimplementedMediaHandler ? "Unimpl. Media Handler" : (BiosJumpTable[idx] == NullBiosHandler ? "Null Handler" : (BiosJumpTable[idx] == WillNotImplement ? "Will not impl." : "C")))));
  }
}

void MPE::PrintInstructionCachePacket(char *buffer, size_t bufSize, const uint32 address)
{
  bool bCacheEntryValid;
  const InstructionCacheEntry * const pEntry = instructionCache->FindInstructionCacheEntry(address,bCacheEntryValid);
  if(bCacheEntryValid && (address == pEntry->pcexec))
  {
    PrintInstructionCachePacket(buffer, bufSize, *pEntry);
  }
  else
  {
    InstructionCacheEntry entry;
    entry.pcexec = address;
    DecompressPacket(GetPointerToMemoryBank(address),entry);
    PrintInstructionCachePacket(buffer, bufSize, entry);
  }
}

void MPE::ExecuteSingleStep()
{
  InvalidateICacheRegion(pcexec, pcexec);
  nativeCodeCache.FlushRegion(pcexec, pcexec);
  FetchDecodeExecute();
}
