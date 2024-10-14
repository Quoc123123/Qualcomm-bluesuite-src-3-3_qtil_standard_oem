///////////////////////////////////////////////////////////////////////
//
//  FILE   :  signal_box.h
//
//            Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  The header file for all the machine dependent
//            definitions of the SignalBox functions.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/signal_box.h#1 $
//
///////////////////////////////////////////////////////////////////////

#ifndef SIGNALBOX_H
#define SIGNALBOX_H

#include "common/nocopy.h"

// The SignalBox maintains a list of Item's.
//
class SignalBox : private NoCopy
{
public:
    // creator and destructor
    SignalBox();
    ~SignalBox();

    class Item;
    friend class Item;

    // The data used by the SignalBox is all hidden in a platform specific
    // data structure
    class SignalBoxData;

    // call this to make the calling thread wait until one of the
    // flags is set.  A timeout can be given in millisecounds.  This
    // will make this function return early (returns true if something
    // is set).  The default value will make the function wait
    // for-ever.
    bool wait(unsigned millisec = 0xffffffff);

    // The two functions below are exposed, as you might want to use
    // them.  It is better to use the member functions of 'Item'

    // call this to say that there is work to do.
    void setItem ( Item & waiting );

    // call this to say that the work is done.
    bool unsetItem ( Item & finished );

protected:
    void AddItem(SignalBox::Item *);

    void RemoveItem(SignalBox::Item *);

private:

    SignalBoxData *m_data;
};

class SignalBox::Item : private NoCopy
{
    friend class SignalBox;
    friend class SignalBox::SignalBoxData;

public:
    // Add this 'Item' to the 'SignalBox'
    Item(SignalBox &box)
     : m_owner(box), m_hasBeenSignalled(false), m_next(0)
    { m_owner.AddItem(this); }

    // Add this 'Item' to the 'SignalBox'
    Item(SignalBox *box)
     : m_owner(*box), m_hasBeenSignalled(false), m_next(0)
    { m_owner.AddItem(this); }

    // Remove this 'Item' to the 'SignalBox'
    ~Item()
    { m_owner.RemoveItem(this); }

    // call this to find out if the flag is set.
    bool getState ()
    { return m_hasBeenSignalled; }

    // Signal to this flag that there is work to do
    void setSignal(void)
    { m_owner.setItem(*this); }

    void set(void)
    { m_owner.setItem(*this); }

    // Signal to this flag that the work is done
    bool unsetSignal(void)
    { return m_owner.unsetItem(*this); }

    bool unset(void)
    { return m_owner.unsetItem(*this); }

    // Wait for _ANY_ flag in the SignalBox to become set
    bool wait()
    { return m_owner.wait(); }

protected:
    // A 'Item' is owned by a 'SignalBox'
    SignalBox&  m_owner;

    // The state of the 'Item'
    bool        m_hasBeenSignalled;

    // This will form a linked list of all Item's in a Box.
    Item*       m_next;
};

//  Most common way to use the signal box.
class SingleSignal
{
public:
    SingleSignal () : mItem(mBox) {}
    //  wait until set.
    bool wait() { return mBox.wait(); }
    //  millisecond timeout
    bool wait( unsigned timeout ) { return mBox.wait(timeout); }
    bool get() { return mItem.getState(); }
    void set() { mItem.set(); }
    void unset() { mItem.unset(); }

private:
    SignalBox mBox;
    SignalBox::Item mItem;
};

#endif //ndef SIGNALBOX_H
