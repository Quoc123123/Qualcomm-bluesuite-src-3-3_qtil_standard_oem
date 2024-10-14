////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  FILE:    stop_watch.cpp
//
//  AUTHOR:  ajh
//
//  DATE:    3 October 2001
//
//  PURPOSE: Simple duration measuring object.
//
//  CLASS:   StopWatch
//
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32 // Headers required for Windows implementation
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else // Headers required for non-Windows implementation
extern"C" {
#include "hi_res_clock.h"
}
#endif

// common implementation header
#include "stop_watch.h"



#ifndef _WIN32 // original non-Windows implementation starts here...

static bool initialised = false;

class StopWatch::Impl : public Counter
{
public:
  Impl() 
  {
    if ( !initialised )
    {
      InitHiResClock(0);
      initialised = true;
    }
    HiResClockGetNanoSec ( &start );
  }
  unsigned long millisec()
  {
    HiResClock now;
    HiResClockGetNanoSec ( &now );
    return (unsigned long)(( (now.tv_sec - start.tv_sec) * 1000 ) // 1000 ms = 1s
         + ( (now.tv_nsec - start.tv_nsec) / 1000000 )); // 1ms = 1000000ns
  }
  unsigned long microsec()
  {
    HiResClock now;
    HiResClockGetNanoSec ( &now );
    return (unsigned long)( ((now.tv_sec - start.tv_sec) * 1000000 ) // 1000000 us = 1s
         + ( (now.tv_nsec - start.tv_nsec) / 1000 ) ); // 1us = 1000ns
  }
private:
  HiResClock start;
};

StopWatch::StopWatch() // Records the time it was built
: ptr ( new Impl )
{ }

StopWatch::StopWatch(const StopWatch &x)
: ptr ( x.ptr )
{ }

StopWatch &StopWatch::operator=(const StopWatch &x)
{ ptr = x.ptr; return *this; }

StopWatch::~StopWatch()
{ }

unsigned long StopWatch::duration() // milliseconds since creation.
{ return ptr->millisec(); }

unsigned long StopWatch::uduration() // microseconds since creation.
{ return ptr->microsec(); }


#else // else it is _WIN32


// very simple implementation - just a backing store for the
// values captured when the stopwatch was created
class StopWatch::Impl
{
public:
    Impl() : mStartTime(), mFrequency() {};
    ~Impl() {};

    LARGE_INTEGER mStartTime;
    LARGE_INTEGER mFrequency;

    // captures current count and returns the number of ticks since this instance was created
    LARGE_INTEGER ElapsedTicks()
    {
        LARGE_INTEGER timeNow;
        QueryPerformanceCounter(&timeNow);

        LARGE_INTEGER elapsedInTicks;
        elapsedInTicks.QuadPart = timeNow.QuadPart - mStartTime.QuadPart;

        // Sanity protection code: if the timeNow and startTime were collected on different processor cores,
        // there is a slight chance that one counter is slightly ahead of the other.
        // In which case, there is the tiny possibility that for zero-ish elapsed time we would end up with
        // a very small -ve count. In which case, we just assume no time has actually passed.
        if (elapsedInTicks.QuadPart < 0)
        {
            elapsedInTicks.QuadPart = 0;
        }

        return elapsedInTicks;
    }
};


// Implementation for the stopwatch itself
StopWatch::StopWatch() // Records the time it was built
    : pImpl(new Impl)
{
    QueryPerformanceCounter(&pImpl->mStartTime);
    QueryPerformanceFrequency(&pImpl->mFrequency);
}


StopWatch::StopWatch(const StopWatch &x)
    : pImpl(new Impl)
{
    pImpl->mStartTime = x.pImpl->mStartTime;
    pImpl->mFrequency = x.pImpl->mFrequency;
}


StopWatch &StopWatch::operator=(const StopWatch &x)
{
    pImpl->mStartTime = x.pImpl->mStartTime;
    pImpl->mFrequency = x.pImpl->mFrequency;

    return *this;
}


StopWatch::~StopWatch()
{
    delete pImpl;
}


unsigned long StopWatch::duration() // milliseconds since creation.
{
    LARGE_INTEGER elapsedInTicks = pImpl->ElapsedTicks();

    // convert from ticks to ms using the saved ticks-per-second value
    elapsedInTicks.QuadPart *= 1000; // scale for ms in 1 second
    elapsedInTicks.QuadPart /= pImpl->mFrequency.QuadPart;

    return elapsedInTicks.LowPart;
}


unsigned long StopWatch::uduration() // microseconds since creation.
{
    LARGE_INTEGER elapsedInTicks = pImpl->ElapsedTicks();

    // convert from ticks to us using the saved ticks-per-second value
    elapsedInTicks.QuadPart *= 1000000; // scale for us in 1 second
    elapsedInTicks.QuadPart /= pImpl->mFrequency.QuadPart;

    return elapsedInTicks.LowPart;
}


#endif // end of _WIN32 section
