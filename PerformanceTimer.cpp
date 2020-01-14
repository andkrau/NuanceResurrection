#include <windows.h>

_LARGE_INTEGER tickFrequency, counterStart, counterEnd, dummyCounter;

double GetTimeDeltaMs()
{
  QueryPerformanceFrequency(&tickFrequency);
  return (((double)(counterEnd.QuadPart - counterStart.QuadPart)) * 1000) / tickFrequency.QuadPart;
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
  QueryPerformanceCounter(&counterEnd);
  QueryPerformanceFrequency(&tickFrequency);
  return (((double)(counterEnd.QuadPart - counterStart.QuadPart)) * 1000) / tickFrequency.QuadPart;
}