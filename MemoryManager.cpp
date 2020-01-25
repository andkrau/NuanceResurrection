#include "basetypes.h"
#include <stdio.h>
#include "MemoryManager.h"

MemoryManager::MemoryManager(uint32 maxBytes, uint32 defaultAlignment)
{
  this->defaultAlignment = defaultAlignment;
  this->maxBytes = maxBytes;
}

bool MemoryManager::TestForInvalidPowerOfTwo(uint32 requestedAlignment)
{
  return (requestedAlignment & (requestedAlignment - 1)) != 0;
}

void MemoryManager::Add(uint32 startAddress, uint32 endAddress, uint32 index)
{
  FreeMemoryBlock newFreeBlock;
  uint32 vectorSize;

  newFreeBlock.startAddress = startAddress;
  newFreeBlock.endAddress = endAddress;
  newFreeBlock.numBytes = endAddress - startAddress + 1;

  vectorSize = (uint32)freeBlockVector.size();

  if(vectorSize == 0)
  {
    //Only block in list so just add it
    freeBlockVector.insert(freeBlockVector.begin(),newFreeBlock);
    return;
  }

  if((index == 0) || (index >= vectorSize))
  {
    //Dont know where to put it
    for(index = 0; index < vectorSize; index++)
    {
      if(freeBlockVector[index].startAddress > startAddress)
      {
        break;
      }
    }
  }

  if(index == vectorSize)
  {
    //Add free block to end of list: check to see if new entry can
    //be combined with entry at end of list
    if(startAddress != (freeBlockVector.back().endAddress + 1))
    {
      //New block cannot be combined with the block currently at the end of the list so 
      //just add the new block to the end of the list
      freeBlockVector.push_back(newFreeBlock);
    }
    else
    {
      //Combine new block with existing block entry at end of list
      freeBlockVector.back().numBytes += newFreeBlock.numBytes;
      freeBlockVector.back().endAddress += newFreeBlock.numBytes;
    }
  }
  else
  {
    //An entry exists after insertion point so try to combine ([new][old])
    if(endAddress != (freeBlockVector[index].startAddress - 1))
    {
      //The existing block is not contiguous with the new block so no
      //combining can be done
      freeBlockVector.insert(freeBlockVector.begin() + index, newFreeBlock);
    }
    else
    {
      //Another free block exists at the index at which we are inserting
      //and the existing block starts at the memory address immediately
      //following the end address of the new free block so they can be
      //combined

      //Add free block after the provided index and combine with the free block
      //that currently follows the block at the provided index
      freeBlockVector[index].startAddress = newFreeBlock.startAddress;
      freeBlockVector[index].numBytes += newFreeBlock.numBytes;
    }

    //check to see if an entry exists to the left of the insertion point
    if(index != 0)
    {
      //An entry exists so try to combine the entry just added with the
      //entry immediately to the left of the insertion point: ([old][new_merged])
      if(freeBlockVector[index - 1].endAddress == (freeBlockVector[index].startAddress - 1))
      {
        //Combine previous entry (index-1) with newly inserted entry (index)
        freeBlockVector[index - 1].endAddress = freeBlockVector[index].endAddress;
        freeBlockVector[index - 1].numBytes += freeBlockVector[index].numBytes;
        freeBlockVector.erase(freeBlockVector.begin() + index);
      }
    }
  }
}

void MemoryManager::AddAllocatedBlock(uint32 startAddress, uint32 numBytes)
{
  AllocatedMemoryBlock newAllocatedBlock;
  uint32 vectorSize;
  uint32 index;

  newAllocatedBlock.startAddress = startAddress;
  newAllocatedBlock.numBytes = numBytes;

  vectorSize = (uint32)allocatedBlockVector.size();

  for(index = 0; index < vectorSize; index++)
  {
    if(allocatedBlockVector[index].startAddress > startAddress)
    {
      break;
    }
  }

  allocatedBlockVector.insert(allocatedBlockVector.begin() + index, newAllocatedBlock);
}

void MemoryManager::Free(uint32 startAddress)
{
  FreeMemoryBlock newFreeBlock;
  uint32 vectorSize, index;

  vectorSize = (uint32)allocatedBlockVector.size();

  for(index = 0; index < vectorSize; index++)
  {
    if(allocatedBlockVector[index].startAddress == startAddress)
    {
      newFreeBlock.startAddress = allocatedBlockVector[index].startAddress;
      newFreeBlock.numBytes = allocatedBlockVector[index].numBytes;
      newFreeBlock.endAddress = newFreeBlock.startAddress + newFreeBlock.numBytes - 1;
      //Remove the allocated block structure from the allocated block list
      allocatedBlockVector.erase(allocatedBlockVector.begin() + index);
      //Add a new memory block item into the free block list (specify index equal to size
      //to force search for proper insertion point)
      Add(newFreeBlock.startAddress,newFreeBlock.endAddress,(uint32)freeBlockVector.size());
      return;
    }
  }
}

