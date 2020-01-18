#include "basetypes.h"
#include "Bios.h"
#include "comm.h"
#include "mpe.h"
#include "mpe_alloc.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"

extern NuonEnvironment nuonEnv;
extern uint32 media_mpe_allocated;
extern uint32 media_mpe;

const uint32 mpeFlags_init[4] = {
(MPE_HAS_CACHES | MPE_IRAM_8K | MPE_DTRAM_8K), //MPE0
0, //MPE1
0, //MPE2
MPE_HAS_CACHES //MPE3
};

uint32 mpeFlags[4];

void ResetMPEFlags(MPE &mpe)
{
  for(uint32 i = 0; i < 4; i++)
    mpeFlags[i] = mpeFlags_init[i];

  mpeFlags[mpe.mpeIndex] |= (MPE_ALLOC_BIOS|MPE_ALLOC_USER);
}

void MPEAlloc(MPE &mpe)
{
  const uint32 requestedFlags = mpe.regs[0];
  
  mpe.regs[0] = 0xFFFFFFFF; //-1

  for(uint32 i = 0; i < 3; i++)
  {
    if(!(mpeFlags[i] & MPE_ALLOC_USER))
    {
      //The MPE is not allocated by the USER so it is available
      if((mpeFlags[i] & ~(MPE_ALLOC_USER|MPE_ALLOC_BIOS)) == (requestedFlags & ~(MPE_ALLOC_USER|MPE_ALLOC_BIOS)))
      {
        //The MPE matched all requested flags
        const bool bRequestedMiniBIOS = requestedFlags & MPE_HAS_MINI_BIOS;
        if(!bRequestedMiniBIOS && (mpeFlags[i] & MPE_HAS_MINI_BIOS))
        {          
          //MINIBIOS was not explicitly requested and this MPE
          //has MINIBIOS so don't allocate it
          continue;
        }

        if(mpeFlags_init[i] != (requestedFlags & ~(MPE_ALLOC_USER|MPE_ALLOC_BIOS)))
        {
          //The requested flags don't match the initial flags so don't allocate it
          //This is something the VMLabs BIOS checks but probably isnt necessary
          continue;
        }

        mpeFlags[i] |= MPE_ALLOC_USER;
        mpe.regs[0] = i;
        return;
      }
    }
  }
}

void MPEAllocSpecific(MPE &mpe)
{
  if(mpe.regs[0] < 3)
  {
    if(mpeFlags[mpe.regs[0]] & MPE_ALLOC_ANY)
    {
      //MPE already allocated
      mpe.regs[0] = 0xFFFFFFFF; //-1
    }
    else
    {
      //Mark MPE as allocated
      mpeFlags[mpe.regs[0]] |= MPE_ALLOC_USER;
    }
  }
  else
  {
    //Invalid MPE number, return already allocated
    mpe.regs[0] = 0xFFFFFFFF; //-1
  }
}

void MPEFree(MPE &mpe)
{
  if(mpe.regs[0] < 3)
  {
    if(mpeFlags[mpe.regs[0]] & MPE_ALLOC_USER)
    {
      //MPE allocated by user: mark as free for user allocation
      mpeFlags[mpe.regs[0]] &= ~MPE_ALLOC_USER;
      mpe.regs[0] = 0;
    }
  }
  else
  {
    //Invalid MPE number, return already free
    mpe.regs[0] = 0xFFFFFFFF; //-1
  }
}

void MPEStatus(MPE &mpe)
{
  if(mpe.regs[0] < 4)
  {
    mpe.regs[0] = mpeFlags[mpe.regs[0]];
  }
  else
  {
    mpe.regs[0] = 0;
  }
}

void MPEsAvailable(MPE &mpe)
{
  if(mpe.regs[0] == 1)
  {
    //Return number of MPEs in system
    mpe.regs[0] = 4;
  }
  else
  {
    mpe.regs[0] = 0;

    for(int i = 0; i < 3; i++)
    {
      if(!(mpeFlags[i] & MPE_ALLOC_ANY))
      {
        mpe.regs[0]++;
      }
    }
  }
}

