///////////////////////////////////////////////////////////////////////
//
//  FILE   :  thread.cpp
//
//            Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  The POSIX version of the Threadable class.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/posix/thread.cpp#1 $
//
///////////////////////////////////////////////////////////////////////

#define __EXTENSIONS__ 1
#include "thread/thread.h"

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>

#if 1
#define MESSAGE(MSG) do { } while(0)
#else
#define MESSAGE(MSG) error_printf MSG
static void error_printf(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
#endif

static void fatal_error(const char *str, int err)
{
    fprintf(stderr, "%s: %s", str, strerror(err));
    abort();
}

//
// This is the class that holds all of the platform specific
// data (and also the static 'ThreadFunc' function).
//
class Threadable::ThreadData : private NoCopy
{
public:
    // The ID of the thread
    pthread_t       m_threadId;

    // These provide protection for the data in this structure
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;

    // All of the data below this point is protected by the mutex above.
    // The data items are setup in the constructor, and can each
    // be set at _ONE_ other point.

    // This is the thread exit code.  It is set to 0 at the start.  If
    // the thread is killed (does not exit when stop is called), then
    // it will remain at that value, otherwise it is set to the return
    // value of ''ThreadFunc''
    int m_exitCode;

    // This bool is set to true by the ctor.
    // The ''ThreadFunc'' is expected to check on it status (by
    // calling ''KeepGoing'') to see if it should keep going.  It
    // is set to false when ''Stop'' is called.
    bool m_keepGoing;

    // This enum contains the state of the child thread.  It used to
    // be two bool's.
    enum
    {
        stat_not_running,
        stat_waiting_to_start,
        stat_running
    } m_status;

    ThreadData();
    ~ThreadData();

    // Static member functions to do the casting
    // of 'LPVOID' to 'Threadable'
    static void* __ThreadFunker(void* lpThreadParameter);
    static void __ThreadCleanup(void* lpThreadParameter);

    bool _WaitForStop(unsigned millisec);
};

//
// Constructor
//
Threadable::ThreadData::ThreadData() :
    m_threadId(0),
    m_exitCode(0),
    m_keepGoing(true),
    m_status(stat_not_running)
 {
    int stat;

    stat = pthread_mutex_init ( &m_mutex, NULL );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::ThreadData - mutex init", stat);
    }

    stat = pthread_cond_init ( &m_cond , NULL );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::ThreadData - cond init", stat);
    }
}

Threadable::ThreadData::~ThreadData()
{
    int stat;

    stat = pthread_cond_destroy( &m_cond );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::~ThreadData - cond destroy", stat);
    }
    stat = pthread_mutex_destroy(&m_mutex);
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::~ThreadData - mutex destroy", stat);
    }
}

//
// This is the function that gets called to begin the new thread
// It casts the void* to a 'Threadable', and
// calls the 'ThreadFunc' function.
//
/*static*/
void* Threadable::ThreadData::__ThreadFunker(void* lpThreadParameter)
{
    int exitCode;
    int stat;

    // Get the pointer to the underlying 'Threadable'
    assert(lpThreadParameter != 0);
    Threadable* thread = static_cast<Threadable*>(lpThreadParameter);

    // This Lock/Unlock sequence is so that the set-up of the bool's in the
    // ''Start'' function is forced to happen.
    //
    stat = pthread_mutex_lock( &thread->m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::__ThreadFunker - mutex lock", stat);
    }

    // Set the event here saying that the thread is running
    assert(thread->m_data->m_status == ThreadData::stat_waiting_to_start);
    thread->m_data->m_status = ThreadData::stat_running;

    // Signal to any other threads
    stat = pthread_cond_signal( &thread->m_data->m_cond );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::__ThreadFunker - cond signal", stat);
    }

    stat = pthread_mutex_unlock( &thread->m_data->m_mutex );
    if(stat != 0)
    {
        // Problem!!
        fatal_error("Threadable::ThreadData::__ThreadFunker - mutex unlock", stat);
    }

    // Add the cleanup handler.  This does nothing but set a flag to say
    // that we have finished.  It is usefull to have this sort of info though.
    // After the thread has set this flag, it must exit, without using any
    // of the thread resources.
    pthread_cleanup_push(__ThreadCleanup, lpThreadParameter);

    MESSAGE(("Threadable::ThreadData::ThreadFunker(%p): "
               "Starting user function\n", 
               thread));

    // Run the ThreadFunc
    if (thread->m_data->m_keepGoing)
        exitCode = thread->ThreadFunc();
    else
        exitCode = 0;

    MESSAGE(("Threadable::ThreadData::ThreadFunker(%p): "
               "User function over\n", 
               thread));

    // save the exit code
    thread->m_data->m_exitCode = exitCode;

    // This will cause the cleanup handler to be called.  After this
    // point, the thread must assume that the data structure pointed to
    // by 'thread' does not exist any more!
    pthread_cleanup_pop(1);

    MESSAGE(("Threadable - Child thread has ended\n"));

    // Return the exit code
    return (void*)exitCode;
}

