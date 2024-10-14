/**********************************************************************
 *
 *  strong_prime.c
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To implement functions to find strong primes using the gordon 
 *  algorithm.
 *
 **********************************************************************/

#include <stdlib.h>
#include "keygen_private.h"
#include "trial_division.h"
#include <string.h>

typedef struct pk
{
    dword degree;
    dword prime[17];
}
primeKey32;

#define T_FIDDLE (11)
#define S_FIDDLE (8)

/***************************************************************************/

/* - get_Gordon_p0_32 ()                                           */
/* This function gets the constant p0 = 2*(s^(r-2) mod r)*s - 1,   */
/* where s and r are primes of roughly the same bitlength          */
/*                                                                 */
/* INPUTS: s[] and r[] - The input prime numbers.                  */
/*              temp[] - scratch space                             */
/*                size - The size, in uint32, of s[], r[] and      */
/*                       temp[].                                   */
/*                                                                 */
/* OUTPUT: r[] = r*s                                               */
/*         s[] = p0 = 2*(s^(r-2) mod r)*s - 1                      */

void get_Gordon_p0_32 (dword s[/*size*/], dword r[/*size*/],
                  dword temp[/*size*/], word size)
{
    dModulus ms;
    dword *temp1, *temp2;
    bool prime;
    word rsize;

    if (size > (word)KEY_WIDTH_W)
        size = (word)KEY_WIDTH_W;

    temp1  = (dword *)calloc(size, sizeof(dword));
    temp2  = (dword *)calloc(size, sizeof(dword));

    /* get the real size of r[] */
    rsize = size + 1;
    while (!r[--rsize - 1] && (rsize > 0))
        ;

    /* temp = temp1 = s */
    memcpy (temp, s, sizeof(dword)*size);
    memcpy (temp1, s, sizeof(dword)*size);
    /* temp2 = r */
    memcpy (temp2, r, sizeof(dword)*size);

    /* get the dModulus for r[] */
    prime = create_Modulus_struct (r, rsize, &ms, TRUE);

    /* s must be less than r in order the Montgomery algorithm works correctly */
    while (mp_gte (temp, r, size))
        mp_sub (temp, r, size);

    /* Set r = r-2 */
    mp_subWC (r, size, 2);

    /* get temp = s^(r-2) mod r */
    mp_exp (temp, r, rsize, &ms);

    /* get s = temp*s = (s^(r-2) mod r)*s */
    mp_multiply (s, temp, size, size, r);

    /* set s = 2*(s^(r-2) mod r)*s */
    mp_Lshift (s, 1, size);

    /* set p0 = s = 2*(s^(r-2) mod r)*s - 1 */
    mp_subWC (s, size, 1);

    /* Restore original value of r */
    memcpy (r, temp2, sizeof(dword)*size);

    /* get r = r*s */
    mp_multiply (r, temp1, size, size, temp);

    free ((void *) temp1);
    free ((void *) temp2);
}

