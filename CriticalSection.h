#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include <windows.h>

class CriticalSection
{
  public:
  CriticalSection::CriticalSection();
  CriticalSection::~CriticalSection();
  void CriticalSection::Enter()
  {
    EnterCriticalSection(&criticalSection);
  }

  void CriticalSection::Leave()
  {
    LeaveCriticalSection(&criticalSection);
  }

  private:
  CRITICAL_SECTION criticalSection;
};

#endif