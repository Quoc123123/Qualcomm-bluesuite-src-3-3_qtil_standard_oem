/**********************************************************************
*
*  FILE   :  hi_res_clock.c
*
*            Copyright (c) 2010-2022 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  The POSIX HiResClock.  This should return the
*            time in a number of different accuracies.
*
*            This file uses pre-processor macros. You might
*            need to give some -D options to the compiler to get the
*            best performance. The best thing to do is to define
*            _GNU_SOURCE, as that switches on most usefull things.
*
*            We use thtree methods of getting the time, they are;
*            clock_gettime, gettimeofday and time.
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/time/posix/hi_res_clock.c#1 $
*
***********************************************************************/

#include "time/hi_res_clock.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void (* backwards_timestamp_callback)(const char *prev_timestamp, const char *next_timestamp) = NULL;

/* This pulls in the Posix.1b realtime stuff, if available */
#include <time.h>

#if defined (POSIX_NO_CLOCK_GETTIME) && !defined (BSD_NO_GETTIMEOFDAY)

/* This pulls in the BSD stuff needed for timestamping with subsecond
   precision */
#include <sys/time.h>

/* Appears to be needed under Linux; can't do much harm under other Un*xes */
#include <unistd.h>

#endif /* defined (POSIX_NO_CLOCK_GETTIME) && !defined (BSD_NO_GETTIMEOFDAY) */

#ifndef POSIX_NO_CLOCK_GETTIME

#define OUTPUT_SUBSECOND
#define TIME_TYPE struct timespec

static int compare_timestamps(struct timespec *prev, struct timespec *next)
{
    if(prev->tv_sec < next->tv_sec) return 1;
    else if(prev->tv_sec == next->tv_sec) return prev->tv_nsec <= next->tv_nsec;
    else return 0;
}
static void record_and_check_timestamp(TIME_TYPE *next);
static long my_clock_gettime(clockid_t which_clock, struct timespec *tp)
{
    long r = clock_gettime(which_clock, tp);
    if(which_clock == CLOCK_REALTIME) record_and_check_timestamp(tp);
    return r;
}
#define clock_gettime my_clock_gettime

/* Get the time in nanosecounds */
void HiResClockGetNanoSec(HiResClock *ret)
{
    struct timespec ts;
    clock_gettime (CLOCK_REALTIME, &ts);
    ret->tv_sec = ts.tv_sec;
    ret->tv_nsec = ts.tv_nsec;
}

/* Get the time in millisecounds */
unsigned long int HiResClockGetMilliSec(void)
{
    struct timespec ts;
    clock_gettime (CLOCK_REALTIME, &ts);
    return (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000UL);
}

/* Get the time in secounds */
time_t HiResClockGetSec(void)
{
    struct timespec ts;
    clock_gettime (CLOCK_REALTIME, &ts);
    return ts.tv_sec;
}

/* Init the clock routines, this will optionally return the resolution */
void InitHiResClock(HiResClock *res)
{
    if (res != NULL)
    {
        struct timespec ts;
        clock_getres (CLOCK_REALTIME, &ts);
        res->tv_sec = ts.tv_sec;
        res->tv_nsec = ts.tv_nsec;
    }
}

#else /* POSIX_NO_CLOCK_GETTIME */
#ifndef BSD_NO_GETTIMEOFDAY

#define OUTPUT_SUBSECOND
#define TIME_TYPE struct timeval

static int compare_timestamps(struct timeval *prev, struct timeval *next)
{
    if(prev->tv_sec < next->tv_sec) return 1;
    else if(prev->tv_sec == next->tv_sec) return prev->tv_usec <= next->tv_usec;
    else return 0;
}
static void record_and_check_timestamp(TIME_TYPE *next);
static int my_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    int r = gettimeofday(tv, tz);
    if(tz == NULL) record_and_check_timestamp(tv);
    return r;
}
#define gettimeofday my_gettimeofday

/* Get the time in nanosecounds */
void HiResClockGetNanoSec(HiResClock *ret)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    ret->tv_sec = tv.tv_sec;
    ret->tv_nsec = tv.tv_usec * 1000;
}

/* Get the time in millisecounds */
unsigned long int HiResClockGetMilliSec(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (tv.tv_sec * 1000UL) + (tv.tv_usec / 1000);
}

/* Get the time in secounds */
time_t HiResClockGetSec(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return tv.tv_sec;
}

