/**********************************************************************
*
*  FILE   :  ichar.h
*
*            Copyright (c) 2001-2020 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  ichar functions.
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/unicode/ichar.h#1 $
*
***********************************************************************/

#ifndef UNICODE__ICHAR_H
#define UNICODE__ICHAR_H


/* The UNICODE__ICHAR_WIDE define selects wide character support */
/* Allow _UNICODE as an alternative for Windows platforms */

#ifdef _UNICODE
#define UNICODE__ICHAR_WIDE
#endif

/* Macro to select appropriate function name based on compilation options */
#ifdef UNICODE__ICHAR_WIDE
#define ICHAR_SELECT(a,w) w
#else
#define ICHAR_SELECT(a,w) a
#endif

/* A few routines also need to be different on WINCE, this Macro allows that implementation to be picked when required */
#if defined(_WINCE) || defined(_WIN32_WCE)
#define ICHAR_SELECT(a,w,ce) ce
#else
#define ICHAR_SELECT_CE(a,w,ce) ICHAR_SELECT(a,w)
#endif

/* Include appropriate header files */

#include <stdio.h>
#include <stdarg.h>
#if !defined(_WINCE) && !defined(_WIN32_WCE)
#  include <time.h>
#  include <string.h>
#  include <ctype.h>
#  include <stdlib.h>
#  include <locale.h>
#  ifdef __cplusplus
#    include <iosfwd>
#    include <iostream>
#    include <sstream>
#  endif
#endif /* _WINCE */
#if defined(__APPLE__) || defined(_WINCE)
    #include <stdlib.h>
#else
    #include <wchar.h>
#endif

#ifdef __cplusplus
#  include <string>
#  if !defined(WIN32) && !defined(_WINCE) && !defined(_WIN32_WCE)
typedef std::basic_string <wchar_t> wstring;
#  endif
#endif



/* Define the appropriate character type */

#ifdef UNICODE__ICHAR_WIDE
typedef wchar_t         ichar;
typedef wint_t          iint;
#define ICHAR_MIN       WCHAR_MIN
#define ICHAR_MAX       WCHAR_MAX
#define IEOF            WEOF
#else
typedef char            ichar;
typedef int             iint;
#define ICHAR_MIN       CHAR_MIN
#define ICHAR_MAX       CHAR_MAX
#define IEOF            EOF
#endif


/* Allow character and string literals to be specified appropriately */

#ifdef UNICODE__ICHAR_WIDE
#define II(x)           _II(x)
#define _II(x)          L ## x
#else
#define II(x)           x
#endif


/* Equivalents of standard C library functions */

