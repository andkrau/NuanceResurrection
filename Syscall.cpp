#include "NuonEnvironment.h"
#include "Bios.h"
#include "comm.h"

extern NuonEnvironment *nuonEnv;
extern uint32 media_mpe;
extern uint32 media_mpe_allocated;

static uint32 WaitForInterruptList[] = {0,0,0,0};

void Syscall_InterruptTriggered(MPE * const mpe)
{
  const uint32 mpeIndex = mpe->mpeIndex;
  const uint32 interruptMask = WaitForInterruptList[mpeIndex];

  if(interruptMask)
  {
    if(mpe->intsrc & (mpe->inten1 | (1 << mpe->inten2sel)) & interruptMask)
    {
      //enabled interrupt is waiting to be processed so wake up the processor
      mpe->mpectl |= MPECTRL_MPEGO;
      //no longer waiting for interrupts
      WaitForInterruptList[mpeIndex] = 0;
      //register the state change in case we want to optimize the outer dispatch loop
      nuonEnv->bProcessorStartStopChange = true;
    }
  }
}

void Syscall_WaitForInterruptCommon(const uint32 mpeIndex, const uint32 interruptMask)
{
  MPE * const mpe = nuonEnv->mpe[mpeIndex];
  if(interruptMask && !(mpe->intsrc & (mpe->inten1 | (1 << mpe->inten2sel)) & interruptMask))
  {
    //stop
    mpe->mpectl &= ~MPECTRL_MPEGO;
    //clear hw and sw masks
    mpe->intctl = 0;
    WaitForInterruptList[mpe->mpeIndex] = interruptMask;
      //register the state change in case we want to optimize the outer dispatch loop
    nuonEnv->bProcessorStartStopChange = true;
  }
}

void Syscall_WaitForCommPacket(MPE * const mpe)
{
  const uint32 mpeIndex = mpe->mpeIndex;
  uint32 queue_head;
  uint32 queue_tail;
  uint32 *memPtr;
  if(media_mpe_allocated && (mpeIndex == media_mpe))
  {
    if(mpeIndex == 0)
    {
      memPtr = (uint32 *)nuonEnv->GetPointerToMemory(mpe,MINIBIOS_QUEUE2_TAIL_ADDRESS,false);
    }
    else
    {
      memPtr = (uint32 *)nuonEnv->GetPointerToMemory(mpe,MINIBIOSX_QUEUE2_TAIL_ADDRESS,false);
    }

    if(*memPtr == *(memPtr + 1))
    {
      //If queue2 head equals queue2 tail then there are no user packets available so the processor should wait for interrupts
      Syscall_WaitForInterruptCommon(mpeIndex,mpe->inten1 | (1 << mpe->inten2sel));
    }
  }
  else if(mpeIndex == 3)
  {
    //MPE3 is running the BIOS so check the BIOS commrecv packet storage address
    memPtr = (uint32 *)nuonEnv->GetPointerToMemory(mpe,COMMRECV_PACKET_AVAILABLE_ADDRESS,false);
    if(!*memPtr)
    {
      //No user comm packet is available so wait for interrupts
      Syscall_WaitForInterruptCommon(mpeIndex,mpe->inten1 | (1 << mpe->inten2sel));
    }
  }
  else if(!(mpe->commctl & COMM_RECV_BUFFER_FULL_BIT))
  {
    //The commrecv buffer is empty so there is obviously no available packet and we should wait for interrupts
    Syscall_WaitForInterruptCommon(mpeIndex,mpe->inten1 | (1 << mpe->inten2sel));
  }
}

void Syscall_WaitForInterrupt(MPE * const mpe)
{
  const uint32 interruptMask = mpe->regs[0];
  const uint32 mpeIndex = mpe->mpeIndex;

  Syscall_WaitForInterruptCommon(mpeIndex, interruptMask);
}

void Syscall_ClearWaitForInterrupt(MPE * const mpe)
{
  WaitForInterruptList[mpe->mpeIndex] = 0;
}

void ExecuteSyscall_Interrupts(MPE * const mpe, const uint32 function)
{
  const uint32 mpeIndex = mpe->mpeIndex;

  switch(function)
  {
    case 0:
      Syscall_WaitForInterrupt(mpe);
      break;
    case 1:
      Syscall_WaitForCommPacket(mpe);
      break;
  }
}

void ExecuteSyscall(MPE * const mpe, const uint32 syscall)
{
  const uint32 category = syscall >> 24;
  const uint32 function = syscall & 0xFFFFFFUL;

  //Category 0: Compiler hints
    //1: Set invariant region
    //2: Clear invariant region
  //Category 1: Interrupts
    //0: WaitForInterrupt
    //1: WaitForCommPacket
  switch(category)
  {
    case 0:
      break;
    case 1:
      ExecuteSyscall_Interrupts(mpe, function);
      break;   
  }
}
