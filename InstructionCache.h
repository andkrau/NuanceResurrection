#ifndef INSTRUCTION_CACHE_H
#define INSTRUCTION_CACHE_H

#include "basetypes.h"

#define MAX_INSTRUCTIONS_PER_PACKET (5UL)
#define FIELDS_PER_NUANCE (5UL)
#define DEFAULT_NUM_CACHE_ENTRIES (8192UL)
#define FIXED_FIELD(slot,field) (((slot) * FIELDS_PER_NUANCE) + (field))

#define SLOT_ECU (0)
#define SLOT_RCU (1)
#define SLOT_ALU (2)
#define SLOT_MUL (3)
#define SLOT_MEM (4)

#define FIELD_RCU_HANDLER (0)
#define FIELD_RCU_INFO (1)
#define FIELD_RCU_SRC (2)
#define FIELD_RCU_DEST (3)

#define FIELD_ECU_HANDLER (0)
#define FIELD_ECU_CONDITION (1)
#define FIELD_ECU_ADDRESS (2)
#define FIELD_ECU_PCROUTE (3)
#define FIELD_ECU_PCFETCHNEXT (4)

#define FIELD_ALU_HANDLER (0)
#define FIELD_ALU_INFO (1)
#define FIELD_ALU_SRC1 (2)
#define FIELD_ALU_SRC2 (3)
#define FIELD_ALU_DEST (4)
#define RCU_DEC_RC0 (0x02)
#define RCU_DEC_RC1 (0x01)

#define FIELD_MUL_HANDLER (0)
#define FIELD_MUL_INFO (1)
#define FIELD_MUL_SRC1 (2)
#define FIELD_MUL_SRC2 (3)
#define FIELD_MUL_DEST (4)

#define FIELD_MEM_HANDLER (0)
#define FIELD_MEM_INFO (1)
#define FIELD_MEM_TO (2)
#define FIELD_MEM_FROM (3)
#define FIELD_MEM_POINTER (4)

#define FIELD_CONSTANT_HANDLER (0)
#define FIELD_CONSTANT_ADDRESS (1)
#define FIELD_CONSTANT_VALUE (2)
#define FIELD_CONSTANT_FLAGMASK (3)
#define FIELD_CONSTANT_FLAGVALUES (4)

#define MEM_INFO_LINEAR_ABSOLUTE (0x00000001UL)
#define MEM_INFO_LINEAR_INDIRECT (0x00000002UL)
#define MEM_INFO_BILINEAR_XY (0x00000004UL)
#define MEM_INFO_BILINEAR_UV (0x00000008UL)

//This flag indicates a packet is compiled
#define PACKETINFO_COMPILED (0x80000000UL)
//This flag indicates a given packet block should never be compiled 
#define PACKETINFO_NEVERCOMPILE (0x40000000UL)
//This flag indicates the packet may trigger an exception
#define PACKETINFO_EXCEPTION (0x20000000UL)
//This flag indicates that the packet contains an instruction requiring pcfetchnext
#define PACKETINFO_NEEDS_PCFETCHNEXT (0x10000000UL)
//This flag indicates that the packet contains a branch that is always taken
#define PACKETINFO_BRANCH_ALWAYS (0x08000000UL)
//This flag indicates that the packet contains a branch that may or may not be taken
#define PACKETINFO_BRANCH_CONDITIONAL (0x04000000UL)
//This flag indicates that the packet contains a non-delayed branch
#define PACKETINFO_BRANCH_NOP (0x02000000UL)
//This flag indicates a packet that contains an indirect load or store
#define PACKETINFO_MEMORY_INDIRECT (0x01000000UL)
//This flag indicates a packet containing a direct load or store to a control register
#define PACKETINFO_MEMORY_IO (0x00800000UL)
//This flag indicates a packet where the ECU unit is known to be inhibited
#define SUPERBLOCKINFO_INHIBIT_ECU (0x00400000UL)
//This flag indicates packets where the ECU inhibit state cannot be predetermined
#define SUPERBLOCKINFO_CHECK_ECU_INHIBIT (0x00200000UL)
//This flag indicates packets where the ECU skip counter must be checked
#define SUPERBLOCKINFO_CHECK_ECUSKIPCOUNTER (0x00100000UL)
//This flag denotes packets that require register/flag states be consistent w/ interpretation
#define SUPERBLOCKINFO_SYNC (0x00080000UL)
//This flag indicates an instruction marks the start of a packet
#define SUPERBLOCKINFO_PACKETSTART (0x00040000UL)
//This flag indicates an instruction marks the end of a packet
#define SUPERBLOCKINFO_PACKETEND (0x00020000UL)
//This flag indicates an instruction must be emitted as-is to maintain proper state
#define SUPERBLOCKINFO_LOCKED (0x00010000UL)
//This flag indicates an instruction that is considered dead code
#define SUPERBLOCKINFO_DEAD (0x00008000UL)
//This flag indicates a packet that cannot be compiled natively
#define SUPERBLOCKINFO_NONATIVECOMPILE (0x00004000UL)


