/**********************************************************************
 *
 *  prime_filter.c
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To implement functions which check for primality.
 *
 **********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "keygen_private.h"

const word PbaseT[7] = {2, 3, 5, 7, 11, 13, 17};

const dword fermatP[FPSIZE] = {3, 5, 17, 257, 65537};

/* - inc_search_trial_div32 ()                                               */
/* This function performs trial division by Fermat primes and small primes   */
/* of odd numbers of the form 6n-1 and 6n+1, starting from N = 6n-1 up to    */
/* 6n-1 + 6*(max-1) and 6n+1 up to 6n+1 + 6*(max-1), with the results being  */
/* output to the boolean tables SixNm1[] and SixNp1[] (TRUE means that       */
/* number is composite, FALSE means that number might be a probable prime.   */
/*                                                                           */
/* INPUTS:   odd_rand[] - An odd random number of the form 6n-1 from which   */
/*                        the incremental search must start.                 */
/*                 size - The size, in uint32, of odd_rand[].                */
/*            SixNm1[]  - Where the trial division results will be stored.   */
/*                        A FALSE value in the "i"th entry means that the    */
/*                        number odd_rand[] + 6*i might be prime. A TRUE     */
/*                        value means the number is composite.               */
/*            SixNp1[]  - Where the trial division results will be stored.   */
/*                        A FALSE value in the "i"th entry means that the    */
/*                        number odd_rand[] + 2 + 6*i might be prime. A      */
/*                        TRUE value means the number is composite.          */
/*                  max - The size of both tables SixNm1[] and SixNp1[].     */
/*                        Must be defined by MAX_PRIME_SEARCH.               */
/*                 inc1 - If inc1 = 0 than the functions that test the       */
/*                        odd_rand divisibility by the small primes p_i will */
/*                        initialise the uint64 arrays that store the        */
/*                        remainders with odd_rand mod p_i. If inc1 != 0     */
/*                        it will be added to those uint64 arrays and new    */
/*                        (incremental) remainders will be calculated (and   */
/*                        odd_rand will not be used).                        */
/*                                                                           */
/* OUTPUT: SixNm1[] and SixNp1[] - The boolean tables associated with the    */
/*                                 numbers of the form 6n-1 and 6n+1         */
/*                                 respectively. A TRUE entry means that the */
/*                                 associated number is composite (i.e.      */
/*                                 failed the trial-division stage). A false */
/*                                 entry means that the associated number is */
/*                                 ready for the Miller-Rabin test.          */
/*                                                                           */
/* The function returns TRUE if one or more numbers did not divide by any of */
/* the tried small primes. If all tested numbers were found to be composite, */
/* the function returns FALSE.                                               */
/* WARNING: The input array odd_rand[] is modifyied! I'll try to change this */
/* in a future version (CL)                                                  */

