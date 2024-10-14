/**********************************************************************
*
* types_t.h
*
* Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* Type definitions.
*
**********************************************************************/
#ifndef COMMON_TYPES_T_H
#define COMMON_TYPES_T_H


#define HOSTTOOLS_OLD_INTEGER_TYPES

#ifndef HOSTTOOLS_OLD_INTEGER_TYPES

/*
 * Visual Studio doesn't include inttypes.h prior to VS2010
 */
#ifdef _MSC_VER
#if _MSC_VER >= 1600
#include <stdint.h>
#else
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned long long  uint64_t;
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed long         int32_t;
typedef signed long long    int64_t;
#endif /* _MSC_VER >= 1600 */
#else
#include <sys/types.h>
/*
 * Pull in standard C99 header with exact width definitions
 */
#include <inttypes.h>
#endif

typedef uint8_t     bool_t;

/*
 * [u]int24_t is defined as [u]int32_t. This is the only data type which is not
 * "exactly the size" that it represents.
 */
typedef uint32_t    uint24_t;
typedef int32_t     int24_t;

typedef uint16_t    phandle_t;

#else  /* HOSTTOOLS_OLD_INTEGER_TYPES */

#ifdef __linux__
#include <sys/types.h>
#endif

#include "common/types.h"

typedef uint8     bool_t;

#if defined(__sun__) || defined(__APPLE_CC__) || defined(__linux__)
#include <inttypes.h> /* C9X header */
#elif defined(_MSC_VER) && _MSC_VER >= 1600
#include <stdint.h>
#else
typedef uint8     uint8_t;
typedef uint16    uint16_t;
typedef uint32    uint32_t;
typedef uint64    uint64_t;
typedef int8      int8_t;
typedef int16     int16_t;
typedef int32     int32_t;
#endif

typedef uint32_t    uint24_t;
typedef int32_t     int24_t;

typedef uint16_t    phandle_t;

#endif /* HOSTTOOLS_OLD_INTEGER_TYPES */

#endif /* COMMON_TYPES_T_H */
