///////////////////////////////////////////////////////////////////////
//
//  FILE   :  signal_box.cpp
//
//            Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  The header file for all the WIN32 definitions of
//            the SignalBox class.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/posix/signal_box.cpp#1 $
//
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "thread/signal_box.h"
#include "thread/critical_section.h"
#include "thread/error_msg.h"
#include "time/hi_res_clock.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static void fatal_error(const char *str, int err)
{
    fprintf(stderr, "%s: %s", str, strerror(err));
    abort();
}

class SignalBox::SignalBoxData : private NoCopy
{
public:
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
    volatile int    m_signalled;
    Item*           m_items;

    SignalBoxData();
    ~SignalBoxData();

    void _locked_addItem(Item *waiting);
    void _locked_removeItem(Item *waiting);
    void _locked_setItem(Item &waiting);
    bool _locked_unsetItem(Item &waiting);
    bool _locked_wait(unsigned millisec);
};

SignalBox::SignalBoxData::SignalBoxData()
    : m_signalled(0), m_items(0)
{
    int err;

    if ((err = pthread_mutex_init (&m_mutex, NULL)) != 0)
        fatal_error("SignalBox::SignalBoxData::SignalBoxData() - Failed to init mutex", err);
    if ((err = pthread_cond_init (&m_cond, NULL)) != 0)
        fatal_error("SignalBox::SignalBoxData::SignalBoxData() - Failed to init condition", err);
}

SignalBox::SignalBoxData::~SignalBoxData()
{
    int err;

    if ((err = pthread_cond_destroy (&m_cond)) != 0)
        fatal_error("SignalBox::SignalBoxData::~SignalBoxData() - Failed to destroy condition", err);
    if ((err = pthread_mutex_destroy (&m_mutex)) != 0)
        fatal_error("SignalBox::SignalBoxData::~SignalBoxData() - Failed to destroy mutex", err);
}

void SignalBox::SignalBoxData::_locked_addItem(Item *si)
{
    si->m_next = m_items;
    m_items = si;

    si->m_hasBeenSignalled = false;
}

void SignalBox::SignalBoxData::_locked_removeItem(Item *killMe)
{
    SignalBox::Item **si;

    for (si = &m_items; *si != 0; si = &((*si)->m_next))
    {
        if(*si == killMe)
            break;
    }

    if (*si == 0)
        ERROR_MSG(("SignalBox::Item is not in the SignalBox!\n"));
    assert(*si != 0);
    assert(*si == killMe);
    
    *si = (*si)->m_next;
}

void SignalBox::SignalBoxData::_locked_setItem(Item &waiting)
{
    int err;

    Item *si;
    for (si = m_items; si != 0; si = si->m_next)
    {
        if (si == &waiting)
            break;
    }
    
    if (si == 0)
        ERROR_MSG(("SignalBox::Item is not in the SignalBox!\n"));
    assert(si != 0);
    assert(si == &waiting);
    
    // Note that the item has been set
    waiting.m_hasBeenSignalled = true;
    
    // Set the signal
    m_signalled = 1;

    // Signal to waiting threads
    if ((err = pthread_cond_signal (&m_cond)) != 0)
        fatal_error("SignalBox::SignalBoxData::_locked_setItem() - Failed to set condition", err);
}

bool SignalBox::SignalBoxData::_locked_unsetItem(Item &waiting)
{
    bool oneOrMoreSet = false;

    Item *si;
    for (si = m_items; si != 0; si = si->m_next)
    {
        if (si == &waiting)
            break;

        if (si->m_hasBeenSignalled)
            oneOrMoreSet = true;
    }

    // oneOrMoreSet is true if one of the SignalItem's in
    // the _part_of_the_linked_list_before_si_ is set

    if(si == 0)
        ERROR_MSG(("SignalBox::Item is not in the SignalBox!\n"));
    assert(si != 0);
    assert(si == &waiting);

    // note that this SignalItem has been dealt with
    waiting.m_hasBeenSignalled = false;

    // If an earlier item in the signal box was set, then we return
    // true and dont unset the signal.
    if (oneOrMoreSet)
        return true;

    // Step through the rest, to see if more work needs to be done.
    // If any other item in the signal box is set we return true and
    // do not unset the signal.
    for (si = si->m_next; si != 0; si = si->m_next)
        if(si->m_hasBeenSignalled)
            return true;

    m_signalled = 0;

    return false;
}

