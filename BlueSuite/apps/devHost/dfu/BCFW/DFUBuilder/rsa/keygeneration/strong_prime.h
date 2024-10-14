/**********************************************************************
 *
 *  strong_prime.h
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To provide fundtions to find strong primes using the Gordon 
 *  algorithm.
 *
 **********************************************************************/

#ifndef __STRONG_PRIME_H__
#define __STRONG_PRIME_H__

#include "keygen_private.h"

void get_Gordon_p0_32 (dword s[/*size*/], dword r[/*size*/],
		  dword temp[/*size*/], word size);

void next_primeN32 (dword prime_rand[/*size*/], word size, dword *degree, word t,
                    dword odd_rand[/*size*/], bool randn );

bool strong_prime (dword prime_rand[/*size*/], word size, dword *degree,
		      word t );

#endif
