#include <windows.h>
#include <time.h>
#include "basetypes.h"
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "timer.h"
#include "mpe.h"

extern NuonEnvironment *nuonEnv;

bool bHighPerformanceTimerAvailable = false;
extern _LARGE_INTEGER tickFrequency;
uint64 tickCount;
_LARGE_INTEGER ticksAtBootTime;
bool bSysTimer0Initialized  = false;
bool bSysTimer1Initialized  = false;
bool bSysTimer2Initialized = false;
uint32 hSysTimer0;
uint32 hSysTimer1;
uint32 hSysTimer2;

void CALLBACK SysTimer0Callback(uint32 wTimerID, uint32 msg, int32 dwUser, int32 dw1, int32 dw2) 
{ 
  nuonEnv->mpe[0]->TriggerInterrupt(INT_SYSTIMER0);
  nuonEnv->mpe[1]->TriggerInterrupt(INT_SYSTIMER0);
  nuonEnv->mpe[2]->TriggerInterrupt(INT_SYSTIMER0);
  nuonEnv->mpe[3]->TriggerInterrupt(INT_SYSTIMER0);
} 

void CALLBACK SysTimer1Callback(uint32 wTimerID, uint32 msg, int32 dwUser, int32 dw1, int32 dw2) 
{ 
  nuonEnv->mpe[0]->TriggerInterrupt(INT_SYSTIMER1);
  nuonEnv->mpe[1]->TriggerInterrupt(INT_SYSTIMER1);
  nuonEnv->mpe[2]->TriggerInterrupt(INT_SYSTIMER1);
  nuonEnv->mpe[3]->TriggerInterrupt(INT_SYSTIMER1);
}

void CALLBACK SysTimer2Callback(uint32 wTimerID, uint32 msg, int32 dwUser, int32 dw1, int32 dw2) 
{ 
  static uint64 cycleCounter[4] = {0,0,0,0};
  static uint64 max_delta[4] = {0,0,0,0};

//  for(uint32 i = 0; i < 4; i++)
//  {
//    uint64 delta = nuonEnv->mpe[i]->cycleCounter - cycleCounter[i];
//    if(delta > max_delta[i])
//    {
//      max_delta[i] = delta;
//    }
//    cycleCounter[i] = nuonEnv->mpe[i]->cycleCounter;
//  }
  IncrementVideoFieldCounter();
  nuonEnv->TriggerVideoInterrupt();
}

void InitializeTimingMethod(void)
{
  bHighPerformanceTimerAvailable = false;

  if(QueryPerformanceFrequency((_LARGE_INTEGER *)&tickFrequency) == TRUE)
  {
    bHighPerformanceTimerAvailable = true;
    QueryPerformanceCounter((_LARGE_INTEGER *)&ticksAtBootTime);
  }
  else
  {
    ticksAtBootTime.QuadPart = time(NULL);
  }
}

