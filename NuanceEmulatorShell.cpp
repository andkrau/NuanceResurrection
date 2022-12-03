#include "EmulatorShell.h"
#include "NuanceEmulatorShell.h"
#include "basetypes.h"
#include "NuonEnvironment.h"
#include "NuanceRegisterWindow.h"
#include "SuperBlock.h"
#include "comm.h"

#include <cstdio>
#include "qapplication.h"
#include <qcombobox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

extern NuonEnvironment *nuonEnv;
bool bProcessorThreadRunning = false;

void NuanceEmulatorShell::OnMPESelect(int)
{
  uint32 currentMPE;

  currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();
  registeredRegisterWindow->SetCurrentMPE(currentMPE);
  registeredRegisterWindow->UpdateDebugDisplay(currentMPE);
  UpdateDisassembly();
}

void NuanceEmulatorShell::OnRefreshRate(int)
{
  uint32 refreshRate;

  refreshRate = ddlbRefreshRate->currentText().left(2).toInt();

  nuonEnv->vblank_frequency = refreshRate;
  nuonEnv->cyclesPerVideoDisplay = 54000000/refreshRate;
  nuonEnv->videoDisplayCycleCount = nuonEnv->cyclesPerVideoDisplay;
  if(bProcessorThreadRunning)
  {
    registeredDisplay->killTimers();
    registeredDisplay->startTimer(1000.0/(refreshRate*10));
  }
}

void NuanceEmulatorShell::OnLoadFile()
{
  uint32 currentMPE;
  bool bSuccess;
  QFileDialog fileDialog(0,0,true);
  fileDialog.setCaption(QString("Load File"));
  
  fileDialog.show();
  QString fileName = fileDialog.selectedFile();
  
  currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();

  if(!fileName.isEmpty())
  {
    bSuccess = nuonEnv->mpe[currentMPE]->LoadNuonRomFile((char *)(fileName.data()));
    if(bSuccess)
    {
      QMessageBox::information(this,"Success","NUONROM-DISK file successfully loaded");
    }
    else
    {
      bSuccess = nuonEnv->mpe[currentMPE]->LoadCoffFile((char *)(fileName.data()));
      if(bSuccess)
      {
        QMessageBox::information(this,"Success","COFF file successfully loaded");
      }
      else
      {
        QMessageBox::warning(this,"Error","Invalid COFF or NUONROM-DISK file");
      }
    }
  }
  UpdateDisassembly();
}

extern void DoUpdate(void);

void NuanceEmulatorShell::OnRunMPE()
{
  uint32 bpAddr = 0;

  FILE *inFile;
  inFile = fopen("breakpoint.txt","r");

  if(inFile)
  {
    fscanf(inFile,"%x",&bpAddr);
    fclose(inFile);
  }

  nuonEnv->mpe[0]->breakpointAddress = bpAddr;
  nuonEnv->mpe[1]->breakpointAddress = bpAddr;
  nuonEnv->mpe[2]->breakpointAddress = bpAddr;
  nuonEnv->mpe[3]->breakpointAddress = bpAddr;

  nuonEnv->cycleCounter = 0;

  UpdateButtonStates(false);
  startTimer(1000.0/nuonEnv->vblank_frequency);
  processorThread->start();
  bProcessorThreadRunning = true;
}

void NuanceEmulatorShell::UpdateDisassembly()
{
  uint32 currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();
  char buffer[1024];

  sprintf(buffer,"pcexec = $%8.8lX",nuonEnv->mpe[currentMPE]->pcexec);
  ebStdOut->setText(QString(buffer));
  buffer[0] = '\0';
  nuonEnv->mpe[currentMPE]->PrintInstructionCachePacket(buffer,nuonEnv->mpe[currentMPE]->pcexec);
  ebStdOut->append(QString("{"));
  buffer[strlen(buffer) - 1] = '\0';
  ebStdOut->append(QString(buffer));
  ebStdOut->append(QString("}"));
}

void NuanceEmulatorShell::OnStopMPE()
{
  uint32 currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();

  nuonEnv->mpe[0]->mpectl &= ~MPECTRL_MPEGO;
  nuonEnv->mpe[1]->mpectl &= ~MPECTRL_MPEGO;
  nuonEnv->mpe[2]->mpectl &= ~MPECTRL_MPEGO;
  nuonEnv->mpe[3]->mpectl &= ~MPECTRL_MPEGO;
  UpdateDisassembly();
  nuonEnv->bProcessorStartStopChange = true;
  killTimers();
  bProcessorThreadRunning = false;
}

void NuanceEmulatorShell::OnSingleStepMPE()
{
  uint32 currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();
  UpdateButtonStates(false);
  nuonEnv->mpe[0]->ExecuteSingleCycle();
  nuonEnv->mpe[1]->ExecuteSingleCycle();
  nuonEnv->mpe[2]->ExecuteSingleCycle();
  nuonEnv->mpe[3]->ExecuteSingleCycle();
  DoCommBusController();
  UpdateButtonStates(true);
  registeredRegisterWindow->UpdateDebugDisplay(currentMPE);
}

void NuanceEmulatorShell::OnStartMPE()
{
  uint32 currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();
  nuonEnv->mpe[currentMPE]->mpectl |= MPECTRL_MPEGO;
  nuonEnv->bProcessorStartStopChange = true;
  registeredRegisterWindow->UpdateDebugDisplay(currentMPE);
}

void NuanceEmulatorShell::OnResetMPE()
{
  uint32 currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();
  nuonEnv->mpe[currentMPE]->Reset();
  registeredRegisterWindow->UpdateDebugDisplay(currentMPE);
  UpdateDisassembly();
}

void NuanceEmulatorShell::timerEvent(QTimerEvent *timerEvent)
{
  nuonEnv->bRenderVideo = true;
  registeredDisplay->UpdateDisplay();
}

void NuanceEmulatorShell::UpdateButtonStates(bool bState)
{
  uint32 currentMPE = ddlbMPESelect->currentText().mid(3,1).toInt();

  cbResetMPE->setDisabled(!bState);
  cbSingleStepMPE->setDisabled(!bState);
  cbRunMPE->setDisabled(!bState);
  cbStartMPE->setDisabled(!bState);
  cbLoadFile->setDisabled(!bState);
  registeredRegisterWindow->UpdateDebugDisplay(currentMPE);
  UpdateDisassembly();
}

void NuanceEmulatorShell::RegisterApplication(QApplication *app)
{
  registeredApp = app;
}

void NuanceEmulatorShell::RegisterDisplay(NuanceVideoDisplay *display)
{
  registeredDisplay = display;
}

void NuanceEmulatorShell::RegisterRegisterWindow(NuanceRegisterWindow *window)
{
  registeredRegisterWindow = window;
}