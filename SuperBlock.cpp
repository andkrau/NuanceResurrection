#include "basetypes.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "mpe.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "SuperBlock.h"
#include "SuperBlockConstants.h"
#include "X86EmitTypes.h"

extern NuonEnvironment nuonEnv;
extern NativeEmitHandler emitHandlers[];

#ifdef ENABLE_EMULATION_MESSAGEBOXES
static FILE *blockFile;
#endif

#define NOP_IBYTE_LENGTH (2)

#define BRANCH_SLOT (0x4UL)
#define DELAY_SLOT_ANY (0x3UL)
#define DELAY_SLOT_1 (0x2UL)
#define DELAY_SLOT_2 (0x1UL)

static bool IsBranchConditionCompilable(const uint32 startAddress, const uint32 mpeIndex, const uint32 condition)
{
  switch(condition)
  {
    case 0:
      //ne
      return true;
    case 1:
      //c0z
      return true;
    case 2:
      //c1z
      return true;
    case 3:
      //cc
      return true;
    case 4:
      //eq
      return true;
    case 5:
      //cs
      return true;
    case 6:
      //vc
      return true;
    case 7:
      //vs
      return true;
    case 8:
      //lt
      return true;
    case 9:
      //mvc
      return true;
    case 10:
      //mvs
      return true;
    case 11:
      //hi
      return true;
    case 12:
      //le
      return true;
    case 13:
      //ls
      return true;
    case 14:
      //pl
      return true;
    case 15:
      //mi
      return true;
    case 16:
      //gt
      return true;
    case 17:
      //always
      return true;
    case 18:
      //modmi
      return true;
    case 19:
      //modpl
      return true;
    case 20:
      //ge
      return true;
    case 21:
      //modge
      return true;
    case 22:
      //modlt
      return true;
    case 23:
      //never
      return false;
    case 24:
      //c0ne
      return true;
    case 25:
      //never
      return false;
    case 26:
      //never
      return false;
    case 27:
      //cf0lo
      return true;
    case 28:
      //c1ne
      return true;
    case 29:
      //cf0hi
      return true;
    case 30:
      //cf1lo
      return true;
    case 31:
      //cf1hi
      return true;
    default:
      return false;
  }
}

SuperBlock::SuperBlock(MPE * const mpe)
    : pMPE(mpe)
    , constants(SuperBlockConstants(mpe))
{
  //!! init_array((uint8*)instructions, ((MAX_SUPERBLOCK_PACKETS + 2) * (MAX_SUPERBLOCK_INSTRUCTIONS_PER_PACKET + 2)) * sizeof(InstructionEntry));
  //!! init_array((uint8*)packets, (MAX_SUPERBLOCK_PACKETS + 2) * sizeof(PacketEntry));

  numInstructions = 0;
  numPackets = 0;

#ifdef ENABLE_EMULATION_MESSAGEBOXES
  blockFile = nullptr;
#endif
}

SuperBlock::~SuperBlock()
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  if(blockFile)
    fclose(blockFile);
#endif
}

static void GetFlagString(const uint32 flags, char *buffer)
{
  char tempStr[128];

  buffer[0] = '\0';

  if(flags & SUPERBLOCKINFO_PACKETSTART)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_PACKETSTART\n");
    strcat(buffer,tempStr);
  }
  
  if(flags & SUPERBLOCKINFO_PACKETEND)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_PACKETEND\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_LOCKED)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_LOCKED\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_NONATIVECOMPILE)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_NONATIVECOMPILE\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_DEAD)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_DEAD\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_SYNC)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_SYNC\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_INHIBIT_ECU)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_INHIBIT_ECU\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_CHECK_ECU_INHIBIT)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_CHECK_ECU_INHIBIT\n");
    strcat(buffer,tempStr);
  }

  if(flags & SUPERBLOCKINFO_CHECK_ECUSKIPCOUNTER)
  {
    sprintf(tempStr,"SUPERBLOCKINFO_CHECK_ECU_SKIPCOUNTER\n");
    strcat(buffer,tempStr);
  }

  if(buffer[0] == '\0')
  {
    sprintf(buffer,"NONE\n");
  }
}

static void GetIFlagsString(char *buffer, const uint32 dep)
{
  const bool bN = dep & DEPENDENCY_FLAG_N;
  const bool bV = dep & DEPENDENCY_FLAG_V;
  const bool bZ = dep & DEPENDENCY_FLAG_Z;
  const bool bC = dep & DEPENDENCY_FLAG_C;
  const bool bMV = dep & DEPENDENCY_FLAG_MV;
  const bool bC0Z = dep & DEPENDENCY_FLAG_C0Z;
  const bool bC1Z = dep & DEPENDENCY_FLAG_C1Z;
  const bool bMODGE = dep & DEPENDENCY_FLAG_MODGE;
  const bool bMODMI = dep & DEPENDENCY_FLAG_MODMI;
  const bool bCF0 = dep & DEPENDENCY_FLAG_CP0;
  const bool bCF1 = dep & DEPENDENCY_FLAG_CP1;

  sprintf(buffer,"[%s%s%s%s%s%s%s%s%s%s%s]",
    bN ? "N " : "",
    bV ? "V " : "",
    bZ ? "Z " : "",
    bC ? "C " : "",
    bMV ? "MV " : "",
    bC0Z ? "C0Z " : "",
    bC1Z ? "C1Z " : "",
    bMODGE ? "MODGE " : "",
    bMODMI ? "MODMI " : "",
    bCF0 ? "CF0 " : "",
    bCF1 ? "CF1 " : "");
}

