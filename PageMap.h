#ifndef PAGEMAP_H
#define PAGEMAP_H

#include "basetypes.h"

#define NUM_ROOT_PAGENODE_ENTRIES (512)
#define NUM_LEVEL1_PAGENODE_ENTRIES (8192)
#define NUM_LEVEL2_PAGENODE_ENTRIES (1024)

class NativeCodeCacheEntry;

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
  NativeCodeCacheEntry *AllocatePage(uint32 address);
  NativeCodeCacheEntry *FindEntry(uint32 address);
  NativeCodeCacheEntry *AllocateEntry(uint32 address);
  void UpdateEntry(uint32 address,NativeCodeCacheEntry &entry);
  uint32 Invalidate();
  void InvalidateEntry(uint32 address);
  void InvalidateRegion(uint32 startAddress, uint32 endAddress);
private:
  RootPageNode root;
};


#endif