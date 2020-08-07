#ifndef timerH
#define timerH

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

uint64 useconds_since_start();

void TimeElapsed(MPE &mpe);
void TimeOfDay(MPE &mpe);
void TimerInit(const uint32 whichTimer, const uint32 rate);
void TimerInit(MPE &mpe);

#endif
