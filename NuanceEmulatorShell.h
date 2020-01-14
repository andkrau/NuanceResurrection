#ifndef NUANCEEMULATORSHELL_H
#define NUANCEEMULATORSHELL_H

#include "EmulatorShell.h"
#include "QApplication.h"
#include "QObject.h"
#include "QCombobox.h"
#include "QPixmap.h"
#include "NuonEnvironment.h"
#include "NuanceVideoDisplay.h"
#include "NuanceRegisterWindow.h"
#include "NuanceProcessorThread.h"

extern NuonEnvironment *nuonEnv;

class NuanceEmulatorShell : public EmulatorShell
{
public:
  NuanceEmulatorShell() : EmulatorShell()
  {
    setIcon(QPixmap("nuance.bmp","BMP"));
    setWFlags(Qt::WidgetFlags::WStyle_DialogBorder);
    setMinimumWidth(699);
    setMinimumHeight(301);
    setMaximumWidth(699);
    setMaximumHeight(301);
    ddlbMPESelect->setCurrentItem(3);
    ddlbRefreshRate->setCurrentItem(3);
    processorThread = new NuanceProcessorThread;
    processorThread->RegisterEmulatorShell(this);
    nuonEnv->RegisterEditControls(ebStdOut, ebStdErr);
  }

  ~NuanceEmulatorShell()
  {
    if(processorThread)
    {
      delete processorThread;
    }
  }

  void RegisterApplication(QApplication *);
  void RegisterDisplay(NuanceVideoDisplay *);
  void RegisterRegisterWindow(NuanceRegisterWindow *);
  void UpdateDisassembly(void);
  void UpdateButtonStates(bool);
private:
  void OnMPESelect(int);
  void OnRefreshRate(int);
  void OnLoadFile();
  void OnStartMPE();
  void OnStopMPE();
  void OnRunMPE();
  void OnResetMPE();
  void OnSingleStepMPE();
  virtual void timerEvent(QTimerEvent *);
  QApplication *registeredApp;
  NuanceVideoDisplay *registeredDisplay;
  NuanceRegisterWindow *registeredRegisterWindow;
  NuanceProcessorThread *processorThread;
};

#endif