/*static*/
void Threadable::ThreadData::__ThreadCleanup(void* lpThreadParameter)
{
    int stat;

    // Get the pointer to the underlying 'Threadable'
    assert(lpThreadParameter != 0);
    Threadable* thread = static_cast<Threadable*>(lpThreadParameter);

    MESSAGE(("ThreadCleanup(%p)\n", thread));

    stat = pthread_mutex_lock( &thread->m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::__ThreadCleanup - mutex lock", stat);
    }

    // This thread has now ended!
    thread->m_data->m_keepGoing = false;
    assert(thread->m_data->m_status == ThreadData::stat_running);
    thread->m_data->m_status = ThreadData::stat_not_running;

    // Signal to any other threads
    stat = pthread_cond_signal( &thread->m_data->m_cond );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::__ThreadCleanup - cond signal", stat);
    }

    // This unlock is the last possible use of the 'thread' data structure!
    stat = pthread_mutex_unlock( &thread->m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::ThreadData::__ThreadCleanup - mutex unlock", stat);
    }
}

//
// Create a new 'Threadable' object
//
Threadable::Threadable()
{
    m_data = new ThreadData();
    assert(m_data != 0);

    MESSAGE(("Threadable::Threadable(%p)\n", this));
}

//
// This will hopefully stop the thread if it is running, and
// clean up any data that has been allocated
//
Threadable::~Threadable()
{
    assert(m_data != 0);

    Kill();

    /* If we delete this, with the thread still runnning
     * then we will crash.  We might as well wait here for a while then.
     */
    while(IsActive())
        /*EMPTY*/;

    delete m_data;

    MESSAGE(("Threadable::~Threadable(%p)\n", this));
}

//
// Wait up to millisec for the thread to stop
// m_data->m_mutex should be locked when calling this
//
bool Threadable::ThreadData::_WaitForStop(unsigned millisec)
{
    int stat;

    if (m_status == ThreadData::stat_not_running)
        return true;

    assert(m_status == ThreadData::stat_waiting_to_start ||
           m_status == ThreadData::stat_running);

    if (millisec > 0)
    {
        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, 0);
        timeout.tv_sec = now.tv_sec + (millisec / 1000);
        timeout.tv_nsec = now.tv_usec * 1000 + (millisec % 1000) * 1000000;
        while(timeout.tv_nsec > 1000000000)
        {
            timeout.tv_nsec -= 1000000000;
            timeout.tv_sec  += 1;
        }
        // timeout is when we will stop waiting
        do
        {
            stat = pthread_cond_timedwait(&m_cond,
                              &m_mutex,
                              &timeout);

            if (stat != ETIMEDOUT && stat != 0)
            {
                MESSAGE(("Threadable::_WaitForStop(%p): "
                       "Failed to signal cond variable? %d\n",
                       this, stat));
            }
        } while (((m_status == ThreadData::stat_waiting_to_start)
            || (m_status == ThreadData::stat_running))
            && stat != ETIMEDOUT);
    }
    else
    {
        do
        {
            stat = pthread_cond_wait(&m_cond,
                              &m_mutex);

            if (stat != 0)
            {
                MESSAGE(("Threadable::_WaitForStop(%p): "
                       "Failed to signal cond variable? %d\n",
                       this, stat));
            }
        } while (((m_status == ThreadData::stat_waiting_to_start)
            || (m_status == ThreadData::stat_running)));
    }

    // pthreads have sensible cleanup functionality, so all that remains is to
    // return the result
    if (stat == ETIMEDOUT)
        return false;

    return true;
}

