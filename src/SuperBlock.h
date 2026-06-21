#ifndef INSTRUCTION_SUPERBLOCK_H
#define INSTRUCTION_SUPERBLOCK_H

#include "basetypes.h"
#include "InstructionCache.h"
#include "NativeCodeCache.h"
#include "SuperBlockConstants.h"

#define MAX_SUPERBLOCK_PACKETS 120
#define MAX_SUPERBLOCK_INSTRUCTIONS_PER_PACKET 5

class CompilerOptions final
{
public:
  CompilerOptions()
  {
    // When enabled, MPE3's system-bus native blocks are capped to a single packet (see SuperBlock::FetchSuperBlock)
    // so MPE3 yields to the sub-MPEs at per-packet granularity. Needed for games with tight cross-MPE handshakes
    // in MPE3 system-bus code (e.g. suspected for T3K's level select), which otherwise livelock under the JIT. Should be low perf cost
    bMPE3PacketHack = 1;
    bConstantPropagation = false;
    bDeadCodeElimination = false;
#if defined(_WIN64) || (defined(__x86_64__) && !defined(_WIN32))
  #ifdef USE_ASMJIT
    bAllowCompile = true;
  #else
    bAllowCompile = false; // 'old' JIT cannot emit 64bit-, only 32bit-x86 code
  #endif
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__)
    bAllowCompile = false; // no ARM native emitter yet: interpreter only
#else
    bAllowCompile = true;
#endif
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    bDumpBlocks = false;
#endif
  }

  char bMPE3PacketHack; // 0: disabled, 1: enabled, 2: T3K-level-select-hang-range-only
  bool bConstantPropagation;
  bool bDeadCodeElimination;
  bool bAllowCompile;
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  bool bDumpBlocks;
#endif
};

enum class SuperBlockCompileType
{
  SUPERBLOCKCOMPILETYPE_UNKNOWN = 0U,
  SUPERBLOCKCOMPILETYPE_IL_SINGLE,
  SUPERBLOCKCOMPILETYPE_IL_BLOCK,
  SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK
};

class SuperBlock final
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
