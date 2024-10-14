/******************************************************************************
FILENAME:    crypt_exponentiation.c

Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     exponentiation routines used in rsa

******************************************************************************/
#include "crypt_private.h"
#include <stdlib.h>
#include <string.h>

/******************************************************************************
Local functions
******************************************************************************/

static void Mont_mult (word A[/* size */], const word B[/* size */], const Modulus *ms,
                word P[/* size + 1 */], const word size);

static void multiprec_subtract (word p[/*size*/], const word m[/*size*/], const word size);

static word multiprec_degreeof(const word x[/*size*/], const word size);

static word x_notlessthan_y (const word x[/*size*/], const word y[/*size*/], const word size);

/********************************************************/
/* multiprec_expFP ()                                   */
/* Performs modular exponentiation of a multi-precision */
/* unsigned integer if the exponent is 2 or the Fermat  */
/* prime 3, 17 or 65535                                 */
/*                                                      */
/* INPUTS: Base x[],                                    */
/*         wsize = size in words of x[], x[0] is the    */
/*                 LSWord, x[wsize-1] is the MSWord     */
/*         EXP = constant 2, 3, 17 or 65535             */
/*         *ms = the structure M_struct where the       */
/*         modulus ms->M[wsize] is obtained from        */
/*                                                      */
/* OUTPUT: x = x^EXP mod M                              */
/********************************************************/

void multiprec_expFP (word x[/*wsize*/], const dword EXP, const word wsize, const Modulus *ms)
{
    dword i;
    word *A, *temp;

    if (EXP == 0)
        memcpy( x, &(ms->one[0]), sizeof(word)*wsize);

    else if ((EXP == 3) || (EXP == 17) ||(EXP == 65537))
    {
        /* the exponent is a Fermat prime (2^n + 1): */

        /* Allocates memory for temporary storage needed in this function */
        A    = (word *)calloc(wsize, sizeof(word));
        temp = (word *)calloc(wsize + 1, sizeof(word));

        /* initializes A with R^2N mod M */
        memcpy( A, &(ms->R2NmodM[0]), sizeof(word)*wsize);

        Mont_mult(A, x, ms, temp, wsize); /* = x*R^N mod M */
        for (i = (EXP >> 1); i; i >>= 1 )
            Mont_mult(A, A, ms, temp, wsize); /* = (x^(2^i))*R^N mod M */
        Mont_mult(x, A, ms, temp, wsize); /* = x^EXP mod M */
    }
    else if (EXP == 2)
    {
        temp = (word *)calloc(wsize + 1, sizeof(word));
        Mont_mult (x, x, ms, temp, wsize); /* x = x^2 * R^-1 mod M */
        Mont_mult (x, &(ms->R2NmodM[0]), ms, temp, wsize); /* x = x^2 mod M */
    }
}


/************************************************************/
/* - multiprec_exp ()                                       */
/* Performs modular exponentiation of a multi-precision     */
/* unsigned integer raised to the power of a another        */
/* multi-precision integer, both unsigned and "wsize" words */
/* long. If the exponent is zero, 1 is returned, if it is   */
/* 1, the base is returned. If the base is zero, zero is    */
/* returned. Otherwise the function performs a modular      */
/* Montgomery multiplication with an average number of      */
/* single precision multiplications equal to:               */
/*        3*wsize*(wsize + 1)*(exponent_degree + 1)         */
/*                                                          */
/* INPUTS: x[wsize] - multiprecison base . x[0] is the      */
/*                    LSWord, x[wsize-1] is the MSWord.     */
/*         e[wsize] - multiprecision exponent. e[0] is      */
/*                    the LSWord, e[wsize-1] is the MSWord. */
/*         wsize - size (in words) of both the base and the */
/*                 exponent.                                */
/*         ms    - M_struct from where the modulo ms->M[Wn] */
/*                 and other constants necessary for the    */
/*                 Montgomery multiplicationo are obtained. */
/*                                                          */
/* OUTPUT: x[] = x[]^e[] mod M                              */
/*                                                          */
/* IMPORTANT NOTE: Both input pointers x[] and e[] should   */
/* point to two different memory spaces. If they point to   */
/* the same memory space, i.e. x[] = e[] then the output    */
/* will be unpredictable as e[] will change at each         */
/* iteration.                                               */
/************************************************************/