//
// This will start the new thread running, returns 'true'
// if everything is OK.
//
bool Threadable::Start()
{
    int stat;

    assert(m_data != 0);

    MESSAGE(("Threadable::Start\n"));

    stat = pthread_mutex_lock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::Start - mutex lock", stat);
    }

    assert(m_data->m_status == ThreadData::stat_not_running
           || !m_data->m_keepGoing);
    if (m_data->m_status != ThreadData::stat_not_running)
    {
        if (m_data->m_keepGoing)
        {
            stat = pthread_mutex_unlock( &m_data->m_mutex );
            if(stat != 0)
            {
                fatal_error("Threadable::Start - mutex unlock early", stat);
            }
            return false;
        }
        else
        {
            // Stopping, wait for us to finish and cleanup
            m_data->_WaitForStop(0);
        }
    }

    m_data->m_keepGoing = true;

    stat = pthread_create ( &m_data->m_threadId,
                            0,
                            ThreadData::__ThreadFunker,
                            (void*)this );
    if (stat != 0)
    {
        // Problem!!
        MESSAGE(("Create Thread failed! %x\n", stat));

        // Pretend that the thread has ended already
        m_data->m_status = ThreadData::stat_not_running;
    }
    else
    {
        pthread_detach (m_data->m_threadId);

        m_data->m_status = ThreadData::stat_waiting_to_start;
    }

    if (m_data->m_status == ThreadData::stat_waiting_to_start)
    {
        // There is a limit to how long we will wait.
        const int millisec = 1000;
        struct timeval now;
        gettimeofday(&now, 0);
        struct timespec timeout;
        timeout.tv_sec = now.tv_sec + (millisec / 1000);
        timeout.tv_nsec = now.tv_usec * 1000 + (millisec % 1000) * 1000000;
        while(timeout.tv_nsec > 1000000000)
        {
            timeout.tv_nsec -= 1000000000;
            timeout.tv_sec  += 1;
        }
        // timeout is when we will stop waiting
        do
        {
            MESSAGE(("Threadable::Start(%p): "
                       "Starting cond_timedwait\n",
                       this));
        
            stat = pthread_cond_timedwait(&m_data->m_cond,
                                          &m_data->m_mutex,
                                          &timeout);
        
            MESSAGE(("Threadable::Start(%p): "
                       "Done cond_timedwait %d\n",
                       this, stat));
        
            if(stat != ETIMEDOUT && stat != 0)
            {
                MESSAGE(("Threadable::Start(%p): "
                           "Failed to signal cond variable? %d\n",
                           this, stat));
            }
        } while ((m_data->m_status == ThreadData::stat_waiting_to_start)
                 && (stat != ETIMEDOUT));

        // At this point, we have either timed out, or we have
        // been signalled
    }

    // Actually run the thread now!
    stat = pthread_mutex_unlock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::Start - mutex unlock end", stat);
    }

    return true;
}

//
// Returns 'true' if the thread is running
//
bool Threadable::IsActive()
{
    int stat;
    bool isRunning;

    assert(m_data != 0);

    stat = pthread_mutex_lock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::IsActive - mutex lock", stat);
    }

    isRunning = m_data->m_status != ThreadData::stat_not_running;

    stat = pthread_mutex_unlock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::IsActive - mutex unlock", stat);
    }

    // Are we running the other thread.
    return isRunning;
}

//
// Returns the exit code of the thread
//
int Threadable::GetExitCode()
{
    assert(m_data != 0);

    return m_data->m_exitCode;
}

//
// Should the run function keep going?
//
bool Threadable::KeepGoing()
{
    int stat;
    bool keepGoing;

    assert(m_data != 0);

    stat = pthread_mutex_lock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::KeepGoing - mutex lock", stat);
    }

    keepGoing = m_data->m_keepGoing;

    stat = pthread_mutex_unlock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::KeepGoing - mutex unlock", stat);
    }

    return keepGoing;
}

//
// Signal to the run function that we should stop soon.
//
void Threadable::Stop()
{
    int stat;

    assert(m_data != 0);

    MESSAGE(("Threadable::Stop(%p)\n", this));

    stat = pthread_mutex_lock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::Stop - mutex lock", stat);
    }
    
    m_data->m_keepGoing = false;

    stat = pthread_mutex_unlock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::Stop - mutex unlock", stat);
    }
}

