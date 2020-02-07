#include "basetypes.h"
#include <windows.h>

_LARGE_INTEGER tickFrequency, counterStart, counterEnd, dummyCounter;

double GetTimeDeltaMs()
{
  //QueryPerformanceFrequency(&tickFrequency);
  return (double)((counterEnd.QuadPart - counterStart.QuadPart) * 1000) / (double)tickFrequency.QuadPart;
}

void StartPerformanceTimer()
{
  QueryPerformanceCounter(&counterStart);
}

void StopPerformanceTimer()
{
  QueryPerformanceCounter(&counterEnd);
}

double GetPerformanceTimerOverheadMs()
{
  QueryPerformanceCounter(&counterStart);
  QueryPerformanceCounter(&dummyCounter);
  //QueryPerformanceFrequency(&tickFrequency);
  QueryPerformanceCounter(&counterEnd);
  return (double)((counterEnd.QuadPart - counterStart.QuadPart) * 1000) / (double)tickFrequency.QuadPart;
}
