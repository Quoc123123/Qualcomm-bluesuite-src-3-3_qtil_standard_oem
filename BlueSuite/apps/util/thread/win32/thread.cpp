///////////////////////////////////////////////////////////////////////
//
//  FILE   :  thread.cpp
//
//            Copyright (c) 2010-2018 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  AUTHOR :  Mark Marshall
//
//  PURPOSE:  Implementation for all the WIN32 definitions of
//            the Threadable class.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/win32/thread.cpp#1 $
//
///////////////////////////////////////////////////////////////////////

#include "thread/thread.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdlib.h> // for atexit
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#if !defined _WINCE && !defined _WIN32_WCE
#include <assert.h>
#define ASSERT assert
#else
#include <windef.h>
#include <dbgapi.h>
#endif

#include "../error_msg.h"

static void TLSCleanup();

//
// This is the class that holds all of the platform specific
// data (and also the static 'ThreadFunc' function).
//
class Threadable::ThreadData : private NoCopy
{
public:
    HANDLE      m_threadHandle; // This is the handle of the thread
    CRITICAL_SECTION m_criticalsection; // Ensure state transitions are atomic.
    bool m_started;
    bool m_dont_call_threadfunc;
    bool m_keepgoing;
    DWORD       m_threadId;     // What is this for?
    DWORD       m_exitCode;     // This is the thread exit code

    static DWORD WINAPI __ThreadFunker(LPVOID lpThreadParameter);

    ThreadData();
    ~ThreadData();
};

//
// Constructors are dull sometimes!
//
Threadable::ThreadData::ThreadData() :
    m_threadHandle(NULL),
    m_started(false),
    m_dont_call_threadfunc(false),
    m_keepgoing(true),
    m_threadId(0),
    m_exitCode(0)
{
    InitializeCriticalSection(&m_criticalsection);
}

Threadable::ThreadData::~ThreadData()
{
    if (NULL != m_threadHandle)
    {
        CloseHandle(m_threadHandle);
        m_threadHandle = NULL;
    }
    DeleteCriticalSection(&m_criticalsection);
}

//
// This is the function that gets called to begin the new thread
// It calls the 'ThreadFunc' function.
//
DWORD WINAPI Threadable::ThreadData::__ThreadFunker(LPVOID lpThreadParameter)
{
    DWORD exitCode;
    Threadable* thread = static_cast<Threadable*>(lpThreadParameter);
    ASSERT(thread != 0);
    ASSERT(thread->m_data != 0);
    EnterCriticalSection(&(thread->m_data->m_criticalsection));
    thread->m_data->m_started = true;
    if (thread->m_data->m_dont_call_threadfunc)
    {
        exitCode = thread->m_data->m_exitCode = 0xdead1;
        LeaveCriticalSection(&(thread->m_data->m_criticalsection));
        return exitCode;
    }
    LeaveCriticalSection(&(thread->m_data->m_criticalsection));
    exitCode = thread->ThreadFunc();
    thread->m_data->m_exitCode = exitCode;
    thread->m_data->m_keepgoing = false;
    TLSCleanup();
    return exitCode;
}

//
// Create a new 'Threadable' object
//
Threadable::Threadable()
{
    m_data = new ThreadData();
}

//
// This will hopefully stop the thread if it is running, and
// clean up any data that has been allocated
//
Threadable::~Threadable()
{
    ASSERT(m_data != 0);

    if (IsActive())
    {
        // Derived class hasn't cleaned up after itself. Kill the thread. (Although the still running thread at this point is
        // probably trying to access data that has been deallocated and so is likely to crash).
        ERROR_MSG((("Derived class should have stopped the thread. Killing thread (possibly unsafely).\n")));
        Kill();
    }
    delete m_data;
}

//
// This will start the new thread running, returns 'true'
// if everything is OK.
//
bool Threadable::Start()
{
    ASSERT(m_data != 0);
    ASSERT(!m_data->m_started || !m_data->m_keepgoing);
    if (m_data->m_started)
    {
        // Thread has been started, don't want to lose this without stopping
        if (!m_data->m_keepgoing)
        {
            WaitForStop(0);
        }
        else
        {
            return false;
        }
    }
    // Should be stopped now
    m_data->m_keepgoing = true;
    m_data->m_dont_call_threadfunc = false;
    HANDLE hnd = CreateThread (NULL, 0, ThreadData::__ThreadFunker, (LPVOID) this, 0, &m_data->m_threadId);
    OUTPUT_HANDLE_CREATE(hnd);
    if (hnd == NULL)
    {
        // Problem!!
        DWORD err = GetLastError();
        ERROR_MSG(("Create Thread Failed! %x\n", err));
        return false;
    }
    else
    {
        m_data->m_started = true;
        m_data->m_threadHandle = hnd;
        return true;
    }
}

