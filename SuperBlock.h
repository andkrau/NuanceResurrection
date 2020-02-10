#ifndef INSTRUCTION_SUPERBLOCK_H
#define INSTRUCTION_SUPERBLOCK_H

#include "basetypes.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "NativeCodeCache.h"

#define MAX_SUPERBLOCK_PACKETS 120
#define MAX_SUPERBLOCK_INSTRUCTIONS_PER_PACKET 5

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
#ifdef _WIN64
    bAllowCompile = false; //!!
#else
    bAllowCompile = true;
#endif
    bDumpBlocks = false;
  }

  bool bT3KCompilerHack;
  bool bConstantPropagation;
  bool bDeadCodeElimination;
  bool bAllowCompile; //!! on 64bit this is always force disabled for now, as no x64 code can be emitted
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
  SuperBlock(MPE * const mpe);
  ~SuperBlock();

  void PrintBlockToFile(SuperBlockCompileType blockType, uint32 size);
  void AddPacketToList(InstructionCacheEntry &packet, const uint32 index);
  bool AddInstructionsToList(InstructionCacheEntry &packet, PacketEntry * const pPacketEntry, const uint32 index, const bool bExplicitNOP = false);
  NativeCodeCacheEntryPoint CompileBlock(const uint32 address, NativeCodeCache &codeCache, const SuperBlockCompileType eCompileType, const bool bSinglePacket, bool &bError);
  bool EmitCodeBlock(NativeCodeCache &codeCache, SuperBlockCompileType compileType, const bool bContainsBranch);
  void UpdateDependencyInfo();
  void PerformConstantPropagation();
  uint32 PerformDeadCodeElimination();
  int32 FetchSuperBlock(uint32 address, bool &bContainsBranch);

  MPE *pMPE;
  InstructionEntry *instructions;
  uint32 numInstructions;

private:
  PacketEntry *packets;
  SuperBlockConstants *constants;
  uint32 numPackets;
  uint32 packetsProcessed;
  uint32 startAddress;
  uint32 exitAddress;
  uint32 nextDelayCounter;
  bool bSinglePacket;
  bool bAllowBlockCompile;
  bool bCanEmitNativeCode;
};

#endif