//This flag indicates a packet that contains an operand dependency requiring operand copy
#define PACKETINFO_DEPENDENCY_PRESENT (0x00000080UL)
//This flag indicates an ALU instruction is present in the packet
#define PACKETINFO_ALU (0x00000040UL)
//This flag indicates an MUL instruction is present in the packet
#define PACKETINFO_MUL (0x00000020UL)
//This flag indicates an MEM instruction is present in the packet
#define PACKETINFO_MEM (0x00000010UL)
//This flag indicates an RCU instruction is present in the packet
#define PACKETINFO_RCU (0x00000008UL)
//This flag indicates an ECU instruction is present in the packet
#define PACKETINFO_ECU (0x00000004UL)
//This flag indicates the packet contains a breakpoint
#define PACKETINFO_BREAKPOINT (0x00000002UL)
//This flag indicates that the packet is a NOP or BREAK,NOP
#define PACKETINFO_NOP (0x00000001UL)
#define GETPACKETEXECUTIONUNITS(x) (((x) & (PACKETINFO_ALU|PACKETINFO_MUL|PACKETINFO_MEM|PACKETINFO_RCU|PACKETINFO_ECU)) >> 2)

struct Nuance
{
  size_t fields[FIELDS_PER_NUANCE]; //!! size_t due to some being real pointers, i.e. to support 64bit compiles
};

struct PacketEntry
{
  uint32 pcexec;
  uint32 pcroute;
  //uint32 pcfetchnext;
  //uint32 instructionCount;
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

class InstructionCacheEntry
{
public:
  InstructionCacheEntry() { pcexec = 0; }

  uint32* pRegs; // points to either a MPE reg set or its temp reg set (48 entries)

  uint32 handlers[MAX_INSTRUCTIONS_PER_PACKET];
  uint32 nuanceCount;
  uint32 packetInfo;
  uint32 pcexec;
  uint32 frequencyCount;
  uint32 pcroute;
  uint32 pcfetchnext;
  uint32 ecuConditionCode;
  size_t nuances[MAX_INSTRUCTIONS_PER_PACKET * FIELDS_PER_NUANCE]; //!! size_t due to some being real pointers, i.e. to support 64bit compiles

  // do not change order of these 4, see below
  uint32 scalarInputDependencies[MAX_INSTRUCTIONS_PER_PACKET];
  uint32 miscInputDependencies[MAX_INSTRUCTIONS_PER_PACKET];
  uint32 scalarOutputDependencies[MAX_INSTRUCTIONS_PER_PACKET];
  uint32 miscOutputDependencies[MAX_INSTRUCTIONS_PER_PACKET];

  void ClearDependencies()
  {
    memset(scalarInputDependencies, 0, MAX_INSTRUCTIONS_PER_PACKET*4*sizeof(uint32)); // clear all 4 arrays above
  }

  void CopyInstructionData(const uint32 toSlot, const InstructionCacheEntry &src, const uint32 fromSlot);
};

class InstructionCache
{
public:
  InstructionCache(const uint32 numEntries);
  ~InstructionCache();

  void Invalidate();
  void InvalidateRegion(const uint32 start, const uint32 end);

  InstructionCacheEntry *FindInstructionCacheEntry(const uint32 addressKey, bool &bValid) const
  {
    const uint32 cacheEntryIndex = (addressKey >> 1) & (numEntries - 1);
    bValid = (validBitmap[cacheEntryIndex >> 5] & (0x80000000UL >> (cacheEntryIndex & 0x1FUL)));
    return &cacheEntries[cacheEntryIndex];
  }

  void SetEntryValid(const uint32 addressKey)
  {
    const uint32 cacheEntryIndex = (addressKey >> 1) & (numEntries - 1);
    validBitmap[cacheEntryIndex >> 5] |= (0x80000000UL >> (cacheEntryIndex & 0x1FUL));
  }

  void ClearCompiledStates()
  {
    for(uint32 i = 0; i < numEntries; i++)
      cacheEntries[i].packetInfo &= ~PACKETINFO_COMPILED;
  }

private:
  uint32 *validBitmap;
  InstructionCacheEntry *cacheEntries;
  uint32 numEntries;
};

#endif