void TimeOfDay(MPE *mpe)
{
  time_t currTime;
  tm *pcTime;
  _currenttime *nuonTime;
  uint32 ptrNuonTime, getset;

  ptrNuonTime = mpe->regs[0];
  getset = mpe->regs[1];

  if(getset == 0)
  {
    nuonTime = (_currenttime *)nuonEnv->GetPointerToMemory(mpe,ptrNuonTime,true);

    //Get time and fill time structure

    time(&currTime);
    pcTime = localtime(&currTime);

    nuonTime->sec = pcTime->tm_sec;
    nuonTime->min = pcTime->tm_min;
    nuonTime->hour = pcTime->tm_hour;
    nuonTime->wday = pcTime->tm_wday;
    nuonTime->mday = pcTime->tm_mday;
    nuonTime->month = pcTime->tm_mon + 1; //according to the SDK, january = 1
    nuonTime->year = pcTime->tm_year + 1900; //Nuon year field is the actual year value, not a delta
    nuonTime->isdst = pcTime->tm_isdst;
    nuonTime->timezone = _timezone/60; //Nuon timezone field is in minutes rather than seconds

    //A value of -1 in the Nuon field indicates that the system doesn't keep track of DST.
    if(pcTime->tm_isdst != -1)
    {
      nuonTime->isdst = 1;
    }

    //The pointer in _tzname[1] points to the daylight savings time zone name
    //if available, and is set to NULL if not available.  I am assuming that
    //if windows is not set up to keep track of DST, _tzname[1] is set to NULL.

    //Almost all windows users will allow windows to keep track of DST, so it
    //wont hurt too much if this assumption is incorrect.  This may have to
    //be taken care of differently on other platforms

    if(_tzname[1] == NULL)
    {
      nuonTime->isdst = -1;
    }

    SwapVectorBytes((uint32 *)&nuonTime->sec);
    SwapVectorBytes((uint32 *)&nuonTime->mday);
    SwapScalarBytes((uint32 *)&nuonTime->isdst);
    SwapScalarBytes((uint32 *)&nuonTime->timezone);

    mpe->regs[0] = 0;
  }
  else
  {
    //Set time using values in time structure

    //Not allowed to set the time
    mpe->regs[0] = -1;
  }
}

void TimeElapsed(MPE *mpe)
{
  uint64 seconds, useconds, mseconds;
  uint32 *memPtr, ptrSecs, ptrUSecs;
  double float_seconds;
  _LARGE_INTEGER counter;
  

  if(bHighPerformanceTimerAvailable)
  {
    QueryPerformanceCounter((_LARGE_INTEGER *)&counter);
    
    counter.QuadPart -= ticksAtBootTime.QuadPart;
    float_seconds = (double)counter.QuadPart / (double)tickFrequency.QuadPart;
    seconds = float_seconds;
    useconds = 1000000.0 * (float_seconds - seconds);
    mseconds = float_seconds * 1000.0;
  }
  else
  {
    seconds = time(NULL) - ticksAtBootTime.QuadPart;
    mseconds = seconds*1000;
    useconds = 0;
  }

  ptrSecs = mpe->regs[0];
  ptrUSecs = mpe->regs[1];

  //Store seconds if pointer is not NULL
  if(ptrSecs)
  {
    memPtr = (uint32 *)nuonEnv->GetPointerToMemory(mpe,ptrSecs,true);
    *memPtr = seconds;
    SwapScalarBytes(memPtr);
  }

  //Store microseconds if pointer is not NULL
  if(ptrUSecs)
  {
    memPtr = (uint32 *)nuonEnv->GetPointerToMemory(mpe,ptrUSecs,true);
    *memPtr = useconds;
    SwapScalarBytes(memPtr);
  }

  mpe->regs[0] = mseconds;
}

void TimerInit(uint32 whichTimer, uint32 rate)
{
  if(whichTimer == 0)
  {
    timeKillEvent(hSysTimer0);
    hSysTimer0 = timeSetEvent(rate/1000,0,(LPTIMECALLBACK)SysTimer0Callback,0,TIME_PERIODIC);
  }
  else if(whichTimer == 1)
  {
    timeKillEvent(hSysTimer1);
    hSysTimer1 = timeSetEvent(rate/1000,0,(LPTIMECALLBACK)SysTimer1Callback,0,TIME_PERIODIC);
  }
  else
  {
    timeKillEvent(hSysTimer2);
    hSysTimer2 = timeSetEvent(rate/1000,0,(LPTIMECALLBACK)SysTimer2Callback,0,TIME_PERIODIC);
  }

}

void TimerInit(MPE *mpe)
{
  int32 whichTimer = mpe->regs[0];
  int32 rate = mpe->regs[1];

  if((whichTimer) < 0 || (whichTimer > 1))
  {
    mpe->regs[0] = 0;
  }
  else
  {
    TimerInit(whichTimer,rate);
    mpe->regs[0] = 1;
  }
}