bool inc_search_trial_div32 (dword odd_rand[/*size*/], word size,
                             const dword prime[/*psize*/], const word psize,
                             PrimeOrder32b *primefactor/*[PSIZE][PORDER32b]*/,
                             bool SixNm1[/* max */], bool SixNp1[/* max */],
                             int max, word inc1)
{
    bool composite, *SixN;
    int k;
    word inc;
    const word mask = 6;
    uint64 Mod_FermatPrimes [FPSIZE];
    uint64 ModsumFP32;
    uint64 Mod_sum32[PSIZE];

    /* Initialise values and tables */
    k = 0;
    inc = 4;

    memset(&SixNm1[0], FALSE, sizeof(bool)*max);
    memset(&SixNp1[0], FALSE, sizeof(bool)*max);

    for (k = 0; k < 2*max; k++)
    {
        /* Test trial division by Fermat primes */
        composite = FermatPrimeDividesP (odd_rand, size, &Mod_FermatPrimes[0]/*[ FPSIZE ]*/, &ModsumFP32, inc1);

        if (composite)
        {
            if (inc == 4)
                SixN = SixNm1;
            else
                SixN = SixNp1;

            /* Update table SixNm1[] or SixNp1[] using the array Mod_sumFP[] */
            /* and fermatP[]. The update is done only if any of the Fermat   */
            /* primes divides odd_rand.                                      */
            update_SixN_table32 (SixN, max, &fermatP[0], (word)FPSIZE, &Mod_FermatPrimes[0], k>>1);
        }

        /* Test trial division by small primes (that are not Fermat primes) */
        composite = multiprec_TrialDivide32 (odd_rand, size,
        /* the test factors (prime numbers < B) */
        prime/*[PSIZE]*/, psize,
        /* The table with the orders of B^i mop p */
        primefactor/*[PSIZE][PORDER32b]*/,
        /* the remainders' table to be initialized or updated: */
        &Mod_sum32[0]/*[ PSIZE ]*/, inc1);


        /* Update table SixNm1[] or SixNp1[] using the array Mod_sum32[] and */
        /* prime_table[]. The update is done only if any of the small primes */
        /* divides odd_rand.                                                 */
        if (composite)
        {
            if (inc == 4)
                SixN = SixNm1;
            else
                SixN = SixNp1;

            update_SixN_table32 (SixN, max, prime, psize, Mod_sum32, k>>1);
        }


        /* Increment odd_rand by 2 or 4 */
        inc ^= mask;
        inc1 = inc;
    }

    /* print_SixN(SixNm1, SixNp1, max); <----------------------------- */

    for (k = 0; k < max; k++)
        if (!SixNm1[k])
            return TRUE;

    for (k = 0; k < max; k++)
        if (!SixNp1[k])
            return TRUE;

    return FALSE;

}


/* This function tests the divisibility of an odd  multiprecision number by */
/* first 5 Fermat prime numbers (of the form 2^n + 1, n = 0, 1, 2, ...),    */
/* i.e. the numbers:                                                        */
/* [0] =     3                                                              */
/* [1] =     5                                                              */
/* [2] =    17                                                              */
/* [3] =   257                                                              */
/* [4] = 65537                                                              */
/*                                                                          */
/* INPUTS: odd_num[] - The multiprecision odd number to be tested           */
/*              size - size of odd_num[], in words                          */
/*        *Mod_FP    - a pointer to an array of 5  dword cells where the    */
/*                     output will be returned. Any entry 0 in this table   */
/*                     indicates that the associated prime divides odd_num. */
/*          *modsum -  A pointer to a uint64 varible that will hold the sum */
/*                     of all digits of odd_num[].                          */
/*              inc - Indicates whether or not the variable modsum is going */
/*                    to be initialised (inc = 0) with the sum of all       */
/*                    digits of odd_num[] or just be incremented (inc != 0) */
/*                    such that modsum = modsum + inc.                      */
/*                                                                          */
/* OUTPUT: Mod_FP[]    - The remainder of the division of odd_num[] by the  */
/*                       Fermat primes stored in fermatP[]                  */
/*                                                                          */
/*         The function returns: TRUE, if odd_num[] is divisible by any of  */
/*                                     the 5 Fermat primes or is even       */
/*                               FALSE, if odd_num[] is NOT divisible by    */
/*                                      any of the 5 Fermat primes and  it  */
/*                                      is not even.                        */
/*                                                                          */

bool FermatPrimeDividesP (dword odd_num[/* size */], word size,
                          uint64 Mod_FP[/* FPSIZE */], uint64 *modsum, word inc)
{
    int i;
    bool divides = FALSE;

    if (!inc)
    {
        (*modsum) = 0;

        /* Adds all digits of odd_num[] to get the remainder */
        for (i = 0; i < size; i++)
            (*modsum) += (uint64)odd_num[i];

        /* test if the number is even */
        if ( !(odd_num[0] & 1) )
            divides = TRUE;
    }
    else
        (*modsum) += inc;

    /* tests for divisibility by each of the considered Fermat primes: */
    for (i = 0; i < FPSIZE && !divides; i++)
    {
        Mod_FP[i] = (*modsum) % fermatP[i];
        if (Mod_FP[i] == (uint64)0)
            divides = TRUE;
    }

    return divides;
}


