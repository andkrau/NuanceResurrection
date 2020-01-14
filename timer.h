//---------------------------------------------------------------------------

#ifndef timerH
#define timerH
//---------------------------------------------------------------------------

class MPE;

struct _currenttime {
    __int32 sec;
    __int32 min;
    __int32 hour;
    __int32 wday;
    __int32 mday;
    __int32 month;
    __int32 year;
    __int32 isdst;     /* flag: for definitions, see below */
    __int32 timezone;  /* minutes west of Greenwich */
    __int32 reserved[3]; /* reserved for future expansion */
};

void InitializeTimingMethod(void);
void TimeElapsed(MPE *mpe);
void TimeOfDay(MPE *mpe);
void TimerInit(uint32 whichTimer, uint32 rate);
void TimerInit(MPE *mpe);

#endif
