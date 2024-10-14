#ifndef CSR_TYPES_H
#define CSR_TYPES_H
/**********************************************************************

   FILE   :  csr_types.h

             Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
             All Rights Reserved.
             Qualcomm Technologies International, Ltd. Confidential and Proprietary.

   PURPOSE:

    CSR Synergy type definitions as used in some components common to 
    driver and tools.

***********************************************************************/

#include <stddef.h>
#include "common/types.h"

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef NULL
#define NULL (0L)
#endif

#define CSRMAX(a,b)    (((a) > (b)) ? (a) : (b))
#define CSRMIN(a,b)    (((a) < (b)) ? (a) : (b))

/* To shut lint up. */
#define CSR_UNUSED(x)          (void)(x)
#define CSR_PARAM_UNUSED(x)    ((void)(x))

#define CSR_INVALID_PHANDLE     0xFFFF /* Invalid protocol handle setting */
#define CSR_INVALID_TIMERID     (0)

/* Data types */

typedef size_t          CsrSize;

typedef uint8           CsrUint8;
typedef uint16          CsrUint16;
typedef uint32          CsrUint32;

typedef int8            CsrInt8;
typedef int16           CsrInt16;
typedef int32           CsrInt32;

typedef char            CsrCharString;

typedef unsigned char   CsrBool;

typedef unsigned char   CsrUcs2String; /* UCS2 oriented byte-pairs ordered (MSB,LSB) strings */
typedef CsrUint16       CsrUtf16String; /* 16-bit UTF16 strings */
typedef CsrUint32       CsrUint24;

#endif