extern NuancePrintHandler printHandlers[];

bool SuperBlock::EmitCodeBlock(NativeCodeCache &codeCache, SuperBlockCompileType compileType, const bool bContainsBranch)
{
  codeCache.emitVars.bSaveRegs = false;
  codeCache.emitVars.bCheckECUSkipCounter = false;
  codeCache.emitVars.bUsesMMX = false;

  if(!bAllowBlockCompile)
    compileType = SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE;
  else if(compileType != SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE)
  {
    if(!bCanEmitNativeCode)
      compileType = SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_BLOCK;
  }

  uint32 numLiveInstructions = 0;
  InstructionEntry* pInstruction = instructions;

  if((compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE) || (compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_BLOCK))
  {
    uint8 * const entryPoint = codeCache.GetEmitPointer();
    Nuance* ptrEmitNuance = (Nuance *)entryPoint;

    for(uint32 i = numInstructions; i > 0; i--)
    {
      const uint32 flags = pInstruction->flags;
      if(!(flags & (SUPERBLOCKINFO_PACKETSTART | SUPERBLOCKINFO_PACKETEND)))
      {
        if(!(flags & SUPERBLOCKINFO_DEAD))
        {
          numLiveInstructions++;
          *ptrEmitNuance = pInstruction->instruction;
          ptrEmitNuance++;
        }
      }
      else if(flags & SUPERBLOCKINFO_PACKETSTART)
      {
        //Emit SaveRegs or SaveFlags only if the packet contains live instructions
        if(pInstruction->packet->liveCount)
        {
          numLiveInstructions++;

          if(flags & PACKETINFO_DEPENDENCY_PRESENT)
          {
            pInstruction->instruction.fields[0] = Handler_SaveRegs;
            ptrEmitNuance->fields[0] = Handler_SaveRegs;
            ptrEmitNuance->fields[1] = pInstruction->scalarOpDependencies;
            ptrEmitNuance->fields[2] = pInstruction->miscOpDependencies;
            if(pInstruction->scalarOpDependencies | (pInstruction->miscOpDependencies & ~DEPENDENCY_FLAG_ALLFLAGS))
            {
              codeCache.emitVars.bSaveRegs = true;
            }
          }
          else
          {
            pInstruction->instruction.fields[0] = Handler_SaveFlags;
            ptrEmitNuance->fields[0] = Handler_SaveFlags;
            ptrEmitNuance->fields[1] = 0;
            ptrEmitNuance->fields[2] = 0;
          }
          ptrEmitNuance++;
        }
      }
      else //PACKET_END
      {
        //Emit CheckECUSkipCounter only if there are live instructions (nu uh!)
        //if(pInstruction->packet->liveCount)
        {
          numLiveInstructions++;

          ptrEmitNuance->fields[0] = Handler_CheckECUSkipCounter;
          ptrEmitNuance->fields[1] = 0;
          ptrEmitNuance->fields[2] = 0;
          ptrEmitNuance++;
        }
      }
      pInstruction++;
    }
    const uint32 emittedBytes = numLiveInstructions * sizeof(Nuance);
    codeCache.ReleaseBuffer((NativeCodeCacheEntryPoint)entryPoint, startAddress, exitAddress, emittedBytes, packetsProcessed, numLiveInstructions, compileType, nextDelayCounter,4);
    return codeCache.IsBeyondThreshold();
  }
  else if(compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK)
  {
    uint8 * const entryPoint = codeCache.GetEmitPointer();
    codeCache.emitVars.pInstructionEntry = pInstruction;
    
    if(numInstructions > 0)
    {
      codeCache.X86Emit_PUSHAD();
      codeCache.X86Emit_MOVIR((uint32)&(codeCache.emitVars.mpe->cc), x86Reg_esi);
      codeCache.X86Emit_MOVIR((uint32)&(codeCache.emitVars.mpe->tempCC), x86Reg_edi);
      if(bContainsBranch)
      {
        codeCache.X86Emit_MOVIM(exitAddress, x86MemPtr_dword, (uint32)&(codeCache.emitVars.mpe->pcfetchnext));
      }
    }

    for(uint32 i = numInstructions; i > 0; i--)
    {
      codeCache.emitVars.pInstructionEntry = pInstruction;
      const uint32 flags = pInstruction->flags;
      if(!(flags & (SUPERBLOCKINFO_PACKETSTART | SUPERBLOCKINFO_PACKETEND)))
      {
        if(!(flags & SUPERBLOCKINFO_DEAD))
        {
          numLiveInstructions++;
          codeCache.emitVars.scalarRegOutDep = pInstruction->scalarOutputDependencies;
          codeCache.emitVars.miscRegOutDep = pInstruction->miscOutputDependencies;

          ((NativeEmitHandler)(emitHandlers[pInstruction->instruction.fields[0]]))(&codeCache.emitVars,pInstruction->instruction);
        }
      }
      else if(flags & SUPERBLOCKINFO_PACKETSTART)
      {
        //Emit SaveRegs or SaveFlags only if the packet contains live instructions
        if(pInstruction->packet->liveCount)
        {
          numLiveInstructions++;
          pInstruction->instruction.fields[0] = Handler_SaveRegs;
          codeCache.emitVars.scalarRegDep = 0;
          codeCache.emitVars.miscRegDep = 0;

          if(flags & PACKETINFO_DEPENDENCY_PRESENT)
          {
            codeCache.emitVars.scalarRegDep = pInstruction->scalarOpDependencies;
            codeCache.emitVars.miscRegDep = pInstruction->miscOpDependencies;

            pInstruction->instruction.fields[1] = pInstruction->scalarOpDependencies;
            pInstruction->instruction.fields[2] = pInstruction->miscOpDependencies;
            if(pInstruction->scalarOpDependencies | (pInstruction->miscOpDependencies & ~DEPENDENCY_FLAG_ALLFLAGS))
            {
              codeCache.emitVars.bSaveRegs = true;
            }
          }
          else
          {
            pInstruction->instruction.fields[1] = 0;
            pInstruction->instruction.fields[2] = 0;
          }
          Emit_SaveRegs(&codeCache.emitVars,pInstruction->instruction);
        }
      }
      pInstruction++;
    }

    codeCache.X86Emit_MOVIM(exitAddress, x86MemPtr_dword,(uint32)&(codeCache.emitVars.mpe->pcexec));

    if(codeCache.emitVars.bSaveRegs || codeCache.emitVars.bUsesMMX)
      codeCache.X86Emit_EMMS();

    if(numInstructions > 0)
      codeCache.X86Emit_POPAD();

    codeCache.X86Emit_RETN();
    const uint32 emittedBytes = (uint32)(codeCache.GetEmitPointer() - entryPoint);
    codeCache.ReleaseBuffer((NativeCodeCacheEntryPoint)entryPoint, startAddress, exitAddress, emittedBytes, packetsProcessed, numLiveInstructions, compileType, nextDelayCounter,4);
    return codeCache.IsBeyondThreshold();
  }

  return false;
}

