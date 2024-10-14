// Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// system_wide_mutex.h
// A system-wide mutex class.
// This can be used to guard objects across inter-process boundries.
// Speed isn't an issue.

// As with all these classes, the header is a pretty generic C++ class.  
// The implementation is entrirely platform specific.

#ifndef THREAD__SYSTEM_WIDE_MUTEX_H
#define THREAD__SYSTEM_WIDE_MUTEX_H

#include "common/nocopy.h"
#include "common/types.h"
#include <stdlib.h>

class SystemWideMutex : private NoCopy
{
public:
    SystemWideMutex(const char * mutex_name, bool acquire = false, bool exclusive = false);
    SystemWideMutex(const wchar_t * mutex_name, bool acquire = false, bool exclusive = false);
    ~SystemWideMutex();

    bool Acquire(uint32 dwMilliseconds = NO_TIMEOUT);
    void Release();
    bool IsAcquired() const;

    enum {
        NO_TIMEOUT = 0xffffffff
    };

private:
    // Data hiding using pimpl
    class MutexData;
    // Trailing underscore nomenclature for member variables taken from A. Alexandescu
    MutexData *data_;
};

// This class will provide a lock for the above mutex. On deletion, release will be called.
class SystemWideMutexLock: private NoCopy
{
public:
    SystemWideMutexLock(SystemWideMutex * mutex) : mutex_(mutex)
        { mutex_->Acquire(SystemWideMutex::NO_TIMEOUT); };
    ~SystemWideMutexLock()
        { mutex_->Release(); };

private:
    SystemWideMutex *mutex_;
};
#endif