void *MemoryManager::Allocate(uint32 requestedBytes, uint32 requestedAlignment)
{
  FreeMemoryBlock fmb2;
  uint32 index, adjustedAddress;
  uint32 vectorSize;

  if(requestedBytes == 0)
  {
    requestedBytes = 1;
  }

  //force vector alignment minimum
  if(requestedAlignment < 16)
  {
    requestedAlignment = 16;
  }

  //stricter alignments must be powers of two
  if(TestForInvalidPowerOfTwo(requestedAlignment))
  {
    return NULL;
  }

  //pad requested byte count to the nearest vector
  requestedBytes = (requestedBytes + 15) & 0xFFFFFFF0UL;

  vectorSize = (uint32)freeBlockVector.size();

  if(vectorSize == 0)
  {
    return NULL;
  }

  for(index = 0; index < vectorSize; index++)
  {
    if(freeBlockVector[index].numBytes >= requestedBytes)
    {
      //Find the first address greater or equal to the block starting address
      //that is aligned to the number of bytes specified by requestedAlignment
      adjustedAddress = AlignAddress(freeBlockVector[index].startAddress, requestedAlignment);
      if(adjustedAddress <= freeBlockVector[index].endAddress)
      {
        if((freeBlockVector[index].endAddress - adjustedAddress + 1) >= requestedBytes)
        {
          //Block candidate meets all requirements
          AddAllocatedBlock(adjustedAddress, requestedBytes);

          if(requestedBytes == freeBlockVector[index].numBytes)
          {
            //Entire block is used so delete the block from the Free Block list
            freeBlockVector.erase(freeBlockVector.begin() + index);
          }
          else
          {
            if(adjustedAddress == freeBlockVector[index].startAddress)
            {
              //Allocated block begins right at start_address
              freeBlockVector[index].startAddress = adjustedAddress + requestedBytes;
              freeBlockVector[index].numBytes -= requestedBytes;
              //End address of new free block stays the same
            }
            else if((adjustedAddress + requestedBytes - 1) == freeBlockVector[index].endAddress)
            {
              //Allocated block ends right at end_address
              //Start address of new free block stays the same
              freeBlockVector[index].numBytes -= requestedBytes;
              freeBlockVector[index].endAddress = adjustedAddress - 1;
            }
            else
            {
              //Allocated block splits free block in two
              fmb2.endAddress = freeBlockVector[index].endAddress;
              //NewFreeBlock1: pFB1.start_address stays the same
              freeBlockVector[index].endAddress = adjustedAddress - 1;
              freeBlockVector[index].numBytes = 
                freeBlockVector[index].endAddress - freeBlockVector[index].startAddress + 1;
              //NewFreeBlock2: pFB2.end_address stays the same
              fmb2.startAddress = adjustedAddress + requestedBytes;
              fmb2.numBytes = fmb2.endAddress - fmb2.startAddress + 1;
              //Insert pFB2 into ordered Free Blocks list
              if((index + 1) != vectorSize)
              {
                Add(fmb2.startAddress,fmb2.endAddress,index+1);
              }
              else
              {
                freeBlockVector.push_back(fmb2);
              }
            }
          }

          return (void *)adjustedAddress;
        }
        //Not enough bytes in block after aligning the starting address
      }
      //Aligned starting address is outside of block candidate
    }
    //Trivial rejection of block candidates that do not have enough bytes
    //to meet the request under any circumstance
  }
  //No block was able to meet the requirements
  return NULL;
}

inline uint32 MemoryManager::AlignAddress(uint32 address, uint32 alignment)
{
  return (address + alignment - 1) & (~(alignment - 1));
}