/*
** update_SixN_table32()
** This function is used in the trial division stage of the incremental search
** for prime numbers. The function expects a boolean table SixN[] of a given
** size "max" where each successive entry corresponds to the odd numbers 6n-1,
** 6(n+1)-1, ... 6(n+max-1)_1 or 6n+1, 6(n+1)+1, ... , 6(n+max-1)+1. When a
** given odd number (of the form 6n-1 or 6n+1) is found to be divisible by one
** of the small primes p_i < B then the entry in the table SixN[]
** corresponding to that number is set to TRUE, as well as all subsequent
** entries starting from that initial entry and spaced at p_i, 2*p_i, ...,
** j*p_i, such that j*p_i < max. This works because all the small primes p_i's
** are relatively prime to 6. The exceptions are 2 and 3 but because only odd
** numbers are considered and the odd numbers considered (6n-1 and 6n+1) are
** never divible by 3, the divisibility update works fine.
**
** INPUTS: SixN[] - A boolean table associated with odd numbers of the form
**                  6n-1 and 6n+1.
**            max - The size of SixN[].
**        prime[] - The table containing the small primes used in the
**                  divisibilty test.
**          psize - The size of prime[] and mod_sum[].
**      mod_sum[] - The array containing the remainder (modulo) of the
**                  division of the odd number being tested by all small
**                  primes in the table prime[].
**              k - The entry point in the table SixN[] where the update
**                  should start from.
**
** OUTPUT : SixN[] - The updated boolean table.
*/

void update_SixN_table32 (bool SixN[/*max*/],
                          word max, const dword prime[/*psize*/],
                          word psize, uint64 mod_sum[/*psize*/], int k)
{
    int i, j;

    /* Update table SixN[] using either the arrays ModsumFP32[]  */
    /* and fermatP[] or the arrays Mod_sumFP[] and prime_table[]: */
    for (i = 0; i < psize; i++)
    if (mod_sum[i] == 0)
    {
        for (j = 0; (j + k) < max; j += prime[i])
            SixN[j + k] = TRUE;
    }
}

/* - multiprec_TrialDivide32 ()                                              */
/* This functions tests if an odd_number is divisible any prime number below */
/* a specified bound B defined by the array of primes prime[] and psize,     */
/* i.e. prime[psize-1] < B. For example, if B = 256 than only 20% of all odd */
/* number candidates will pass this trial division. This function must be    */
/* called before calling the more computing intensive function               */
/* Miller_Rabin_test32(). This function requires a table of the order of     */
/* (2^32)^i, i = 1, ..., PORDER32b, modulo all the primes in the prime[]     */
/* table (defined in "prime32b_order.h").                                    */
/*                                                                           */
/* INPUTS: odd_num[] - The odd multiprecision number to be tested            */
/*             dsize - The size in uint32 of odd_num[]                       */
/*         prime[]   - A table of prime numbers defined in "prime32b_order.h"*/
/*         psize     - The size of the prime[] table                         */
/*   prime_order[][] - A two dimensional table defined in "prime32b_order.h" */
/*                     that contains the order of                            */
/*                     (2^32)^i, i = 1, ..., PORDER32b, modulo all the       */
/*                     primes in the prime[] table                           */
/*                                                                           */
/*         The function returns: TRUE, if odd_num[] is divisible by any of   */
/*                                     the small primes in the prime[] table.*/
/*                               FALSE, if odd_num[] is NOT divisible by     */
/*                                      any of the small primes in the X     */
/*                                      prime[] table.                       */

bool multiprec_TrialDivide32 (dword odd_dnum[/*size*/], word dsize,

                              /* the test factors (prime numbers < B) */
                              const dword prime[/*psize*/], word psize,

                              /* The table with the orders of B^i mop p */
                              PrimeOrder32b * prime_order/*[PSIZE][PORDER32b]*/,

                              /* the remainders' table to be initialized or updated: */
                              uint64 Mod_sum[/* psize */], word inc)
{
    int i, j;
    bool divides = FALSE;

    if (!inc)
    {/* multiply prime orders with number digits and add: */
        for(i = 0; i < psize; i++)
        {
            Mod_sum[i] = (uint64)odd_dnum[0];

            for (j = 0; j < dsize - 1; j++)
                Mod_sum[i] += (uint64)odd_dnum[j+1]*(uint64)((*prime_order)[i][j]);
        }
    }
    else /* update Mod_sum table with the increment */
        for (i = 0; i < psize; i++)
            Mod_sum[i] += (uint64)inc;


    /* Calculate Mod_sum[i] mod prime[i] */
    for (i = 0; i < psize; i++)
    {
        Mod_sum[i] %= (uint64)prime[i];
        if (Mod_sum[i] == (uint64)0)
            divides = TRUE;
    }

    return divides;
}

