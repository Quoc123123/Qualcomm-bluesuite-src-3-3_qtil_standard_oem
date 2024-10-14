/********************************************************************************
 *
 *  portability.h
 *
 *  Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Common declarations with platform conditionals to emeliorate portability 
 *  headaches.
 *
 *******************************************************************************/

#ifndef COMMON_PORTABILITY_H
#define COMMON_PORTABILITY_H

#ifdef __GNUC__
#define __inline __inline__
#endif

/// Use this macro to check that the value of an enum or number is between 0 and the MAX_VALUE (inclusive)
#define ASSERT_IF_VALUE_OUT_OF_RANGE(VALUE, MAX_VALUE) \
    assert((VALUE) >= 0 && (VALUE) <= (MAX_VALUE))

#ifndef _MSC_VER
#if __linux__ == 1
#include <unistd.h>
#endif
#endif

#if !(defined WIN32 || defined _WIN32_WCE)
#include "common/types.h"
typedef          long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef uint32 DWORD;
typedef uint32 UINT;
#ifdef __cplusplus
typedef bool BOOL;
#endif
static const uint32 INFINITE = ~((uint32)0);
#define MAX_PATH 65536
#define _I64_MIN (-9223372036854775807ULL - 1)
#define _I64_MAX 9223372036854775807ULL
#define ATOI64 atoll
#else
#define ATOI64 _atoi64
#endif

/* Define STRTOUI64 as a 64-bit strtoul
          STRTOI64 as a 64-bit strtol */
#if (0UL - 1UL) >= 18446744073709551615UL
    /* unsigned long is 64-bits */
    #if defined _WIN32
        #define STRTOUI64   _strtoui64
        #define STRTOI64    _strtoi64
        #define WCSTOUI64   _wcstoui64
        #define WCSTOI64    _wcstoi64
    #else /* assume GNU/Linux */
        #define STRTOUI64   strtoul
        #define STRTOI64    strtol
        #define WCSTOUI64   wcstoul
        #define WCSTOI64    wcstol
    #endif
#elif (0ULL - 1ULL) >= 18446744073709551615ULL
    /* unsigned long is less than 64-bits but unsigned long long is 64-bits */
    #define STRTOUI64   strtoull
    #define STRTOI64    strtoll
    #define WCSTOUI64   wcstoull
    #define WCSTOI64    wcstoll
#else
    /* You need to define STRTOUI64/STRTOI64 to a 64-bit strtoul/strtol */
#endif


/* Visual Studio 2005 deprecates various POSIX and other fairly
   standard functions, most of them simply require an underscore
   prefix to remove the warning... _MSC_VER is 1400 for Visual Studio
   2005 */
#if _MSC_VER >= 1400
# define STRICMP _stricmp
# define STRNICMP _strnicmp
# define ISATTY _isatty
# define STRDUP _strdup
# include <stdio.h>
# include <stdarg.h>
# ifdef UNDER_CE
#  define _isatty(x) 0
# else
#  include <io.h>
# endif
# if _MSC_VER >= 1900
#  define SNPRINTF snprintf
#  define VSNPRINTF vsnprintf
# else
   __inline int CsrVsnprintf(char *apDest, size_t aSize, const char *apFmt, va_list aArgs)
   {
       int r = _vsnprintf(apDest, aSize, apFmt, aArgs);
       if(apDest && aSize > 0) apDest[aSize-1]='\0';
       // if we replaced the last character with \0 then reduce r because we just truncated the output by 1.
       if(r > 0 && (size_t)r == aSize)
       {
           --r;
       }
       return r;
   }
   __inline int CsrSnprintf(char *apDest, size_t aSize, const char *apFmt, ...)
   {
       int r;
       va_list args;
       va_start(args, apFmt);
       r = CsrVsnprintf(apDest, aSize, apFmt, args);
       va_end(args);
       return r;
   }
#  define SNPRINTF CsrSnprintf
#  define VSNPRINTF CsrVsnprintf
# endif
# define FILENO _fileno
# define CHDIR _chdir
# define GETCWD _getcwd
# define STRLWR _strlwr

