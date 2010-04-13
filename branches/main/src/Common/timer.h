//////////////////////////////////////////////////////////////////////////////
// timer.h
// =======
// High Resolution CTimer.
// This timer is able to measure the elapsed time with 1 micro-second accuracy
// in both Windows, Linux and Unix system 
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2003-01-13
// UPDATED: 2006-01-13
//
// Copyright (c) 2003 Song Ho Ahn
//////////////////////////////////////////////////////////////////////////////

#if !defined  (_timer_H__7E97003A_4E75_4fde_9612_B3060C4C8DC9)
#define _timer_H__7E97003A_4E75_4fde_9612_B3060C4C8DC9

#if defined (_WIN32)
#include <windows.h>
#include "errorTest.h"
#else          // Unix based system specific
#include <sys/time.h>
#endif


class CTimer
{
public:
    CTimer();                                    // default constructor
    ~CTimer();                                   // default destructor

    void   start();                             // start timer
    void   stop();                              // stop timer
    double getElapsedTime();                    // get elapsed time in second
    double getElapsedTimeInSec();               // get elapsed time in second (same as getElapsedTime)
    double getElapsedTimeInMilliSec();          // get elapsed time in millisecond (one thousandth of a second, ms, 10^-3 ∫¡√Î)
    double getElapsedTimeInMicroSec();          // get elapsed time in microsecond (one millionth of a second, us, 10^-6 Œ¢√Î)
                                                // get elapsed time in nanosecond  (one billionth of a second, ns, 10^-9 ƒ…√Î)


private:
    double startTimeInMicroSec;                 // starting time in micro-second
    double endTimeInMicroSec;                   // ending time in micro-second
    bool   stopped;                             // stop flag 

#if defined (_WIN32)
    LARGE_INTEGER frequency;                    // ticks per second
    LARGE_INTEGER startCount;                   //
    LARGE_INTEGER endCount;                     //
#else
    timeval startCount;                         //
    timeval endCount;                           //
#endif

};

#endif // _timer_H__7E97003A_4E75_4fde_9612_B3060C4C8DC9
