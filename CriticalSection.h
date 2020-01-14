#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include <windows.h>

class CriticalSection
{
public:
  CriticalSection();
  ~CriticalSection();

  void Enter()
  {
    EnterCriticalSection(&criticalSection);
  }

  void Leave()
  {
    LeaveCriticalSection(&criticalSection);
  }

private:
  CRITICAL_SECTION criticalSection;
};

#endif