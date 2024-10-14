/**********************************************************************
//
//  FILE   :  hi_res_clock.c
//
//            Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  The Windows32 version of the HiResClock. This should
//            return the time in a number of different accuracies.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/time/win32/hi_res_clock.c#1 $
//
***********************************************************************/

#include "time/hi_res_clock.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if !defined(_WINCE) && !defined(_WIN32_WCE)
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <winbase.h>
#endif

#define MICROSECS_PER_SEC (1000000)

void (* backwards_timestamp_callback)(const char *prev_timestamp, const char *next_timestamp) = NULL;

/* If we can we use the performance counter.  If we can (Not WinCE) we
 * then only make one call to _ftime.  All of the other times are
 * calculated by calling QueryPerformaceCounter, and adding the value
 * to the original value.  This total is then offset with the original
 * cal to _ftime. */

static LARGE_INTEGER perfc_freq;
static LARGE_INTEGER perfc_offset;

static BOOL use_perfc = FALSE;

static BOOL time_lib_initialised = FALSE;

/* Get the time in seconds */
time_t HiResClockGetSec(void)
{
    if (use_perfc)
    {
        BOOL success;
        LARGE_INTEGER t;

        success = QueryPerformanceCounter (&t);
        assert (success);

        t.QuadPart = t.QuadPart + perfc_offset.QuadPart;

        return (time_t)(t.QuadPart / perfc_freq.LowPart);
    }
    else
    {
#if !defined(_WINCE) && !defined(_WIN32_WCE)
        struct _timeb created;
        _ftime(&created);
        return created.time;
#else
        assert(0);
        exit(1);
        return 0;
#endif
    }
}

/* Get the time in milliseconds */
unsigned long int HiResClockGetMilliSec(void)
{
    if (use_perfc)
    {
        BOOL success;
        LARGE_INTEGER t;

        success = QueryPerformanceCounter (&t);
        assert (success);

        t.QuadPart = t.QuadPart + perfc_offset.QuadPart;

        return (unsigned long int)
            ((t.QuadPart * 1000UL) / perfc_freq.LowPart);
    }
    else
    {
#if !defined(_WINCE) && !defined(_WIN32_WCE)
        struct _timeb created;
        _ftime(&created);
        return (unsigned long)((created.time * 1000UL) + created.millitm);
#else
        assert(0);
        exit(1);
        return 0;
#endif
    }
}

/* Get the time in nanoseconds */
void HiResClockGetNanoSec(HiResClock *res)
{
    if (res == NULL)
        return;

    if (use_perfc)
    {
        LARGE_INTEGER t;
        time_t secs;
        unsigned long frac;
        long int nsec;

        BOOL success = QueryPerformanceCounter (&t);
        assert (success);

        t.QuadPart = t.QuadPart + perfc_offset.QuadPart;

        secs = (time_t)(t.QuadPart / perfc_freq.LowPart);
        frac = (unsigned long)(t.QuadPart % perfc_freq.LowPart);

        // Here we divide frac by 2, to make it fit into an signed int (31 bits used).
        // This we then subsequently multiply it by 2 billion to get
        // the value which can be turned into nsecs.
        nsec = (long)(Int32x32To64(frac / 2, 2000000000L)/perfc_freq.LowPart);

        res->tv_sec = secs;
        res->tv_nsec = nsec;
    }
    else
    {
#if !defined(_WINCE) && !defined(_WIN32_WCE)
        struct _timeb created;
        _ftime(&created);
        res->tv_sec = created.time;
        res->tv_nsec = created.millitm * 1000000;
#else
        assert(0);
        exit(1);
#endif
    }
}

/* FILE Time is nanosec from 1st Jan 1601
 * micro sec from 1st Jan 1601 to 1st Jan 1970 : 0x00295E96.48864000
 */

ULONGLONG HiResClockGetEpochMicroSec(void)
{
    const ULONGLONG FILE_TIME_OFFSET_TO_EPOCH_BASE = 0x295E9648864000ULL;
    ULONGLONG usec;
    FILETIME file_time;
    SYSTEMTIME system_time;

    GetLocalTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);

    usec = (ULONGLONG)file_time.dwHighDateTime << 32;
    usec += file_time.dwLowDateTime;
    usec /= 10; /* convert to micro sec from 100 nsec units */

    /* Convert to epoch */
    usec -= FILE_TIME_OFFSET_TO_EPOCH_BASE;

    return usec;
}