/* - Prime_test32 ()                                               */
/* This function tests a given odd number for primality using the  */
/* Miller-Rabin test for probable primes and for the first t prime */
/* numbers as the base for those tests                             */
/*                                                                 */
/* INPUT: odd_num[] - The multiprecision number to be tested       */
/*        size      - The size, in uint32, of odd_num              */
/*        t         - the number of prime bases the input odd_num  */
/*                  - will be tested against.                      */
/*                                                                 */
/* OUTPUT: The function returns                                    */
/*         FALSE - odd_num is composite, or                        */
/*         TRUE  - odd_num is probably prime (with a probability   */
/*                 depending up on the number t of tested bases)   */

bool Prime_test32 (dword odd_num[/*size*/], word size, word t,const dword prime_tab[/*psize*/], const word psize)
{
    dModulus ms;
    dword *temp, *temp1;
    dword base, p;
    bool prime;
    word rsize;

    /* If odd_num is even return imediately */
    if (!(odd_num[0] & 1))
        return FALSE;

    if (size > (word)KEY_WIDTH_W)
        size = (word)KEY_WIDTH_W;

    /* get the real size of odd_num[] */
    rsize = size + 1;
    while (!odd_num[--rsize - 1] && (rsize > 0))
        ;

    /* Create memory space and initialize it to zero */
    temp   = (dword *)calloc(rsize, sizeof(dword));
    temp1  = (dword *)calloc(rsize, sizeof(dword));

    /* get the M_struct32 for odd_num */
    prime = create_Modulus_struct (odd_num, rsize, &ms, TRUE);

    if(prime)
    {
        (void) psize;
        for (p = 0; (p < t) && prime; p++)
        {
            if (p > 6)
                base = prime_tab[p - 4];
            else
                base = (dword)PbaseT[p];
            prime = Miller_Rabin_test32 (odd_num, rsize, &ms, base, temp, temp1);
        }
    }

    free ((void *) temp);
    free ((void *) temp1);

    return prime;
}



/* - create_Modulus_struct()                                */
/* This function generates a data struct necessary for      */
/* performing modular multiplication using the Montgomery   */
/* algorithm with uint32 as the basic data type [see        */
/* function Mont_mult32()]. Usually the modulus degree is   */
/* 32*size - 1 and in this case the boolean input generic   */
/* must be set to FALSE to allow a faster computation.      */
/* Nevertheless sometimes the degree of the modulus varies  */
/* between 32*size - 1 and 32*(size - 1), e.g. during a     */
/* prime or strong prime number search, so to accommodate   */
/* this the input variable generic must be set to TRUE.     */
/*                                                          */
/* INPUTS: M[]  - the modulus                               */
/*         size - the size in uint32 of M[]                 */
/*         ms   - the structure where the output will be    */
/*                returned.                                 */
/*         generic - FALSE => degree of M[] is 32*size - 1, */
/*                   TRUE => degree of M[] >= 32*(size - 1) */
/*                           and <= 32*size - 1.            */
/*         The function returns the real size, in uint32,   */
/*         of M[]                                           */

bool create_Modulus_struct (dword M[/*size*/], word size,
                            dModulus *ms, bool generic )
{
    if ( size <= (word)KEY_WIDTH_W )
    {
        /* clear the M_struct */
        memset (&(ms->M[0]), 0, sizeof(dword)*KEY_WIDTH_W);
        memset (&(ms->RNmodM[0]), 0, sizeof(dword)*KEY_WIDTH_W);
        memset (&(ms->R2NmodM[0]), 0, sizeof(dword)*KEY_WIDTH_W);
        memset (&(ms->one[0]), 0, sizeof(dword)*KEY_WIDTH_W);

        /* copy the modulus M into the M_struct */
        memcpy( &(ms->M[0]), M, sizeof(dword)*size);
        ms->M_dash = get_inv_ofM (ms->M[0]);

        /* get R mod M = R - M */
        if (generic)
            get_RmodM32G (&(ms->RNmodM[0]), M, size);
        else
            mp_sub (&(ms->RNmodM[0]), M, size);

        /* get R^2 mod M */
        if (generic)
            get_R2modM32G (&(ms->R2NmodM[0]), M, size);
        else
            get_R2modM32 (&(ms->R2NmodM[0]), M, size);

        ms->one[0] = 1;

        return TRUE;
    }
    else
        return FALSE;
}