#else
# ifdef _MSC_VER
#  define STRICMP stricmp
#  define STRNICMP strnicmp
#  include <stdio.h>
#  include <stdarg.h>
  __inline int CsrVsnprintf(char *apDest, size_t aSize, const char *apFmt, va_list aArgs)
  {
      int r = _vsnprintf(apDest, aSize, apFmt, aArgs);
      if(apDest && aSize > 0) apDest[aSize-1]='\0';
      // if we replaced the last character with \0 then reduce r because we just truncated the output by 1.
      if(r > 0 && (size_t)r == aSize)
      {
          --r;
      }
      return r;
  }
  __inline int CsrSnprintf(char *apDest, size_t aSize, const char *apFmt, ...)
  {
      int r;
      va_list args;
      va_start(args, apFmt);
      r = CsrVsnprintf(apDest, aSize, apFmt, args);
      va_end(args);
      return r;
  }
# define SNPRINTF CsrSnprintf
# define VSNPRINTF CsrVsnprintf
# else
#  include <stdio.h>
#  include <strings.h>
#  define STRICMP strcasecmp
#  define STRNICMP strncasecmp
#  define SNPRINTF snprintf
#  define VSNPRINTF vsnprintf
# endif
# define ISATTY isatty
# define STRDUP strdup
# define FILENO fileno
# define CHDIR chdir
# define GETCWD getcwd
# define STRLWR strlwr
#endif /* _MSC_VER >= 1400 */

/* Microsoft don't define va_copy until VS2013 so conditionally define it */
#ifdef _MSC_VER
#   ifndef va_copy
#       define va_copy(d,s) ((d) = (s))
#   endif
#endif

#if defined (_WIN32) && !defined (_WIN32_WCE)
/* vsnprintf doesn't return the length written, when buffer is null
 * using VS2005. Use _vscprintf instead. */
#define VPRINTF_COUNT(f, v) _vscprintf((f), (v))
#elif defined (_WIN32_WCE)
/* WINCE doesn't even have _vscprintf. Write to the null device.*/
#include <stdarg.h>
static int VPRINTF_COUNT(const wchar_t *apFmt, va_list aArgs)
{
    FILE *pFile;

    if (NULL == (pFile = fopen("nul:", "w")))
    {
        return -1;
    }
    else
    {
        int size = vfwprintf(pFile, apFmt, aArgs);
        fclose(pFile);
        return size;
    }
}
#else
#define VPRINTF_COUNT(f, v) VSNPRINTF(NULL, 0, (f), (v))
#endif

#ifndef _GNU_SOURCE
#ifdef PORTABILITY_DEFINE_STRCASESTR
/*
This is an implementation of a case insensitive strstr.
To prevent this being included unnecessarily make it conditional
on the macro PORTABILITY_DEFINE_STRCASESTR.

Usage:
    #define PORTABILITY_DEFINE_STRCASESTR
    #include "common/portability.h"


strcasestr is a non-standard extension provided by GNU C lib
but is only available if _GNU_SOURCE is defined.
MS Visual Studio don't provide an implementation.
*/
#include <ctype.h>

static char *strcasestr(const char *str, const char *substr)
{
    /* define uintptr_t locally so we don't have a dependency on stdint.h */
#ifdef _M_X64 /* Detect LLP64 (MSVC/Windows x64) */
    typedef unsigned long long int uintptr_t;
#else
    typedef unsigned long int uintptr_t;
#endif
    const char *r = NULL;
    if(*substr)
    {
        const char *s, *e;
        /*
        Calculate last worthwhile start character: beyond which there is insufficient
        characters remaining for a match.
        1, e = end of str (points to null terminator)
        2, reduce e by length of substr
        */
        for(e = str; *e; e++) { /* do nothing */ }
        for(s = substr; *s; s++, e--) { /* do nothing */ }
        /* iterate through str looking for matching start char */
        for(s = str; s <= e; s++)
        {
            if(tolower(*s) == tolower(*substr))
            {   /* compare rest of string */
                const char *p = s+1;
                const char *q = substr+1;
                while(*p && *q && (tolower(*p) == tolower(*q)))
                {
                    p++;
                    q++;
                }
                if(!*q)
                {   /* matched */
                    r = s;
                    break;
                }
            }
        }
    }
    else
    {
        r = str;
    }
    /* cast away const */
    return (char *)(uintptr_t)(const void*) r;
}
#else
/* strcasestr not included. #define PORTABILITY_DEFINE_STRCASESTR before ALL #includes */
#endif /* #ifdef PORTABILITY_DEFINE_STRCASESTR */
#endif /* #ifdef _GNU_SOURCE */

#endif /* ifndef COMMON_PORTABILITY_H */
