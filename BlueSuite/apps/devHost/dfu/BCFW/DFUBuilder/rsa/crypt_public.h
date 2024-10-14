/******************************************************************************
FILENAME:    crypt_public.h

Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     16 bit rsa encryption/decryption routines and common data types
             sign and decrypt are in separate .c file, so they can be
	     linked separately.

******************************************************************************/
#include "dfu_private.h"

#ifndef __CRYPT_PUBLIC_H__
#define __CRYPT_PUBLIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
**  DATA TYPES
******************************************************************************/

enum { KEY_WIDTH = 64 ,  /* in words */
       WORD_BITS = 16 ,  /* so signature is 1024 bits wide */
       mod16 = 0xFFFF };

typedef struct M_struct
{
    word M [ KEY_WIDTH ];               /* the modulus M = p1*p2 */

    /* parameters specific to the Montgomery multiplication: */
    word M_dash;                        /* -M^(-1) mod b, b = 2^T */
    word R2NmodM [ KEY_WIDTH ];         /* R^2 mod M, R = b^(T*size) */
    word RNmodM [ KEY_WIDTH ];          /* R mod M  */
    word one [ KEY_WIDTH ];             /* 1 */
} Modulus;

typedef struct Secret_key
{
    word key[ KEY_WIDTH ];      /* The secret/private key. Do not reveal! */
    word size;                  /* the size of the key in words: */
    Modulus mod;                /* the modulus */
} RSA_Key;

/******************************************************************************
**  FUNCTIONS
******************************************************************************/

int crypt_sign ( uint16 * block , const RSA_Key * key );

int crypt_decrypt ( uint16 * block , const RSA_Key * key );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif
