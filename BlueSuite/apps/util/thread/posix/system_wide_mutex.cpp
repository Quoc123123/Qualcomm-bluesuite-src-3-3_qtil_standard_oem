// Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// Linux implememtation of the Mutex. Uses the System IV semaphores.
//
// $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/posix/system_wide_mutex.cpp#1 $

#include "thread/system_wide_mutex.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

// m_acquired is the golden version of this bool. The bool in the generic
// wrapper mirrors this one (they should both be the same). The destructor
// of the MutexData needs to know whether we own the mutex or not.
class SystemWideMutex::MutexData
{
public:
    MutexData(const char *mutex_name);
    ~MutexData();

    bool OpenMutex();

    bool Acquire(uint32 milliseconds);
    void Release();

    char *m_mutex_name;
    int m_proj_id;
    unsigned m_acquired;
    int m_shr_sem;
    bool m_creator;

    void print_error(const char *str);
};

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// union semun is defined by including <sys/sem.h>
#else
// According to X/OPEN we have to define it ourselves.
union semun {
    int val;                // Value for SETVAL.
    struct semid_ds *buf;   // Buffer for IPC_STAT, IPC_SET.
    unsigned short *array;  // Array for GETALL, SETALL.
    // Linux specific part:
    struct seminfo *__buf;  // Buffer for IPC_INFO.
};
#endif

// We need to create a semaphore key. The best way to do that is to
// base it on a file (according to documentation). So we
// create a file in /tmp/ that is hopefully unique enough but not too
// unique.
SystemWideMutex::MutexData::MutexData(const char *mutex_name)
    :
    m_acquired(0),
    m_shr_sem(-1)
{
    static const char prefix[] = "/tmp/csr_mutex_";

    m_mutex_name = (char *) malloc(strlen(mutex_name) + strlen(prefix) + 1);
    sprintf(m_mutex_name, "%s%s", prefix, mutex_name);

    char *sp = m_mutex_name + strlen(prefix);
    m_proj_id = 0;
    while (*sp)
    {
        if (strchr(".\\/\"*? \t\n\r", *sp))
        {
            m_proj_id = ((m_proj_id << 4) ^ *sp) % 255;
            *sp = '-';
        }
        ++sp;
    }
    ++m_proj_id;
}

SystemWideMutex::MutexData::~MutexData()
{
    if (!(m_shr_sem == -1) && m_creator)
    {
        // We created the semaphore, so we should delete it.
        // We need to acquire it first to make sure we don't delete it while someone else is in their critical section.
        if (!m_acquired)
        {
            struct sembuf semBuf;
            semBuf.sem_num = 0;
            semBuf.sem_op = -1;
            semBuf.sem_flg = SEM_UNDO;
            if (semop(m_shr_sem, &semBuf, 1) == -1)
            {
                print_error("Failed to acquire semaphore before deletion");
            }
        }

        int fd = open(m_mutex_name, O_WRONLY); // Keep the file in existence until we've deleted the semaphore object.
        if (fd == -1)
        {
            print_error("Failed to open semaphore file");
        }
        if (unlink(m_mutex_name) != 0)
        {
            print_error("Failed to delete semaphore file");
        }
        if (semctl(m_shr_sem, 0, IPC_RMID) == -1)
        {
            print_error("Failed to delete semaphore");
        }
        close(fd);
    }
    else
    {
        while (m_acquired)
        {
            Release();
        }
    }

    free(m_mutex_name);
}

