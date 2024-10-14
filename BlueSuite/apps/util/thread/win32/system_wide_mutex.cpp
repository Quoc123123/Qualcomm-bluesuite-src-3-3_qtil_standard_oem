// Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// Win32 implememtation of the Mutex. Uses the CreateMutex function.
//
// $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/win32/system_wide_mutex.cpp#1 $

#include "thread/system_wide_mutex.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <tchar.h>

#include "thread/error_msg.h"

#if defined(MUTEX_LOG) && defined(EXTRA_DEBUG)
#define DATA_LOG_MSG(x) WriteLog(sprintf_static(x))
#define LOG_MSG(x) data_->DATA_LOG_MSG(x)
#else
#define DATA_LOG_MSG(x) do { } while(0)
#define LOG_MSG(x) DATA_LOG_MSG(x)
#endif 

class SystemWideMutex::MutexData
{
public:
    MutexData(const char * mutex_name, bool exclusive);
    MutexData(const wchar_t * mutex_name, bool exclusive);
    ~MutexData();

    bool Acquire(uint32 milliseconds);
    void Release(bool ignore_error);

    const bool m_exclusive;
    HANDLE m_handle;
    unsigned m_acquired;

#ifdef MUTEX_LOG
    void WriteLog(const char *string);
    bool enable_logging;
#endif
};

SystemWideMutex::MutexData::MutexData(const char *mutex_name, bool exclusive)
    :
    m_exclusive(exclusive),
    m_handle(INVALID_HANDLE_VALUE),
    m_acquired(0)
{
    static const char prefix[] = "csr_mutex_";
    // This call previously used _alloca.  It is probably better to do so,
    // but I'm a coward when it comes to new things... this string gets
    // deleted at the end of the function.

#ifdef MUTEX_LOG
    enable_logging = (strcmp(mutex_name, "spi_app_mutex" ) == 0);
#endif

    TCHAR * string = new TCHAR [ strlen(mutex_name) + strlen(prefix) + 1 ];
    _stprintf(string, TEXT("%hs%hs"), prefix, mutex_name);

    TCHAR *sp = string;
    while (*sp)
    {
        if (_tcschr(TEXT(".\\/\"*"), *sp))
            *sp = '-';
        sp++;
    }
    m_handle = CreateMutex(NULL, 0, string);
    OUTPUT_HANDLE_CREATE(m_handle);

    // OutputDebugString("Mutex created:");
    // OutputDebugString(string);
    // OutputDebugString("\n");
    // int lasterr = GetLastError();
    // assert(m_handle);
    delete[] string;
}

SystemWideMutex::MutexData::MutexData(const wchar_t *mutex_name, bool exclusive)
    :
    m_exclusive(exclusive),
    m_handle(INVALID_HANDLE_VALUE),
    m_acquired(0)
{
    static const wchar_t prefix[] = L"csr_mutex_";
    // This call previously used _alloca.  It is probably better to do so,
    // but I'm a coward when it comes to new things... this string gets
    // deleted at the end of the function.

#ifdef MUTEX_LOG
    enable_logging = (wcscmp(mutex_name, L"spi_app_mutex" ) == 0);
#endif

    wchar_t * string = new wchar_t [ wcslen(mutex_name) + wcslen(prefix) + 1 ];
    wcscpy(string, prefix);
    wcscat(string, mutex_name);

    wchar_t *sp = string;
    while (*sp)
    {
        if (wcschr(L".\\/\"*", *sp))
            *sp = L'-';
        sp++;
    }
    m_handle = CreateMutexW(NULL, 0, string);
    OUTPUT_HANDLE_CREATE(m_handle);

    // OutputDebugString("Mutex created:");
    // OutputDebugString(string);
    // OutputDebugString("\n");
    // int lasterr = GetLastError();
    // assert(m_handle);
    delete[] string;
}

SystemWideMutex::MutexData::~MutexData()
{
    CloseHandle(m_handle);
    OUTPUT_HANDLE_CLOSE(m_handle);
}