#ifdef  __cplusplus
extern "C" {
#endif

/* String manipulation */
#define istrchr ICHAR_SELECT(strchr, wcschr)
#define istrcspn ICHAR_SELECT(strcspn, wcscspn)
#define istrcat ICHAR_SELECT(strcat, wcscat)
#define istrncat ICHAR_SELECT(strncat, wcsncat)
#define istrcpy ICHAR_SELECT(strcpy, wcscpy)
#define istrncpy ICHAR_SELECT(strncpy, wcsncpy)
#define istrpbrk ICHAR_SELECT(strpbrk, wcspbrk)
#define istrrchr ICHAR_SELECT_CE(strrchr, wcsrchr, istrrchr_ce)
#define istrspn ICHAR_SELECT(strspn, wcsspn)
#define istrstr ICHAR_SELECT(strstr, wcsstr)
#define istrtok ICHAR_SELECT(strtok, wcstok)
#define istrcmp ICHAR_SELECT_CE(strcmp, wcscmp, istrcmp_ce)
#define istrncmp ICHAR_SELECT(strncmp, wcsncmp)

#ifdef _MSC_VER
#define istricmp ICHAR_SELECT_CE(_stricmp, _wcsicmp, istricmp_ce)
#define istrnicmp ICHAR_SELECT_CE(_strnicmp, _wcsnicmp, istrnicmp_ce)
#else
#define istricmp strcasecmp
#define istrnicmp strncasecmp
#endif

#define istrlen ICHAR_SELECT(strlen, wcslen)
#define istrdup ICHAR_SELECT(istrdupA, istrdupW)

#if !defined _WINCE && !defined _WIN32_WCE
#define istrcoll ICHAR_SELECT(strcoll, wcscoll)
#define istrxfrm ICHAR_SELECT(strxfrm, wcsxfrm)
#endif

/* Character manipulation */
#define iisalnum ICHAR_SELECT_CE(isalnum, iswalnum, iisalnum_ce)
#define iisalpha ICHAR_SELECT(isalpha, iswalpha)
#define iisdigit ICHAR_SELECT_CE(isdigit, iswdigit, iisdigit_ce)
#define iisgraph ICHAR_SELECT(isgraph, iswgraph)
#define iislower ICHAR_SELECT(islower, iswlower)
#define iisprint ICHAR_SELECT(isprint, iswprint)
#define iispunct ICHAR_SELECT(ispunct, iswpunct)
#define iisspace ICHAR_SELECT_CE(isspace, iswspace, iisspace_ce)
#define iisupper ICHAR_SELECT(isupper, iswupper)
#define iisascii ICHAR_SELECT(isascii, iswascii)
#define iiscntrl ICHAR_SELECT(iscntrl, iswcntrl)
#define iisxdigit ICHAR_SELECT_CE(isxdigit, iswxdigit, iisxdigit_ce)
#define itoupper ICHAR_SELECT(toupper, towupper)
#define itolower ICHAR_SELECT(tolower, towlower)

/* Formatted I/O */
#define iprintf ICHAR_SELECT(printf, wprintf)
#define ifprintf ICHAR_SELECT(fprintf, fwprintf)
#define isprintf ICHAR_SELECT(snprintf, swprintf) /* usage: int isprintf(ichar *, size_t, const ichar *, ...); */
#define ivprintf ICHAR_SELECT(vprintf, vwprintf)
#define ivfprintf ICHAR_SELECT(vfprintf, vfwprintf)
#define ivsprintf ICHAR_SELECT(vsnprintf, vswprintf) /* usage: int ivsprintf(ichar *, size_t, const ichar *, va_list); */
/*
    The standard library does not include va_arg versions of the formatted
    input functions:
        int iscanf(const ichar *, ...);
        int ifscanf(FILE *, const ichar *, ...);
        int isscanf(const ichar *, const ichar *, ...);
*/

/* Unformatted I/O */
#define ifgetc ICHAR_SELECT(fgetc, fgetwc)
#define ifgets ICHAR_SELECT(fgets, fgetws)
#define ifputc ICHAR_SELECT(fputc, fputwc)
#define ifputs ICHAR_SELECT(fputs, fputws)
#define igetc ICHAR_SELECT(getc, getwc)
#define igetchar ICHAR_SELECT(getchar, getwchar)
/* ichar *igets(ichar *); Use ifgset()*/
#define iputc ICHAR_SELECT(putc, putwc)
#define iputchar ICHAR_SELECT(putchar, putwchar)
#define iputs ICHAR_SELECT(puts, _putws)
#define iungetc ICHAR_SELECT(ungetc, ungetwc)

/* String conversion */
#define istrtod ICHAR_SELECT(strtod, wcstod)
#define istrtol ICHAR_SELECT_CE(strtol, wcstol, istrtol_ce)
#define istrtoul ICHAR_SELECT_CE(strtoul, wcstoul, istrtoul_ce)
#define iatoi ICHAR_SELECT(atoi, _wtoi)
#define iatol ICHAR_SELECT(atol, _wtol)
#define iatof ICHAR_SELECT(atof, _wtof)

/*
Mappings to macros defined in common/portability.h (which you'll need to
include separately)
*/
#ifdef UNICODE__ICHAR_WIDE
    #define ISTRTOUI64  WCSTOUI64
    #define ISTRTOI64   WCSTOI64
#else
    #define ISTRTOUI64  STRTOUI64
    #define ISTRTOI64   STRTOI64
#endif

/* Command execution */
#define isystem ICHAR_SELECT_CE(system, _wsystem, isystem_ce_unsupported)
#define igetenv ICHAR_SELECT_CE(getenv, _wgetenv, igetenv_ce_unsupported)

/* Time processing */
#define iasctime ICHAR_SELECT_CE(asctime, _wasctime, iasctime_ce_unsupported)
#define ictime ICHAR_SELECT_CE(ctime, _wctime, ictime_ce_unsupported)
#define istrftime ICHAR_SELECT_CE(strftime, wcsftime, istrftime_ce_unsupported)

/* File handling */
#define ifopen ICHAR_SELECT(fopen, _wfopen)
#define ifreopen ICHAR_SELECT(freopen, _wfreopen)
#define iperror ICHAR_SELECT_CE(perror, _wperror, iperror_ce_unsupported)
/* #define itmpnam ICHAR_SELECT_CE(tmpnam, _wtmpnam, itmpnam_ce_unsupported)*/ /* don't use this function, use mkstemp() instead */
#define iremove ICHAR_SELECT_CE(remove, _wremove, iremove_ce_unsupported)
#define irename ICHAR_SELECT_CE(rename, _wrename, irename_ce_unsupported)

/* Locale handling */
#define isetlocale ICHAR_SELECT_CE(setlocale, _wsetlocale, isetlocale_ce_unsupported)

#if defined _WINCE || defined _WIN32_WCE
/* a few C-style routines implemented specifically for WINCE (implemented in ichar.c) */
ichar *istrrchr_ce(const ichar *apString, iint c);
int istrcmp_ce(const ichar *apString1, const ichar *apString2);
int istricmp_ce(const ichar *apString1, const ichar *apString2);
int istrnicmp_ce(const ichar *apString1, const ichar *apString2, size_t count);
int iisalnum_ce(iint c);
int iisdigit_ce(iint c);
int iisspace_ce(iint c);
int iisxdigit_ce(iint c);
long istrtol_ce(const ichar *nptr, ichar **endptr, int base);
unsigned long istrtoul_ce(const ichar *nptr, ichar **endptr, int base);
#endif

/* other C-style routines implemented in ichar.c */
wchar_t *istrdupW(const wchar_t *apString);
char *istrdupA(const char *apString);

#ifdef  __cplusplus
}
#endif