/* Get the time in seconds as a string */
void HiResClockHMS(char *buf)
{
    *buf = 0;

#if !defined(_WINCE) && !defined(_WIN32_WCE)
    if (use_perfc)
    {
        BOOL success;
        LARGE_INTEGER t;
        time_t secs;
        unsigned int msecs;
        struct tm *p;
        char temp[16];
        size_t n;

        success = QueryPerformanceCounter (&t);
        assert(success);

        t.QuadPart = t.QuadPart + perfc_offset.QuadPart;

        secs = (time_t)(t.QuadPart / perfc_freq.LowPart);

        // Here we divide perfc_freq.LowPart by 2 to fit into a signed integer.
        msecs = MulDiv((int)((t.QuadPart % perfc_freq.LowPart) / 2),
                   1000, (perfc_freq.LowPart / 2));

        if ((p = localtime(&secs)) == NULL)
            return;

        n = strftime(temp, sizeof(temp), "%H:%M:%S", p);
        assert(n != 0);

        _snprintf(buf, 13, "%s.%03u", temp, msecs);
    }
    else
    {
        struct _timeb created;
        struct tm *p;
        char temp[16];
        size_t n;

        _ftime(&created);

        if ((p = localtime(&created.time)) == NULL)
            return;

        n = strftime(temp, sizeof(temp), "%H:%M:%S", p);
        assert(n != 0);

        _snprintf(buf, 13, "%s.%03d", temp, created.millitm);
    }
#else /* _WINCE */
    {
        SYSTEMTIME t;
        GetLocalTime(&t);
        sprintf (buf, "%02u:%02u:%02u.%03u",
            t.wHour, t.wMinute, t.wSecond,t.wMilliseconds);
    }
#endif /* !_WINCE */
}

/* Init the clock routines, this will optionally return the resolution */
void InitHiResClock(HiResClock* res)
{
    HiResClock resolution;

    use_perfc = QueryPerformanceFrequency(&perfc_freq);

    if (use_perfc)
    {
        BOOL success;
        LARGE_INTEGER now, offset;
#if !defined(_WINCE) && !defined(_WIN32_WCE)
        struct _timeb tb;
#else
        SYSTEMTIME st;
#endif

        /* This is fair */
        assert(perfc_freq.HighPart == 0);

        /* So mark, whats the plan here?
         *
         * Well, we get the Performance Counter time, which is
         * in arbitary units from an arbitary start time.  We
         * also get the current time in milliseconds since the
         * epoch.  From these two values we work out the
         * difference between the Epoch and the time the
         * Performance Counter is counting from (in arbitary
         * units).  When we later want to calculate a time we
         * add the value calculated here to the value returned
         * by the QueryPerformanceCounter, and this gives us
         * the time from the epoch in arbiary units.  We can
         * then convert that to better units by dividing by
         * the value obtained by the call to
         * QueryPerformanceFrequency.
         */

        success = QueryPerformanceCounter(&now);
        assert(success);

#if !defined(_WINCE) && !defined(_WIN32_WCE)
        _ftime(&tb);

        offset.QuadPart =
            UInt32x32To64(tb.time , perfc_freq.LowPart / 2) * 2;
        offset.QuadPart +=
            MulDiv(tb.millitm, (perfc_freq.LowPart / 2), 500);
#else
        GetLocalTime(&st);
        {
            FILETIME ft;
            if ( SystemTimeToFileTime(&st,&ft) )
            {
                offset.QuadPart = ((((__int64)ft.dwHighDateTime*0x100000000)
                                   + ft.dwLowDateTime)
                                * perfc_freq.LowPart ) / 10000000;
            }
        }
#endif

        perfc_offset.QuadPart = offset.QuadPart - now.QuadPart;

        resolution.tv_sec = 0;
        resolution.tv_nsec = 1000000000 / perfc_freq.LowPart;
    }
    else
    {
        resolution.tv_sec = 0;
        resolution.tv_nsec = 1000000;
    }

    time_lib_initialised = TRUE;

    if (res != NULL)
        *res = resolution;
}

void HiResClockSleepMilliSec(HiResClockMilliSec n)
{
    Sleep(n);
}

/*****************************************************************
 * If high-res timing is supported, sleeps for 'n' microseconds.
 * (Note that this sleep time is approximate.)
 * Otherwise, just returns. Note that, like most other functions
 * in this library, this function relies on InitHiResClock having
 * been called previously.
 * Consequently (as per the comments for InitHiResClock) it is not
 * thread-safe on Windows.
 *****************************************************************/
void HiResClockSleepMicroSec(HiResClockMicroSec n)
{
    assert(time_lib_initialised);
    assert( n < MICROSECS_PER_SEC); //Could allow larger n's without the calculations overflowing LONGLONG type,
                                    //but this method is only intended for waits much shorter than a second so
                                    //calling with a larger value is likely to indicate a mistake in the calling code.

    if (use_perfc)
    {
        BOOL success;
        LARGE_INTEGER initial_time;
        LONGLONG      start_time;
        LARGE_INTEGER current_time;
        LONGLONG sleeptime_times_freq = (LONGLONG)n * perfc_freq.QuadPart;

        success = QueryPerformanceCounter(&initial_time);
        assert(success);
        start_time = initial_time.QuadPart;

        do
        {
            success = QueryPerformanceCounter(&current_time);
            assert(success);
        } while (((current_time.QuadPart - start_time) * MICROSECS_PER_SEC) < sleeptime_times_freq);
    }
}