void multiprec_exp (word x[/*wsize*/], const word e[/*wsize*/], const word wsize, const Modulus *ms)
{
    word degree, wdegree;
    int i, rem;
    word *A, *temp;

    /* Get the degree of x[] */
    degree = multiprec_degreeof(x, wsize);

    if (degree > (wsize * WORD_BITS - 1))
        /* If this is true it implies the base x[] is zero: */
        return;

    /* Get the degree of e[] */
    degree = multiprec_degreeof(e, wsize);


    if (degree > (wsize * WORD_BITS - 1))
    {
        /* If this is true it implies the exponent e[] is zero: */
        memcpy( x, &(ms->one[0]), sizeof(word)*wsize);
        return;
    }
    else if (degree == 0)
        /* If this is true it implies the exponent e[ ] is one: */
        return;

    /* Allocates memory for temporary storage needed in this function */
    A    = (word *)calloc(wsize, sizeof(word));
    temp = (word *)calloc(wsize + 1, sizeof(word));

    /* The degree in words of the exponent e[] */
    wdegree = (degree/WORD_BITS);

    /* Number of significant bits (minus 1) */
    /* in the MSWord of the exponent e[]    */
    rem = degree - wdegree*WORD_BITS;

    /* initializes A with R mod M */
    memcpy(A, &(ms->RNmodM[0]), sizeof(word)*wsize);

    /* x' = x*R (mod M) */
    Mont_mult(x, (word *)&(ms->R2NmodM[0]), ms, temp, wsize);

    for (i = wdegree; i >  - 1; i--)
    {
        while ( rem > -1)
        {
            Mont_mult(A, A, ms, temp, wsize);  /* A = A*A*R^-1 mod M */

            if ((e[i] >> rem) & 1)
                Mont_mult(A, x, ms, temp, wsize); /* A = A*x'*R^-1 mod M */

            rem--;
        }

        rem = WORD_BITS-1;
    }

    /* A = A*R^-1 mod M */
    Mont_mult(A, &(ms->one[0]), ms, temp, wsize);

    /* The output result x[] = A[] = x[]^e[] */
    memcpy(x, A, sizeof(word)*wsize);

    free ((void *) A);
    free ((void *) temp);

}

/************************************************************/
/* - Mont_mult ()                                           */
/* The following function performs the Montgomery modular   */
/* multiplication/reduction of two numbers A and B such     */
/* that A = A*B*R^(-1) mod M, where A, B and M are          */
/* "size" words long multi precision numbers/arrays such    */
/* that A[0], B[0] and M[0] are their least significant     */
/* digits and A[size-1], B[size-1] and M[size-1] are        */
/* their most significant digits. [NOTE; In the XAP         */
/* assembly function "xap_mont_mult()" (see below) these    */
/* numbers must be input in reverse order, with the most    */
/* significant digit in position [0] (the output will       */
/* also be in reverse order) ].                             */
/* The other input variable P[size + 1] is only used for    */
/* storing temporary results within this function. It is    */
/* passed as a reference to save time having to allocate    */
/* size+1 words of memory each time this function is        */
/* called.                                                  */
/*                                                          */
/* INPUT: A, B, P and the M_struct                          */
/*                                                          */
/* OUTPUT: A = A*B*R^(-1) mod M                             */
/************************************************************/

