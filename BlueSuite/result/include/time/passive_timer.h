///////////////////////////////////////////////////////////////////////
//
//  FILE   :  passive_timer.h
//
//  Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CLASS  :  PassiveTimer
//
//  PURPOSE:  Timers that can be set and checked.  When they are ready 
//            they will ''fire''.  They are not active, you need to
//            call a check function on each timer periodically, to see
//            if it has expired (hence the name passive).  It is 
//            expected that you will have a 'forever' loop with a 
//            blocking call in it.  Call the CheckTimer function, to
//            see how long it is until the next timer fires, and then
//            make the blocking call, giving this time as the period 
//            of time to block for.
//
///////////////////////////////////////////////////////////////////////

#ifndef PASSIVE_TIMER_H__
#define PASSIVE_TIMER_H__

#include <list>
#include "common/nocopy.h"
#include "time/hi_res_clock.h"

///////////////////////////////////////////////////////////////////////
// The contents of this file have been made more complex than it needs to
// be because it needs to compile on broken compilers (MSVC6).  This means
// that templated member functions need to be defined inline, and even then
// we can still blow up the compiler.  Most of the templated stuff has been
// moved into some global classes that are now hidden in this namespace.

namespace PassiveTimerPrivate
{
    struct PassiveTimerCallback : public NoCopy
    {
        virtual ~PassiveTimerCallback() {}
        virtual void operator()() = 0;
    };

    template<class XParam>
    struct mfTimerCallback : public PassiveTimerCallback
    {
        typedef void (XParam::* XParamFunc)(void);
        XParam *o_;
        XParamFunc f_;

        mfTimerCallback(XParam *o, XParamFunc f) : o_(o), f_(f) {}

        virtual void operator()()
        {
            if (o_ != 0 && f_ != 0)
                (o_->*f_)();
        };
    };

    struct fvTimerCallback : public PassiveTimerCallback
    {
        typedef void (* Func)(void);
        Func f_;

        fvTimerCallback(Func f) : f_(f) {}

        virtual void operator()()
        {
            if (f_ != 0)
                f_();
        };
    };

    template<typename Param>
    struct fpTimerCallback : public PassiveTimerCallback
    {
        typedef void (* Func)(Param);
        Func f_;
        Param p_;

        fpTimerCallback(Func f, Param p) : f_(f), p_(p) {}

        virtual void operator()()
        {
            if (f_ != 0)
                f_(p_);
        };
    };
};

struct PassiveTimer : public NoCopy
{
public:
    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::Timer  -  Ctor for a Timer
    //
    // FUNCTION
    //  Set the state to 'unset', and the function pointer to something 
    //  useful.
    //
    PassiveTimer(HiResClockMilliSec period = 0)
        : time_(0), period_(period), func_(0), state_(timer_unset)
    {}

    /////////////////////////////////////////////////////////////////////////////
    // NAME
    //  PassiveTimer::~PassiveTimer  -  Dtor for a PassiveTimer
    //
    // FUNCTION
    //  Cleans up the Timer Function object
    //
    virtual ~PassiveTimer();

    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::Bind  -  Bind a function to the timer
    //
    // FUNCTION
    //  There are a number of Bind functions.  Each can be used to bind a 
    //  different function object to the PassiveTimer.
    //
    template <typename Object>
    void Bind(Object *o, void (Object::* f)())
    {
        if (func_)
            delete func_;
        func_ = new PassiveTimerPrivate::mfTimerCallback<Object>(o, f);
    }

    void Bind(void (* f)(void))
    {
        if (func_)
            delete func_;
        func_ = new PassiveTimerPrivate::fvTimerCallback(f);
    }

    template <typename Param>
    void Bind(void (* f)(Param), Param p)
    {
        if (func_)
            delete func_;
        func_ = new PassiveTimerPrivate::fpTimerCallback<Param>(f, p);
    }

    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::CheckTimer  -  See if the timer should fire yet
    //
    // FUNCTION
    //  This will move the state of the timer from pending to fired
    //  if the timer has expired.  It will return the number of milliseconds
    //  until the timer will expire if it is still pending.  It will return 
    //  ULONG_MAX if the timer has fired or is not pending.
    //
    HiResClockMilliSec CheckTimer(HiResClockMilliSec now);

    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::CheckTimer  -  See if the timer should fire yet
    //
    // FUNCTION
    //  This will move the state of the timer from pending to fired
    //  if the timer has expired.  It will return the number of milliseconds
    //  until the timer will expire if it is still pending.  It will return 
    //  ULONG_MAX if the timer has fired or is not pending.
    //
    //  This function will also set 'fired' to true if this timer fires.
    //  It will not change the value fo 'fired' if this timer does not fire.
    //  This is not the same as checking the state before and after this
    //  call.  The action function could reset the state.
    //
    HiResClockMilliSec CheckTimer(HiResClockMilliSec now, bool &fired);

    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::SetTimer  -  Set the timer to fire
    //
    // FUNCTION
    //  This will set the timer to fire in 'period' milliseconds 
    //  from now.
    //
    void SetTimer();

    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::SetPeriod  -  Set the period of the timer
    //
    // FUNCTION
    //  Set the period of the timer.  Timers are one shot, so this function 
    //  might be misnamed
    //
    void SetPeriod(HiResClockMilliSec p);

    /////////////////////////////////////////////////////////////////////////
    // NAME
    //  Timer::Cancel  -  Cancel a timer
    //
    // FUNCTION
    //  Causes a timer to not fire
    //
    void Cancel();

protected:
    // The time at which this timer fires
    HiResClockMilliSec time_;

    // The time delay between setting the timer and it firing
    HiResClockMilliSec period_;

    // Function to call on timeout
    PassiveTimerPrivate::PassiveTimerCallback *func_;

    // The state of the timer
    enum { timer_unset, timer_pending, timer_fired } state_;
};


typedef std::list<PassiveTimer *> PassiveTimerList;


/////////////////////////////////////////////////////////////////////////////
// NAME
//  CheckPassiveTimers  -  Check the timers
//
// FUNCTION
//  Check a list of timers, and if any have fired, execute
//  the correct code.  Return the number of milliseconds
//  until the next timer will fire (or max)

HiResClockMilliSec CheckPassiveTimers(const PassiveTimerList &list, HiResClockMilliSec max);


#endif /* PASSIVE_TIMER_H__ */