bool SystemWideMutex::MutexData::OpenMutex()
{
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    for (unsigned int attempts = 0; attempts < 5 && m_shr_sem == -1; ++attempts)
    {
        // First see if the file exists already.
        key_t semkey = ftok(m_mutex_name, m_proj_id);
        if (semkey == -1)
        {
            // Couldn't get a token, probably because the file doesn't exist.
            int fd = -1;
            char *temp_name = (char *) malloc(strlen(m_mutex_name) + 6 + 2 + 1);
            for (unsigned int file_attempts = 0; file_attempts < 100 && fd==-1; ++file_attempts)
            {
                sprintf(temp_name, "%s%06d%02u", m_mutex_name, getpid() % 1000000, file_attempts);
                fd = open(temp_name, O_CREAT | O_EXCL | O_WRONLY, perms);
            }
            if (fd == -1)
            {
                print_error("Failed to create semaphore temporary file");
            }
            else
            {
                fchmod(fd, perms);
                close(fd);
                // Temp file created, now get the token and create the semaphore.
                semkey = ftok(temp_name, m_proj_id);
                if (semkey == -1)
                {
                    print_error("Failed to get semaphore token");
                }
                else
                {
                    m_shr_sem = semget(semkey, 1, IPC_CREAT | perms);
                    if (m_shr_sem == -1)
                    {
                        print_error("Failed to create semaphore");
                    }
                    else
                    {
                        union semun arg;
                        arg.val = 1;
                        if (semctl(m_shr_sem, 0, SETVAL, arg) == -1)
                        {
                            print_error("Failed to set initial value of semaphore");
                            semctl(m_shr_sem, 0, IPC_RMID);
                            m_shr_sem = -1;
                        }
                        else
                        {
                            if (link(temp_name, m_mutex_name) != 0)
                            {
                                // This could mean someone else has created the semaphore while we were, not necessarily an error.
                                //fprintf(stderr, "Semaphore create stalled - retrying.\n");
                                semctl(m_shr_sem, 0, IPC_RMID);
                                m_shr_sem = -1;
                            }
                            else
                            {
                                // We linked the file to the new location. semaphore created!
                                //fprintf(stderr, "Semaphore created.\n");
                                m_creator = true;
                            }
                        }
                    }
                }
                unlink(temp_name);
            }
            free(temp_name);
        }
        else
        {
            m_shr_sem = semget(semkey, 1, 0);
            if (m_shr_sem == -1)
            {
                switch (errno)
                {
                case EACCES:
                    fprintf(stderr, "The shared resource is opened by another user and is not configured to share with you.");
                    attempts = 1; // Intransient error, don't try again.
                    break;
                case ENOENT: // No semaphore set exists for semkey and semflg (last arg in semget) did not specify IPC_CREAT.
                    (void)unlink(m_mutex_name);
                    break;
                default:
                    print_error("Failed to open semaphore");
                    break;
                }
            }
            else
            {
                // Semaphore opened!
                //fprintf(stderr, "Existing semaphore opened.\n");
                m_creator = false;
            }
        }
    }

    if (m_shr_sem == -1 && access(m_mutex_name, F_OK)==0)
    {
        fprintf(stderr, "Failed to open semaphore but lock file %s exists - lock file may be stale.\n", m_mutex_name);
    }

    return !(m_shr_sem == -1);
}

bool SystemWideMutex::MutexData::Acquire(uint32 milliseconds)
{
    struct sembuf semBuf;
    timespec ts;

    if (m_acquired)
    {
        // We already have it. Up the count and do nothing.
        ++m_acquired;
    }
    else
    {
        unsigned int attempts = 5;
        do
        {
            (void)OpenMutex();

            if (m_shr_sem != -1)
            {
                if (milliseconds != NO_TIMEOUT)
                {
                    ts.tv_sec = milliseconds / 1000;
                    ts.tv_nsec = (milliseconds % 1000) * 1000000;
                }

                semBuf.sem_num = 0;
                semBuf.sem_op = -1;
                semBuf.sem_flg = SEM_UNDO;
#ifdef __UCLIBC__
                /* UCLIBC doesn't have semtimedop, so fake it using semop. */
                if (semop(m_shr_sem, &semBuf, 1) != 0)
#else
                if (semtimedop(m_shr_sem, &semBuf, 1, milliseconds == NO_TIMEOUT ? NULL : &ts) != 0)
#endif
                {
                    switch (errno)
                    {
                    case EAGAIN:
                        if (milliseconds != NO_TIMEOUT)
                        {
                            //fprintf(stderr, "Timed out waiting for semaphore");
                            attempts = 1; // Bona fide timeout, don't try again.
                        }
                        break;
                    case EIDRM:
                    case EINVAL:
                        // Semaphore doesn't exist.
                        //fprintf(stderr, "Semaphore disappeared: %d.\n", errno);
                        if (m_creator) print_error("Owned semaphore disappeared");
                        m_shr_sem = -1;
                        break;
                    default:
                        print_error("Failed to get semaphore");
                        break;
                    }
                }
                else
                {
                    m_acquired = 1;
                }
            }
        } while (--attempts > 0 && !m_acquired);
    }
    return m_acquired ? true : false;
}

void SystemWideMutex::MutexData::Release()
{
    if (m_shr_sem == -1)
    {
        return;
    }

    if (m_acquired)
    {
        if (--m_acquired == 0)
        {
            //fprintf(stderr, "Freeing semaphore.\n");

            struct sembuf semBuf;

            semBuf.sem_num = 0;
            semBuf.sem_op = 1;
            semBuf.sem_flg = SEM_UNDO;
            if (semop(m_shr_sem, &semBuf, 1) != 0)
            {
                print_error("Failed to free semaphore");
                ++m_acquired;
            }
        }
    }
}

void SystemWideMutex::MutexData::print_error(const char *str)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s for mutex %s", str, m_mutex_name);
    perror(buf);
}

SystemWideMutex::SystemWideMutex(const char *mutex_name, bool acquire, bool exclusive)
    :
    data_(new MutexData(mutex_name))
{
    if (acquire)
    {
        Acquire(NO_TIMEOUT);
    }
}

SystemWideMutex::~SystemWideMutex()
{
    delete data_;
    data_ = NULL;
}

bool SystemWideMutex::Acquire(uint32 milliseconds)
{
    return data_->Acquire(milliseconds);
}

void SystemWideMutex::Release()
{
    data_->Release();
}

bool SystemWideMutex::IsAcquired() const
{
    return data_->m_acquired ? true : false;
}
