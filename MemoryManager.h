//---------------------------------------------------------------------------
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
//---------------------------------------------------------------------------

#include <vector>
using namespace std;

class FreeMemoryBlock
{
  public:
  uint32 startAddress;
  uint32 endAddress;
  uint32 numBytes;
};

class AllocatedMemoryBlock
{
  public:
  uint32 startAddress;
  uint32 numBytes;
};

class MemoryManager
{
public:
  MemoryManager(uint32 maxBytes, uint32 defaultAlignment = 16);
  void AddAllocatedBlock(uint32 base, uint32 numBytes);
  void Add(uint32 startAddress, uint32 endAddress, uint32 index = 0);
  void *Allocate(uint32 requestedBytes, uint32 requestedAlignment);
  void Free(uint32 address);
  inline void Reset()
  {
    freeBlockVector.clear();
    allocatedBlockVector.clear();
  }

private:
  uint32 AlignAddress(uint32 address, uint32 alignment);
  bool TestForInvalidPowerOfTwo(uint32 requestedAlignment);
  uint32 maxBytes;
  uint32 defaultAlignment;
  vector<FreeMemoryBlock> freeBlockVector;
  vector<AllocatedMemoryBlock> allocatedBlockVector;
};

/*
void MemAlloc(MPE *);
void MemFree(MPE *);
void MemInit(MPE *);
void MemLocalScratch(MPE *the_mpe);
void FreeListEntryMemory(TList *blockList);
*/

#endif