/* - get_inv_ofM ()                                            */
/* The following function gets the smallest word of -M^(-1)    */
/* where M is a number with Wn digits over the base R = 2^T.   */
/* This value is essential to compute the Montgomery           */
/* multiplication and reduction.                               */
/*                                                             */
/* INPUT: m0, the least significant word of M (the modulus).   */
/*                                                             */
/* OUTPUT: the function returns the word value                 */
/*         -M^(-1) mod (2^DWORD_BITS)                          */

dword get_inv_ofM (dword m0)
{
    uint64 P = 0;
    const uint64 mask = 1;
    word i;
    dword m_dash = 1;

    /* P is initialized with m0 */
    P = (uint64) m0;

    /* obtains M^(-1) mod R */
    for (i = 1; i < DWORD_BITS; i++)
        if ((P >> i) & mask)
        {
            m_dash += ((dword)1 << i);
            P += (m0 << i);
        }

    /* obtains -M^(-1) mod R */
    m_dash = (dword)(~m_dash + 1);

    return m_dash;
}

/* - get_RmodM32G ()                                               */
/* This function obtains the value of of 2^(size*DWORD_BITS)       */
/* modulo a modulus M where the degree of M can vary between       */
/* size*DWORD_BITS - 1 and size*(DWORD_BITS - 1). It is expected   */
/* that the input size strictly corresponds to the minimum number  */
/* of double words necessary to represent the modulus M.           */
/*                                                                 */
/* INPUT: RmodM[] - A container for 2^(size*DWORD_BITS) mod M[]    */
/*            M[] - The modulus, a multiprecision number           */
/*           size - The size in uint32 of both the input and       */
/*                  output numbers                                 */
/*        The function returns the real size (in uint32) of the    */
/*        output number.                                           */

word get_RmodM32G (dword RmodM[/*size*/], dword M[/*size*/], word size)
{
    int k, dN, dm, dy, subs = 0;
    word rsize;

    dm = mp_degree (M, size);

    /* the real size, in uint32, of the modulus M */
    rsize = (dm/DWORD_BITS) + 1;

    /* The mininum degree multiple of 32 bigger than dm */
    dN = rsize*DWORD_BITS;

    k = dN - dm - 1;

    /* Scale the modulus M up to be a number one */
    /* degree less than dN i.e. M = M*2^k.     */
    mp_Lshift (M, k, rsize);

    /* clear the memory space where RN mod M will be returned */
    memset (RmodM, 0, sizeof(dword)*(size));

    /* get R mod M*2^k = R - M*2^k */
    mp_sub (RmodM, M, rsize);

    /* At this point we have RmodM = R mod M*2^k */
    dy = mp_degree (RmodM, rsize);

    /* Set M to its initial value */
    mp_Rshift (M, k, rsize);

    while (dy > dm)
    {
        k = dy - dm - 1;
        mp_Lshift (M, k, rsize);
        mp_sub ( RmodM, M, rsize);
        subs++;
        dy = mp_degree (RmodM, rsize);
        /* restore the value of M */
        mp_Rshift (M, k, rsize);
    }

    /* If RmodM >= M do RmodM = RmodM - M */
    if (mp_gte (RmodM, M, rsize))
    {
        mp_sub ( RmodM, M, rsize);
        subs++;
    }

    return rsize;
}

/* - get_R2modM32 ()                                         */
/* Same as get_R2modM() but using 32-bit (uint32) arithmetic */
/* instead of 16-bit (uint16) arithmetic                     */

