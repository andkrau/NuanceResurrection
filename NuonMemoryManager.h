#ifndef NUONMEMORYMANAGER_H
#define NUONMEMORYMANAGER_H

#include "basetypes.h"
#include "MemoryManager.h"

class NuonMemoryManager
{
public:
  NuonMemoryManager();
  ~NuonMemoryManager();
  uint32 Alloc(uint32 requestedBytes, uint32 requestedAlignment, uint32 flags);
  void Free(uint32 address);
  void Init();
private:
  MemoryManager *mainBusMemoryManager;
  MemoryManager *otherBusMemoryManager;
};

#endif