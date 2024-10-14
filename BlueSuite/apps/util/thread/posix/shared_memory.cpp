/**********************************************************************
*
* shared_memory.cpp
*
* Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* Shared memory class (Posix implementation).
*
**********************************************************************/
#include "thread/shared_memory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits>
#include <stdint.h>

static const char prefix[] = "/csr_shm_";

typedef uint32_t refcount;

class SharedMemory::Impl
{
public:
    Impl() : mShmFd(-1), mShmName(NULL), mShmMem(NULL) {}
    int mShmFd;
    char *mShmName;
    void *mShmMem;
};

SharedMemory::SharedMemory(const char *name, size_t len) 
: mpImpl(new Impl), mGlobalMutex(name), mCreated(false), mName(name), mLen(len)
{
}

SharedMemory::~SharedMemory()
{
    if (mpImpl->mShmMem)
    {
        SystemWideMutexLock lock(&mGlobalMutex);
        if (--((refcount *)mpImpl->mShmMem)[0] == 0)
        {
            shm_unlink(mpImpl->mShmName);
        }
        munmap(mpImpl->mShmMem, mLen+1);
    }
    delete [] mpImpl->mShmName;
    if (mpImpl->mShmFd != -1)
    {
        close(mpImpl->mShmFd);
    }
}

void *SharedMemory::ptr()
{
    if (!mpImpl->mShmMem)
    {
        static const int perms = 0666;
		static const int umaskPerms = 0111;		// rw for user, group, others
		
        CriticalSection::Lock locallock(&mLocalMutex);
        SystemWideMutexLock swmlock(&mGlobalMutex);
        mpImpl->mShmName = new char[sizeof(prefix)+mName.size()];
        sprintf(mpImpl->mShmName, "%s%s", prefix, mName.c_str());
        
		// set shared object's file permission as described for umaskPerms
		mode_t currentMask = umask(umaskPerms);
		
		mpImpl->mShmFd = shm_open(mpImpl->mShmName, O_RDWR|O_CREAT|O_EXCL, perms);
        
		// restore the umask as it was before setting it here
		umask(currentMask);	
		
		if (mpImpl->mShmFd == -1)
        {
            mCreated = false;
            mpImpl->mShmFd = shm_open(mpImpl->mShmName, O_RDWR, perms);
        }
        else
        {
            mCreated = true;
        }
        if (mpImpl->mShmFd != -1)
        {
            ftruncate(mpImpl->mShmFd, mLen + sizeof(refcount));
            mpImpl->mShmMem = mmap(0, mLen + sizeof(refcount), PROT_READ|PROT_WRITE, MAP_SHARED, mpImpl->mShmFd, 0);
            if (mpImpl->mShmMem == MAP_FAILED)
            {
                mpImpl->mShmMem = NULL;
            }
        }
        if (mpImpl->mShmMem && mCreated)
        {
            ((refcount *)mpImpl->mShmMem)[0] = 1;
        }
        else if (mpImpl->mShmMem && ((refcount *)mpImpl->mShmMem)[0] < std::numeric_limits<refcount>::max())
        {
            ++((refcount *)mpImpl->mShmMem)[0];
        }
        else
        {
            if (mCreated)
            {
                shm_unlink(mpImpl->mShmName);
            }
            delete [] mpImpl->mShmName;
            mpImpl->mShmName = NULL;
        }
    }
    return mpImpl->mShmMem ? (void *)((refcount *)mpImpl->mShmMem + 1) : NULL;
}