NativeCodeCacheEntryPoint SuperBlock::CompileBlock(const uint32 address, NativeCodeCache &codeCache, const SuperBlockCompileType compileType, const bool bLimitToSinglePacket, bool &bError)
{
  const NativeCodeCacheEntryPoint entryPoint = (NativeCodeCacheEntryPoint)codeCache.GetEmitPointer();
  bCanEmitNativeCode = true;
  bSinglePacket = bLimitToSinglePacket;
  bool bContainsBranch = false;

  bError = false;
  constants.bConstantPropagated = false;

  bAllowBlockCompile = (compileType != SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE);

  //Step 1, fetch the block (or superblock)
  //int32 fetchSuperBlockResult; //!! never used
  if((/*fetchSuperBlockResult =*/ FetchSuperBlock(address,bContainsBranch)) <= 0)
  {
    //For whatever reason, no entries were added to the instruction list.  This is most likely due to the first instruction candidate being
    //a delayed branch followed by an instruction that cannot be compiled in a delay slot such as a control register memory operation or
    //another branch instruction.  In such cases, CompileBlock returns a negative result and the caller should assume the packet needs to
    //be interpreted.

    bError = true;
    return (NativeCodeCacheEntryPoint)-1;
  }

  if(codeCache.GetAvailableCodeBufferSize() <= (numInstructions * sizeof(Nuance)))
  {
    //Not enough room, or too close for comfort: return false to indicate cache should be
    //flushed and compile request reissued

    //Note that this check only works for code blocks being emitted as Nuances.  In this case,
    //the required size will always be less than or equal to the number of Nuances in the 
    //SuperBlock multiplied by the number of fields per Nuance multiplied by the number of
    //bytes per field.  In all but the most trivial cases, if there is enough room to emit
    //native code for a block, there will also be enough room to emit the Nuances directly.
    //This means that this check wont cause many early failures when compiling
    //to native code.  Native code emission will still require a more robust check at the
    //end of the emit phase

    return (NativeCodeCacheEntryPoint)0;
  }

  //Step 2, perform constant propagation
  if(bSinglePacket)
  {
    UpdateDependencyInfo();
  }
  else
  {
    if(nuonEnv.compilerOptions.bConstantPropagation)
    {
      //Step 2, perform constant propagation
      UpdateDependencyInfo();
      PerformConstantPropagation();
    }
    
    if(nuonEnv.compilerOptions.bDeadCodeElimination)
    {
      //Step 3, perform dead code elimination
      UpdateDependencyInfo();
      PerformDeadCodeElimination();
    }

    UpdateDependencyInfo();
  }
  
  //Step 4, emit native code or IL nodes based on compile type and
  //update code cache entry fields appropriately
  if(!EmitCodeBlock(codeCache, compileType, bContainsBranch))
  {
    return entryPoint;
  }
  else
  {
    bError = true;
    return (NativeCodeCacheEntryPoint)0;
  }
}

