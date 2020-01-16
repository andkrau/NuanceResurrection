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

  NativeCodeCacheEntry *AllocatePage(const uint32 address);
  NativeCodeCacheEntry *FindEntry(const uint32 address);
  NativeCodeCacheEntry *AllocateEntry(const uint32 address);
  void UpdateEntry(const uint32 address,NativeCodeCacheEntry &entry);
  void Invalidate();
  void InvalidateEntry(const uint32 address);
  void InvalidateRegion(const uint32 startAddress, const uint32 endAddress);

private:
  RootPageNode root;
};


#endif