void next_primeN32 (dword prime_rand[/*size*/], word size, dword *degree, word t,
                    dword odd_rand[/*size*/], bool randn )
{
    int i, max;
    bool found, composite;
    uint64 rem;
    word inc;

    /* AJH */
    bool SixNplus1 [MAX_PRIME_SEARCH];
    bool SixNminus1 [MAX_PRIME_SEARCH];
    uint64 Mod_FermatPrimes [FPSIZE];
    uint64 ModsumFP32;
    uint64 Mod_sum32[PSIZE];

    /* Initialise values and tables */
    max = (int)MAX_PRIME_SEARCH;
    rem = 0;
    inc = 0;

    if (randn) /* gets an odd random number of the form 6N-1 */
    {
        /* Get an odd random number of degree "degree - 1" */
        get_nbit_odd_rand32 (odd_rand, size, (*degree) -1 );

        /* Set the odd number to be of the form 3N */
        for (i = 0; i < size; i++)
        rem += (uint64)odd_rand[i];

        i = (int)(rem % (uint64)3);

        if (i == 1)
            mp_addWC (odd_rand, size, 2);
        else if (i == 2)
            mp_subWC (odd_rand, size, 2);

        /* Set the number to be of the form 6N */
        mp_Lshift (odd_rand, 1, size);

        /* Set the number to be of the form 6N-1 */
        mp_subWC (odd_rand, size, 1);

        /*rsize = mp_degree  (odd_rand, size)/DWORD_BITS + 1; */

        /* Proceed now for incremental search */
    }
    else
    { 
        /* This is a user provided number. If the number is of the form 6n+1,    */
        /* test for primality, If found prime return, if not proceed as usual    */
        /* for incremental search. If the number is not of the form 6n+1 set it  */
        /* to be of the form 6n-1 and proceed for the incremental search.        */

        odd_rand[0] |= 1; /* make sure the number is odd */

        /* Check odd_num[] mod 3 */
        for (i = 0; i < size; i++)
            rem += (uint64)odd_rand[i];

        i = (int)(rem % (uint64)3);

        if (i == 0)
            mp_addWC (odd_rand, size, 2);
        else if (i == 1)
        { /* the number is of the form 6n+1. Check it for primality */
            composite = FermatPrimeDividesP (odd_rand, size,
            &Mod_FermatPrimes[0]/*[ FPSIZE ]*/,
            &ModsumFP32, 0);

            if (!composite)
                composite = multiprec_TrialDivide32 (odd_rand, size,
                                    &prime_table[0]/*[PSIZE]*/, (word)PSIZE,
                                    &primefactor/*[PSIZE][PORDER32b]*/,
                                    &Mod_sum32[0]/*[ PSIZE ]*/, 0);

            if (!composite)
                if (Prime_test32 (odd_rand, size, t, prime_table , PSIZE))
                {  /* The number is a probable prime */
                    memcpy(prime_rand, odd_rand, sizeof(dword)*size);
                    *degree = mp_degree  (prime_rand, size);
                    return;
                }

            /* set odd_num[] to be of the form 6n - 1 */
            mp_addWC (odd_rand, size, 4);

        }

    }

    memcpy(prime_rand, odd_rand, sizeof(dword)*size);

    while (TRUE) /* Cycle until a prime is found */
    {
        /*Perform trial divisions on the numbers generated by the incremental */
        /*search, starting from the number obtained above, i.e. of the form   */
        /*6n-1, before applying the more costly Miller-Rabin test:            */
        found = inc_search_trial_div32 (prime_rand, size, prime_table , PSIZE ,
                                        &primefactor, SixNminus1, SixNplus1, max, inc);

        /* Perform the Miller-Rabin test on all odd numbers */
        /* that passed the previous trial division test:    */
        if (found)
        {
            /* Check for probable primes of the form 6n - 1: */
            for (i = 0; i < max; i++)
            {
                if ( !SixNminus1[i] )
                {
                    if (Prime_test32 (prime_rand, size, t, prime_table , PSIZE))
                    {
                        *degree = mp_degree  (prime_rand, size);
                        return ;
                    }
                    else
                        SixNminus1[i] = TRUE;
                }

                mp_addWC (prime_rand, size, 2);

                if (!SixNplus1[i])
                {
                    if (Prime_test32 (prime_rand, size, t, prime_table , PSIZE))
                    {
                        *degree = mp_degree  (prime_rand, size);
                        return ;
                    }
                    else
                        SixNplus1[i] = TRUE;
                }

                mp_addWC (prime_rand, size, 4);
            }

        }
        else
            mp_addWC (prime_rand, size, 6*max);

    }

}