/* Init the clock routines, this will optionally return the resolution */
void InitHiResClock(HiResClock *res)
{
    if(res != NULL)
    {
        res->tv_sec = 0;
        res->tv_nsec = 1000;
    }
}

/* Sleep for n milliseconds */
void HiResClockSleepMilliSec(unsigned long int n)
{
    struct timespec ts;
    ts.tv_sec = n/1000;
    ts.tv_nsec = (n%1000)*1000000;
    nanosleep(&ts,NULL);
}

/* Sleep for n microseconds */
void HiResClockSleepMicroSec(unsigned long int n)
{
    struct timespec ts;
    ts.tv_sec = n/1000000;
    ts.tv_nsec = (n%1000000)*1000;
    nanosleep(&ts,NULL);
}

#else /* BSD_NO_GETTIMEOFDAY */
#define TIME_TYPE time_t

static int compare_timestamps(time_t *prev, time_t *next)
{
    return *prev <= *next;
}
static void record_and_check_timestamp(TIME_TYPE *next);
static time_t my_time(time_t *t)
{
    time(t);
    record_and_check_timestamp(t);
    return *t;
}
#define time my_time

/* Get the time in nanosecounds */
void HiResClockGetNanoSec(HiResClock *ret)
{
    ret->tv_sec = time(NULL);
    ret->tv_nsec = 0;
}

/* Get the time in millisecounds */
unsigned long int HiResClockGetMilliSec(void)
{
    return time(NULL) * 1000UL;
}

/* Get the time in secounds */
time_t HiResClockGetSec(void)
{
    return time(NULL);
}

void HiResClockHMS(char *buf)
{
    time_t t = time(NULL);
    struct tm res;
    strftime(buf, 13, "%H:%M:%S", localtime_r(&t, &res));
}

/* Init the clock routines, this will optionally return the resolution */
void InitHiResClock(HiResClock *res)
{
    if(res != NULL)
    {
        res->tv_sec = 1;
        res->tv_nsec = 0;
    }
}

#endif /* BSD_NO_GETTIMEOFDAY */
#endif /* POSIX_NO_CLOCK_GETTIME */

void HiResClockHMSFromPreset(char *buf, const HiResClock preset)
{
    char buf2[16];
    struct tm res;
    strftime(buf2, sizeof(buf2), "%H:%M:%S", localtime_r(&preset.tv_sec, &res));
#ifdef OUTPUT_SUBSECOND
#ifdef TIME_MICROSECOND_RESOLUTION
    sprintf(buf, "%.8s.%06li", buf2, preset.tv_nsec / 1000);
#else
    sprintf(buf, "%.8s.%03li", buf2, preset.tv_nsec / 1000000);
#endif
#endif
}

unsigned long long HiResClockGetEpochMicroSec(void)
{
    HiResClock hrClk;
    unsigned long long usec;

    HiResClockGetNanoSec(&hrClk);

    usec = (unsigned long long)(hrClk.tv_sec) * 1000000ULL;
    usec += (hrClk.tv_nsec / 1000UL);
    return usec;
}

void HiResClockHMS(char *buf)
{
    HiResClock clock;
    HiResClockGetNanoSec(&clock);
    HiResClockHMSFromPreset(buf, clock);
}

/* implementation of checking timestamps don't go backwards */

#include <pthread.h>

static pthread_once_t key_init = PTHREAD_ONCE_INIT;
static pthread_key_t key;

void cleanup_stored_time(void *p) { if(p) free(p); }

static void init_key()
{
    pthread_key_create(&key, cleanup_stored_time);
}

void record_and_check_timestamp(TIME_TYPE *next)
{
    pthread_once(&key_init, init_key);
    void *prev = pthread_getspecific(key);
    if(prev)
    {
        if(!compare_timestamps((TIME_TYPE *) prev, next))
        {
            char time1[sizeof(TIME_TYPE)*2 + 1];
            char time2[sizeof(TIME_TYPE)*2 + 1];
            int i;
            for(i = 0; i < sizeof(TIME_TYPE); ++i)
            {
                sprintf(time1+i*2, "%02x", ((unsigned char *)prev)[i]);
                sprintf(time2+i*2, "%02x", ((unsigned char *)next)[i]);
            }
            if(backwards_timestamp_callback) (*backwards_timestamp_callback)(time1, time2);
        }
    }
    else
    {
        prev = malloc(sizeof(TIME_TYPE));
        pthread_setspecific(key, prev);
    }
    *((TIME_TYPE *) prev) = *next;
}