//
// Returns 'true' if the thread is running
//
bool Threadable::IsActive()
{
    if (m_data->m_started && m_data->m_threadHandle != NULL)
    {
        // Has thread finished?
        DWORD state = WaitForSingleObject(m_data->m_threadHandle, 0);
        ASSERT(state == WAIT_OBJECT_0 || state == WAIT_TIMEOUT);

        return (state != WAIT_OBJECT_0);
    }
    else
    {
        // Thread has not started, or has been killed.
        return false;
    }
}

int Threadable::GetExitCode()
{
    ASSERT(m_data != 0);

    return (int)m_data->m_exitCode;
}

//
// Should the run function keep going?
//
bool Threadable::KeepGoing()
{
    return m_data->m_keepgoing;
}

//
// Signal to the run function that we should stop soon.
//
void Threadable::Stop()
{
    EnterCriticalSection(&(m_data->m_criticalsection));
    if (!m_data->m_started)
    {
        // The thread has not started yet. When it has, it should exit as soon as possible without calling threadfunc.
        m_data->m_dont_call_threadfunc = true;
    }
    m_data->m_keepgoing = false;
    LeaveCriticalSection(&(m_data->m_criticalsection));
}

//
// Do a WaitForObject on the thread handle and return whether the thread has
// exited
//
bool Threadable::WaitForStop(unsigned millisec)
{
    ASSERT(m_data != 0);
    if (!m_data->m_threadHandle) return true;
    if(millisec == 0) millisec = INFINITE;
    DWORD state = WaitForSingleObject(m_data->m_threadHandle, millisec);
    ASSERT(state == WAIT_OBJECT_0 || state == WAIT_TIMEOUT);
    if (state == WAIT_OBJECT_0)
    {
        // Oh look, what a handy time to perform some cleanup
        CloseHandle(m_data->m_threadHandle);
        m_data->m_threadHandle = NULL;
        m_data->m_started = false;
        return true;
    }
    else
    {
        return false;
    }
}

//
// Wait for the other thread to end.
// If it does not stop by itslef after the timeout
// Then we take drastic measures
//
void Threadable::Kill(unsigned millisec /*= 1000*/)
{
    EnterCriticalSection(&(m_data->m_criticalsection));
    if (!m_data->m_started)
    {
        // The thread has not started yet. When it has, it should exit as soon as possible without calling threadfunc.
        m_data->m_dont_call_threadfunc = true;
        m_data->m_keepgoing = false;
    }
    else
    {
        // Call Stop first to give the thread a chance to exit cleanly.
        Stop();
    }
    LeaveCriticalSection(&(m_data->m_criticalsection));
    if (!WaitForStop(millisec))
    {
        TerminateThread(m_data->m_threadHandle, 0xdead2);
        CloseHandle(m_data->m_threadHandle);
        m_data->m_threadHandle = NULL;
        m_data->m_started = false;
    }
}
/*
    // Some derived classes overload Start to not start the thread.
    if (m_data->m_threadHandle != NULL)
    {
        // Give stop a little time to work
        if(millisec == 0) millisec = INFINITE;
        DWORD state = WaitForSingleObject(m_data->m_threadHandle, millisec);
        ASSERT(state == WAIT_OBJECT_0 || state == WAIT_TIMEOUT);
        if (state == WAIT_OBJECT_0)
        {
            // Thread has finished.
        }
        else
        {
            // Thread is still running. Kill it.
            TerminateThread(m_data->m_threadHandle, 0xdead2);
        }
        CloseHandle(m_data->m_threadHandle);
        m_data->m_threadHandle = NULL;
        m_data->m_started = false;
    }
}*/

void Threadable::LowPriority(void)
{
    ASSERT(m_data->m_threadHandle != 0);
    SetThreadPriority(m_data->m_threadHandle, THREAD_PRIORITY_LOWEST);
}