void SuperBlock::PerformConstantPropagation()
{
  constants.ClearConstants();
  constants.FirstInstruction(instructions);
  for(uint32 i = numInstructions; i > 0; i--)
  {     
    constants.PropagateConstants();
    constants.NextInstruction();
  }
}

#ifdef ENABLE_EMULATION_MESSAGEBOXES
void SuperBlock::PrintBlockToFile(SuperBlockCompileType compileType, uint32 size)
{
  if(!blockFile)
  {
    char fileStr[128];
    sprintf(fileStr,"SuperBlocks%li.txt",pMPE->mpeIndex);
    blockFile = fopen(fileStr,"w");
    if(!blockFile)
      return;
  }

  InstructionEntry *pCurrentInstruction = instructions;

  //fprintf(blockFile,"***\n");
  fprintf(blockFile,"****************************************\n");
  fprintf(blockFile,"Virtual Address: $%8lx\n",startAddress);
  fprintf(blockFile,"Constants Propagated: %s\n",constants.bConstantPropagated ? "TRUE" : "FALSE");
  fprintf(blockFile,"Instruction Count: %li\n",numInstructions);
  fprintf(blockFile,"Packet Count: %li\n",packetsProcessed);
  fprintf(blockFile,"Code Size: %lu bytes\n",size);
  fprintf(blockFile,"Code Cache Usage: %lu bytes\n",pMPE->nativeCodeCache.GetUsedCodeBufferSize());
  if(compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE)
  {
    fprintf(blockFile,"Compile Type: IL single\n\n");
  }
  else if(compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_BLOCK)
  {
    fprintf(blockFile,"Compile Type: IL block\n\n");
  }
  else if(compileType == SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK)
  {
    fprintf(blockFile,"Compile Type: Native block\n\n");
  }
  else
  {
    fprintf(blockFile,"Compile Type: Unknown\n\n");
  }

  if(numInstructions > 0)
  {
    for(uint32 j = numInstructions; j > 0; j--)
    {
      uint32 handler = pCurrentInstruction->instruction.fields[0];
      if((handler == Handler_PacketEnd) && (pCurrentInstruction->packet->flags & SUPERBLOCKINFO_CHECK_ECUSKIPCOUNTER))
      {
        handler = Handler_CheckECUSkipCounter;
      }
      if(pCurrentInstruction->flags & SUPERBLOCKINFO_DEAD)
      {
        fprintf(blockFile,"*DEAD*: ");
      }
      char tempStr[2048];
      (printHandlers[handler])(tempStr, pCurrentInstruction->instruction, false);
      fprintf(blockFile,"%s ",tempStr);
      if(!(pCurrentInstruction->flags & (SUPERBLOCKINFO_PACKETSTART | SUPERBLOCKINFO_PACKETEND)))
      //if(0)
      {
        fprintf(blockFile,"(ScalarInDep: $%8.8X, MiscInDep: $%8.8X, ScalarOutDep: $%8.8X, MiscOutDep: $%8.8X, ",
          pCurrentInstruction->scalarInputDependencies,pCurrentInstruction->miscInputDependencies,
          pCurrentInstruction->scalarOutputDependencies,pCurrentInstruction->miscOutputDependencies);
        GetIFlagsString(tempStr,pCurrentInstruction->miscInputDependencies);
        fprintf(blockFile,"FlagsInDep: %s ",tempStr);
        GetIFlagsString(tempStr,pCurrentInstruction->miscOutputDependencies);
        fprintf(blockFile,"FlagsOutDep: %s)\n",tempStr);
      }
      else if(pCurrentInstruction->flags & SUPERBLOCKINFO_PACKETSTART)
      {
        if(pCurrentInstruction->packet->flags & SUPERBLOCKINFO_NONATIVECOMPILE)
        {
          fprintf(blockFile,"(IL) ");
        }
        else
        {
          fprintf(blockFile,"(NATIVE) ");
        }

        if(pCurrentInstruction->flags & PACKETINFO_DEPENDENCY_PRESENT)
        {
          fprintf(blockFile,"(DEPENDENCY_PRESENT)");        
        }
        
        fprintf(blockFile,"\n");
      }
      else
      {
        fprintf(blockFile,"\n");
      }
      
      pCurrentInstruction++;
    }
    fprintf(blockFile,"\n");
  }
  else
  {
    fprintf(blockFile,"EMPTY BLOCK\n\n");  
  }
  fprintf(blockFile,"Next Virtual Address: $%8lx\n",exitAddress);
  fprintf(blockFile,"Next Delay Counter: $%lx\n",nextDelayCounter);
  //fprintf(blockFile,"***\n\n");
  fprintf(blockFile,"****************************************\n\n");
  fflush(blockFile);
}
#endif

