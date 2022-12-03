#include <cstdio>
#include "mpe.h"
#include "comm.h"
#include "NuonEnvironment.h"
#include "NuanceEmulatorShell.h"
#include "NuanceProcessorThread.h"
#include "video.h"

extern bool bUpdateVideo;
extern NuonEnvironment *nuonEnv;

void NuanceProcessorThread::Execute_3()
{
  bool bAnyRunning;

  do
  {
    bAnyRunning = nuonEnv->mpe[3]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;
  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_32()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[3]->ExecuteSingleCycle();
    nuonEnv->mpe[2]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[2]->mpectl & MPECTRL_MPEGO);

  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_31()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[3]->ExecuteSingleCycle();
    nuonEnv->mpe[1]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[1]->mpectl & MPECTRL_MPEGO);

  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_30()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[3]->ExecuteSingleCycle();
    nuonEnv->mpe[0]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    if(nuonEnv->cycleCounter >= (54000000/nuonEnv->vblank_frequency))
    {
      nuonEnv->cycleCounter -= (54000000/nuonEnv->vblank_frequency);
    }

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[0]->mpectl & MPECTRL_MPEGO);
  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_320()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[3]->ExecuteSingleCycle();
    nuonEnv->mpe[2]->ExecuteSingleCycle();
    nuonEnv->mpe[0]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[0]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[2]->mpectl & MPECTRL_MPEGO);
  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_321()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[3]->ExecuteSingleCycle();
    nuonEnv->mpe[2]->ExecuteSingleCycle();
    nuonEnv->mpe[1]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[2]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[1]->mpectl & MPECTRL_MPEGO);

  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_310()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[3]->ExecuteSingleCycle();
    nuonEnv->mpe[1]->ExecuteSingleCycle();
    nuonEnv->mpe[0]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[0]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[1]->mpectl & MPECTRL_MPEGO);

  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::Execute_All()
{
  bool bAnyRunning;

  do
  {
    nuonEnv->mpe[0]->ExecuteSingleCycle();
    nuonEnv->mpe[1]->ExecuteSingleCycle();
    nuonEnv->mpe[2]->ExecuteSingleCycle();
    nuonEnv->mpe[3]->ExecuteSingleCycle();

    if(nuonEnv->pendingCommRequests)
    {
      DoCommBusController();
    }

    nuonEnv->cycleCounter++;

    bAnyRunning =
      (nuonEnv->mpe[3]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[0]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[2]->mpectl & MPECTRL_MPEGO) ||
      (nuonEnv->mpe[1]->mpectl & MPECTRL_MPEGO);
  }
  while(bAnyRunning && !(nuonEnv->bProcessorStartStopChange));
}

void NuanceProcessorThread::run()
{
  uint32 whichFunction;

  nuonEnv->cycleCounter = 0;

  do
  {
    whichFunction = 0;
    nuonEnv->bProcessorStartStopChange = false;

    if(nuonEnv->mpe[0]->mpectl & MPECTRL_MPEGO)
    {
      whichFunction |= 0x1;
    }

    if(nuonEnv->mpe[1]->mpectl & MPECTRL_MPEGO)
    {
      whichFunction |= 0x2;
    }

    if(nuonEnv->mpe[2]->mpectl & MPECTRL_MPEGO)
    {
      whichFunction |= 0x4;
    }

    switch(whichFunction)
    {
      case 0:
        Execute_3();
        break;
      case 1:
        Execute_30();
        break;
      case 2:
        Execute_31();
        break;
      case 3:
        Execute_310();
        break;
      case 4:
        Execute_32();
        break;
      case 5:
        Execute_320();
        break;
      case 6:
        Execute_321();
        break;
      case 7:
        Execute_All();
        break;
    }
  }
  while(nuonEnv->bProcessorStartStopChange);

  if(registeredEmulatorShell)
  {
    registeredEmulatorShell->UpdateButtonStates(true);
  }
}