void get_R2modM32 (dword R2modM[/*size*/], dword M[/*size*/], word size)
{
    dword i;
    int j, subs = 0;
    bool mswOf;

    memset (R2modM, 0, sizeof(dword)*(size));

    /* get R mod M = R - M */
    mp_sub (R2modM, M, size);

    for (i = 0; i < DWORD_BITS *size; i++)
    {
        mswOf = (R2modM[size-1] >> (DWORD_BITS -1)) == 1;

        /* R2modM = R2modM*2 */
        for(j = size-1; j > 0; j--)
            R2modM[j] = (R2modM[j] << 1) | (R2modM[j-1] >> (DWORD_BITS  - 1));
        R2modM[0] = R2modM[0] << 1;

        /* If R2modM >= M do R2modM = R2modM - M */
        if ( mswOf || mp_gte (R2modM, M, size))
        {
            mp_sub( R2modM, M, size);
            subs++;
        }
    }
}


/* - get_R2modM32G ()                                              */
/* Same as get_R2modM32() but allows the degree of M to be between */
/* size*DWORD_BITS  - 1 and size*(DWORD_BITS  - 1). It is expected */
/* that the input size strictly corresponds to the minimum number  */
/* of double words necessary to represent the modulus M.           */
/* This function is compatible but more generic than the function  */
/* get_R2modM32(). The function returns the real size of the       */
/* modulus M.                                                      */

word get_R2modM32G (dword R2modM[/*size*/], dword M[/*size*/], word size)
{
    int j, k, dN, dm, dy, subs = 0;
    bool mswOf;
    word i, rsize;

    dm = mp_degree (M, size);

    /* the real size, in uint32, of the modulus M */
    rsize = (dm/DWORD_BITS ) + 1;

    /* The mininum degree multiple of 32 bigger than dm */
    dN = rsize*DWORD_BITS ;

    k = dN - dm - 1;

    /* Scale the modulus up to be a number one */
    /* degree less than dN i.e. M = M*2^k.     */
    mp_Lshift (M, k, rsize);

    /* clear the memory space where R^2N mod M will be returned */
    memset (R2modM, 0, sizeof(dword)*(size));

    /* get R mod M*2^k = R - M*2^k */
    mp_sub (R2modM, M, rsize);

    for (i = 0; i < DWORD_BITS *rsize; i++)
    {
        mswOf = (R2modM[rsize-1] >> (DWORD_BITS -1)) == 1;

        /* R2modM = R2modM*2 */
        for(j = rsize-1; j > 0; j--)
            R2modM[j] = (R2modM[j] << 1) | (R2modM[j-1] >> (DWORD_BITS  - 1));
        R2modM[0] = R2modM[0] << 1;

        /* If R2modM >= M do R2modM = R2modM - M */
        if ( mswOf || mp_gte (R2modM, M, rsize))
        {
            mp_sub( R2modM, M, rsize);
            subs++;
        }
    }

    /* At this point we have R2modM = R^2N mod M*2^k */
    dy = mp_degree (R2modM, rsize);

    /* Set M to its initial value */
    mp_Rshift(M, k, rsize);

    while (dy > dm)
    {
        k = dy - dm - 1;
        mp_Lshift(M, k, rsize);
        mp_sub( R2modM, M, rsize);
        subs++;
        dy = mp_degree (R2modM, rsize);
        /* restore the value of M */
        mp_Rshift(M, k, rsize);
    }

    /* If R2modM >= M do R2modM = R2modM - M */
    if (mp_gte (R2modM, M, rsize))
    {
        mp_sub( R2modM, M, rsize);
        subs++;
    }

    return rsize;
}