void Threadable::NormalPriority(void)
{
    ASSERT(m_data->m_threadHandle != 0);
    SetThreadPriority(m_data->m_threadHandle, THREAD_PRIORITY_NORMAL);
}

void Threadable::HighPriority(void)
{
    ASSERT(m_data->m_threadHandle != 0);
    SetThreadPriority(m_data->m_threadHandle, THREAD_PRIORITY_HIGHEST);
}

struct ThreadID::Impl
{
    DWORD id;
};

ThreadID::ThreadID() : impl(new Impl) {}

ThreadID::ThreadID(const ThreadID &x) : impl(new Impl)
{
    impl->id = x.impl->id;
}

ThreadID &ThreadID::operator=(const ThreadID &x)
{
    impl->id = x.impl->id;
    return *this;
}

ThreadID::~ThreadID()
{
    delete impl;
}

bool ThreadID::operator==(const ThreadID &x)
{
    return impl->id == x.impl->id;
}

ThreadID ThreadID::self()
{
    ThreadID r;
    r.impl->id = GetCurrentThreadId();
    return r;
}

unsigned long ThreadID::Id()
{
    return (unsigned long) GetCurrentThreadId();
}

/*
 * This is far harder than it should be because of having to deal with DLLs...
 *
 * TLS must be cleaned up in 2 circumstances:
 * 1. When the thread exits (obviously).
 * 2. When the DLL/exe it's in unloads.
 * We have to clean up when the DLL unloads because the cleanup function may well be going with it, so we have to
 * do it immediately.
 *
 * To handle this each TLS entry is added to 2 linked lists - a list of all variables by thread and a list
 * of all threads by variable.
 * When the thread exits the thread list is traversed and everything cleaned up and excised from the variable lists.
 * When a variable is destroyed (usually because its execution unit is unloading) its list is traversed and everything cleaned up
 * and excised from the thread lists.
 * Because they be edited up from other threads, the heads of the per thread tls lists are stored in main memory, and are added
 * to a process global list that is traversed and cleaned up on process exit.
 *
 * In addition, there may be multiple thread.libs (one for each DLL), so the entry point for the per thread list must be stored
 * in process global space, not just as a variable (because there would a separate instance of it per thread.lib). This is done
 * by creating a small memory mapped IPC area named based on the process ID.
 *
 * Because threads exiting and DLLs unloading may theoretically collide, a mutex is locked when creating and destroying
 * a TLS object or exiting a thread. This mutex is not necessary when simply reading and writing TLS memory, so it shouldn't slow
 * things down too much. Again this mutex must be shared between all instances of thread.lib so it's a full kernel mutex, not a
 * critical section.
 *
 * Thread local "shortcuts" are stored per variable that point directly at the right TLS entry for speed of finding.
 */

class ThreadSpecificPtrImpl
{
public:
    ThreadSpecificPtrImpl(void (*cleanup)(void *));
    ~ThreadSpecificPtrImpl();

    void set(void *);
    void *get();

    DWORD m_key;
    void (*m_cleanup)(void *);

    struct TLSEntry
    {
        ThreadSpecificPtrImpl *m_tls;
        void *m_value;
        TLSEntry *m_thread_next;
        TLSEntry **m_thread_prevp;
        TLSEntry *m_var_next;
        TLSEntry **m_var_prevp;
        void init(ThreadSpecificPtrImpl *tls, void *value)
        {
            m_tls = tls;
            m_value = value;
        }
        void destroy() {}
    };

    struct TLSSentry
    {
        TLSEntry *m_head;
        TLSSentry *m_next;
        TLSSentry *m_prev;

        void init() {}
        void destroy() {}
    };

    TLSEntry *m_var_tls_entries;

    class ThreadTLSEntries
    {
    public:
        void init();
        void destroy();

        unsigned m_refcount;

        TLSSentry *get_list_head(bool create);
        void set_list_head(TLSSentry *);

    private:
        HANDLE hGlobalMemory;
        struct GlobalMemory {
            unsigned refcount;
            DWORD tls_key;
            TLSSentry *sentry_list;
        } *pGlobalMemory;
    };

    class TLSGlobalMutex
    {
    public:
        TLSGlobalMutex() : hGlobalMutex(NULL) { }
        ~TLSGlobalMutex() { CloseMutexHandle(); }

        void lock();
        void unlock();

