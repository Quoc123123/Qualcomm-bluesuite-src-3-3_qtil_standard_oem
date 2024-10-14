/*******************************************************************************
**
**  FILE     :  dfu_private.h
**
**  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
**  All Rights Reserved.
**  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
**
**  PURPOSE  :  To contain generic definitions
**
*******************************************************************************/

#ifndef __DFU_PRIVATE_H__
#define __DFU_PRIVATE_H__

/* Always use the "common/types.h" header file if this next line is inserted */
#define USE_COMMON_TYPES_H

#ifdef USE_COMMON_TYPES_H

#include "common/types.h"

#else

#include <sys/types.h>

#ifdef WIN32
#ifdef __cplusplus
/*  Switch off the warning which says name mangling has produced something
    too long (ie you are using the STL...) */
#pragma warning(disable:4786)
#endif

#ifdef WHAT_I_WANT
/*  this is what I want to use, since these are supposedly synonymous
    with the below, but they break the Windows STL, so who can say... */
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64   uint64;
#endif

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

#else

#ifdef __CYGWIN__

typedef u_int8_t uint8;
typedef u_int16_t uint16;
typedef u_int32_t uint32;

#else
#ifdef __sparc

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

#else
#ifdef __linux__

typedef u_int8_t uint8;
typedef u_int16_t uint16;
typedef u_int32_t uint32;

#else
#ifdef __APPLE__

typedef u_int8_t uint8;
typedef u_int16_t uint16;
/*typedef u_int32_t uint32;*/
typedef unsigned long int uint32;

#else
#error INVALID PLATFORM
#endif
#endif
#endif
#endif
#endif

#endif
#ifdef __cplusplus

#include <vector>
inline uint16 make_word(unsigned char a, unsigned char b)
{
    return (((unsigned) b) << 8) + a;
}

#else

typedef uint8 bool;
enum { FALSE = 0 , TRUE = 1 };

#endif

/* Pretend to be a XAP fr a while...  well, OK, XAP doesn't do quad
* length words, but...
*/
typedef uint16 word;
typedef uint32 dword;

/*
Define maximum value for dword (a 32-bit unsigned value).
When dword is larger than 32-bits we get an injection bug from arithemtic
wraparound into bits 32 and above. DWORD_MAX is used for remedial action to
clamp overflows to 32-bits.
We want a literal constant so the compiler can optimise it out when dword
is actually 32-bits.
*/
#define DWORD_MAX (0xFFFFFFFFu)

#endif
