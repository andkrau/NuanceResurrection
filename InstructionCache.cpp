#include "basetypes.h"
#include "InstructionCache.h"

InstructionCache::InstructionCache(const uint32 desiredEntries)
{
  numEntries = desiredEntries;

  if(!numEntries)
  {
    numEntries = DEFAULT_NUM_CACHE_ENTRIES;
  }

  validBitmap = new uint32[(numEntries/32) + 1];
  //!! init_array((uint8*)validBitmap, ((numEntries / 32) + 1)*sizeof(uint32));
  cacheEntries = new InstructionCacheEntry[numEntries];
  //!! init_array((uint8*)cacheEntries, numEntries*sizeof(InstructionCacheEntry));

  Invalidate();
}

InstructionCache::~InstructionCache()
{
  if(cacheEntries)
  {
    delete [] cacheEntries;
  }

  if(validBitmap)
  {
    delete [] validBitmap;
  }
}

void InstructionCache::Invalidate()
{
  memset(validBitmap, 0, (numEntries/32 + 1)*sizeof(uint32));
}

void InstructionCache::InvalidateRegion(const uint32 start, const uint32 end)
{
  uint32 mask = 0x80000000UL;
  uint32 validBitmapIndex = 0;

  for(uint32 i = 0; i < numEntries; i++)
  {
    const uint32 tag = cacheEntries[i].pcexec;
    if((tag >= start) && (tag <= end))
    {
      validBitmap[validBitmapIndex] &= ~mask;
    }
    if(mask == 0x01)
    {
      validBitmapIndex++;
      mask = 0x80000000UL;
    }
    else
    {
      mask >>= 1;
    }
  }
}

void InstructionCacheEntry::CopyInstructionData(const uint32 toSlot, const InstructionCacheEntry &src, const uint32 fromSlot)
{
  nuances[FIXED_FIELD(toSlot,0)] = src.nuances[FIXED_FIELD(fromSlot,0)];
  nuances[FIXED_FIELD(toSlot,1)] = src.nuances[FIXED_FIELD(fromSlot,1)];
  nuances[FIXED_FIELD(toSlot,2)] = src.nuances[FIXED_FIELD(fromSlot,2)];
  nuances[FIXED_FIELD(toSlot,3)] = src.nuances[FIXED_FIELD(fromSlot,3)];
  nuances[FIXED_FIELD(toSlot,4)] = src.nuances[FIXED_FIELD(fromSlot,4)];
  scalarInputDependencies[toSlot] = src.scalarInputDependencies[fromSlot];
  miscInputDependencies[toSlot] = src.miscInputDependencies[fromSlot];
  scalarOutputDependencies[toSlot] = src.scalarOutputDependencies[fromSlot];
  miscOutputDependencies[toSlot] = src.miscOutputDependencies[fromSlot];
}