        void CloseMutexHandle()
        {
            if (hGlobalMutex)
            {
                CloseHandle(hGlobalMutex);
                hGlobalMutex = NULL;
            }
        }

    private:
        HANDLE hGlobalMutex;
    };

    // The one and only instance of the TLSGlobalMutex locking manager...
    static TLSGlobalMutex tls_global_mutex;
    // ...and the routine to tidy it up in the event that it still has an open handle when the DLL is unloaded
    static void AtExitFreeHandle(void) { tls_global_mutex.CloseMutexHandle(); }

    static void ThreadCleanup();
};
ThreadSpecificPtrImpl::TLSGlobalMutex ThreadSpecificPtrImpl::tls_global_mutex;

// we need to guarantee that this is deleted after all TLS objects, so make it a ref counted pointer.
static ThreadSpecificPtrImpl::ThreadTLSEntries *thread_tls_entries = NULL;

static void TLSCleanup() { ThreadSpecificPtrImpl::ThreadCleanup(); }


void ThreadSpecificPtrImpl::TLSGlobalMutex::lock()
{
    if (!hGlobalMutex)
    {
        enum { MAX_CHARS_FOR_TLS_MUTEX_NAME = 32 };
        TCHAR mutex_name[MAX_CHARS_FOR_TLS_MUTEX_NAME];
        _stprintf(mutex_name, _T("TLS Mutex %08x"), GetCurrentProcessId());
        hGlobalMutex = CreateMutex(NULL, FALSE, mutex_name);

        // We created the handle and it is now open, so register a helper that can close it at exit
        // (which since we are probably in a DLL would usually be when this DLL is being unloaded)
        atexit(AtExitFreeHandle);
    }

    WaitForSingleObject(hGlobalMutex, INFINITE);
}

void ThreadSpecificPtrImpl::TLSGlobalMutex::unlock()
{

    ReleaseMutex(hGlobalMutex);
}

void ThreadSpecificPtrImpl::ThreadTLSEntries::init()
{
    hGlobalMemory = NULL;
    pGlobalMemory = NULL;
    m_refcount = 0;
    //_RPT2(_CRT_WARN, "ThreadTLSEntries of %x in %x\n", this, GetCurrentThreadId());
}

ThreadSpecificPtrImpl::TLSSentry *ThreadSpecificPtrImpl::ThreadTLSEntries::get_list_head(bool create)
{
    //_RPT2(_CRT_WARN, "get_list_head of %x in %x\n", this, GetCurrentThreadId());

    // Check if anyone has initialised the handle yet.
    // This avoids doing a lock if the handle already exists (and it usually does!)
    if (create && !hGlobalMemory)
    {
        enum { MAX_CHARS_FOR_TLS_MEMORY_NAME = 32 };
        TCHAR memmap_name[MAX_CHARS_FOR_TLS_MEMORY_NAME];
        _stprintf(memmap_name, _T("TLS Memory %08x"), GetCurrentProcessId());

        // Need a global lock because we're changing global memmap object...
        // This needs to be done before checking the handle value again, because otherwise a race condition ensues...
        tls_global_mutex.lock();
        if (!hGlobalMemory)
        {
            HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(*pGlobalMemory), memmap_name);
            bool created = (GetLastError() != ERROR_ALREADY_EXISTS);
            pGlobalMemory = (GlobalMemory *)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);
            if (created)
            {
                pGlobalMemory->refcount = 0;
                pGlobalMemory->tls_key = TlsAlloc();
                pGlobalMemory->sentry_list = NULL;
            }
            ++pGlobalMemory->refcount;
            hGlobalMemory = hFileMap;
        }
        tls_global_mutex.unlock();
    }

    TLSSentry *head = hGlobalMemory ? (TLSSentry *)TlsGetValue(pGlobalMemory->tls_key) : NULL;

    if (create && !head)
    {
        head = (TLSSentry *) GlobalAlloc(GMEM_FIXED, sizeof(TLSSentry));
        //_RPT3(_CRT_WARN, "allocated TLSSentry at %x for %x in %x\n", head, this, GetCurrentThreadId());
        head->m_head = NULL;

        // need mutex because we're changing global sentry list.
        tls_global_mutex.lock();
        {
            head->m_next = pGlobalMemory->sentry_list;
            head->m_prev = NULL;
            if (head->m_next)
            {
                head->m_next->m_prev = head;
            }
            pGlobalMemory->sentry_list = head;
        }
        tls_global_mutex.unlock();

        TlsSetValue(pGlobalMemory->tls_key, (void *)head);
    }
    return head;
}

