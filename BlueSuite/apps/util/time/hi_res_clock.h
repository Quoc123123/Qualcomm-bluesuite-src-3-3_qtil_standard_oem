/**********************************************************************
*
*  FILE   :  hi_res_clock.h
*
*            Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  The interface to the HiResClock. This should return the
*            time in a number of different resolutions. It must NOT
*            be used for busy waiting!
*
***********************************************************************/

#ifndef HI_RES_CLOCK_H
#define HI_RES_CLOCK_H

#if !defined _WINCE && !defined _WIN32_WCE
#include <time.h>
#endif
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif


/* This is used to store the time.  It is the same as a 'struct
   timespec', but that type is not fully portable. */

typedef struct HiResClock_tag
{
    time_t tv_sec;
    long int tv_nsec;
} HiResClock;


/* This is the type returned by the 'HiResClockGetMilliSec' function. */

typedef unsigned long int HiResClockMilliSec;
typedef unsigned long int HiResClockMicroSec;



/* This is the maximum amount of time that we can use without worrying
   about wrap-around (half the range of the type above) */

#define HRC_MAX_MILLISEC (LONG_MAX)

/****************************************************************************
NAME HiResClockHMSFromPreset

DESCRIPTION
    If the program has already called HiResClockGetNanoSec previously, use
    this function to to output the time in the same way as HiResClockHMS.
*/
void HiResClockHMSFromPreset(char *buf, const HiResClock preset);

/****************************************************************************
NAME
    HiResClockGetSec  -  Get the time in seconds

DESCRIPTION
    This will return the time in seconds.  This is the number of
    seconds since the Epoch.
*/

time_t HiResClockGetSec(void);


/****************************************************************************
NAME
    HiResClockGetMilliSec  -  Get the time in milli-seconds

DESCRIPTION
    This will return the time in milli seconds.  This is the
    number of seconds since the Epoch.  We attempt to use a method
    that will return the maximum resolution.
*/

HiResClockMilliSec HiResClockGetMilliSec(void);


/****************************************************************************
NAME
    HiResClockGetNanoSec  -  Get the time in nano-seconds

DESCRIPTION
    This will return the time in nano seconds.  This is the number
    of seconds since the Epoch.  We attempt to use a method that
    will return the maximum resolution.  The structure
    'HiResClock' is very simular to a 'struct timespec', but they
    are not as fully portable as we would like.
*/

void HiResClockGetNanoSec(HiResClock *ret);

/****************************************************************************
NAME
    HiResClockGetEpochMicroSec  -  Get the time in micro-seconds

DESCRIPTION
    This will return the time in micro seconds.  This is the number
    of micro seconds since the Epoch.
*/

unsigned long long HiResClockGetEpochMicroSec(void);

/****************************************************************************
NAME
    HiResClockHMS  -  Get the time in seconds as a string

DESCRIPTION
    This will return the time in seconds.  The buffer is filled
    with a string containg hours, minutes, seconds and possibly
    milliseconds. Where TIME_MICROSECOND_RESOLUTION is defined,
    it will also include microseconds where possible.

    It is useful for timestamping things.

    The buffer must be at least TIME_HMS_BUF_LEN bytes long.
*/

#define TIME_HMS_BUF_LEN 16

void HiResClockHMS(char *buf);


/****************************************************************************
NAME
    InitHiResClock  -  Initialise the Hi-Res clock

DESCRIPTION
    This must be called before any of the other functions in this
    library.  It can be called multiple times.  It will optionally
    return a rough guess at the accuracy of the clock.  This guess
    is almost certainly an overestimate.

    This function is also possibly not thread safe (it is on all
    Posix versions and WinCE, but not windows).  It is possible to
    call any of the other functions from multiple threads at the
    same time, and they should all work, but if this function is
    called from one thread while another HiResClock function is
    running it might go horribly wrong (basically don't call this
    function more than you need to).
*/

void InitHiResClock(HiResClock *res);


/****************************************************************************
NAME
    HiResClockSleepMilliSec  -  Block the current thread for n milliseconds
*/

void HiResClockSleepMilliSec(HiResClockMilliSec n);

/****************************************************************************
See description preceding function definition for details
*****************************************************************************/

void HiResClockSleepMicroSec(HiResClockMicroSec n);


/*
 * Set the callback to use when a backwards time stamp is detected
 */
extern void (* backwards_timestamp_callback)(const char *prev, const char *next);

#ifdef __cplusplus
}
#endif

#endif /* HI_RES_CLOCK_H */