void Mont_mult (word A[/* size */], const word B[/* size */], const Modulus *ms,
                word P[/* size + 1 */], const word size)
{
    /* word P [size + 1]; The result product is (size + 1) words wide */
    static dword X, Y; /* X and Y are supposed to be registers 2WORD_BITS bits long */
    word q;     /* the quotient q is 1 word (WORD_BITS bits) long, i.e. q < R */
    int i,j;

    /* Sets P to zero: */
    memset (P, 0, sizeof(word)*(size + 1));

    /* A[i] loop: */
    for (i = 0; i < size ; i++)
    {
        X = P[0] + A[i]*(dword)B[0];
        X &= DWORD_MAX; /* prevent injection bug from bits above 31 */
        q = (word)(X & mod16)*(ms->M_dash);
        Y = q*(dword)(ms->M[0]) + (X & mod16);
        Y &= DWORD_MAX; /* prevent injection bug from bits above 31 */

        /* B[j] loop: */
        for (j = 1; j < size ; j++)
        {
            X = P[j] + A[i]*(dword)B[j] + (X >> WORD_BITS);
            X &= DWORD_MAX; /* prevent injection bug from bits above 31 */
            /* Y >> WORD_BITS performs Y div R (R = 2^WORD_BITS) */
            Y = q*(dword)(ms->M[j]) + (X & mod16) + (Y >> WORD_BITS);
            Y &= DWORD_MAX; /* prevent injection bug from bits above 31 */
            P[j - 1] = (word)(Y & mod16); /* P[j - 1] = Y mod R */
        }
        /* at this point we have that j = size */
        X = P[j] + (X >> WORD_BITS) + (Y >> WORD_BITS);
        X &= DWORD_MAX; /* prevent injection bug from bits above 31 */
        P[j - 1] = (word)(X & mod16);
        P[j] = (word)(X >> WORD_BITS);

    }


    /* If P >= M do P = P - M */
    if ( P[size] || x_notlessthan_y (P, &(ms->M[0]), size))
        multiprec_subtract( P, &(ms->M[0]), size);


    /* Copy result into A, the input/output variable */
    memcpy( A, P, sizeof(word)*size);
}

/* - multiprec_subtract ()                               */
/* This function subtracts the multi-precision unsigned  */
/* integer m[size] from p[size]. It is assumed  the MSB  */
/* is in position [size - 1] while the LSB is in         */
/* position [0]. If m is greater than p the most         */
/* significant bits in the result will be all 1 (this    */
/* corresponds to sign extension).                       */
/*                                                       */
/* INPUTS: array of unsigned p[] and m[], both "size"    */
/* words long                                            */
/*                                                       */
/* OUTPUT: array of unsigned p such that p = p - m       */

void multiprec_subtract (word p[/*size*/], const word m[/*size*/], const word size)
{
    int i;
    dword t2;

    t2 = (dword)p[0] - (dword)m[0];
    p[0] = (word)(t2 & (dword)mod16);

    for (i = 1; i < size ; i++)
    {
        t2 &= DWORD_MAX; /* bit 31 is carry: prevent injection bug from bits above 31 */
        t2 = (dword)p[i] - (t2 >> (2*WORD_BITS - 1)) - (dword)m[i];
        p[i] = (word)(t2 & (dword)mod16);
    }
}

/* - multiprec_degreeof()                                           */
/* The following function gets the degree (in bits) of the input    */
/* polynomial, assuming the maximum degree is size * WORD_BITS - 1). This   */
/* function returns a valid polynomial degree only if x[] is not    */
/* zero or otherwise it will return -1.                             */
/*                                                                  */
/* INPUTS: x[] - the array containg the multi-precision polynomial. */
/*               MSB is in x[size-1] and the LSB is in x[0].        */
/*        size - the size in words of the array x[]                 */
/*                                                                  */
/* OUTPUT: the function return value is the degree of x[] viewed as */
/*         a polynomial expressed in the base 2 (binary)..          */

word multiprec_degreeof(const word x[/*size*/], const word size)
{
    int i;
    word j, degree;

    for (i = size - 1; (i > -1) && !(x[i]) ; i--);

    degree = (i+1) * WORD_BITS - 1;
    j = 0;
    while (!((x[i] << j) & 0x8000) && (j++ < WORD_BITS))
        degree--;

    return degree;
}

/* - x_notlessthan_y ()                                        */
/* This function returns the value TRUE if the multi-precision */
/* integer number x[size] is greater than or equal to y[size], */
/* otherwise, returns FALSE. Note that the LSB must be in      */
/* position [0] while the MSB must be in position [size-1].    */
/*                                                             */
/* INPUTS: unsigned x and y. both "size" words long.           */
/*                                                             */
/* OUTPUT: TRUE if x >= y, FALSE if x < y                      */

word x_notlessthan_y (const word x[/*size*/], const word y[/*size*/], const word size)
{
    int i;

    for (i = size - 1; i > -1; i--)
    {
        if (x[i] > y[i])
            return TRUE;
        else if  (x[i] < y[i])
            return FALSE;
    }
    /* If they are equal, return true: */
    return TRUE;
}