void ThreadSpecificPtrImpl::ThreadTLSEntries::set_list_head(TLSSentry *new_sentry)
{
    ASSERT(pGlobalMemory || !new_sentry);
    if (hGlobalMemory && pGlobalMemory)
    {
        pGlobalMemory->sentry_list = new_sentry;
    }
}

void ThreadSpecificPtrImpl::ThreadTLSEntries::destroy()
{
    //_RPT2(_CRT_WARN, "~ThreadTLSEntries of %x in %x\n", this, GetCurrentThreadId());

    HANDLE hFileMap = hGlobalMemory;
    if (hFileMap)
    {
        // only want to be destroyed once
        hGlobalMemory = NULL;

        // need mutex because we're changing global memmap object
        tls_global_mutex.lock();
        {
            if (--pGlobalMemory->refcount == 0)
            {
                while (pGlobalMemory->sentry_list)
                {
                    TLSSentry *n = pGlobalMemory->sentry_list;
                    pGlobalMemory->sentry_list = n->m_next;
                    n->destroy();
                    GlobalFree(n);
                    //_RPT3(_CRT_WARN, "freeing TLSSentry at %x in %x in %x\n", n, this, GetCurrentThreadId());
                }

                TlsFree(pGlobalMemory->tls_key);
            }
            UnmapViewOfFile(pGlobalMemory);
            pGlobalMemory = NULL;
            CloseHandle(hFileMap);
        }
        tls_global_mutex.unlock();
    }
}

ThreadSpecificPtrImpl::ThreadSpecificPtrImpl(void(*cleanup)(void *)) :
    m_cleanup(cleanup),
    m_key(TlsAlloc()),
    m_var_tls_entries(NULL)
{
    //_RPT2(_CRT_WARN, "ThreadSpecificPtrImpl of %x in %x\n", this, GetCurrentThreadId());

    if (!thread_tls_entries)
    {
        thread_tls_entries = static_cast<ThreadTLSEntries*>(GlobalAlloc(GMEM_FIXED, sizeof(ThreadTLSEntries)));
        //_RPT3(_CRT_WARN, "allocated ThreadTLSEntries at %x for %x in %x\n", thread_tls_entries, this, GetCurrentThreadId());
        thread_tls_entries->init();
    }
    ++thread_tls_entries->m_refcount;
}

ThreadSpecificPtrImpl::~ThreadSpecificPtrImpl()
{
    //_RPT2(_CRT_WARN, "~ThreadSpecificPtrImpl of %x in %x\n", this, GetCurrentThreadId());

    // if this is destructing it either means:
    // 1. we're in a DLL that's being unloaded - we should clean up for all threads that have used us.
    // 2. we're in the main exe and it's exiting - we should only have the main thread left and it's
    //    exiting so we should clean up.

    // keep going while there are entries to be removed
    TLSEntry *n = m_var_tls_entries;
    while (n)
    {
        // another thread may exit while we're unloading this DLL, so mutex required.
        tls_global_mutex.lock();
        {
            m_var_tls_entries = n->m_var_next;

            *n->m_thread_prevp = n->m_thread_next;
            if (n->m_thread_next)
            {
                n->m_thread_next->m_thread_prevp = n->m_thread_prevp;
            }
        }

        tls_global_mutex.unlock();

        try
        {
            if (n->m_tls->m_cleanup && n->m_value)
            {
                (*n->m_tls->m_cleanup)(n->m_value);
            }
        }
        catch (...) {}

        n->destroy();
        GlobalFree(n);
        //_RPT3(_CRT_WARN, "freeing TLSEntry at %x of %x in %x\n", n, this, GetCurrentThreadId());
        n = m_var_tls_entries;
    }

    TlsFree(m_key);

    if (thread_tls_entries && --thread_tls_entries->m_refcount == 0)
    {
        thread_tls_entries->destroy();
        GlobalFree(thread_tls_entries);
        //_RPT3(_CRT_WARN, "freeing ThreadTLSEntries at %x of %x in %x\n", thread_tls_entries, this, GetCurrentThreadId());
        thread_tls_entries = NULL;
    }
}

