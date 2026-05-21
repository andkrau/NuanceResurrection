#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class CriticalSection final
{
public:
  CriticalSection()
  {
    InitializeCriticalSection(&criticalSection);
  }

  ~CriticalSection()
  {
    DeleteCriticalSection(&criticalSection);
  }

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
