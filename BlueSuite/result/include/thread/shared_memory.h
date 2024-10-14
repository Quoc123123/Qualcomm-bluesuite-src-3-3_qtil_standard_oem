/**********************************************************************
*
* shared_memory.h
*
* Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* A memory buffer shared between processes.
* The buffer is referred to by name.
* The size is fixed, and opening the same shared memory multiple
* times with different sizes will have undefined results.
*
* The shared memory is not allocated until dereferenced with ptr().
* If there was a problem creating or accessing the shared memory then ptr will
* return NULL. If it succeeded then ptr will return the address it was mapped
* in at and the created() function can be used to tell whether the memory block
* was just created, so as to tell whether it needs to be initialised.
*
* Because the shared memory is not created until ptr() is dereferenced, the
* following is a safe way to atomically allocate and initialise shared memory:
* SharedMemory mem("MyMemory", 1024);
* {
*   SystemWideMutexLock lock(&mem.global_mutex());
*   void *p = mem.ptr();
*   if (p && mem.created()) {
*     ... initialise shared memory buffer ...
*   }
* }
*
* Shared memory is public readable and writeable to all users.
*
**********************************************************************/
#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include "critical_section.h"
#include "system_wide_mutex.h"
#include "common/nocopy.h"
#include <string>
#include <memory>

class SharedMemory : private NoCopy
{
public:
    SharedMemory(const char *name, size_t len);
    ~SharedMemory();
    void *ptr();
    CriticalSection &local_mutex() { return mLocalMutex; }
    SystemWideMutex &global_mutex() { return mGlobalMutex; }
    bool created() { return mCreated; }

private:
    class Impl;
    std::auto_ptr<Impl> mpImpl;
    CriticalSection mLocalMutex;
    SystemWideMutex mGlobalMutex;
    bool mCreated;
    std::string mName;
    size_t mLen;
};

/***********************************************
 * An object of type T shared between processes.
 * The object is referred to by name (the name space is shared
 * between SharedMemory and SharedObject objects).
 * When the object is first created the constructor will run, but
 * the destructor will never be run.
 **********************************************/
template <class T> class SharedObject : private NoCopy
{
public:
    SharedObject(const char *name) : mMem(name, sizeof(T))
    {
        SystemWideMutexLock lock(&global_mutex());
        T *t =(T *)  mMem.ptr();
        if (t && mMem.created())
        {
            new (t) T;
        }
    }
    T *ptr()
    {
        return (T *) mMem.ptr();
    }
    T *operator*()
    {
        return ptr();
    }
    T *operator->()
    {
        return ptr();
    }
    CriticalSection &local_mutex() { return mMem.local_mutex(); }
    SystemWideMutex &global_mutex() { return mMem.global_mutex(); }
    
    /**
     * Tells whether the memory associated with the object has just been allocated
     * or it was already created.
     * This method is usefull to know whether the objects need to be initializated.
     * @return true if the object has been created, false if it already existed
     */
    bool created() { return mMem.created(); }

private:
    SharedMemory mMem;
};

#endif
