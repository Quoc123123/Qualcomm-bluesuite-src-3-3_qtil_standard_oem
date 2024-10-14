//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// ERROR_MSG.H
//////////////////////////////////////////////////////////////////////////////
// This header file defines the macro ERROR_MSG in a sensible way.
// It calls TRACE if WIN32 && DEBUG
// It does nothing if WIN32 && (WINCE || !DEBUG)
// It calls fprintf(stderr) if !WIN32 && DEBUG
// It does nothing if !WIN32 && !DEBUG
//
// The parameters to it must be in brackets, as in:
//
//  ERROR_MSG(("Null Pointer! (%p)\n", ptr));
//
// NO UNICODE PLEASE - CrtDbgReport does not support it.  Debug
// messages can be in english only
//

#ifndef ERROR_MSG_H__
#define ERROR_MSG_H__

#ifdef _WIN32

#if (!defined EXTRA_DEBUG) || (defined _WINCE) || (defined _WIN32_WCE)

#define ERROR_MSG(MSG) do { } while(0)
#define OUTPUT_HANDLE_CREATE(handle) do { } while(0)
#define OUTPUT_HANDLE_CLOSE(handle) do { } while(0)
#define OUTPUT_SOCKET_CREATE(handle) do { } while(0)
#define OUTPUT_SOCKET_CLOSE(handle) do { } while(0)


#else /* _DEBUG */

#include <crtdbg.h>
#include <stdio.h>
#include <stdarg.h>

#define ERROR_MSG(MSG) do { \
    if ((1 == _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, \
        "%s", sprintf_static MSG))) \
    _CrtDbgBreak(); \
    } while(0)

static const char* sprintf_static(const char *fmt, ...)
{
    static char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return buf;
}

#define OUTPUT_HANDLE_CREATE(handle) OutputDebugStringA(sprintf_static("%s (%d): HandleCreation %s %lx\n", __FILE__, __LINE__, #handle, handle ))

#define OUTPUT_HANDLE_CLOSE(handle) OutputDebugStringA(sprintf_static("%s (%d): HandleClose %s %lx\n", __FILE__, __LINE__, #handle, handle))

#define OUTPUT_SOCKET_CREATE(handle) OutputDebugStringA(sprintf_static("%s (%d): SocketCreation %s %lx\n", __FILE__, __LINE__, #handle, handle ))

#define OUTPUT_SOCKET_CLOSE(handle) OutputDebugStringA(sprintf_static("%s (%d): SocketClose %s %lx\n", __FILE__, __LINE__, #handle, handle))




#endif /* _DEBUG */

#else /* _WIN32 */

#ifdef _DEBUG

#include <stdarg.h>

#define ERROR_MSG(MSG) error_printf MSG

static void error_printf(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

#else /* _DEBUG */

#define ERROR_MSG(MSG) do { } while(0)

#endif /* _DEBUG */

#endif /* _WIN32 */

#endif /* ERROR_MSG_H__ */