ThreadSpecificPtrImpl *new_thread_specific_ptr_impl(void (*cleanup)(void *))
{
    return new ThreadSpecificPtrImpl(cleanup);
}

void delete_thread_specific_ptr_impl(ThreadSpecificPtrImpl *p)
{
    delete p;
}

void thread_specific_ptr_impl_set(ThreadSpecificPtrImpl *me, void *value)
{
    me->set(value);
}

void ThreadSpecificPtrImpl::set(void *value)
{
    //_RPT3(_CRT_WARN, "tls set of %x to %x in %x\n", this, value, GetCurrentThreadId());

    TLSEntry *p = (TLSEntry *)TlsGetValue(m_key);
    if (!p)
    {
        p = (TLSEntry *) GlobalAlloc(GMEM_FIXED, sizeof(TLSEntry));
        //_RPT3(_CRT_WARN, "allocated TLSEntry at %x for %x in %x\n", p, this, GetCurrentThreadId());
        p->init(this, value);

        // we know the DLL isn't unloading and this thread isn't exiting, so no mutex for this.
        TLSSentry *sentry = thread_tls_entries->get_list_head(true);

        p->m_thread_next = sentry->m_head;
        sentry->m_head = p;
        p->m_thread_prevp = &sentry->m_head;
        if (p->m_thread_next)
        {
            p->m_thread_next->m_thread_prevp = &p->m_thread_next;
        }

        // now need mutex because we're adding to cross-thread tls list.
        tls_global_mutex.lock();
        {
            p->m_var_next = m_var_tls_entries;
            m_var_tls_entries = p;
            p->m_var_prevp = &m_var_tls_entries;
            if (p->m_var_next)
            {
                p->m_var_next->m_var_prevp = &p->m_var_next;
            }
        }
        tls_global_mutex.unlock();

        TlsSetValue(m_key, (void *) p);
    }
    else
    {
        // we know the DLL isn't unloading and this thread isn't exiting, so no mutex for this.
        p->m_value = value;
    }
}

void* thread_specific_ptr_impl_get(ThreadSpecificPtrImpl *me)
{
    return me->get();
}

void *ThreadSpecificPtrImpl::get()
{
    TLSEntry *p = (TLSEntry *)TlsGetValue(m_key);
    return p ? p->m_value : NULL;
}

void ThreadSpecificPtrImpl::ThreadCleanup()
{
    //_RPT2(_CRT_WARN, "ThreadCleanup using %x in %x\n", thread_tls_entries, GetCurrentThreadId());

    TLSSentry *sentry = thread_tls_entries ? thread_tls_entries->get_list_head(false) : NULL;
    // sentry is only deleted on thread exit (which we're doing now) and when all thread DLLs have unloaded
    // (which can't be the case because we're still here), so no mutex required yet, so don't have to worry
    // about sentry disappearing entirely while we're unlocked.
    if (sentry)
    {
        // however sentry's data could be changed, so we need a lock now.
        tls_global_mutex.lock();
        while(sentry->m_head)
        {
            TLSEntry *n = sentry->m_head;
            sentry->m_head = n->m_thread_next;
            *n->m_var_prevp = n->m_var_next;
            if (n->m_var_next)
            {
                n->m_var_next->m_var_prevp = n->m_var_prevp;
            }

            tls_global_mutex.unlock();
            try
            {
                if (n->m_tls->m_cleanup && n->m_value)
                {
                    (*n->m_tls->m_cleanup)(n->m_value);
                }
            }
            catch (...) {}

            n->destroy();
            GlobalFree(n);
            //_RPT2(_CRT_WARN, "freeing TLSEntry at %x in %x\n", n, GetCurrentThreadId());
            tls_global_mutex.lock();
        }

        if (sentry->m_prev)
        {
            sentry->m_prev->m_next = sentry->m_next;
        }
        else
        {
            thread_tls_entries->set_list_head(sentry->m_next);
        }

        if (sentry->m_next)
        {
            sentry->m_next->m_prev = sentry->m_prev;
        }
        tls_global_mutex.unlock();

        // sentry has been removed from global sentry list, and thread local pointer to it will disappear with us - time to delete the sentry.
        sentry->destroy();
        GlobalFree(sentry);
        //_RPT2(_CRT_WARN, "freeing TLSSentry at %x in %x\n", sentry, GetCurrentThreadId());
    }
}
