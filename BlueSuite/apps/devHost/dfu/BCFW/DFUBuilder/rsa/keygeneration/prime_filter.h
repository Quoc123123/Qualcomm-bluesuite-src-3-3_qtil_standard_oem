/**********************************************************************
 *
 *  prime_filter.h
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To provide functions which check for primality.
 *
 **********************************************************************/

#ifndef __PRIME_FILTER_H__
#define __PRIME_FILTER_H__

/* The maximum number of numbers of the form 6n-1 or 6n+1 used in the  */
/* function get_primeN32 () for finding a prime number. The  number of */
/* searched numbers will be twice this value. In the current           */
/* implementation the value of MAX_PRIME_SEARCH must be no greater     */
/* than (65535-2)/6 => <= 10922, though the the functions can be       */
/* easily changed to accommodate a bigger search span.                 */
#define MAX_PRIME_SEARCH (100)

#define GORDON_I_MAX (200)
#define PSIZE (164)
#define FPSIZE (5)
#define PORDER32b (31)
typedef const word PrimeOrder32b [PSIZE][PORDER32b];

bool multiprec_TrialDivide32 (dword odd_dnum[], word dsize,
                              const dword prime[], word psize,
                              PrimeOrder32b *prime_order,
                              uint64 Mod_sum[], word inc);

void update_SixN_table32 (bool SixN[],
                          word max, const dword prime[],
                          word psize, uint64 mod_sum[], int k);

bool FermatPrimeDividesP (dword odd_num[], word size,
                          uint64 Mod_FP[], uint64 *modsum, word inc);

bool inc_search_trial_div32 (dword *odd_rand, word size,
                              const dword prime[/*psize*/], const word psize,
			      PrimeOrder32b *primefactor/*[PSIZE][PORDER32b]*/,
                         bool *SixNm1, bool *SixNp1,
                         int max, word inc1);

bool Prime_test32 (dword *odd_num, word size, word t ,const dword prime[/*psize*/], const word psize);

bool create_Modulus_struct (dword *M, word size,
			dModulus *ms, bool generic );

dword get_inv_ofM (dword m0);
word get_RmodM32G (dword *RmodM, dword *M, word size);
word get_R2modM32G (dword *R2modM, dword *M, word size);
void get_R2modM32 (dword *R2modM, dword *M, word size);

bool Miller_Rabin_test32 (dword *odd_num, word size, dModulus *ms,
			  dword base, dword *temp, dword *temp1);

word Miller_Rabin_Rshift (dword *MR_exp, const word size);

#endif
