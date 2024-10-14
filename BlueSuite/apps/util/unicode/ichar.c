/**********************************************************************
*
*  FILE   :  ichar.c
*
*            Copyright (c) 2001-2020 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  ichar functions.
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/unicode/ichar.c#1 $
*
***********************************************************************/


/* Include library header file */

#include "ichar.h"
#if (defined(WIN32) && !defined(_WINCE)) || !defined(_POSIX_C_SOURCE)
#include "common/portability.h"
#endif

/* Include system header files */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>


/* String manipulation */

#if defined _WINCE || defined _WIN32_WCE
ichar *istrrchr_ce(const ichar *apString, iint c)
{
    const ichar *last = NULL;
    while (*apString)
    {
        if (*apString == c)
            last = apString;
        ++apString;
    }
    return (ichar *)last;
}


int istrcmp_ce(const ichar *apString1, const ichar *apString2)
{
    while (*apString1 && *apString2 && (*apString1 == *apString2))
    {
        apString1++;
        apString2++;
    }
    return (*apString1 - *apString2);
}


int istricmp_ce(const ichar *apString1, const ichar *apString2)
{
    while (*apString1 && *apString2 && (itolower(*apString1) == itolower(*apString2)))
    {
        apString1++;
        apString2++;
    }
    return itolower(*apString1) - itolower(*apString2);
}


int istrnicmp_ce(const ichar *apString1, const ichar *apString2, size_t count)
{
    size_t i = 0;
    while (*apString1 && *apString2 && (itolower(*apString1) == itolower(*apString2)) && i < count)
    {
        apString1++;
        apString2++;
        i++;
    }
    return (i==count) ? 0 : itolower(*apString1) - itolower(*apString2);
}
#endif


wchar_t *istrdupW(const wchar_t *apString)
{
#ifndef _POSIX_C_SOURCE
    return _wcsdup(apString);
#else
    wchar_t* s = NULL;
    if (apString)
    {
        s = calloc(wcslen(apString)+1,sizeof(wchar_t));
        wcscpy(s, apString);
    }
    return s;
#endif
}

char *istrdupA(const char *apString)
{
#ifndef _POSIX_C_SOURCE
    return STRDUP(apString);
#else
    char* s = NULL;
    if (apString)
    {
        s = calloc(strlen(apString) + 1, sizeof(char));
        strcpy(s, apString);
    }
    return s;
#endif
}


/* Character manipulation */

#if defined _WINCE || defined _WIN32_WCE
int iisalnum_ce(iint c)
{
    return (  ((c >= 'A') && (c <= 'Z'))
           || ((c >= '0') && (c <= '9'))
           || ((c >= 'a') && (c <= 'z')) );
}


int iisdigit_ce(iint c)
{
    return ((c >= II('0')) && (c <= II('9')));
}


int iisspace_ce(iint c)
{
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'));
}


int iisxdigit_ce(iint c)
{
    return ( ((c >= II('A')) && (c <= II('F')))
           || ((c >= II('a')) && (c <= II('f')))
           || ((c >= II('0')) && (c <= II('9'))) );
}
#endif



/* Formatted I/O */

/* Can be mapped to existing routines in the standard library */
/* so no special local implementation required for these here */


/* Unformatted I/O */


/* Don't use this function, use ifgets instead
ichar *igets(ichar *buffer)
{
    return ICHAR_SELECT(gets, _getws)(buffer);
}
*/


/* String conversion */

#if defined _WINCE || defined _WIN32_WCE
long istrtol_ce(const ichar *nptr, ichar **endptr, int base)
{
    //  ASSERT(base == 10);
    const ichar *p = nptr;
    if (*p == II('-'))
    {
        p++;
        return 0L - (long)istrtoul(p, endptr, base);
    }
    if (*p == II('+'))
        p++;
    return (long)istrtoul(p, endptr, base);
}


unsigned long istrtoul_ce(const ichar *nptr, ichar **endptr, int base)
{
    const ichar *p = nptr;
    unsigned long val = 0;
    // ASSERT(base == 10);
    while (iisdigit(*p))
    {
        val = val * 10;
        val = val + ((*p) - II('0'));
        p++;
    }
    if (endptr)
        *endptr = (ichar *)p;
    return val;
}
#endif


/* Command execution */

/* Can be mapped to existing routines in the standard library */
/* so no special local implementation required for these here */


/* Time processing */

/* Can be mapped to existing routines in the standard library */
/* so no special local implementation required for these here */


/* File handling */

/* Can be mapped to existing routines in the standard library */
/* so no special local implementation required for these here */