/* A few C++ library additions (implemented in inarrow.cpp) */

#if defined(__cplusplus)

/* A string type for C++ */
#ifdef UNICODE__ICHAR_WIDE
typedef std::wstring    istring;
#else
typedef std::string     istring;
#endif

/* stream types */
#ifdef UNICODE__ICHAR_WIDE
typedef std::wostream    iiostream;
typedef std::wistream    iiistream;
typedef std::wofstream   iiofstream;
typedef std::wifstream   iiifstream;
#else
typedef std::ostream     iiostream;
typedef std::istream     iiistream;
typedef std::ofstream    iiofstream;
typedef std::ifstream    iiifstream;
#endif

/* A string stream types for C++. Prefix with ii due to clash */
#ifdef UNICODE__ICHAR_WIDE
typedef std::wistringstream iiistringstream;
typedef std::wostringstream iiostringstream;
typedef std::wstringstream  iistringstream;
#else
typedef std::istringstream  iiistringstream;
typedef std::ostringstream  iiostringstream;
typedef std::stringstream   iistringstream;
#endif

/* String conversions */
#ifdef WIN32
std::string Utf16To8String(const wchar_t *pWideIn);
std::wstring Utf8To16String(const char *pCharIn);
inline std::string Utf16To8String(const std::wstring &aString) { return Utf16To8String(aString.c_str()); };
inline std::wstring Utf8To16String(const std::string &aString) { return Utf8To16String(aString.c_str()); };
#endif
std::wstring iwiden(const std::string &aString);
inline std::wstring iwiden(const std::wstring &aString) { return aString; };
inline std::string inarrow(const std::string &aString) { return aString; };
std::string inarrow(const std::wstring &aString);

#define icoerce ICHAR_SELECT(icoerceA, icoerceW)
inline std::string icoerceA(const std::string &aString) { return aString; };
inline std::string icoerceA(const std::wstring &aString) { return inarrow(aString); };
inline std::wstring icoerceW(const std::string &aString) { return iwiden(aString); };
inline std::wstring icoerceW(const std::wstring &aString) { return aString; };

/* Filename conversions */
std::wstring iwiden_filename(const std::string &aFilename);
inline std::wstring iwiden_filename(const std::wstring &aFilename) { return aFilename; };
inline std::string inarrow_filename(const std::string &aFilename) { return aFilename; };
std::string inarrow_filename(const std::wstring &aFilename);

#define icoerce_filename ICHAR_SELECT(icoerce_filenameA, icoerce_filenameW)
inline std::string icoerce_filenameA(const std::string &aFilename) { return aFilename; };
inline std::string icoerce_filenameA(const std::wstring &aFilename) { return inarrow_filename(aFilename); };
inline std::wstring icoerce_filenameW(const std::string &aFilename) { return iwiden_filename(aFilename); };
inline std::wstring icoerce_filenameW(const std::wstring &aFilename) { return aFilename; };

/* Standard I/O streams */
#ifdef UNICODE__ICHAR_WIDE
#define icin            std::wcin
#define icout           std::wcout
#define icerr           std::wcerr
#define iclog           std::wclog
#else
#define icin            std::cin
#define icout           std::cout
#define icerr           std::cerr
#define iclog           std::clog
#endif

#endif

/* C wrappers around the above C++ functions */
#ifdef __cplusplus
extern "C"
{
#endif

#define iinarrowdup ICHAR_SELECT(istrdupA, iinarrowdupW)
#define iccoercedup ICHAR_SELECT(istrdupA, iccoercedupW)
char *iinarrowdupW(const wchar_t *apString);
wchar_t *iccoercedupW(const char *apString);

#ifdef __cplusplus
}
#endif

#endif