/*
void WriteFreeBlocks(FILE *f, TList *t)
{
  FreeBlockStruct *pFB;

  fprintf(f,"Free Blocks\n==========\n");
  for(long i = 0; i < t->Count; i++)
  {
    pFB = (FreeBlockStruct *)(t->Items[i]);
    fprintf(f,"[start: %x, end: %x, length %x]\n",pFB->start_address,
      pFB->end_address, pFB->num_bytes);
  }
  fprintf(f,"==========\n");
}

void WriteAllocatedBlocks(FILE *f, TList *t)
{
  AllocatedBlockStruct *pAB;

  fprintf(f,"Allocated Blocks\n==========\n");
  for(long i = 0; i < t->Count; i++)
  {
    pAB = (AllocatedBlockStruct *)(t->Items[i]);
    fprintf(f,"[start: %x, length %x]\n",pAB->start_address,
      pAB->num_bytes);
  }
  fprintf(f,"==========\n");
}

void WriteMemoryBlocks(FILE *f)
{
  WriteFreeBlocks(f,MainBusFreeBlocks);
  WriteAllocatedBlocks(f,MainBusAllocatedBlocks);
  WriteFreeBlocks(f,OtherBusFreeBlocks);
  WriteAllocatedBlocks(f,OtherBusAllocatedBlocks);
}
void MemInit(MPE *mpe)
{
  long i, b;

  FreeBlockStruct *pFB, *pFB2;
  AllocatedBlockStruct *pAB;

  FreeListEntryMemory(OtherBusFreeBlocks);
  FreeListEntryMemory(OtherBusAllocatedBlocks);
  FreeListEntryMemory(MainBusFreeBlocks);
  FreeListEntryMemory(MainBusAllocatedBlocks);

  OtherBusFreeBlocks->Clear();
  OtherBusAllocatedBlocks->Clear();
  MainBusFreeBlocks->Clear();
  MainBusAllocatedBlocks->Clear();

  pFB = new FreeBlockStruct;
  pFB->start_address = MAIN_BUS_BASE;
  pFB->num_bytes = MAIN_BUS_SIZE;
  pFB->end_address = MAIN_BUS_BASE + pFB->num_bytes - 1;
  MainBusFreeBlocks->Add(pFB);

  pFB = new FreeBlockStruct;
  pFB->start_address = SYSTEM_BUS_BASE + (64 * 1024);
  pFB->end_address = (BIOS_FUNCTIONS_BASE - 1);
  pFB->num_bytes = pFB->end_address - pFB->start_address + 1;

  OtherBusFreeBlocks->Add(pFB);

  //Insert the 64K block of System Bus kernel memory into the allocated block list
  AllocateMemoryBlock(SYSTEM_BUS_BASE, (64 * 1024), OtherBusAllocatedBlocks);
  //Insert the 640K block of System Bus kernel memory into the allocated block list
  AllocateMemoryBlock(BIOS_FUNCTIONS_BASE, BIOS_FUNCTIONS_SIZE, OtherBusAllocatedBlocks);

  if(SYSTEM_BUS_SIZE > (8 * 1024 * 1024))
  {
    //Main Bus DRAM is greater than 8 megs
    pFB2 = new FreeBlockStruct;
    pFB2->start_address = BIOS_FUNCTIONS_BASE + BIOS_FUNCTIONS_SIZE;
    pFB2->num_bytes = SYSTEM_BUS_SIZE - (8 * 1024 * 1024);
    pFB2->end_address = pFB2->start_address + pFB2->num_bytes - 1;
    OtherBusFreeBlocks->Add(pFB2);
  }

// MEMORY MANAGEMENT TEST BEGIN
  FILE *f = NULL; //fopen("e:\\memory.txt","w");
  long ptr1, ptr2, ptr3;

  if(f)
  {
    if(!mpe)
    {
      mpe = nuonEnv->mpe[3];
    }

    fprintf(f,"Initial memory configuration\n");
    WriteMemoryBlocks(f);

    fprintf(f,"Requesting 256 bytes of Main Bus RAM (align 256)\n");
    mpe->regs.scalarRegs[0] = 256; //bytes
    mpe->regs.scalarRegs[1] = 256; //alignment
    mpe->regs.scalarRegs[2] = kMemSDRAM; //memory bank
    MemAlloc(mpe);
    ptr1 = mpe->regs.scalarRegs[0];

    WriteMemoryBlocks(f);

    fprintf(f,"Requesting 256 bytes of Main Bus RAM (align 512)\n");
    mpe->regs.scalarRegs[0] = 256; //bytes
    mpe->regs.scalarRegs[1] = 512; //alignment
    mpe->regs.scalarRegs[2] = kMemSDRAM; //memory bank
    MemAlloc(mpe);
    ptr2 = mpe->regs.scalarRegs[0];

    WriteMemoryBlocks(f);

    fprintf(f,"Requesting 256 bytes of Main Bus RAM (align 2)\n");
    mpe->regs.scalarRegs[0] = 256; //bytes
    mpe->regs.scalarRegs[1] = 256; //alignment
    mpe->regs.scalarRegs[2] = kMemSDRAM; //memory bank
    MemAlloc(mpe);
    ptr3 = mpe->regs.scalarRegs[0];

    WriteMemoryBlocks(f);

    fprintf(f,"Freeing first block of Main Bus RAM\n");
    mpe->regs.scalarRegs[0] = ptr1; //pointer to memory block
    MemFree(mpe);

    WriteMemoryBlocks(f);

    fprintf(f,"Freeing second block of Main Bus RAM\n");
    mpe->regs.scalarRegs[0] = ptr2; //pointer to memory block
    MemFree(mpe);

    WriteMemoryBlocks(f);

    fprintf(f,"Freeing third block of Main Bus RAM\n");
    mpe->regs.scalarRegs[0] = ptr3; //pointer to memory block
    MemFree(mpe);

    WriteMemoryBlocks(f);

    fclose(f);
  }
// MEMORY MANAGEMENT TEST END
}

*/




