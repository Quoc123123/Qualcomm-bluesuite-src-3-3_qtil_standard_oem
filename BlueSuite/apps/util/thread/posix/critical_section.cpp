///////////////////////////////////////////////////////////////////////
//
//  FILE   :  posix\critical_section.cpp
//
//            Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  Hold machine dependent definitions of Critical
//            sections as standard functions which are used in
//            the rest of the code.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/posix/critical_section.cpp#1 $
//
///////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "thread/critical_section.h"

static void fatal_error(const char *str, int err)
{
    fprintf(stderr, "%s: %s", str, strerror(err));
    abort();
}

void *CriticalSection::InitialiseCS(bool recursive)
{
    int err;
    pthread_mutexattr_t attrs;
    err = pthread_mutexattr_init(&attrs);
    if (err != 0)
    {
        fatal_error("CriticalSection::InitialiseCS - init failed (attr init)", err);
    }
    if(recursive)
    {
        err = pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
        if (err != 0)
        {
            fatal_error("CriticalSection::InitialiseCS - init failed (set type)", err);
        }
    }
    pthread_mutex_t *section;
    section = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
    err = pthread_mutex_init (section, &attrs);
    if (err != 0)
    {
        fatal_error("CriticalSection::InitialiseCS - init failed", err);
    }
    return section;
}

void CriticalSection::EnterCS(void *section)
{
    int err = pthread_mutex_lock ((pthread_mutex_t *) section);
    if (err != 0)
    {
      fatal_error("CriticalSection::EnterCS - lock failed", err);
    }
}

void CriticalSection::LeaveCS(void *section)
{
    int err = pthread_mutex_unlock ((pthread_mutex_t *) section);
    if (err != 0)
    {
      fatal_error ("CriticalSection::LeaveCS - unlock failed", err);
    }
}

void CriticalSection::DestroyCS(void *section)
{
    int err = pthread_mutex_destroy ((pthread_mutex_t *) section);
    if (err != 0)
    {
      fatal_error ("CriticalSection::DestroyCS - destroy failed", err);
    }
    free (section);
}

