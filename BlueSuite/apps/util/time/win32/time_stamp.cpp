// Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// Simple time stamp
// Introduced for marking packets in WinPrimDebug

#include "time/time_stamp.h"

#include <time.h>
#if !defined(_WINCE) && !defined(_WIN32_WCE)
#include <windows.h>
#else
#include <winbase.h>
#endif

class TimeStamp::Impl : public Counter
{
    SYSTEMTIME created;
public:
    Impl()
    { GetLocalTime(&created); }
    Impl( unsigned long systime )
    { 
        if ( first )
        {
            SYSTEMTIME now;
            GetLocalTime(&now);
            firstTick = GetTickCount();
            SystemTimeToFileTime(&now,&firstTime);
            first = false;
        }
        FILETIME result = firstTime;
        advance(&result,(systime - firstTick));
        FileTimeToSystemTime(&result,&created);
    }
    static bool first;
    static DWORD firstTick;
    static FILETIME firstTime;


    std::string hms() const
    {
        char result[32];
        sprintf ( result, "%02d:%02d:%02d.%03d" , created.wHour , created.wMinute , created.wSecond , created.wMilliseconds );
        return result;
    }

    std::string ymdhm() const
    {
        char result[32];
        static const char month[12][4]  = {"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
        sprintf ( result, "%04d-%s-%02d %02d:%02d" , created.wYear , month[created.wMonth-1] , created.wDay , created.wHour , created.wMinute );
        return result;
    }

    void advance(int msec)
    {
        advance(&created,msec);
    }

    void advance ( SYSTEMTIME * start, int msec )
    {
        FILETIME filetime;
        SystemTimeToFileTime(start, &filetime);
        advance(&filetime,msec);
        FileTimeToSystemTime(&filetime, start);
    }

    void advance ( FILETIME * start, int msec )
    {
        ULARGE_INTEGER linear;
        linear.LowPart  = start->dwLowDateTime;
        linear.HighPart = start->dwHighDateTime;
        linear.QuadPart += 10000 * (ULONGLONG) msec; // time is in 100nsec units
        start->dwLowDateTime  = linear.LowPart;
        start->dwHighDateTime = linear.HighPart;
    }
};

bool TimeStamp::Impl::first = true;
DWORD TimeStamp::Impl::firstTick;
FILETIME TimeStamp::Impl::firstTime;

TimeStamp::TimeStamp()
    : ptr(new Impl)
{ }

TimeStamp::TimeStamp(const TimeStamp &x)
    : ptr(x.ptr)
{ }

TimeStamp &TimeStamp::operator=(const TimeStamp &x)
{ ptr = x.ptr; return *this; }

TimeStamp::~TimeStamp()
{ }

std::string TimeStamp::hms() const
{ return ptr->hms(); }

std::string TimeStamp::hmsu() const
{ return ptr->hms(); }

std::string TimeStamp::ymdhm() const
{ return ptr->ymdhm(); }

TimeStamp TimeStamp::add_milliseconds(unsigned int msec) const
{
    Impl *new_impl = new Impl(*ptr);
    new_impl->advance(msec);
    return TimeStamp(new_impl);
}

TimeStamp::TimeStamp(Impl *p)
    : ptr(p)
{ }

TimeStamp TimeStamp::make( unsigned long time )
{
    return TimeStamp( new Impl(time) );
}
