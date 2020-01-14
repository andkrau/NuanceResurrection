#include "PageMap.h"
#include "Basetypes.h"
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
  uint32 nRoot, nL1;
  Level1PageNode *pL1;
  NativeCodeCacheEntry *pL2;

  for(nRoot = 0; nRoot < NUM_ROOT_PAGENODE_ENTRIES; nRoot++)
  {
    pL1 = root.entries[nRoot];
    if(pL1)
    {
      for(nL1 = 0; nL1 < NUM_LEVEL1_PAGENODE_ENTRIES; nL1++)
      {
        pL2 = pL1->entries[nL1];

        if(pL2)
        {
          delete [] pL2;
        }
      }
      delete pL1;
    }
  }
}

NativeCodeCacheEntry *PageMap::AllocateEntry(uint32 address)
{
  return &((AllocatePage(address))[address & 0x3FFUL]);
}

void PageMap::UpdateEntry(uint32 address,NativeCodeCacheEntry &entry)
{
  *(AllocateEntry(address)) = entry;
}

NativeCodeCacheEntry *PageMap::AllocatePage(uint32 address)
{
  uint32 rootIndex = address >> 23;
  uint32 l1Index = (address >> 10) & 0x1FFFUL;

  Level1PageNode *pL1Node;
  NativeCodeCacheEntry *pL2Node;

  pL1Node = root.entries[rootIndex];

  if(!pL1Node)
  {
    pL1Node = new Level1PageNode;
    for(uint32 i = 0; i < NUM_LEVEL1_PAGENODE_ENTRIES; i++)
    {
      pL1Node->entries[i] = 0;  
    }
    root.entries[rootIndex] = pL1Node;
  }

  pL2Node = pL1Node->entries[l1Index];
  if(!pL2Node)
  {
    pL2Node = new NativeCodeCacheEntry[NUM_LEVEL2_PAGENODE_ENTRIES];
    for(uint32 i = 0; i < NUM_LEVEL2_PAGENODE_ENTRIES; i++)
    {
      pL2Node[i].virtualAddress = 1;  
    }
    pL1Node->entries[l1Index] = pL2Node;
  }

  return pL2Node;
}

NativeCodeCacheEntry *PageMap::FindEntry(uint32 address)
{
  uint32 rootIndex = address >> 23;
  uint32 l1Index;
  uint32 l2Index;

  Level1PageNode *pL1Node;
  NativeCodeCacheEntry *pL2Node;

  pL1Node = root.entries[rootIndex];

  if(!pL1Node)
  {
    return 0;
  }
  
  l1Index = (address >> 10) & 0x1FFFUL;
  pL2Node = pL1Node->entries[l1Index];
  
  if(!pL2Node)
  {
    return 0;
  }

  l2Index = address & 0x3FFUL;
  return &(pL2Node[l2Index]);
}

void PageMap::InvalidateEntry(uint32 address)
{ 
  uint32 rootIndex = (address >> 23);
  uint32 l1Index;
  uint32 l2Index;

  Level1PageNode *pL1Node;
  NativeCodeCacheEntry *pL2Node;

  pL1Node = root.entries[rootIndex];

  if(pL1Node)
  {
    l1Index = (address >> 10) & 0x1FFFUL;
    pL2Node = pL1Node->entries[l1Index];
    {
      if(pL2Node)
      {
        l2Index = address & 0x3FFUL;
        pL2Node[l2Index].virtualAddress = 1;
      }
    }
  }
}

uint32 PageMap::Invalidate(void)
{ 
  Level1PageNode *pL1Node;
  NativeCodeCacheEntry *pL2Node;
  uint32 numInvalidations = 0;
  uint32 rootIndex, l1Index, l2Index;
  uint32 lastRootIndex, lastL1Index, lastL2Index;

  for(rootIndex = 0; rootIndex < NUM_ROOT_PAGENODE_ENTRIES; rootIndex++)
  {
    if(root.entries[rootIndex])
    {
      pL1Node = root.entries[rootIndex];
      for(l1Index = 0; l1Index < NUM_LEVEL1_PAGENODE_ENTRIES; l1Index++)
      {
        if(pL1Node->entries[l1Index])
        {
          pL2Node = pL1Node->entries[l1Index];
          for(l2Index = 0; l2Index < NUM_LEVEL2_PAGENODE_ENTRIES; l2Index++)
          {
            pL2Node[l2Index].virtualAddress = 1;
          }
        }
      }
    }
  }

   return numInvalidations;
}

void PageMap::InvalidateRegion(uint32 start, uint32 end)
{
  uint32 address = start;
  NativeCodeCacheEntry *entry;

  while(address <= end)
  {
    InvalidateEntry(address);
    address++;
  }
}