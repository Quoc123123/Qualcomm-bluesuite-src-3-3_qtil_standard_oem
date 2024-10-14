///////////////////////////////////////////////////////////////////////
//
//  FILE   :  passive_timer.cpp
//
//  Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CLASS  :  PassiveTimer
//
//  PURPOSE:  Timers that can be set and checked.  When they are ready 
//            they will ''fire''.
//
///////////////////////////////////////////////////////////////////////

#include <list>
#include <algorithm>
#include "time/passive_timer.h"


/////////////////////////////////////////////////////////////////////////////
// NAME
//  PassiveTimer::~PassiveTimer  -  Dtor for a PassiveTimer
//

PassiveTimer::~PassiveTimer()
{
    if (func_ != 0)
        delete func_;
}


/////////////////////////////////////////////////////////////////////////////
// NAME
//  PassiveTimer::CheckTimer  -  Update the state of a timer
//

HiResClockMilliSec PassiveTimer::CheckTimer(HiResClockMilliSec now)
{
    if (state_ != timer_pending)
        return HRC_MAX_MILLISEC;

    const signed long int diff = time_ - now;

    if (diff > 0)
        return diff;

    state_ = timer_fired;

    if (func_)
        (*func_)();

    // If this timer was reset by the action function
    if (state_ == timer_pending)
        return time_ - now;

    return HRC_MAX_MILLISEC;
}


HiResClockMilliSec PassiveTimer::CheckTimer(HiResClockMilliSec now, bool &fired)
{
    if (state_ != timer_pending)
        return HRC_MAX_MILLISEC;

    const signed long int diff = time_ - now;

    if (diff > 0)
        return diff;

    state_ = timer_fired;

    fired = true;

    if (func_)
        (*func_)();

    return HRC_MAX_MILLISEC;
}


/////////////////////////////////////////////////////////////////////////////
// NAME
//  PassiveTimer::SetTimer  -  Set the timer to fire in 'period' milliseconds
//

void PassiveTimer::SetTimer()
{
    if (period_ != 0)
    {
        state_ = timer_pending;
        time_ = HiResClockGetMilliSec() + period_;
    }
}


/////////////////////////////////////////////////////////////////////////////
// NAME
//  PassiveTimer::SetPeriod  -  Set the period of the timer
//

void PassiveTimer::SetPeriod(HiResClockMilliSec p)
{
    state_ = timer_unset;
    period_ = p;
}


/////////////////////////////////////////////////////////////////////////////
// NAME
//  PassiveTimer::Cancel  -  Stop the Timer from Firing
//

void PassiveTimer::Cancel()
{
    state_ = timer_unset;
}


/////////////////////////////////////////////////////////////////////////////
// NAME
//  PassiveTimerPrivate::TimerListCheck  -  Helper class
//
// FUNCTION
//  This class is used in the 'CheckPassiveTimers' function.  It is
//  passed to the for_each function.
//

namespace PassiveTimerPrivate
{
    struct TimerListCheck
    {
        HiResClockMilliSec min_;
        HiResClockMilliSec now_;
        bool any_fired_;

        TimerListCheck(HiResClockMilliSec max, HiResClockMilliSec now)
            : min_(max), now_(now), any_fired_(false) {}

        void operator()(PassiveTimer *t)
        {
            HiResClockMilliSec v = t->CheckTimer(now_, any_fired_);
            if (v < min_)
                min_ = v;
        }
    };
};


/////////////////////////////////////////////////////////////////////////////
// NAME
//  CheckPassiveTimers  -  Check the timers
//

HiResClockMilliSec CheckPassiveTimers(const PassiveTimerList &list, HiResClockMilliSec max)
{
    const HiResClockMilliSec now = HiResClockGetMilliSec();
    HiResClockMilliSec until_next;

    for (;;)
    {
        PassiveTimerPrivate::TimerListCheck tlc = 
            std::for_each(list.begin(), list.end(), PassiveTimerPrivate::TimerListCheck(max, now));

        until_next = tlc.min_;

        if (!tlc.any_fired_)
            break;
    }

    return until_next;
}
