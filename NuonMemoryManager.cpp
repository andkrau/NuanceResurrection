#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
 #include <windows.h>
#endif
#include "Bios.h"
#include "NuonMemoryMap.h"
#include "NuonMemoryManager.h"
#include "MemoryManager.h"

uint32 NuonMemoryManager::Alloc(const uint32 requestedBytes, const uint32 requestedAlignment, const uint32 flags)
{
  uint32 address = 0;

  if(flags & kMemSDRAM)
  {
    address = mainBusMemoryManager.Allocate(requestedBytes,requestedAlignment);
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    if((address != 0) && ((address < MAIN_BUS_BASE) || (address >= MAIN_BUS_BASE + MAIN_BUS_SIZE)))
    {
      char msg[128];
      sprintf(msg,"Illegal Main Bus memory address allocation: %8.8lX",address);
      MessageBox(NULL,"MemAlloc Error",msg,MB_OK);
    }
#endif
  }
  
  if((address == 0) && (flags & kMemSysRam))
  {
    address = otherBusMemoryManager.Allocate(requestedBytes,requestedAlignment);
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    if((address != 0) && ((address < SYSTEM_BUS_BASE) || (address >= BIOS_FUNCTIONS_BASE)))
    {
      char msg[128];
      sprintf(msg,"Illegal Other Bus memory address allocation: %8.8lX",address);
      MessageBox(NULL,"MemAlloc Error",msg,MB_OK);
    }
#endif
  }

  return address;
}

void NuonMemoryManager::Free(uint32 address)
{
  if((address >= MAIN_BUS_BASE) && (address < (MAIN_BUS_BASE + MAIN_BUS_SIZE)))
  {
    mainBusMemoryManager.Free(address);
  }
  else if((address >= SYSTEM_BUS_BASE) && (address < (SYSTEM_BUS_BASE + SYSTEM_BUS_SIZE)))
  {
    otherBusMemoryManager.Free(address);
  }
}

void NuonMemoryManager::Init()
{
  otherBusMemoryManager.Reset();
  mainBusMemoryManager.Reset();

  mainBusMemoryManager.Add(MAIN_BUS_BASE, MAIN_BUS_BASE + MAIN_BUS_SIZE - 1);

  otherBusMemoryManager.Add(SYSTEM_BUS_BASE + BIOS_LOWMEM_SIZE, BIOS_FUNCTIONS_BASE - 1);
  //Insert the 640K block of System Bus kernel memory into the allocated block list
  otherBusMemoryManager.AddAllocatedBlock(BIOS_FUNCTIONS_BASE, BIOS_FUNCTIONS_SIZE);
  //Insert the 64K block of System Bus kernel memory into the allocated block list
  otherBusMemoryManager.AddAllocatedBlock(SYSTEM_BUS_BASE, BIOS_LOWMEM_SIZE);

  if(SYSTEM_BUS_SIZE > (8UL*1024UL*1024UL))
  {
    //Main Bus DRAM is greater than 8 megs
    otherBusMemoryManager.Add(BIOS_FUNCTIONS_BASE + BIOS_FUNCTIONS_SIZE, SYSTEM_BUS_SIZE - (8UL*1024UL*1024UL));
  }
}