bool SystemWideMutex::MutexData::Acquire(uint32 milliseconds)
{
    if (!m_exclusive && m_acquired > 0)
    {
        ++m_acquired;
    }
    else
    {
        if (NO_TIMEOUT != INFINITE && milliseconds == NO_TIMEOUT) milliseconds = INFINITE;

        DWORD wait_result = WaitForSingleObject(m_handle, milliseconds);
        switch (wait_result)
        {
        case WAIT_ABANDONED:
            // Another thread died while holding the mutex, we now have it.
        case WAIT_OBJECT_0:
            // We have acquired the mutex.
            ++m_acquired;
            break;
        case WAIT_TIMEOUT:
            // We failed to get the mutex.
            break;
        default:
            // We should never get here?
            break;
        }

#ifdef MUTEX_LOG
        if (enable_logging)
        {
            if (wait_result == WAIT_ABANDONED || wait_result == WAIT_OBJECT_0)
            {
                LOG_MSG(("Acquired mutex"));
            }
            else
            {
                LOG_MSG(("Failed to acquire mutex"));
            }
        }
#endif
    }

    return m_acquired > 0;
}

void SystemWideMutex::MutexData::Release(bool ignore_error)
{
    if (m_acquired > 0)
    {
        if (--m_acquired == 0)
        {
            BOOL release_result = ReleaseMutex(m_handle);
            if (!release_result && !ignore_error)
            {
                ++m_acquired;
            }
#ifdef MUTEX_LOG
            else
            {
                if (enable_logging)
                {
                    DATA_LOG_MSG(("Released mutex"));
                }
            }
#endif
        }
    }
}

#ifdef MUTEX_LOG
void SystemWideMutex::MutexData::WriteLog(const char *string)
{
    if (enable_logging)
    {
        FILE* file = fopen("c:\\spi_app_mutex.log", "ab");
        char buf[128];
        _strtime(buf);
        fputs(buf, file);
        fputs(" ", file);
        // Prepare a string containing 30 characters containing the end of the exe name + the beginning of the arguments.
        {
            const char *cmd = GetCommandLineA();
            size_t pos;
            size_t len = strlen(cmd);
            const char * path_break = strstr(cmd,".exe\"");
            if (!path_break) 
                path_break = strstr(cmd,".EXE\"");
            if (!path_break) 
                path_break = strstr(cmd,"\" ");
            if (!path_break) 
                path_break = strstr(cmd," ");
            if (path_break)
            {
                const int temp = static_cast<int>(path_break - cmd - 14);
                if (temp < 0)
                {
                    pos = 0;
                }
                else
                {
                    pos = static_cast<size_t>(temp);
                }
            }
            else
            {
                pos = len;
            }
            
            char *bufptr = buf;
            for (size_t i = pos; i < pos + 30; i++)
            {
                if (i < len)
                    *bufptr++ = cmd[i];
                else
                    *bufptr++ = ' ';
            }
            *bufptr = '\0';
                    
            fputs(buf, file);
            fputs(" ", file);
        }
        fputs(string, file);
        fputs("\r\n", file);
        fclose(file);
    }
}
#endif

SystemWideMutex::SystemWideMutex(const char *mutex_name, bool acquire, bool exclusive)
    :
    data_(new MutexData(mutex_name, exclusive))
{
    assert(data_);

    if (acquire)
    {
        Acquire(0);
    }
}

SystemWideMutex::SystemWideMutex(const wchar_t *mutex_name, bool acquire, bool exclusive)
    :
    data_(new MutexData(mutex_name, exclusive))
{
    assert(data_);

    if (acquire)
    {
        Acquire(0);
    }
}

SystemWideMutex::~SystemWideMutex()
{
    assert(data_);

    while (IsAcquired())
    {
        data_->Release(true);
    }

    delete data_;
    data_ = NULL;
}

bool SystemWideMutex::Acquire(uint32 milliseconds)
{
    assert(data_);

    return data_->Acquire(milliseconds);
}

void SystemWideMutex::Release()
{
    assert(data_);

    data_->Release(false);
}

bool SystemWideMutex::IsAcquired() const
{
    assert(data_);

    return data_->m_acquired > 0;
}
