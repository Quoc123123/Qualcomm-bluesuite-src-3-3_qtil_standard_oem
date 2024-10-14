///////////////////////////////////////////////////////////////////////
//
//  FILE   :  signal_box.cpp
//
//            Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  AUTHOR :  Mark Marshall
//
//  PURPOSE:  The header file for all the WIN32 definitions of
//            the SignalBox class.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/win32/signal_box.cpp#1 $
//
///////////////////////////////////////////////////////////////////////

#include "thread/signal_box.h"
#include "thread/critical_section.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <tchar.h>

#if !defined _WINCE && !defined _WIN32_WCE
#include <assert.h>
#else
#define assert(TEST) do{}while(0)
#endif

#include "../error_msg.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class SignalBox::SignalBoxData : private NoCopy
{
public:
    CriticalSection m_protection;
    HANDLE      m_event;
    Item*       m_items;

    SignalBoxData();
    ~SignalBoxData();

    void _locked_setItem ( Item & waiting )
    {
        assert(this != 0);

        Item *si;
        for (si = m_items; si != 0; si = si->m_next)
        {
            if(si == &waiting)
                break;
        }
    
        // This should not happen.  It means that the programmer has
        // made a boob!
        if(si == 0)
            ERROR_MSG(("SignalBox::Item is not in the SignalBox!\n"));
        assert(si != 0);
        assert(si == &waiting);

        waiting.m_hasBeenSignalled = true;
    
        SetEvent(m_event);
    }

    bool _locked_unsetItem ( Item & waiting )
    {
        assert(this != 0);
        
        bool oneOrMoreSet = false;

        Item *si;
        for (si = m_items; si != 0; si = si->m_next)
        {
            if(si == &waiting)
                break;

            if(si->m_hasBeenSignalled)
                oneOrMoreSet = true;
        }
        
        // oneOrMoreSet is true if one of the SignalItem's in the
        // _part_of_the_linked_list_before_si_ is set

        // This should not happen.  It means that the programmer has
        // made a boob!
        if(si == 0)
            ERROR_MSG(("SignalBox::Item is not in the SignalBox!\n"));
        assert(si != 0);
        assert(si == &waiting);
        
        // note that this SignalItem has been dealt with
        waiting.m_hasBeenSignalled = false;

        if (oneOrMoreSet)
            return true;

        // Step through the rest, to see if more work needs to be done
        for (si = si->m_next; si != 0; si = si->m_next)
            if(si->m_hasBeenSignalled)
                return true;

        ResetEvent(m_event);

        return false;
    }

};

SignalBox::SignalBoxData::SignalBoxData() : m_items(0)
{
    m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    OUTPUT_HANDLE_CREATE(m_event);
    if(m_event == NULL)
    {
        DWORD err = GetLastError();
        ERROR_MSG(("Cannot create EVENT! %x\n", err));
    }
}

SignalBox::SignalBoxData::~SignalBoxData()
{
    if(m_event != NULL)
        CloseHandle(m_event);
    OUTPUT_HANDLE_CLOSE(m_event);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SignalBox::SignalBox()
{
    m_data = new SignalBoxData();
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
    assert(m_data != 0);

    CriticalSection::Lock lock(m_data->m_protection);

    si->m_next = m_data->m_items;
    m_data->m_items = si;

    si->m_hasBeenSignalled = false;
}

//
// This will remove the SignalItem from the linked list.
// 
void SignalBox::RemoveItem(SignalBox::Item *killMe)
{
    assert(m_data != 0);

    CriticalSection::Lock lock(m_data->m_protection);

    // Unset the item before we remove it
    m_data->_locked_unsetItem(*killMe);
    SignalBox::Item **si;
    for (si = &(m_data->m_items); *si != 0;
     si = &((*si)->m_next))
    {
        if(*si == killMe)
            break;
    }

    // This should not happen.  It means that the programmer has
    // made a boob!
    if(*si == 0)
        ERROR_MSG(("SignalBox::Item is not in the SignalBox!\n"));
    assert(*si != 0);
    assert(*si == killMe);
    
    *si = (*si)->m_next;
}

void SignalBox::setItem(Item &waiting)
{
    assert(m_data != 0);
    CriticalSection::Lock lock(m_data->m_protection);
    m_data->_locked_setItem(waiting);
}

bool SignalBox::unsetItem(Item &waiting)
{
    assert(m_data != 0);
    CriticalSection::Lock lock(m_data->m_protection);
    return m_data->_locked_unsetItem(waiting);
}

bool SignalBox::wait(unsigned millisec /*= 0xffffffff*/)
{
    DWORD stat;

    assert(m_data != 0);

    // I have decided that it is fine to have a signal box with nothing in it.
    // Wait will return false (which might break other stuff).
    if(m_data->m_items == 0)
    {
        return false;
    }

    stat = WaitForSingleObject(m_data->m_event, millisec);
    if(stat != WAIT_OBJECT_0 && stat != WAIT_TIMEOUT)
    {
        // Problem!!
        DWORD err = GetLastError();
        ERROR_MSG(("WaitForSingleObject failed(SignalBox::wait)! %x\n", err));
        return false;
    }

    return stat == WAIT_OBJECT_0;
}