//
// _WaitForStop for the end user
//
bool Threadable::WaitForStop(unsigned millisec)
{
    int stat;
    bool res;
    
    assert(m_data != 0);

    stat = pthread_mutex_lock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::WaitForStop - mutex lock", stat);
    }

    res = m_data->_WaitForStop(millisec);

    stat = pthread_mutex_unlock( &m_data->m_mutex );
    if (stat != 0)
    {
        fatal_error("Threadable::WaitForStop - mutex unlock", stat);
    }

    return res;
}

//
// Wait for the other thread to end.
// If it does not stop by itself after the timeout then we take
// drastic measures (not that drastic on a nice POSIX system)
//
void Threadable::Kill(unsigned millisec /*= 1000*/)
{
    int stat;

    assert(m_data != 0);

    Stop();

    stat = pthread_mutex_lock( &m_data->m_mutex );
    if(stat != 0)
    {
        fatal_error("Threadable::Kill - mutex lock", stat);
    }

    if (!m_data->_WaitForStop(millisec))
    {
        // Have to kill ourselves
        stat = pthread_cancel( m_data->m_threadId );
        if (stat != 0)
        {
            fatal_error("Threadable::Kill - cancel", stat);
        }
    }

    stat = pthread_mutex_unlock( &m_data->m_mutex );
    if (stat != 0)
    {
        fatal_error("Threadable::Kill - mutex unlock", stat);
    }
}


// Lower the threads priority a couple of points
void Threadable::LowPriority()
{
}

// Set priority back to normal
void Threadable::NormalPriority()
{
}

// Make the thread have more priority
void Threadable::HighPriority()
{
}

struct ThreadID::Impl
{
    pthread_t id;
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
    return pthread_equal(impl->id, x.impl->id);
}

ThreadID ThreadID::self()
{
    ThreadID r;
    r.impl->id = pthread_self();
    return r;
}

unsigned long ThreadID::Id()
{
    return (unsigned long) pthread_self();
}
static pthread_t main_thread_id = pthread_self();

class ThreadSpecificPtrImpl
{
public:
    static pthread_key_t global_key;
    static pthread_once_t global_key_once;

    mutable pthread_key_t m_key;
    mutable pthread_once_t m_key_once;
    mutable bool m_key_set;

    ThreadSpecificPtrImpl(void (*cleanup)(void *)) : m_key_once(PTHREAD_ONCE_INIT), m_key_set(false), m_cleanup(cleanup), m_main_thread_instance(NULL) {}
    ~ThreadSpecificPtrImpl();

    void init();

    void* get();
    void set(void *);

    void (* m_cleanup)(void *);
    void *m_main_thread_instance;

    static void set_global_key();
    static void set_key();
};

pthread_key_t ThreadSpecificPtrImpl::global_key;
pthread_once_t ThreadSpecificPtrImpl::global_key_once = PTHREAD_ONCE_INIT;

ThreadSpecificPtrImpl *new_thread_specific_ptr_impl(void (*cleanup)(void *))
{
    return new ThreadSpecificPtrImpl(cleanup);
}

void delete_thread_specific_ptr_impl(ThreadSpecificPtrImpl *p)
{
    delete p;
}

void ThreadSpecificPtrImpl::set_global_key()
{
    pthread_key_create(&global_key, NULL);
}

void ThreadSpecificPtrImpl::set_key()
{
    ThreadSpecificPtrImpl *me = static_cast<ThreadSpecificPtrImpl*>(pthread_getspecific(global_key));
    pthread_key_create(&me->m_key, me->m_cleanup);
    me->m_key_set = true;
}

void ThreadSpecificPtrImpl::init()
{
    pthread_once(&global_key_once, set_global_key);
    pthread_setspecific(global_key, (void *) this);
    pthread_once(&m_key_once, set_key);
}

void* thread_specific_ptr_impl_get(ThreadSpecificPtrImpl *me)
{
    me->init();
    return pthread_equal(pthread_self(), main_thread_id) ? me->m_main_thread_instance : pthread_getspecific(me->m_key);
}

void thread_specific_ptr_impl_set(ThreadSpecificPtrImpl *me, void *value)
{
    me->init();
    if (pthread_equal(pthread_self(), main_thread_id)) me->m_main_thread_instance = value;
    else pthread_setspecific(me->m_key, (void *) value);
}

ThreadSpecificPtrImpl::~ThreadSpecificPtrImpl()
{
    if(m_key_set) pthread_key_delete(m_key);
    if (m_main_thread_instance && m_cleanup) (*m_cleanup)(m_main_thread_instance);
}