/* - Miller_Rabin_test32()                                                   */
/* This function performs the probabilistic Miller Rabin test for primality  */
/* of a given input odd number using a specified (input) base.               */
/*                                                                           */
/* INPUTS: odd_num[] - A multiprecision odd number to be tested for          */
/*                     primality.                                            */
/*         sisze     - The size of odd_num[], in uint32                      */
/*         *ms       - A pointer to a M_struct32 variable generated using    */
/*                     odd_num[] and the function create_M_struct32(). This  */
/*                     struct is necessary for performing the modular        */
/*                     exponentiations required in the Miller-Rabin test.    */
/*         base      - A single precision number, usually a prime, that will */
/*                     be a "strong witness" if odd_num is found to be       */
/*                     composite or "strong liar" if odd_num is found to     */
/*                     be prime. The first base to be tried must be 2 as     */
/*                     most composite numbers will fail with it and also it  */
/*                     is very efficient to perform exponentiation base 2 in */
/*                     digital computers.                                    */
/*     *temp, *temp1 - Scratch memory space with size double words (uint32)  */
/*                     each.                                                 */
/*                                                                           */
/* OUTPUT: The function returns                                              */
/*         FALSE - odd_num[] is definitely a composite number (not prime)    */
/*         TRUE  - odd_num[] is probably prime, i.e., the input base is a    */
/*                 strong liar" for the primality of odd_num[]               */

bool Miller_Rabin_test32 (dword odd_num[], word size, dModulus *ms,
                          dword base, dword *temp, dword *temp1)
{
    int j;
    word s;

    /* temp1 = odd_num is copied into temp */
    memcpy(temp1, odd_num, sizeof(dword)*size);

    /* temp is set to zero. It will hold the base. */
    memset(temp, 0, sizeof(dword)*size);

    /* temp1 = r such that 2^s * r = n - 1 */
    s = Miller_Rabin_Rshift (temp1, size);

    /* y = temp = base^r mod n */
    if (base == 2)
         mp_2exp (temp, temp1, size, ms);
    else
    {
        temp[0] = base;
        mp_exp (temp, temp1, size, ms);
    }

    /* check if y != 1 */
    if (mp_isequal (temp, ms->one, size))
        return TRUE; /* this means that y = 1 */

    /* check if y == n-1 <=> y + 1 = n */
    mp_addWC(temp, size, 1);

    if (mp_isequal (temp, odd_num, size))
        return TRUE; /* this means that y = n-1 */

    mp_subWC(temp, size, 1);

    for (j = 1; j < s; j++)
    {
        /* y = y^2 mod n */
        mp_expFP (temp, size, 2, ms);

        /* If odd_num is prime than base^[(2^(j-1))r] mod n must be n-1 */
        /* for some j < s. Check if y == n-1 <=> y + 1 = n              */
        mp_addWC(temp, size, 1);
        if (mp_isequal (temp, odd_num, size))
            return TRUE; /* this means that y = n-1 */
        else
        {
            mp_subWC(temp, size, 1);

            /* if y = 1 then odd_num is composite */
            if (mp_isequal (temp, ms->one, size))
                return FALSE; /* this means that y = 1 without the previous */

            /* value of y being equal to n - 1            */
        }
    }

    return FALSE; /* this means odd_num is composite */
}


/* Miller_Rabin_Rshift ()                                       */
/* This function transforms an odd number MR_exp such that it   */
/* is transformed in the form MR_exp - 1 = 2^s * r, where s is  */
/* a positive integer and r is an odd number. This means that   */
/* if we have a number n^r and square it progressively, sooner  */
/* or later the exponent will be 2^s * r.                       */
/*                                                              */
/* INPUTS: MR_ex[] - A multiprecision odd number                */
/*         size    - the size in uint32 of MR_exp[]             */
/*                                                              */
/* OUTPUT: MR_ex[] - An odd number r (see above)                */
/*         The function returns the single-precision value s    */
/*         (see above)                                          */

word Miller_Rabin_Rshift (dword MR_exp[/*size*/], const word size)
{
    int i;
    word j;
    word s;

    i = 0;
    s = 0;
    j = 0;

    /* MR_exp = odd_num - 1 */
    mp_subWC (MR_exp, size, 1);

    /* Check dwords that are zero : */
    while ((i < size) && !MR_exp[i++])
        s += DWORD_BITS;

    i -= 1;

    /* Check number of contiguous zero bits    */
    /* starting in bit position b_0 up to b_31 */
    while ((i < (signed)size) && (j < DWORD_BITS) && (~(MR_exp[i] >> j++) & 1))
        s += 1;

    /* The exponent is formatted to be of the form (2^s)*r    */
    /* setting MR_exp = r and the function's return value = s */
    mp_Rshift (MR_exp, s, size);

    return s;
}

