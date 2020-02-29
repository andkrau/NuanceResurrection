#ifndef PAGEMAP_H
#define PAGEMAP_H

#include "basetypes.h"

typedef void (* NativeCodeCacheEntryPoint)(void);
enum SuperBlockCompileType;

struct NativeCodeCacheEntry
{
  NativeCodeCacheEntryPoint entryPoint;  //Pointer to start of code block
  uint32 virtualAddress;  //Virtual address of instruction starting this code block
  SuperBlockCompileType compileType;  //Compile type: native or IL
  uint32 numPackets;  //Number of packets represented in code block
  uint32 numInstructions;  //Number of explicit packeted instructions contained in code block
  uint32 nextVirtualAddress;  //Virtual address of instruction following this code block
  uint32 nextBranchDelayCount;
  uint32 codeSize;  //Number of bytes used in code block
  uint32 accessCount;  //Number of times code block has been executed
};

#define NUM_ROOT_PAGENODE_ENTRIES (512)
#define NUM_LEVEL1_PAGENODE_ENTRIES (8192)
#define NUM_LEVEL2_PAGENODE_ENTRIES (1024)

//struct Level2PageNode
//{
//  NativeCodeCacheEntry *entries[NUM_LEVEL2_PAGENODE_ENTRIES];
//};

struct Level1PageNode
{
  NativeCodeCacheEntry *entries[NUM_LEVEL1_PAGENODE_ENTRIES];
};

struct RootPageNode
{
  Level1PageNode *entries[NUM_ROOT_PAGENODE_ENTRIES];
};

class PageMap
{
public:
  PageMap();
  ~PageMap();

  NativeCodeCacheEntry *AllocatePage(const uint32 address);
  void UpdateEntry(NativeCodeCacheEntry &entry)
  {
    AllocatePage(entry.virtualAddress)[entry.virtualAddress & 0x3FFUL] = entry;
  }
  void Invalidate();
  void InvalidateEntry(const uint32 address);
  void InvalidateRegion(const uint32 startAddress, const uint32 endAddress)
  {
    uint32 address = startAddress;

    while(address <= endAddress)
    {
      InvalidateEntry(address);
      address++;
    }
  }

  NativeCodeCacheEntry* FindEntry(const uint32 address) const
  {
    const uint32 rootIndex = address >> 23;
    const Level1PageNode* const pL1Node = root.entries[rootIndex];

    if(!pL1Node)
      return 0;
 
    const uint32 l1Index = (address >> 10) & 0x1FFFUL;
    NativeCodeCacheEntry* const pL2Node = pL1Node->entries[l1Index];
  
    if(!pL2Node)
      return 0;

    const uint32 l2Index = address & 0x3FFUL;
    return pL2Node+l2Index;
  }

private:
  RootPageNode root;
};

#endif
