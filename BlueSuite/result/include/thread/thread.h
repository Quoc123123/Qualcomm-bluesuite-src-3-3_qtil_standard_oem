///////////////////////////////////////////////////////////////////////
//
//  FILE   :  thread.h
//
//            Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  The header file for all the machine dependent
//            definitions of the Threadable class.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/result/include/thread/thread.h#1 $
//
///////////////////////////////////////////////////////////////////////

#ifndef THREADABLE_H
#define THREADABLE_H

#include "common/nocopy.h"

class Threadable : private NoCopy
{
private:
    // This function is overridden by the derived class, it does
    // all of the work.
    virtual int ThreadFunc() = 0;

    // This is where we will hide all of the platform specific thread data.
    class ThreadData;
    friend class ThreadData;

    ThreadData* m_data;

public:

    Threadable();

    virtual ~Threadable();

    // Start the thread running
    bool Start();

    // Returns true if the thread is still running
    bool IsActive();
    
    // This gets the exit code of the thread
    int GetExitCode();

    // This should be checked by the 'ThreadFunc' function every so often.
    // If it returns false, then the 'ThreadFunc' function should exit,
    // but should close down all of its stuff first.
    bool KeepGoing();

    // Call this if you want the thread to close down.
    // This only sets a flag, that should be read by the 'ThreadFunc'
    // function.  If you need other actions to take place to signal a
    // stop, then they should be placed in here.  (If for example
    // the ThreadFunc function could be paused in a blocking system call,
    // this should be overriden so that it gets the thread out of
    // that function)
    virtual void Stop();

    // Call this to wait up to millisec ms for the thread to be in a stopped
    // state.  Returns true if the thread made it into a stopped state, false
    // otherwise.
    bool WaitForStop(unsigned millisec);

    // Nicely asks the thread to stop, waits for the specified time. If thread 
    // hasn't stopped, terminates it.
    void Kill(unsigned millisec = 1000);

    // Lower the threads priority a couple of points
    void LowPriority(void);

    // Set priority back to normal
    void NormalPriority(void);

    // Make the thread have more priority
    void HighPriority(void);
};

class ThreadID
{
public:
    ThreadID(const ThreadID &);
    ThreadID &operator=(const ThreadID &);
    bool operator==(const ThreadID &);
    ~ThreadID();

    static ThreadID self();

    /**
     * Returns the current thread ID as a numeric value
     * @return the current thread ID 
     */
    static unsigned long Id();
private:
    ThreadID();

    struct Impl;
    Impl *impl;
};

class ThreadSpecificPtrImpl;
extern ThreadSpecificPtrImpl *new_thread_specific_ptr_impl(void (*)(void *));
extern void delete_thread_specific_ptr_impl(ThreadSpecificPtrImpl *);
extern void *thread_specific_ptr_impl_get(ThreadSpecificPtrImpl *);
extern void thread_specific_ptr_impl_set(ThreadSpecificPtrImpl *, void *);

/*
 * A pointer of arbitrary type, that can have a different value for each thread that accesses it.
 * This object must NOT be of automatic storage (and you almost certainly don't want it to be anyway).
 *
 * examples:
 *
 * ThreadSpecificPtr<data> thread_local_data;
 *
 * void my_func() {
 *   if (!thread_local_data) thread_local_data = new data;
 *   thread_local_data->entry = 5;
 *   thread_local_data->updated = now();
 * }
 *
 * OR:
 * void my_func() {
 *   static ThreadSpecificPtr<int> recursion_count;
 *   if(!recursion_count) recursion_count = new int(0);
 *   if(++*recursion_count == 100) throw infinite_loop_exception();
 *
 *   ...
 * }
 * you may specify a custom cleanup function to be called on thread exit. By default the pointer is cast to the
 * right type and 'deleted' (so the above examples are fine and do not leak, and if thread_local_data had a
 * a destructor it would run).
 * The cleanup function will not be called for NULL pointers.
 * Under windows the cleanup function may run in a different thread to the one the TLS exists in.
 * Remember that the cleanup function may run during program exit (for the main process thread), so other global
 * variables may have been destroyed already.
 *
 * Thread local storage should only be used in the initial thread of a process or a thread 
 * created by the Thread class. Using it within any other type of thread _may_ work but will
 * probably leak memory.
 *
 * In windows TLS does not persist across DLL boundaries, so in the above example the thread_local_data in
 * one instance of MyDLL.dll will be different to the one in another instance of MyDLL.dll even in
 * the same process. This is just as would be the case with normal global and static variables.
 */

template <class T> class ThreadSpecificPtr : private NoCopy
{
public:
    // The default cleanup function. simply deletes the object using stock operator delete
    static void default_cleanup(void *p) { delete (T *) p; }

    // The constructor can optionally take a custom cleanup function.
    explicit ThreadSpecificPtr(void (*cleanup)(void *) = default_cleanup) : m_impl(new_thread_specific_ptr_impl(cleanup)) {}
    ~ThreadSpecificPtr() { delete_thread_specific_ptr_impl(m_impl); }

    ThreadSpecificPtr<T> &operator=(T *p) { thread_specific_ptr_impl_set(m_impl, (void *) p); return *this; }
    operator T*() const { return get(); }
    T* operator->() const { return get(); }

private:
    T* get() const { return (T *) thread_specific_ptr_impl_get(m_impl); }
    ThreadSpecificPtrImpl *m_impl;
};

#endif // def THREADABLE_H

