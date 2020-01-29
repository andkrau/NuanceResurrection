#include "basetypes.h"
#include "Bios.h"
#include "NuonMemoryMap.h"
#include "NuonMemoryManager.h"
#include "MemoryManager.h"

NuonMemoryManager::NuonMemoryManager()
{
  mainBusMemoryManager = new MemoryManager(8*1024*1024,16);
  otherBusMemoryManager = new MemoryManager(8*1024*1024,16);
}

NuonMemoryManager::~NuonMemoryManager()
{
  delete mainBusMemoryManager;
  delete otherBusMemoryManager;
}

uint32 NuonMemoryManager::Alloc(uint32 requestedBytes, uint32 requestedAlignment, uint32 flags)
{
  char msg[128];
  uint32 address = 0;

  if(flags & kMemSDRAM)
  {
    address = mainBusMemoryManager->Allocate(requestedBytes,requestedAlignment);
    if((address != 0) && ((address < MAIN_BUS_BASE) || (address >= MAIN_BUS_BASE + MAIN_BUS_SIZE)))
    {
      sprintf(msg,"Illegal Main Bus memory address allocation: %8.8lX",address);
      //QMessageBox::warning(0,"MemAlloc Error",QString(msg));
    }
  }
  
  if((address == 0) && (flags & kMemSysRam))
  {
    address = otherBusMemoryManager->Allocate(requestedBytes,requestedAlignment);
    if((address != 0) && ((address < SYSTEM_BUS_BASE) || (address >= BIOS_FUNCTIONS_BASE)))
    {
      sprintf(msg,"Illegal Other Bus memory address allocation: %8.8lX",address);
      //QMessageBox::warning(0,"MemAlloc Error",QString(msg));
    }
  }


  return address;
}

void NuonMemoryManager::Free(uint32 address)
{
  if((address >= MAIN_BUS_BASE) && (address < (MAIN_BUS_BASE + MAIN_BUS_SIZE)))
  {
    mainBusMemoryManager->Free(address);
  }
  else if((address >= SYSTEM_BUS_BASE) && (address < (SYSTEM_BUS_BASE + SYSTEM_BUS_SIZE)))
  {
    otherBusMemoryManager->Free(address);
  }
}

void NuonMemoryManager::Init()
{
  otherBusMemoryManager->Reset();
  mainBusMemoryManager->Reset();

  mainBusMemoryManager->Add(MAIN_BUS_BASE, MAIN_BUS_BASE + MAIN_BUS_SIZE - 1);
  otherBusMemoryManager->Add(SYSTEM_BUS_BASE + BIOS_LOWMEM_SIZE, BIOS_FUNCTIONS_BASE - 1);
  //Insert the 640K block of System Bus kernel memory into the allocated block list
  otherBusMemoryManager->AddAllocatedBlock(BIOS_FUNCTIONS_BASE, BIOS_FUNCTIONS_SIZE);
  //Insert the 64K block of System Bus kernel memory into the allocated block list
  otherBusMemoryManager->AddAllocatedBlock(SYSTEM_BUS_BASE, BIOS_LOWMEM_SIZE);

  if(SYSTEM_BUS_SIZE > (8UL*1024UL*1024UL))
  {
    //Main Bus DRAM is greater than 8 megs
    otherBusMemoryManager->Add(BIOS_FUNCTIONS_BASE + BIOS_FUNCTIONS_SIZE, SYSTEM_BUS_SIZE - (8UL*1024UL*1024UL));
  }
}
