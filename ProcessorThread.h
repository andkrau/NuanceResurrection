//---------------------------------------------------------------------------

#ifndef NuanceProcessorThreadH
#define NuanceProcessorThreadH

#include <QThread.h>
//---------------------------------------------------------------------------
class NuanceProcessorThread : public QThread
{
public:
  virtual void run();
private:
  void NuanceProcessorThread::Execute_3();
  void NuanceProcessorThread::Execute_32();
  void NuanceProcessorThread::Execute_31();
  void NuanceProcessorThread::Execute_30();
  void NuanceProcessorThread::Execute_320();
  void NuanceProcessorThread::Execute_321();
  void NuanceProcessorThread::Execute_310();
  void NuanceProcessorThread::Execute_All();
};
//---------------------------------------------------------------------------
#endif
