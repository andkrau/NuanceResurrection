#ifndef INSTRUCTION_SUPERBLOCK_H
#define INSTRUCTION_SUPERBLOCK_H

#include "basetypes.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "NativeCodeCache.h"
#include <stdio.h>

#ifndef SUPERBLOCK_STRUCTS
#define SUPERBLOCK_STRUCTS

class SuperBlockConstants;

class CompilerOptions
{
public:
  CompilerOptions()
  {
    bT3KCompilerHack = true;
    bConstantPropagation = true;
    bDeadCodeElimination = false;
    bAllowCompile = true;
    bDumpBlocks = false;
  }

  bool bT3KCompilerHack;
  bool bConstantPropagation;
  bool bDeadCodeElimination;
  bool bAllowCompile;
  bool bDumpBlocks;
};

struct PacketEntry
{
  uint32 pcexec;
  uint32 pcroute;
  uint32 pcfetchnext;
  uint32 instructionCount;
  uint32 comboScalarInputDependencies;
  uint32 comboMiscInputDependencies;
  uint32 comboScalarOutputDependencies;
  uint32 comboMiscOutputDependencies;
  uint32 flags;
  uint32 liveCount;
};

struct InstructionEntry
{
  PacketEntry *packet;
  Nuance instruction;
  uint32 pcexec;
  uint32 flags;
  uint32 scalarInputDependencies;
  uint32 miscInputDependencies;
  uint32 scalarOutputDependencies;
  uint32 miscOutputDependencies;
  uint32 scalarOpDependencies;
  uint32 miscOpDependencies;
};
#endif

enum SuperBlockCompileType
{
  SUPERBLOCKCOMPILETYPE_UNKNOWN = 0UL,
  SUPERBLOCKCOMPILETYPE_IL_SINGLE,
  SUPERBLOCKCOMPILETYPE_IL_BLOCK,
  SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK,
};

class SuperBlock
{
public:
  SuperBlock(MPE *mpe,uint32 maxPackets,uint32 maxInstructionsPerPacket);
  ~SuperBlock();
  void PrintBlockToFile(SuperBlockCompileType blockType = SUPERBLOCKCOMPILETYPE_UNKNOWN, uint32 size = 0);
  void AddPacketToList(InstructionCacheEntry &packet, uint32 index);
  bool AddInstructionsToList(InstructionCacheEntry &packet, PacketEntry *pPacketEntry, uint32 index, bool bExplicitNOP = false);
  NativeCodeCacheEntryPoint CompileBlock(MPE *mpe, uint32 address, NativeCodeCache &codeCache, SuperBlockCompileType eCompileType, bool bSinglePacket, bool &bError);
  bool EmitCodeBlock(NativeCodeCache &codeCache, SuperBlockCompileType compileType, bool bContainsBranch);
  void UpdateDependencyInfo();
  void PerformConstantPropagation();
  uint32 PerformDeadCodeElimination();
  int32 FetchSuperBlock(MPE &mpe, uint32 address, bool &bContainsBranch);
  uint32 numInstructions;
  uint32 numLiveInstructions;
  uint32 numPackets;
  uint32 packetsProcessed;
  uint32 maxPacketsPerSuperBlock;
  uint32 maxInstructionsPerPacket;
  bool bSinglePacket;
  bool bAllowBlockCompile;
  PacketEntry *packets;
  InstructionEntry *instructions;
  SuperBlockConstants *constants;
  uint32 startAddress;
  uint32 exitAddress;
  uint32 nextDelayCounter;
  MPE *pMPE;
  FILE *blockFile;
private:
  bool bCanEmitNativeCode;
};

#endif