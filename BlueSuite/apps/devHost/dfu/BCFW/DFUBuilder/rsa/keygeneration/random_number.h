/**********************************************************************
 *
 *  random_number.h
 *  
 *  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 **********************************************************************/

#ifndef _RANDOM_NUMBER_H_
#define _RANDOM_NUMBER_H_

#define RAND_N_512b_dword (16)

/* K1 and K2 are indexes for the Exp[] array and also the exponents */
/* used in the random number generation algorithm proposed by Roger */
/* Sewell. So if, for example, we want the exponents K1 = 65537 and */
/* K2 = 3 we should define below K1 = 2 and K2 = 0.                 */

#define K1 (1) /* the exponent K1 will be 17 */
#define K2 (1) /* the exponet K2 will be 17  */

void init_rand_seed ( const dword * random_data );

void random_number32 (dword rand_output[/* rand_size */], word rand_size );

void get_nbit_odd_rand32 (dword odd_rand[/*size*/], word size, dword degree );

#endif