void SuperBlock::AddPacketToList(InstructionCacheEntry &packet, const uint32 index)
{
  packets[index].pcexec = packet.pcexec;
  packets[index].pcroute = packet.pcroute;
  //packets[index].pcfetchnext = packet.pcfetchnext;
  //packets[index].instructionCount = packet.nuanceCount; 
  packets[index].flags = packet.packetInfo;

  uint32 comboScalarInDep = 0;
  uint32 comboMiscInDep = 0;
  uint32 comboScalarOutDep = packet.nuanceCount > 0 ? packet.scalarOutputDependencies[0] : 0;
  uint32 comboMiscOutDep = packet.nuanceCount > 0 ? packet.miscOutputDependencies[0] : 0;

  for(uint32 i = 1; i < packet.nuanceCount; i++)
  {
    comboScalarInDep |= (packet.scalarInputDependencies[i] & comboScalarOutDep);
    comboMiscInDep |= (packet.miscInputDependencies[i] & comboMiscOutDep);
    comboScalarOutDep |= packet.scalarOutputDependencies[i];
    comboMiscOutDep |= packet.miscOutputDependencies[i];
  }

  packets[index].comboMiscInputDependencies = comboMiscInDep;
  packets[index].comboScalarInputDependencies = comboScalarInDep;
  packets[index].comboMiscOutputDependencies = comboMiscOutDep;
  packets[index].comboScalarOutputDependencies = comboScalarOutDep;
}

bool SuperBlock::AddInstructionsToList(InstructionCacheEntry &packet, PacketEntry * const pPacketEntry, const uint32 index, const bool bExplicitNOP)
{
  uint32 nuanceBase = 0;
  InstructionEntry *pCurrentInstruction = &instructions[index];
  bool bPacketAllowsNativeCode = true;

  pCurrentInstruction->instruction.fields[0] = Handler_PacketStart;
  pCurrentInstruction->instruction.fields[1] = pPacketEntry->pcexec;
  pCurrentInstruction->instruction.fields[2] = (size_t)pPacketEntry;
  pCurrentInstruction->packet = pPacketEntry;
  pCurrentInstruction->scalarInputDependencies = pPacketEntry->comboScalarInputDependencies;
  pCurrentInstruction->miscInputDependencies = pPacketEntry->comboMiscInputDependencies;
  pCurrentInstruction->scalarOutputDependencies = pPacketEntry->comboScalarOutputDependencies;
  pCurrentInstruction->miscOutputDependencies = pPacketEntry->comboMiscOutputDependencies;
  pCurrentInstruction->flags = pPacketEntry->flags | SUPERBLOCKINFO_PACKETSTART | SUPERBLOCKINFO_LOCKED;
  pCurrentInstruction++;

  if(bExplicitNOP && (packet.nuanceCount == 0))
  {
    packet.nuanceCount = 1;
    ((Nuance &)(packet.nuances[nuanceBase])).fields[0] = Handler_ECU_NOP;
  }

  for(uint32 i = 0; i < packet.nuanceCount; i++)
  {
    for(uint32 j = 0; j < FIELDS_PER_NUANCE; j++)
    {
      pCurrentInstruction->instruction.fields[j] = ((Nuance &)(packet.nuances[nuanceBase])).fields[j];
    }

    if(((Nuance &)(packet.nuances[nuanceBase])).fields[FIELD_MEM_HANDLER] == Handler_LoadScalarControlRegisterAbsolute)
    {
      //Check for ld_s pcexec, Sk and replace it with mv_s <pcroute>, Sk: some programs read from pcexec
      //which has been updated to point to the next packet prior to execution.  That is, when pcexec is
      //read in packet N, the result is always the address of packet N+1
      if(((Nuance &)(packet.nuances[nuanceBase])).fields[FIELD_MEM_FROM] == 0x20500070)
      {
        pCurrentInstruction->instruction.fields[FIELD_MEM_HANDLER] = Handler_MV_SImmediate;
        pCurrentInstruction->instruction.fields[FIELD_MEM_FROM] = pPacketEntry->pcroute;
      }
    }

    bPacketAllowsNativeCode = bPacketAllowsNativeCode && ((bool)emitHandlers[((Nuance &)(packet.nuances[nuanceBase])).fields[0]]);
    pCurrentInstruction->packet = pPacketEntry;
    pCurrentInstruction->scalarInputDependencies = packet.scalarInputDependencies[i];
    pCurrentInstruction->miscInputDependencies = packet.miscInputDependencies[i];
    pCurrentInstruction->scalarOutputDependencies = packet.scalarOutputDependencies[i];
    pCurrentInstruction->miscOutputDependencies = packet.miscOutputDependencies[i];
    pCurrentInstruction->flags = 0;
    nuanceBase += FIELDS_PER_NUANCE;
    pCurrentInstruction++;
  }
  
  pCurrentInstruction->instruction.fields[0] = Handler_PacketEnd;
  pCurrentInstruction->instruction.fields[1] = pPacketEntry->pcroute;
  pCurrentInstruction->instruction.fields[2] = (size_t)pPacketEntry;
  pCurrentInstruction->packet = pPacketEntry;
  pCurrentInstruction->scalarInputDependencies = 0;
  pCurrentInstruction->miscInputDependencies = 0;
  pCurrentInstruction->scalarOutputDependencies = 0;
  pCurrentInstruction->miscOutputDependencies = 0;
  pCurrentInstruction->flags = (SUPERBLOCKINFO_PACKETEND | SUPERBLOCKINFO_LOCKED);
  if(!bPacketAllowsNativeCode)
  {
    pPacketEntry->flags |= SUPERBLOCKINFO_NONATIVECOMPILE;
  }
  return bPacketAllowsNativeCode;
}