/* strong_primeN32()                                                         */
/* This function returns a random strong prime P using Gordon's algorithm.   */
/* This strong prime P is defined has having a large prime R as one factor   */
/* of P-1, a large prime S as one factor of P+1 and a large prime T as one   */
/* factor of R-1. The function aims to return a strong prime of degree equal */
/* to the requested degree but this cannot be guaranteed, so the user has to */
/* check if the obtained strong prime can be used or not.                    */
/*                                                                           */
/* INPUTS: prime_rand[] - A container where the requested Gordon's strong    */
/*                        prime number will be returned.                     */
/*                 size - The size, in uint32, of prime_rand[].              */
/*               degree - The degree of the requested strong prime  number.  */
/*                        The function might return a strong prime of a      */
/*                        different degree, so the user must check this      */
/*                        variable after a strong prime is obtained.         */
/*                    t - The number of bases to be used in the Miller-Rabim */
/*                        probabilistic primality test. For speed reasons,   */
/*                        it is recomended that t = 1 or t = 2. The greater  */
/*                        the value of t, the greter the confidence that the */
/*                        number is prime but that will also increase the    */
/*                        computation time.                                  */
/*       f2mod3, f1mod3 - Pointer to files where the "fished" prime numbers  */
/*                        should be stored. fsmod3 is the pointer to the     */
/*                        file where primes of the form 6N-1 will be put,    */
/*                        while must f1mod3 point to the file where primes   */
/*                        of the form 6N+1 should be stored.                 */
/*                        Each prime stored as a primeKey32 struct will      */
/*                        occupy 72 bytes of meory space                     */
/*                                                                           */
/* OUTPUT: prime_rand[] - A probable strong prime number with a requested    */
/*                        degree of "degree".                                */
/*               degree - The actual degree of the found probable strong     */
/*                        prime.                                             */
/*                                                                           */
/* The function returns a double word (uint32) were the most significant     */
/* half contains the number of probable primes of the form 6N-1 and the      */
/* least significant half the number of probable primes of the form 6N+1,    */
/* i.e. |<---- return  ---->|                                                */
/*       31 ... 16 15  ... 0                                                 */
/*      | #6n - 1 | #6n + 1 |                                                */
/*      ---------------------                                                */


