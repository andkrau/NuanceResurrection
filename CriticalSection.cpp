#include <windows.h>
#include "CriticalSection.h"

CriticalSection::CriticalSection()
{
  InitializeCriticalSection(&criticalSection);
}

CriticalSection::~CriticalSection()
{
  DeleteCriticalSection(&criticalSection);
}

