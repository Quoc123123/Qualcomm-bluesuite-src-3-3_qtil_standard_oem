/***********************************************************************************
 *
 *  uestores.h
 *
 *  Copyright (c) 2016-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  uenergy memory and storage subsystem details
 *
 ***********************************************************************************/
#ifndef UESTORES_H
#define UESTORES_H

#include <stddef.h>

#if defined(_MSC_VER) && (_MSC_VER < 1600)
    typedef __int16              int16_t;
    typedef unsigned __int16    uint16_t;
#else
#   include <stdint.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif

/* Word (uint16) offsets into Store header */
enum {
    STORE_HDR_FLAGS = 0,
    STORE_HDR_ID,
    STORE_HDR_SIZE,
    STORE_HDR_DATAOFS,
    ADDITIONAL_HDR,
    APPLICATION_IMG = 8
};

/* Word (uint16) offsets into Additional header */
enum {
    ADDITIONAL_HDR_USEDSIZE = 0,
};

/* Define sizes in words (uint16) */
#define HASH_SIZE  (16)
#define SIGNATURE_SIZE (64)
#define FOOTER_SIZE (HASH_SIZE + SIGNATURE_SIZE)


size_t NormaliseUsedSize(size_t aUsedSize);



#ifdef __cplusplus
} /* extern "C" */

#endif /* __cplusplus */
#endif /* UESTORES_H */