bool strong_prime (dword prime_rand[/*size*/], word size, dword *degree,
                      word t )
{
    int i, j, max;
    bool composite, got_one = FALSE;
    dword inc, t_degree, s_degree;
    dword *s_temp, *r_temp, *p_temp, *w_temp;
    word r_total;
    primeKey32 strong;
    dword p2mod3, p1mod3;
    word R_index[10];
    uint64 Mod_FermatPrimes [FPSIZE];
    uint64 ModsumFP32;
    uint64 Mod_sum32[PSIZE];

    /* Initialise values and tables */
    max = (int)MAX_PRIME_SEARCH;
    inc = 0;

    if ( *degree < 32 )
        return FALSE;

    t_degree = ( *degree/3 > T_FIDDLE ) ? *degree/2 - T_FIDDLE : (*degree - 1)/2;

    /* Create memory space and initialize it to zero */
    r_temp  = (dword *)calloc(size, sizeof(dword));
    s_temp  = (dword *)calloc(size, sizeof(dword));
    p_temp  = (dword *)calloc(size, sizeof(dword));
    w_temp  = (dword *)calloc(size, sizeof(dword));

    /* gets the random prime T of degree t_degree: */
    /*
    **  GET A RANDOM INITIAL PRIME
    */
    next_primeN32 (prime_rand, size, &t_degree, 1, r_temp, TRUE );

    /* Find the first prime in the sequence 2*i*t+1, i = 1, 2, 3, ... */
    i = 0;
    composite = TRUE;
    r_total = 0;
    p2mod3 = 0;
    p1mod3 = 0;

    /*
    **  SEARCH 2*i*(prime_rand) + 1 , 0 <= i < 200 FOR A PRIMES.
    */
    while (++i < GORDON_I_MAX)
    {
        memcpy (r_temp, prime_rand, sizeof(dword)*size);
        /* t = 2*i*t + 1 */
        mp_multiplyWC (r_temp, size, (dword)2*i , w_temp );
        mp_addWC (r_temp, size, 1);


        /* Test t for primality */
        composite = FermatPrimeDividesP (r_temp, size,
                                         &Mod_FermatPrimes[0]/*[ FPSIZE ]*/,
                                         &ModsumFP32, 0);

        if (!composite)
            composite = multiprec_TrialDivide32 (r_temp, size,
                                &prime_table[0]/*[PSIZE]*/, (word)PSIZE,
                                &primefactor/*[PSIZE][PORDER32b]*/,
                                &Mod_sum32[0]/*[ PSIZE ]*/, 0);

        if (!composite)
        {
            composite = !Prime_test32 (r_temp, size, t, prime_table , PSIZE);
            if (!composite)
            {
                /* STORE i */
                R_index[r_total++] = i;
                if ( r_total > 10 )
                    i = GORDON_I_MAX;
            }
        }
    }

    /*
    **  USE THE PRIMES FOUND ABOVE (INDEXED BY i)
    */
    for (i = 0; !got_one && i < r_total; i++)
    {
        /* get the prime R previously "fished" */
        memcpy (r_temp, prime_rand, sizeof(dword)*size);
        /* t = 2*i*t + 1 */
        mp_multiplyWC (r_temp, size, (dword)2*R_index[i], w_temp );
        mp_addWC (r_temp, size, 1);

        t_degree = mp_degree  (r_temp, size);

        /* Get a new prime S */
        s_degree = *degree - t_degree - S_FIDDLE;
        next_primeN32 (s_temp, size, &s_degree, 1, p_temp, TRUE );

        /* s_temp = p0 and r_temp = r*s */
        get_Gordon_p0_32 (s_temp, r_temp, p_temp, size);

        /*
        **  Find the first prime in the sequence
        **  p0 + 2*j*r*s, j = 1, 2, 3, ...
        */
        j = 0;

        while (!got_one && ++j < GORDON_I_MAX)
        {
            /* prime_rand = r*s */
            memcpy(p_temp, r_temp, sizeof(dword)*size);

            /* t = 2*j*t + 1 */
            mp_multiplyWC (p_temp, size, (dword)2*j , w_temp);

            /* prime_rand = p0 + 2*j*r*s */
            mp_add (p_temp, s_temp, size);


            /* Test prime_rand for primality */
            composite = FermatPrimeDividesP (p_temp, size,
                                            &Mod_FermatPrimes[0]/*[ FPSIZE ]*/,
                                            &ModsumFP32, 0);

            if (!composite)
            composite = multiprec_TrialDivide32 (p_temp, size,
                                &prime_table[0]/*[PSIZE]*/, (word)PSIZE,
                                &primefactor/*[PSIZE][PORDER32b]*/,
                                &Mod_sum32[0]/*[ PSIZE ]*/, 0);

            if (!composite)
            {
                composite = !Prime_test32 (p_temp, size, t, prime_table , PSIZE);
                if (!composite)
                {
                    strong.degree = mp_degree  (p_temp, size);
                    if ((strong.degree > (*degree - 2)) && (strong.degree < (*degree + 3)))
                    {
                        memset(strong.prime,0,sizeof(dword)*(KEY_WIDTH_W/2+1));
                        memcpy (&strong.prime[0], p_temp, sizeof(dword)*size);
                        if (Mod_FermatPrimes[0] == 2)
                        {
                            got_one = TRUE;
                        }
                    }
                }
            }
        }
    }

    free ((void *) s_temp);
    free ((void *) r_temp);
    free ((void *) p_temp);
    free ((void *) w_temp);

    if ( got_one )
    {
        memcpy ( prime_rand , strong.prime , sizeof(dword)*size );
        *degree = strong.degree;
    }

    return got_one;
}