uint32 SuperBlock::PerformDeadCodeElimination()
{
  uint32 miscRegMask = 0, scalarRegMask = 0, scalarRegMaskNext = 0, miscRegMaskNext = 0;
  uint32 miscInDep = 0, scalarInDep = 0;

  //scalarRegMask and miscRegMask represent all registers which do not need to be 
  //output by the instruction being processed

  //scalarRegMaskNext and miscRegMaskNext work the same way except they aggregrate the
  //mask values to be applied to the instructions in the next packet to be processed.  
  //These variables would not be needed on a single (instruction) issue architecture
  //but enclosing single instructions within packets does not add significant overhead
  //to the compilation process.

  uint32 numLive = 0;
  uint32 i = numInstructions - 1;
 
  for(uint32 j = numInstructions; j > 0; j--)
  {
    const uint32 flags = instructions[i].flags;

    if(!(flags & SUPERBLOCKINFO_DEAD))
    {
      numLive++;

      if(!(flags & (SUPERBLOCKINFO_PACKETSTART | SUPERBLOCKINFO_PACKETEND)))
      {
        const bool bLocked = flags & SUPERBLOCKINFO_LOCKED;
        if(!bLocked)
        {
          //Remove output dependencies which are not used as inputs prior to being modified
          instructions[i].scalarOutputDependencies &= ~scalarRegMask;
          instructions[i].miscOutputDependencies &= ~miscRegMask;
        }

        const uint32 scalarOutDep = instructions[i].scalarOutputDependencies;
        const uint32 miscOutDep = instructions[i].miscOutputDependencies;

        //Keep a running total of all input dependencies for this packet
        scalarInDep |= instructions[i].scalarInputDependencies;
        miscInDep |= (instructions[i].miscInputDependencies & DEPENDENCY_MASK_ALLMISC_NON_MEM);

        //Add all registers which are modified by this instruction
        scalarRegMaskNext |= scalarOutDep;
        miscRegMaskNext |= miscOutDep;

        //Remove all registers which are input dependencies of this packet
        scalarRegMaskNext &= ~scalarInDep;
        miscRegMaskNext &= ~miscInDep;

        if(!bLocked && !(scalarOutDep | miscOutDep))
        {
          //If there are no output dependencies then the current instruction can be eliminated.
          //Locked instructions must not be eliminated (e.g. memory stores to control registers)
          numLive--;
          instructions[i].flags |= SUPERBLOCKINFO_DEAD;
        }
      }
      else if(flags & SUPERBLOCKINFO_PACKETEND)
      {
        if(flags & SUPERBLOCKINFO_SYNC)
        {
          //When the sync flag is set, the register masks are reset so that all output dependencies are preserved.  
          //This is primarily used when allowing superblocks to contain multiple conditional branches.
          scalarRegMask = 0;
          miscRegMask = 0;
        }
        else
        {
          //Grab the dependency information collected from the previously processed packet
          scalarRegMask = scalarRegMaskNext;
          miscRegMask = miscRegMaskNext;
        }

        scalarInDep = 0;
        miscInDep = 0;
        //The initial state of the the masks for the packet above the current one should be the same as the initial state for the current
        //packet.  This is because the masks indicate which dependencies a packet does not need to update: assuming the current packet is empty
        //then the dependencies haven't changed.

        scalarRegMaskNext = scalarRegMask;
        miscRegMaskNext = miscRegMask;
      }
    }
  
    i--;
  }

  return numLive;
}

void SuperBlock::UpdateDependencyInfo()
{
  uint32 miscInDep = 0, miscOutDep = 0, scalarInDep = 0, scalarOutDep = 0, miscOpDep = 0, scalarOpDep = 0;

  InstructionEntry *pCurrentInstruction = instructions;
  InstructionEntry *pCurrentPacket = pCurrentInstruction; //!! only inited to this for paranoia reasons
  for(uint32 j = numInstructions; j > 0; j--)
  {
    const uint32 flags = pCurrentInstruction->flags;
    if(!(flags & SUPERBLOCKINFO_DEAD))
    {
      if(!(flags & (SUPERBLOCKINFO_PACKETSTART | SUPERBLOCKINFO_PACKETEND)))
      {
        //Set flags for dependencies of the current instruction where the
        //input registers were output dependencies of previous instructions in
        //the packet
        scalarOpDep |= (pCurrentInstruction->scalarInputDependencies & scalarOutDep);
        miscOpDep |= (pCurrentInstruction->miscInputDependencies & miscOutDep);
        
        //Add output dependencies of the current instruction to the 
        //aggregate output dependency flags
        scalarOutDep |= pCurrentInstruction->scalarOutputDependencies;
        miscOutDep |= pCurrentInstruction->miscOutputDependencies;
        
        //Aggregate input dependencies for the heck of it
        scalarInDep |= pCurrentInstruction->scalarInputDependencies;
        miscInDep |= pCurrentInstruction->miscInputDependencies;

        pCurrentPacket->packet->liveCount++;
      }
      else
      {
        if(flags & SUPERBLOCKINFO_PACKETSTART)
        {
          //Reset aggregate dependency flags and set the packet pointer to this PacketStart node
          scalarInDep = scalarOutDep = miscInDep = miscOutDep = scalarOpDep = miscOpDep = 0;
          pCurrentPacket = pCurrentInstruction;
          pCurrentPacket->packet->liveCount = 0;
        }
        else
        {
          //Now that the PacketEnd node was reached, store the aggregated packet
          //dependency information back to the last PacketStart node
          pCurrentPacket->scalarInputDependencies = scalarInDep;
          pCurrentPacket->scalarOutputDependencies = scalarOutDep;
          pCurrentPacket->miscInputDependencies = miscInDep;
          pCurrentPacket->miscOutputDependencies = miscOutDep;
          pCurrentPacket->scalarOpDependencies = scalarOpDep;
          pCurrentPacket->miscOpDependencies = miscOpDep;
          pCurrentPacket->flags &= (~PACKETINFO_DEPENDENCY_PRESENT);
          if(scalarOpDep | miscOpDep)
          {
            pCurrentPacket->flags |= PACKETINFO_DEPENDENCY_PRESENT;
          }
        }
      }
    }
    pCurrentInstruction++;
  }
}