void MPERun(MPE &mpe)
{
  const uint32 which = mpe.regs[0];
  const uint32 entrypoint = mpe.regs[1];

  /*
  The following behavior differs from the real BIOS particularly in that
  if the target MPE is allocated the minibios, MPERun should use CommSendInfo
  to send the target MPE a comm packet with the first scalar set to the entry point
  address and comminfo set to $C2.  CommSendInfo does not return until the packet is
  sent, so this behavior requires a native assembly implementation.  This code simply
  duplicates the C2 handler logic, which is normally triggered by a software interrupt
  injected by the level2 handler.  The level1 handler gets called and eventually handles
  the queued C2 packet.  The C2 handler sets rzi1 to the entry point so that it gets called
  when the level1 handler exits
  */

  if(media_mpe_allocated && (which == media_mpe))
  {
    //nuonEnv.mpe[which].rz = nuonEnv.mpe[which].pcexec;
    //nuonEnv.mpe[which].pcexec = entrypoint;
    //nuonEnv.mpe[which].ecuSkipCounter = 0;
    //nuonEnv.mpe[which].excephalten = 0xFFFFFFFE;
    //nuonEnv.mpe[which].sp = 0x20102000;
    //Invalidate cached instruction packets
    //nuonEnv.mpe[which].InvalidateICache();
    nuonEnv.mpe[which].UpdateInvalidateRegion(MPE_IRAM_BASE, OVERLAY_SIZE);
    nuonEnv.mpe[which].Go();
    nuonEnv.bProcessorStartStopChange = true;
    //Let MPERunMediaMPE set a comm packet to the media MPE to start it
    mpe.regs[2] = mpe.rz;
    mpe.rz = MPERUNMEDIAMPE_ADDRESS;
    mpe.ecuSkipCounter = 0;
  }
  else
  {
    nuonEnv.mpe[which].Halt();
    //Set up entry point
    nuonEnv.mpe[which].pcexec = entrypoint;
    //set return address to zero, per vmlabs implementation (MML3D uses this to halt after the pipeline finishes)
    nuonEnv.mpe[which].rz = 0;
    nuonEnv.mpe[which].ecuSkipCounter = 0;
    //Clear exceptions 
    nuonEnv.mpe[which].excepsrc = 0;
    //Mask level1 and level2 interupts
    nuonEnv.mpe[which].intctl = 0x88;
    nuonEnv.mpe[which].sp = 0x20101000;
    //Invalidate cached instruction packets
    //nuonEnv.mpe[which].InvalidateICache();
    //nuonEnv.mpe[which].nativeCodeCache->Flush();
    nuonEnv.mpe[which].UpdateInvalidateRegion(MPE_IRAM_BASE, OVERLAY_SIZE);
    //Sets mpego bit
    nuonEnv.mpe[which].Go();
    nuonEnv.bProcessorStartStopChange = true;
  }
}

void MPERunThread(MPE &mpe)
{
  const uint32 which = mpe.regs[0];
  const uint32 funcptr = mpe.regs[1];
  const uint32 arg = mpe.regs[2];
        uint32 stacktop = mpe.regs[3];

  //assume failure
  mpe.regs[0] = 0;

  //Official implementation requires that the MPE is not allocated by BIOS and 
  //that the MPE has both icache and dcache
  if((mpeFlags[which] & (MPE_ALLOC_BIOS | MPE_HAS_CACHES)) == MPE_HAS_CACHES)
  {
    //Official implementation simply modifies stacktop to be vector aligned
    stacktop &= 0xFFFFFFF0;
    nuonEnv.mpe[which].ecuSkipCounter = 0;

    nuonEnv.mpe[which].Halt();
    //Invalidate cached instruction packets
    //nuonEnv.mpe[which].InvalidateICache();
    //nuonEnv.mpe[which].nativeCodeCache->Flush();
    nuonEnv.mpe[which].UpdateInvalidateRegion(MPE_IRAM_BASE, OVERLAY_SIZE);
    nuonEnv.mpe[which].rz = MPE_THREAD_RETURN_ADDRESS;
    //Set up entry point
    nuonEnv.mpe[which].pcexec = funcptr;
    nuonEnv.mpe[which].ecuSkipCounter = 0;
    //Set up argument using C calling convention (first arg is r0)
    nuonEnv.mpe[which].regs[0] = arg;
    //Set up C stack pointer (r31)
    nuonEnv.mpe[which].regs[31] = stacktop;
  
    //** Stuff done by MPERun
    //Clear exceptions 
    nuonEnv.mpe[which].excepsrc = 0;
    //Mask level1 and level2 interupts
    nuonEnv.mpe[which].intctl = 0x88;

    //** Stuff done by MPERunThread bootcode

    //Clear inten1 
    nuonEnv.mpe[which].inten1 = 0;
    //Clear interrupts 
    nuonEnv.mpe[which].intsrc = 0;
    //Clear dtags
    //Clear itags
    //Clear acshift
    nuonEnv.mpe[which].acshift = 0;

    //Sets mpego bit
    nuonEnv.mpe[which].Go();
    //Return 1 to indicate success;
    mpe.regs[0] = 1;
    nuonEnv.bProcessorStartStopChange = true;
  }
}

