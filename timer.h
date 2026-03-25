#ifndef timerH
#define timerH

class MPE;

struct _currenttime {
    int32 sec;
    int32 min;
    int32 hour;
    int32 wday;
    int32 mday;
    int32 month;
    int32 year;
    int32 isdst;     /* flag: for definitions, see below */
    int32 timezone;  /* minutes west of Greenwich */
    int32 reserved[3]; /* reserved for future expansion */
};

void InitializeTimingMethod();
void DeInitTimingMethod();

uint64 useconds_since_start();

void TimeElapsed(MPE &mpe);
void TimeOfDay(MPE &mpe);
void TimerInit(const uint32 whichTimer, const uint32 rate);
void TimerInit(MPE &mpe);

#endif
