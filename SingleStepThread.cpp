//---------------------------------------------------------------------------
#include "SingleStepThread.h"
#include "mpe.h"
#include "comm.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment *nuonEnv;
extern bool bUpdateVideo;
bool bSingleStepThreadExecuting;

//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Unit1::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
void SingleStepThread::Execute()
{
  bSingleStepThreadExecuting = true;
  nuonEnv->mpe[0]->ExecuteSingleCycle();
  nuonEnv->mpe[1]->ExecuteSingleCycle();
  nuonEnv->mpe[2]->ExecuteSingleCycle();
  nuonEnv->mpe[3]->ExecuteSingleCycle();

  DoCommBusController();
  bSingleStepThreadExecuting = false;
}
//---------------------------------------------------------------------------