int32 SuperBlock::FetchSuperBlock(uint32 packetAddress, bool &bContainsBranch)
{
  uint32 decodeOptions = (DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST | DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST);
  bCanEmitNativeCode = ALLOW_NATIVE_CODE_EMIT;
  bool bFirstNonNOPReached = false;
  bool bForceILBlock = false;

  bContainsBranch = false;

  nextDelayCounter = 0;
  numPackets = 0;
  numInstructions = 0;

  startAddress = packetAddress;

  //If the starting address is in the imaginary overlay region (0x20308000,0x2FF07FFF), mask the ending address so that it is within
  //the real IRAM region of (20300000,20307FFF).  The starting address should not be modified as it is used as the lookup key.

  if((startAddress < MPE1_ADDR_BASE) && (startAddress >= (MPE_IRAM_BASE + OVERLAY_SIZE)))
  {
    packetAddress = (packetAddress & (0xFFF00000 | (OVERLAY_SIZE - 1)));
  }

  exitAddress = packetAddress;

  packetsProcessed = 0;

  int32 packetCounter = bSinglePacket ? 1 : MAX_SUPERBLOCK_PACKETS;

  while(packetCounter > 0)
  {
    InstructionCacheEntry packet;
    packet.pcexec = packetAddress;

    pMPE->DecompressPacket((uint8 *)nuonEnv.GetPointerToMemory(*pMPE,packetAddress,false),packet, decodeOptions);
    packetAddress = packet.pcroute;

    packetsProcessed++;

    if(!(packet.packetInfo & PACKETINFO_NOP) && (packet.nuanceCount != 0))
    {
      //Packet is not a NOP nor is it a packet containing a single inhibited ECU instruction: 
      //check to see if it contains a breakpoint or is marked as non-compilable

      if(!((packet.packetInfo & (PACKETINFO_NEVERCOMPILE | PACKETINFO_BREAKPOINT))))
      {
        //Packet does not contain a NOP: add the packet to the packet list and add the instructions to the instruction list

        if(packet.packetInfo & (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP))
        {
          //Packet contains a branch instruction.  Non-delayed branches are fine.  Delayed branches can only be compiled if
          //the delay slot packets do not contain additional branch instructions, indirect memory operations or direct memory
          //operations on the control registers.

          bContainsBranch = true;

          if(packet.packetInfo & PACKETINFO_BRANCH_ALWAYS)
            decodeOptions |= DECOMPRESS_OPTIONS_INHIBIT_ECU;

          /*if(!IsBranchConditionCompilable(startAddress, pMPE->mpeIndex, packet.ecuConditionCode))// || (numPackets != 0))
          {
            //Packet contains non-compilable instruction: don't add it to the list and stop adding packets
            //Don't modify the current value of the delay counter
            exitAddress = packet.pcexec;
            packetsProcessed--;
            break;
          }*/

          if(!(packet.packetInfo & PACKETINFO_BRANCH_NOP))
          {
            InstructionCacheEntry packetDelaySlot1;
            //Delayed branch with explicit delay slot instructions
            packetDelaySlot1.pcexec = packetAddress;
            pMPE->DecompressPacket((uint8 *)nuonEnv.GetPointerToMemory(*pMPE,packetAddress,false),packetDelaySlot1, decodeOptions);
            packetAddress = packetDelaySlot1.pcroute;

            InstructionCacheEntry packetDelaySlot2;
            packetDelaySlot2.pcexec = packetAddress;
            pMPE->DecompressPacket((uint8 *)nuonEnv.GetPointerToMemory(*pMPE,packetAddress,false),packetDelaySlot2, decodeOptions);
            packetAddress = packetDelaySlot2.pcroute;

            packet.packetInfo |= SUPERBLOCKINFO_CHECK_ECUSKIPCOUNTER;
            packetDelaySlot1.packetInfo |= SUPERBLOCKINFO_CHECK_ECUSKIPCOUNTER;
            packetDelaySlot2.packetInfo |= SUPERBLOCKINFO_CHECK_ECUSKIPCOUNTER;

            if(((packetDelaySlot1.packetInfo | packetDelaySlot2.packetInfo) & (PACKETINFO_MEMORY_IO|PACKETINFO_MEMORY_INDIRECT|PACKETINFO_ECU|PACKETINFO_NEVERCOMPILE)))
            {
              //Packet contains non-compilable instruction: don't add it to the list and stop adding packets
              //Don't modify the current value of the delay counter
              exitAddress = packet.pcexec;
              packetsProcessed--;
              break;

              if(!bFirstNonNOPReached)
                bForceILBlock = true; //!! after break, see also below?!
            }
 
            if(!((packetDelaySlot1.packetInfo | packetDelaySlot2.packetInfo) & (PACKETINFO_MEMORY_IO|PACKETINFO_MEMORY_INDIRECT|PACKETINFO_ECU|PACKETINFO_NEVERCOMPILE)) 
              || bForceILBlock)
            {
              AddPacketToList(packet,numPackets);
              const bool bPacketAllowsNativeCode = AddInstructionsToList(packet,&packets[numPackets],numInstructions);
              bCanEmitNativeCode = bCanEmitNativeCode && bPacketAllowsNativeCode;
              numInstructions += (packet.nuanceCount + 2);
              numPackets++;
              packetCounter--;

              if(bForceILBlock)
              {
                //goto force_il_block; //!! ???
              }

              if((!(packetDelaySlot1.packetInfo & PACKETINFO_NOP) && (packetDelaySlot1.nuanceCount != 0)) || bForceILBlock)
              {
                AddPacketToList(packetDelaySlot1,numPackets);              
                const bool bPacketAllowsNativeCode = AddInstructionsToList(packetDelaySlot1,&packets[numPackets],numInstructions,bForceILBlock);
                bCanEmitNativeCode = bCanEmitNativeCode && bPacketAllowsNativeCode;
                numInstructions += (packetDelaySlot1.nuanceCount + 2);
                numPackets++;
              }
              packetCounter--;

              if((!(packetDelaySlot2.packetInfo & PACKETINFO_NOP) && (packetDelaySlot2.nuanceCount != 0)) || bForceILBlock)
              {
                AddPacketToList(packetDelaySlot2,numPackets);              
                const bool bPacketAllowsNativeCode = AddInstructionsToList(packetDelaySlot2,&packets[numPackets],numInstructions,bForceILBlock);
                bCanEmitNativeCode = bCanEmitNativeCode && bPacketAllowsNativeCode;
                numInstructions += (packetDelaySlot2.nuanceCount + 2);
                numPackets++;
              }
              packetCounter--;

              packetsProcessed += 2;
              packetCounter = 0; //!! ?? never used again anyway

              exitAddress = packetDelaySlot2.pcroute;
//force_il_block:
              if(bForceILBlock)
                bCanEmitNativeCode = false;
              break;
            }
            else
            {
              //Packet contains non-compilable instruction: don't add it to the list and stop adding packets
              //Don't modify the current value of the delay counter
              exitAddress = packet.pcexec;
              packetsProcessed--;
              break;
            }
          }
          else
          {
            //Delayed branch with implicit NOP
            AddPacketToList(packet,numPackets);
            const bool bPacketAllowsNativeCode = AddInstructionsToList(packet,&packets[numPackets],numInstructions);
            bCanEmitNativeCode = bCanEmitNativeCode && bPacketAllowsNativeCode;
            //Increment numInstructions by nuance count and add two to account for PacketStart and PacketEnd
            numInstructions += (packet.nuanceCount + 2);
            //Increment packet count
            numPackets++;
            packetCounter--;
            //Update the exit address
            exitAddress = packet.pcroute;
            break;
          }
        }

        bFirstNonNOPReached = true;

        AddPacketToList(packet,numPackets);
        const bool bPacketAllowsNativeCode = AddInstructionsToList(packet,&packets[numPackets],numInstructions);
        bCanEmitNativeCode = bCanEmitNativeCode && bPacketAllowsNativeCode;
        //Increment numInstructions by nuance count and add two to account for PacketStart and PacketEnd
        numInstructions += (packet.nuanceCount + 2);
        //Increment packet count
        numPackets++;
        packetCounter--;
        //Update the exit address
        exitAddress = packet.pcroute;
      }
      else
      {
        //Packet contains non-compilable instruction: don't add it to the list and stop adding packets
        //Don't modify the current value of the delay counter
        exitAddress = packet.pcexec;
        packetsProcessed--;
        break;
      }
    }
    else
    {
      //Packet is a NOP or the packet contains a single inhibited ECU instruction: 
      //If the packet is a NOP and not in a delay slot, don't add it to the list: just update the exit address.
      //If the packet is in a delay slot, don't add it to the list but decrement the packet counter
      exitAddress = packet.pcroute;
      if(nextDelayCounter || !bAllowBlockCompile)
      //if(nextDelayCounter)
        packetCounter--;
    }

    if(nextDelayCounter > 1)
      nextDelayCounter--;
  }

  //Assume all instructions are live for now
  //Mark the last added instruction with SUPERBLOCKINFO_SYNC to ensure EliminateDeadCode works.
  if(numInstructions)
    instructions[numInstructions-1].flags |= SUPERBLOCKINFO_SYNC;

  return packetsProcessed;
}
