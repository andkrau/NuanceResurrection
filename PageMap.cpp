#include "basetypes.h"
#include "PageMap.h"
#include "NativeCodeCache.h"

PageMap::PageMap()
{
  for(uint32 i = 0; i < NUM_ROOT_PAGENODE_ENTRIES; i++)
  {
    root.entries[i] = 0;
  }
}

PageMap::~PageMap()
{
  for(uint32 nRoot = 0; nRoot < NUM_ROOT_PAGENODE_ENTRIES; nRoot++)
  {
    const Level1PageNode * const pL1 = root.entries[nRoot];
    if(pL1)
    {
      for(uint32 nL1 = 0; nL1 < NUM_LEVEL1_PAGENODE_ENTRIES; nL1++)
      {
        NativeCodeCacheEntry * const pL2 = pL1->entries[nL1];

        if(pL2)
        {
          delete [] pL2;
        }
      }
      delete pL1;
    }
  }
}

NativeCodeCacheEntry *PageMap::AllocateEntry(const uint32 address)
{
  return &((AllocatePage(address))[address & 0x3FFUL]);
}

void PageMap::UpdateEntry(const uint32 address,NativeCodeCacheEntry &entry)
{
  *(AllocateEntry(address)) = entry;
}

NativeCodeCacheEntry *PageMap::AllocatePage(const uint32 address)
{
  const uint32 rootIndex = address >> 23;
  const uint32 l1Index = (address >> 10) & 0x1FFFUL;

  Level1PageNode* pL1Node = root.entries[rootIndex];

  if(!pL1Node)
  {
    pL1Node = new Level1PageNode;
    for(uint32 i = 0; i < NUM_LEVEL1_PAGENODE_ENTRIES; i++)
    {
      pL1Node->entries[i] = 0;  
    }
    root.entries[rootIndex] = pL1Node;
  }

  NativeCodeCacheEntry* pL2Node = pL1Node->entries[l1Index];
  if(!pL2Node)
  {
    pL2Node = new NativeCodeCacheEntry[NUM_LEVEL2_PAGENODE_ENTRIES];
    //!! init_array((uint8*)pL2Node, NUM_LEVEL2_PAGENODE_ENTRIES * sizeof(NativeCodeCacheEntry));
    for(uint32 i = 0; i < NUM_LEVEL2_PAGENODE_ENTRIES; i++)
    {
      pL2Node[i].virtualAddress = 1;  
    }
    pL1Node->entries[l1Index] = pL2Node;
  }

  return pL2Node;
}

void PageMap::InvalidateEntry(const uint32 address)
{ 
  const uint32 rootIndex = (address >> 23);
  const Level1PageNode* const pL1Node = root.entries[rootIndex];

  if(pL1Node)
  {
    const uint32 l1Index = (address >> 10) & 0x1FFFUL;
    NativeCodeCacheEntry * const pL2Node = pL1Node->entries[l1Index];
    {
      if(pL2Node)
      {
        const uint32 l2Index = address & 0x3FFUL;
        pL2Node[l2Index].virtualAddress = 1;
      }
    }
  }
}

void PageMap::Invalidate(void)
{ 
  for(uint32 rootIndex = 0; rootIndex < NUM_ROOT_PAGENODE_ENTRIES; rootIndex++)
  {
    if(root.entries[rootIndex])
    {
      const Level1PageNode * const pL1Node = root.entries[rootIndex];
      for(uint32 l1Index = 0; l1Index < NUM_LEVEL1_PAGENODE_ENTRIES; l1Index++)
      {
        if(pL1Node->entries[l1Index])
        {
          NativeCodeCacheEntry * const pL2Node = pL1Node->entries[l1Index];
          for(uint32 l2Index = 0; l2Index < NUM_LEVEL2_PAGENODE_ENTRIES; l2Index++)
          {
            pL2Node[l2Index].virtualAddress = 1;
          }
        }
      }
    }
  }
}

void PageMap::InvalidateRegion(const uint32 start, const uint32 end)
{
  uint32 address = start;

  while(address <= end)
  {
    InvalidateEntry(address);
    address++;
  }
}