void MPEStop(MPE &mpe)
{
  if(mpe.regs[0] < 4)
  {
    nuonEnv.mpe[mpe.regs[0]].mpectl &= ~MPECTRL_MPEGO;
    nuonEnv.bProcessorStartStopChange = true;
  }
}

void MPELoad(MPE &mpe)
{
  const uint32 which = mpe.regs[0];
  const uint32 mpeaddr = mpe.regs[1];
  const uint32 linkaddr = mpe.regs[2];
  const uint32 size = mpe.regs[3];
  uint8 *mpeMemPtr, *systemMemPtr;

  if(which < 4)
  {
    if((size + (mpeaddr & MPE_VALID_MEMORY_MASK) - 1) <= MPE_VALID_MEMORY_MASK)
    {
      if(linkaddr < SYSTEM_BUS_BASE)
      {
        systemMemPtr = &(nuonEnv.mainBusDRAM[linkaddr & MAIN_BUS_VALID_MEMORY_MASK]);
      }
      else
      {
        systemMemPtr = &(nuonEnv.systemBusDRAM[linkaddr & SYSTEM_BUS_VALID_MEMORY_MASK]);
      }

      mpeMemPtr = &(((uint8 *)(nuonEnv.mpe[which].GetPointerToMemory()))[mpeaddr & MPE_VALID_MEMORY_MASK]);

      for(uint32 count = 0; count < size; count++)
      {
        *mpeMemPtr = *systemMemPtr;

        mpeMemPtr++;
        systemMemPtr++;
      }

      //nuonEnv.mpe[which].InvalidateICacheRegion(mpeaddr, mpeaddr + size - 1);
      //nuonEnv.mpe[which].InvalidateICache();
      //nuonEnv.mpe[which].nativeCodeCache->FlushRegion(mpeaddr, mpeaddr + size - 1);
      nuonEnv.mpe[which].UpdateInvalidateRegion(mpeaddr, size - 1);
      //nuonEnv.mpe[which].nativeCodeCache->FlushRegion(0x20300000, mpeaddr + size - 1);
    }
  }
}

void MPEReadRegister(MPE &mpe)
{
  const uint32 which = mpe.regs[0];
  const uint32 mpeaddr = mpe.regs[1];

  InstructionCacheEntry entry;
  entry.pScalarRegs = mpe.regs;
  entry.pIndexRegs = &mpe.rx;
  entry.pCounterRegs = &mpe.rc0;
  entry.pRzRegs = &mpe.rz;
  entry.pXyctl = &mpe.xyctl;
  entry.pUvctl = &mpe.uvctl;
  entry.pXyrange = &mpe.xyrange;
  entry.pUvrange = &mpe.uvrange;
  entry.pAcshift = &mpe.acshift;
  entry.pSvshift = &mpe.svshift;

  if((which < 4) && (mpeaddr >= MPE_CTRL_BASE) && (mpeaddr < MPE1_ADDR_BASE))
  {
    //Make sure that the temporary scalar and index registers are in sync
    //with the standard registers so that r0-r31, rx, ry, ru, rv and cc will
    //be read correctly.  Doing this means that MPEReadRegister is no longer
    //safe to use on a running MPE unless done so in between cycle emulation
    //on the target processor (this will always be the case when a single
    //thread handles emulation of all four processors)

    nuonEnv.mpe[which].SaveRegisters();

    mpe.regs[0] = nuonEnv.mpe[which].ReadControlRegister(mpeaddr - MPE_CTRL_BASE, &entry);
  }
}

void MPEWriteRegister(MPE&mpe)
{
  const uint32 which = mpe.regs[0];
  const uint32 mpeaddr = mpe.regs[1];
  const uint32 value = mpe.regs[2];

  if((which < 4) && (mpeaddr >= MPE_CTRL_BASE) && (mpeaddr < MPE1_ADDR_BASE))
  {
    nuonEnv.mpe[which].WriteControlRegister(mpeaddr - MPE_CTRL_BASE,value);
  }
}