bool SignalBox::SignalBoxData::_locked_wait(unsigned millisec)
{
    int err;
    HiResClock hrc;
    struct timespec event_time;

    // It is fine to have a signal box with nothing in it.
    // Wait will return false (which might break other stuff).
    if (m_items == 0)
        return false;

    // Check to see if we are already signalled
    if (m_signalled)
        return true;

    // If the timeout is zero we do not wait
    if (millisec == 0)
        return false;
    
    // Set up the time at which we will wait until (only once)
    HiResClockGetNanoSec(&hrc);

    event_time.tv_sec = hrc.tv_sec + millisec / 1000;
    event_time.tv_nsec = hrc.tv_nsec + (millisec % 1000) * 1000000;
    if (event_time.tv_nsec >= 1000000000L)
    {
        event_time.tv_sec++;
        event_time.tv_nsec -= 1000000000L;
    }

    // Loop, waiting for the siganl to be set
    do
    {
        if ((err = pthread_cond_timedwait(
             &m_cond, &m_mutex, &event_time)) != 0)
        {
            if (err == ETIMEDOUT)
                return false;

            fatal_error("SignalBox::SignalBoxData::_locked_wait() - Failed to wait for condition with timeout", err);
        }
    }
    while (!m_signalled);
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SignalBox::SignalBox()
    : m_data(new SignalBoxData())
{
    InitHiResClock(0);
}

SignalBox::~SignalBox()
{
    assert(m_data != 0);
    delete m_data;
}

//
// This is called to create a ''SignalItem''  It adds it to
// the linked list of the 'SignalBox' and sets its state to unsignalled
//
void SignalBox::AddItem(SignalBox::Item *si)
{
    int err;

    assert(m_data != 0);
    
    if ((err = pthread_mutex_lock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::AddItem() - Failed to acquire mutex", err);

    m_data->_locked_addItem(si);

    if ((err = pthread_mutex_unlock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::AddItem() - Failed to release mutex", err);
}

//
// This will remove the SignalItem from the linked list.
// 
void SignalBox::RemoveItem(SignalBox::Item *killMe)
{
    int err;
    assert(m_data != 0);

    if ((err = pthread_mutex_lock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::RemoveItem() - Failed to acquire mutex", err);

    m_data->_locked_unsetItem(*killMe);
    
    m_data->_locked_removeItem(killMe);

    if ((err = pthread_mutex_unlock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::RemoveItem() - Failed to release mutex", err);
}

void SignalBox::setItem(Item &waiting)
{
    int err;
    assert(m_data != 0);

    if ((err = pthread_mutex_lock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::setItem() - Failed to acquire mutex", err);

    m_data->_locked_setItem(waiting);

    if ((err = pthread_mutex_unlock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::setItem() - Failed to release mutex", err);
}

bool SignalBox::unsetItem(Item &waiting)
{
    int err;
    bool ret;
    assert(m_data != 0);

    if ((err = pthread_mutex_lock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::unsetItem() - Failed to acquire mutex", err);

    ret = m_data->_locked_unsetItem(waiting);

    if ((err = pthread_mutex_unlock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::unsetItem() - Failed to release mutex", err);

    return ret;
}

// If millisec == 0, test and return
//    millisec == 0xFFFFFFFF, wait for ever
//
bool SignalBox::wait(unsigned millisec /*= 0xffffffff*/)
{
    int err;
    bool ret;

    assert(m_data != 0);

    if ((err = pthread_mutex_lock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::wait() - Failed to acquire mutex", err);

    ret = m_data->_locked_wait(millisec);

    if ((err = pthread_mutex_unlock (&m_data->m_mutex)) != 0)
        fatal_error("SignalBox::wait() - Failed to release mutex", err);

    return ret;
}
