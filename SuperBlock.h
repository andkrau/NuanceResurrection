#ifndef INSTRUCTION_SUPERBLOCK_H
#define INSTRUCTION_SUPERBLOCK_H

#include "basetypes.h"
#include "InstructionCache.h"
#include "nativecodecache.h"
#include "SuperBlockConstants.h"

#define MAX_SUPERBLOCK_PACKETS 120
#define MAX_SUPERBLOCK_INSTRUCTIONS_PER_PACKET 5

class CompilerOptions
{
public:
  CompilerOptions()
  {
    bT3KCompilerHack = false;
    bConstantPropagation = false;
    bDeadCodeElimination = false;
#ifdef _WIN64
    bAllowCompile = false; //!!
#else
    bAllowCompile = true;
#endif
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    bDumpBlocks = false;
#endif
  }

  bool bT3KCompilerHack;
  bool bConstantPropagation;
  bool bDeadCodeElimination;
  bool bAllowCompile; //!! on 64bit this is always force disabled for now, as no x64 code can be emitted
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  bool bDumpBlocks;
#endif
};

enum class SuperBlockCompileType
{
  SUPERBLOCKCOMPILETYPE_UNKNOWN = 0UL,
  SUPERBLOCKCOMPILETYPE_IL_SINGLE,
  SUPERBLOCKCOMPILETYPE_IL_BLOCK,
  SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK
};

class SuperBlock
{
public:
  SuperBlock(MPE * const mpe);
  ~SuperBlock();

#ifdef ENABLE_EMULATION_MESSAGEBOXES
  void PrintBlockToFile(SuperBlockCompileType blockType, uint32 size);
#endif
  void AddPacketToList(const InstructionCacheEntry &packet, const uint32 index);
  bool AddInstructionsToList(InstructionCacheEntry &packet, PacketEntry * const pPacketEntry, const uint32 index, const bool bExplicitNOP);
  NativeCodeCacheEntryPoint CompileBlock(const uint32 address, NativeCodeCache &codeCache, bool &bError);
  bool EmitCodeBlock(NativeCodeCache &codeCache, const bool bContainsBranch);
  void UpdateDependencyInfo();
  void PerformConstantPropagation();
  uint32 PerformDeadCodeElimination();
  int32 FetchSuperBlock(uint32 address, bool &bContainsBranch);

  MPE *pMPE;
  //allocate enough instruction entries to account for packet start/end IL
  InstructionEntry instructions[(MAX_SUPERBLOCK_PACKETS + 2) * (MAX_SUPERBLOCK_INSTRUCTIONS_PER_PACKET + 2)];
  uint32 numInstructions;

private:
  PacketEntry packets[MAX_SUPERBLOCK_PACKETS + 2];
  SuperBlockConstants constants;
  uint32 numPackets;
  int32 packetsProcessed;
  uint32 startAddress;
  uint32 exitAddress;
  uint32 nextDelayCounter;
  bool bCanEmitNativeCode;
  static constexpr bool bAllowBlockCompile = (COMPILE_TYPE != SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_SINGLE);
};

#endif
