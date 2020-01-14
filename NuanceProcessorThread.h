//---------------------------------------------------------------------------

#ifndef NuanceProcessorThreadH
#define NuanceProcessorThreadH

#include "QThread.h"

class NuanceEmulatorShell;
//---------------------------------------------------------------------------
class NuanceProcessorThread : public QThread
{
private:
  void Execute_3();
  void Execute_32();
  void Execute_31();
  void Execute_30();
  void Execute_320();
  void Execute_321();
  void Execute_310();
  void Execute_All();
  NuanceEmulatorShell *registeredEmulatorShell;
public:
  virtual void run();
  void RegisterEmulatorShell(NuanceEmulatorShell *shell)
  {
    registeredEmulatorShell = shell;
  }
};
//---------------------------------------------------------------------------
#endif
