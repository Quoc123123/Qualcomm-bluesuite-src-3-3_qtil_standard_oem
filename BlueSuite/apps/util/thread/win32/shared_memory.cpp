/**********************************************************************
*
* shared_memory.cpp
*
* Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* Shared memory class (Windows implementation).
*
**********************************************************************/
#include "thread/shared_memory.h"
#include <windows.h>
#include <tchar.h>

static const char prefix[] = "csr_shm_";

class SharedMemory::Impl
{
public:
    Impl() : mFileMapping(NULL), mMemory(NULL) {}
    HANDLE mFileMapping;
    void *mMemory;
};

SharedMemory::SharedMemory(const char *name, size_t len) 
: mpImpl(new Impl), mGlobalMutex(name), mCreated(false), mName(name), mLen(len)
{
}

SharedMemory::~SharedMemory()
{
    if (mpImpl->mMemory)
    {
        UnmapViewOfFile(mpImpl->mMemory);
    }
    if (mpImpl->mFileMapping)
    {
        CloseHandle(mpImpl->mFileMapping);
    }
}

void *SharedMemory::ptr()
{
    if (!mpImpl->mMemory)
    {
        CriticalSection::Lock locallock(&mLocalMutex);
        SystemWideMutexLock swmlock(&mGlobalMutex);
        TCHAR *tname = new TCHAR[sizeof(prefix)+mName.size()];
        _stprintf(tname, TEXT("%hs%hs"), prefix, mName.c_str());
        mpImpl->mFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, static_cast<DWORD>(mLen), tname);
        mCreated = (GetLastError() != ERROR_ALREADY_EXISTS);
        delete [] tname;
        if (mpImpl->mFileMapping != NULL)
        {
            mpImpl->mMemory = MapViewOfFile(mpImpl->mFileMapping, FILE_MAP_WRITE, 0, 0, mLen);
            if (!mpImpl->mMemory)
            {
                CloseHandle(mpImpl->mFileMapping);
                mpImpl->mFileMapping = NULL;
            }
        }
    }
    return mpImpl->mMemory;
}
