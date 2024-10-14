/******************************************************************************
FILENAME:    keygen_public.h

Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     rsa key generation routine common data types

******************************************************************************/
#ifndef __KEYGEN_PUBLIC_H__
#define __KEYGEN_PUBLIC_H__

#include "dfu_private.h"

/*******************************************************************************
**
**  DATA TYPES
**
**  Types used in by the key generation library.
**
**  The library uses 32 bit algorithms, including Montgormery multiplication
**  unlike the signing routines which work at 16 bit.
**  The only reason for this is that it should work slightly faster on 32 bit
**  machines.
**
**  A separate header file provides access to the 16<->32 bit conversion
**  routhines.
**
*******************************************************************************/

#define KEY_WIDTH_W  (32u)
#define DWORD_BITS   (32u)
#define mod32 ((dword)(0xFFFFFFFFul))

typedef struct M_struct32
{
    dword M [ KEY_WIDTH_W ];               /* the modulus M = p1*p2 */

    /* parameters specific to the Montgomery multiplication: */
    dword M_dash;                          /* -M^(-1) mod b, b = 2^T */
    dword R2NmodM [ KEY_WIDTH_W ];         /* R^2 mod M, R = b^(T*size) */
    dword RNmodM [ KEY_WIDTH_W ];          /* R mod M  */
    dword one [ KEY_WIDTH_W ];             /* 1 */
} dModulus;

typedef struct Secret_key32
{
    dword key[ KEY_WIDTH_W ];    /* The secret/private key. Do not reveal! */
    dword size;                  /* the size of the key in words: */
    dModulus mod;                /* the modulus */
} RSA_dKey;


typedef struct sp
{
    dword prob_prime [ KEY_WIDTH_W + 1 ];
    dword degree;
    struct sp * next;
}
Prime_Candidate;

/*******************************************************************************
**
**  FUNCTIONS
**
**  Call initialise_pair_search() to begin.  random_data should point to 
**  32 dwords (1024 bits) of random seed.
**  Pass get_prime_pair() two empty struct, until you have enough prime
**  pairs for the number of keys you want.
**  Then call finished_pair_search() to release the resources used
**  by the prime search.
**
**  Call generate_key to make a private key from the two prime.
**  The associated public key has the same modulus, and key of 3.
**
*******************************************************************************/
void initialise_pair_search ( const dword *random_data );
void get_prime_pair ( Prime_Candidate * one ,
		      Prime_Candidate * two );
void finished_pair_search ( void );

bool generate_key ( RSA_dKey * key,
		    Prime_Candidate * one ,
		    Prime_Candidate * two );

#endif
