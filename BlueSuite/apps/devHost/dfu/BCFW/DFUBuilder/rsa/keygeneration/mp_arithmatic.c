/**********************************************************************
 *
 *  mp_arithmatic.c
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To implement arbitrary precision arithmatic routine for use with 
 *  big numbers!
 *
 **********************************************************************/

#include "keygen_private.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* mp_add ()                                               */
/* This function adds the multi-precision  integers        */
/* m[size] and p[size] modulo DWORD_BITS^size, i.e if the  */
/* result exceeds "size" digits it gets truncated. The MSB */
/* is in position [size - 1] while the LSB is in position  */
/* [0].                                                    */
/*                                                         */
/* INPUTS: array of unsigned p[] and m[], both "size"      */
/* words long.                                             */
/*                                                         */
/* OUTPUT: array of unsigned p such that p = p + m         */

void mp_add ( dword *p , const dword *m , const dword size )
{
    dword i;
    int carry = 0;
    for (i = 0 ; i < size ; i++ )
    {
        /*
        **  for each word, do the addition,
        **  and set the carry flag.
        **  Notice that F + x + 1 = x with carry,
        **  but 0 + x + 0 = x no carry
        **  hence the slightly odd if condition.
        */
        p[i] += m[i];
        if (carry)
            p[i] ++;
        if ( carry ? m[i] >= p[i] : m[i] > p[i] ) 
            carry = 1;
        else
            carry = 0;
    }
    /* discard the final carry - ie work mod size */
}

/* mp_addWC ()                                  */
/* This function adds a dword constant WC       */
/* to the multi-precision input  p[].           */
/*                                              */
/* INPUTS: p[], size of p[] in words and the    */
/*         dword constant WC                    */
/*                                              */
/* OUTPUT: p[] = p[] + K;                       */

void mp_addWC ( dword *p , const dword size , const dword WC )
{
    dword i;
    uint64 t4;
    dword C;

    t4 = (uint64)p[0] + (uint64)WC;
    p[0] = (dword)(t4 & (uint64)mod32);
    C = (dword)(t4 >> DWORD_BITS);

    for (i = 1; (i < size) && C ; i++)
    {
        t4 = (uint64)p[i] + (uint64)C;
        p[i] = (dword)(t4 & (uint64)mod32);
        C = (dword)(t4 >> DWORD_BITS);
    }
}

void mp_addWC_dep ( dword *p , const dword size , const dword WC )
{
    dword i = 0;
    p[0] += WC;
    if ( size > 1 && WC > p[0] ) 
        p[1] ++ ;
    /*
    **  that single carry bit can push each
    **  dword to zero and travel up to the top
    */
    for ( i = 2; i < size && p[i-1] == 0 ; i++ )
        p[i] ++ ;
}

/* mp_subtract ()                                        */
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

void mp_sub ( dword *p , const dword *m , const dword size )
{
    dword i;
    uint64 t2;

    t2 = p[0] - (uint64)m[0];
    p[0] = (dword)(t2 & mod32);

    for (i = 1; i < size ; i++)
    {
        t2 = (uint64)p[i] - (t2 >> (2*DWORD_BITS - 1)) - (uint64)m[i];
        p[i] = (dword)(t2 & (uint64)mod32);
    }
}

void mp_sub_dep ( dword *p , const dword *m , const dword size )
{
    dword i;
    int carry = 0;
    for (i = 0 ; i < size ; i++ )
    {
        /*
        **  for each word, do the addition,
        **  and set the carry flag.
        **  Notice that F + x + 1 = x with carry,
        **  but 0 + x + 0 = x no carry
        **  hence the slightly odd if condition.
        */
        p[i] -= m[i];
        p[i] -= carry;
        if ( carry ? m[i] <= p[i] : m[i] < p[i] ) 
            carry = 1;
        else
            carry = 0;
    }
    /* discard the final carry - ie work mod size */
}

/* - mp_subWC                                         */
/* This function subtracts a small word constant WC   */
/* to the multi-precision input p[].                  */
/*                                                    */
/* INPUTS: p[], size of p[] in words and the word     */
/*         constant WC.                               */
/*                                                    */
/* OUTPUT: p[] = p[] - 1;                             */

void mp_subWC ( dword *p , const dword size , const dword WC )
{
    dword i;
    uint64 t4, C;

    t4 = (uint64)p[0] - (uint64)WC;
    p[0] = (dword)((uint64)t4 & (uint64)mod32);
    C = (t4 >> DWORD_BITS);

    for (i = 1; (i < size) && C ; i++)
    {
        t4 = (uint64)p[i] - C;
        p[i] = (dword)(t4 & (uint64)mod32);
        C = (t4 >> DWORD_BITS);
    }
}

void mp_subWC_dep ( dword *p , const dword size , const dword WC )
{
    dword i = 0;
    p[0] -= WC;
    if ( size > 1 && WC < p[0] ) 
        p[1] -- ;
    /*
    **  that single carry bit can push each
    **  dword to zero and travel up to the top
    */
    for ( i = 2 ; i < size && p[i-1] == 0xFFFFFFFF ; i++ )
        p[i] -- ;
}

/* mp_iszero()                                              */
/* The following function checks if the input array of      */
/* length "size" words long is zero or not. Note that the   */
/* LSB must be in position [0] while the MSB must be in     */
/* position [size-1].                                       */
/*                                                          */
/* INPUT: The number/polynomial x[].                        */
/*                                                          */
/* OUTPUT: The function return value which is TRUE or FALSE */
/* if x[] is zero or not respectively                       */

bool mp_iszero( const dword *x, const dword size)
{
    dword i;

    for (i = 0; i != size ; i++ )
        if (x[i])
            return FALSE;

    return TRUE;
}


/* mp_isequal()                                              */
/* The following function checks if the two input arrays x   */
/* and y, both with length "size" double words (uint32) are  */
/* equal or not. Note that the LSB must be in position [0]   */
/* while the MSB must be in position [size-1].               */
/*                                                           */
/* INPUT: The numbers/polynomials x[].and y[]                */
/*                                                           */
/* OUTPUT: The function return value which is TRUE  if       */
/* x[] = y[] or FALSE if x[] != y[]                          */

bool mp_isequal (const dword *x, const dword *y, const word size)
{
    int i;

    for (i = 0; i < size; i++)
        if (x[i] != y[i])
            return FALSE;

    return TRUE;
}

/* mp_gte ()                                                   */
/* This function returns the value TRUE if the multi-precision */
/* integer number x[size] is greater than or equal to y[size], */
/* otherwise, returns FALSE. Note that the LSB must be in      */
/* position [0] while the MSB must be in position [size-1].    */
/*                                                             */
/* INPUTS: unsigned x and y. both "size" words long.           */
/*                                                             */
/* OUTPUT: TRUE if x >= y, FALSE if x < y                      */

bool mp_gte (const dword *x, const dword *y, const dword size)
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

/* mp_Lshift ()                                                          */
/* The following function performs multi-precison left shift on t bits   */
/* of the array x[] of size "size" words. This left-shift must be viewed */
/* at the normal number representation, with the most significant digit  */
/* at the lefttmost position and the least significant digit at the      */
/* rightmost position BUT this function expects the number to be stored  */
/* in memory in reverse notation as indicated next (if necessary use the */
/* function reverse-array() to reverse the number representation)        */
/*                                                                       */
/* INPUTS: x[] - the array containg the number to be shifted, where the  */
/*               MSB is x[size-1] and the LSB is x[0]                    */
/*         tbits - the number of bits to be left shifted in x            */
/*         size - the size in words of x                                 */
/*                                                                       */
/* OUTPUT: x[] - the input array right shifted t bits, i.e,              */
/*               x = x << tbits = x * (2^tbits).                         */
/*                                                                       */

void mp_Lshift (dword *x, const word tbits, const word size)
{
    dword q, r;
    dword i;
    int j;

    if (tbits != 0)
    {
        /* get how many words of size DWORD_BITS fit into tbits */
        q = (dword)tbits / (dword)DWORD_BITS;

        /* get how many bits of tbits remain after the above fitting */
        r = (dword)tbits % (dword)DWORD_BITS;

        if (q < size)
        {
            if ( r == 0 )
            {
                for(i = (size-1); i >= q; i--)
                    x[i] = x[i-q];
            }
            else
            {
                for(i = (size-1); i > q; i--)
                {
                    dword top = (x[i-q] << r);
                    dword bottom = (x[i-q-1] >> ((dword)DWORD_BITS - r));
                    x[i] = top | bottom;
                }
                x[q] = (x[0] << r);
            }

            /* set the remaining words to zero */
            for (j = q - 1; j > -1; j--)
                x[j] = 0;
        }
        else
        {
            assert ( ("LEFT" , 0) );
            memset ( x , 0 , size * sizeof (dword) );
        }
    }

}

/* mp_Rshift ()                                                          */
/* The following function performs multi-precison right shift on t bits  */
/* of the array x[] of size "size" words. This right-shift must be       */
/* viewed at the normal number representation, with the most significant */
/* digit at the lefttmost position and the least significant digit at    */
/* the rightmost position BUT this function expects the number to be     */
/* stored in memory in reverse notation as indicated next (if necessary  */
/* use the function reverse-array() to reverse the number                */
/* representation).                                                      */
/*                                                                       */
/* INPUTS: x[] - the array containg the number to be shifted, where the  */
/*               MSB is x[size-1] and the LSB is x[0]                    */
/*         tbits - the number of bits to be right shifted in x           */
/*         size - the size in words of x                                 */
/*                                                                       */
/* OUTPUT: x[] - the input array left shifted t bits, i.e,               */
/*               x = x >> tbits = x / (2^tbits)].                        */

void mp_Rshift (dword *x, const word tbits, const word size)
{
    dword q, r;
    dword i;

    if (tbits != 0)
    {
        /* get how many dwords of size T fit into tbits */
        q = (dword)tbits / (dword)DWORD_BITS;

        /* get how many bits of tbits remain after the above fitting */
        r = (dword)tbits % (dword)DWORD_BITS;

        if (q < size)
        {
            for(i = 0; i < (size-q-1); i++)
                x[i] = (x[i+q] >> r) | (x[i+q+1] << ((dword)DWORD_BITS - r));

            x[size-q-1] = (x[size-1] >> r);

            /* set the remaining dwords to zero */
            for (i = size - q; i < size; i++)
                x[i] = 0;
        }
        else
        {
            assert ( ("RIGHT", 0 ) );
            memset ( x , 0 , size * sizeof (dword) );
        }
    }
}


/* mp_degree()                                                      */
/* The following function gets the degree (in bits) of the input    */
/* polynomial, assuming the maximum degree is size * T - 1). This   */
/* function returns a valid polynomial degree only if x[] is not    */
/* zero or otherwise it will return -1.                             */
/*                                                                  */
/* INPUTS: x[] - the array containg the multi-precision polynomial. */
/*               MSB is in x[size-1] and the LSB is in x[0].        */
/*        size - the size in words of the array x[]                 */
/*                                                                  */
/* OUTPUT: the function return value is the degree of x[] viewed as */
/*         a polynomial expressed in the base 2 (binary)..          */

dword mp_degree ( const dword *x, const dword size)
{
    int i;
    unsigned int j;
    dword degree;

    for (i = size - 1; (i > -1) && !(x[i]) ; i--)
        ;

    degree = (i+1) * DWORD_BITS - 1;

    j = 0;
    while (!((x[i] << j) & 0x80000000) && (j++ < DWORD_BITS))
        degree--;

    return degree;
}

/* mp_multiply ()                                            */
/* The following function performs multiple-precision        */
/* multiplication of two numbers x and y, of sizes x_size    */
/* and y_size words long such that y = (y*x) mod (T^y_size). */
/*                                                           */
/* INPUTS: y[], x[], y_size, x_size (size of y[] and x[]     */
/*         in words) and w[]. It is assumed that y_size is   */
/*         big enough to accomodate the product, so y is     */
/*         likely to have some zeroes on the right (MSWords).*/
/*                                                           */
/* OUTPUT: y = y*x mod T^y_size. Note that y is both input   */
/*                               and output                  */
/*                                                           */


void mp_multiply (dword *y, dword *x, const word y_size, const word x_size,
                  dword *w )
{
    uint64 Areg; /* Areg will hold the product of two digits x[i] and y[j] */
    dword Carry; /* Carry will hold the most significant half of Areg      */
    word real_y_size, real_x_size;
    int i,j;


    /* Initializes the output to zero: */
    memset (w, 0, sizeof(dword)*y_size);

    /* excludes the most significant digits of y[] which are zero: */
    real_y_size = y_size;
    for (i = y_size - 1; i > -1 && !y[i]; i--)
        real_y_size--;

    /* excludes the most significant digits of x[] which are zero: */
    real_x_size = x_size;
    for (i = x_size - 1; i > -1 && !x[i] ; i--)
        real_x_size--;

    /* y[i] loop: */
    for (i = 0; i < real_y_size ; i++)
    {

        Carry = 0;

        /* x[j] loop: */
        for (j = 0; j < real_x_size ; j++)
        {
            /*
            **  In a more generic multi-precision multiplication algorithm
            **  we would not have the following if() :
            */
            if ((i+j) < y_size)
            /*
            **  but because the product is modulo T^real_y_size it is pointless
            **  to calculate product
            */
            {
                /* digits greater than that size.         */
                /* This is the most computing itensive operation: */
                Areg = (uint64)w[i + j] + (uint64)y[i]*(uint64)x[j] + (uint64)Carry;
                /* Update the product digit: */
                w[i + j] = (dword)(Areg & mod32);
                /* Save the carry to be used in the next higher position digit product: */
                Carry = (dword)(Areg >> DWORD_BITS);
            }
            else
                break;
        }
        /* In a more generic multiple-precision multiplication function     */
        /* we would need to assign the carry to the next higher position    */
        /* unconditonally, unlike what happens next:                        */

        if ((i + real_x_size) < y_size)
            w[i + real_x_size] = Carry;

    }

    /* Copy result from w into y, the input/output variable */
    memcpy (y, w, sizeof(dword)*y_size);

}


/* mp_multiplyWC32                                              */
/* This function multiplies a multiprecision number with digits */
/* over uint32 by a double word (uint32) number WC.             */
/* INPUT y[], y_size, WC and w, a scratch buffer the same       */
/*       size as y.                                             */
/*                                                              */
/* OUTPUT y[] = y[] * WC                                        */

void mp_multiplyWC (dword *y, const word y_size, const dword WC, dword *w )
{
    uint64 Areg; /* Areg will hold the product of two digits x[i] and y[j] */
    dword Carry; /* Carry will hold the most significant half of Areg      */
    word real_y_size;
    int i;

    /* Initializes the output to zero: */
    memset (w, 0, sizeof(dword)*y_size);

    /* excludes the most significant digits of y[] which are zero: */
    real_y_size = y_size;
    for (i = y_size - 1; i > -1 && !y[i]; i--)
        real_y_size--;

    Carry = 0;

    /* y[i]*WC loop: */
    for (i = 0; i < real_y_size ; i++)
    {
        /* This is the most computing itensive operation: */
        Areg = (uint64)y[i]*(uint64)WC + (uint64)Carry;
        /* Update the product digit: */
        w[i] = (dword)(Areg & mod32);
        /*
        **  Save the carry to be used in the next higher position
        **  digit product:
        */
        Carry = (dword)(Areg >> DWORD_BITS);
    }

    if (i < y_size)
        w[i] = Carry;

    /* Copy result from w into y, the input/output variable */
    memcpy (y, w, sizeof(dword)*y_size);
}

/* - Mont_mult ()                                         */
/* The following function performs the Montgomery modular */
/* multiplication/reduction of two numbers A and B such   */
/* that A = A*B*R^(-1) mod M, where A, B and M are        */
/* "size" words long multi precision numbers/arrays such  */
/* that A[0], B[0] and M[0] are their least significant   */
/* digits and A[size-1], B[size-1] and M[size-1] are      */
/* their most significant digits. [NOTE; In the XAP       */
/* assembly function "xap_mont_mult()" (see below) these  */
/* numbers must be input in reverse order, with the most  */
/* significant digit in position [0] (the output will     */
/* also be in reverse order) ].                           */
/* The other input variable P[size + 1] is only used for  */
/* storing temporary results within this function. It is  */
/* passed as a reference to save time having to allocate  */
/* size+1 words of memory each time this function is      */
/* called.                                                */
/*                                                        */
/* INPUT: A, B, P and the M_struct                        */
/*                                                        */
/* OUTPUT: A = A*B*R^(-1) mod M                           */

void mp_mont_mult (dword *A, const dword *B, const dModulus *ms,
                   dword P[/* size + 1 */], const word size)
{
    /* dword P [size + 1]; The result product is (size + 1) dwords wide */

    static uint64 X, Y; /* X and Y are supposed to be registers 2T bits long */
    dword q;      /* the quotient q is 1 dword (DWORD_BITS bits) long, i.e. q < R32 */
    int i,j;

    /* Sets P to zero: */
    memset (P, 0, sizeof(dword)*(size + 1));

    /* A[i] loop: */
    for (i = 0; i < size ; i++)
    {
        X = P[0] + A[i]*(uint64)B[0];
        q = (dword)(X & mod32)*(ms->M_dash);
        Y = q*(uint64)(ms->M[0]) + (X & mod32);

        /* B[j] loop: */
        for (j = 1; j < size ; j++)
        {
            X = P[j] + A[i]*(uint64)B[j] + (X >> DWORD_BITS);
            /* Y >> DWORD_BITS performs Y div R32 (R32 = 2^DWORD_BITS) */
            Y = q*(uint64)(ms->M[j]) + (X & mod32) + (Y >> DWORD_BITS);
            P[j - 1] = (uint32) (Y & mod32); /* P[j - 1] = Y mod R32 */
        }
        /* at this point we have that j = size */
        X = P[j] + (X >> DWORD_BITS) + (Y >> DWORD_BITS);
        P[j - 1] = (uint32) (X & mod32);
        P[j] = (uint32)(X >> DWORD_BITS);
    }


    /* If P >= M do P = P - M */
    if ( P[size] || mp_gte (P, ms->M, size))
        mp_sub ( P, ms->M, size);


    /* Copy result into A, the input/output variable */
    memcpy( A, P, sizeof(dword)*size);

}


/* - mp_getbit ()                                                       */
/* The following function gets the value (1 or 0) of a specified bit    */
/* number inside a multi-precision number/polynomial.                   */
/*                                                                      */
/* INPUTS: x[] - the array containg the multi-precision number where    */
/*               MSB is x[size-1] and the LSB is x[0]                   */
/*         size      - the size in words of the array x[]               */
/*         bitnumber - the number of the bit whose value will be output */
/*                     starting from position 0 and going up to         */
/*                     position (size*T - 1)                            */
/*                                                                      */
/* OUTPUT: the function returns a the bit value 1 or 0, assuming bits   */
/*         outside the normal range to be zero.                         */

word mp_getbit (dword *x, const dword size, const dword bitnumber)
{
    dword q, r;

    /* definitely only works for 32 bit integers... */
    q = bitnumber / DWORD_BITS;
    r = bitnumber % DWORD_BITS;

    if (q < size)
        return ((x[q] >> r) & (word)1);
    else
        return 0;
}


/* - mp_setbit ()                                                       */
/* The following function sets a value of 1 or 0 to a specified bit     */
/* number inside a multi-precision number/polynomial.                   */
/*                                                                      */
/* INPUTS: x[] - the array containg the multi-precision number where    */
/*               MSB is x[size-1] and the LSB is x[0]                   */
/*         size      - The size in words of the array x[]               */
/*         bitnumber - the number of the bit whose value will be set    */
/*                     starting from position 0 and going up to         */
/*                     position (size*T - 1)                            */
/*         bitvalue  - the new bit value to be set in the position      */
/*                     bitnumber of the array x[]                       */
/*                                                                      */
/* OUTPUT: the function returns a logical TRUE or FALSE if the asked    */
/*         bit value is 1 or 0, respectively.                           */

void mp_setbit (dword *x, const dword size, const dword bitnumber,
                const word bitvalue)
{
    dword q, r;

    /* definitely only works for 32 bit integers... */
    q = bitnumber / DWORD_BITS;
    r = bitnumber % DWORD_BITS;

    if (q < size)
    {
        if (bitvalue) /* set bit to 1 */
            x[q] = x[q] | (1 << r);
        else /* set bit to 0 */
            x[q] = x[q] & ~(1 << r);
    }
}

/*************************************************************
mp_divideby3 () - divide by 3 and return the remainder

INPUTS : x[]  a multiprecision number.
         size the size of x.
OUTPUTS: x[]  x/3.
         returns the remainder.

algorithm:
    for each bit:
        0 -> 0r0
        11 -> 01r00
        100 -> 001r001
        101 -> 001r010

*************************************************************/

word mp_divideby3 ( dword *x , const dword size )
{
    word remainder = 0;
    dword * result;
    dword degree;
    int i;
    word bit;

    degree = mp_degree ( x , size );
    /* if the degree is bigger than it can be, then x must be zero... */
    if ( degree < size * DWORD_BITS )
    {
        result = (dword*) calloc ( size , sizeof(dword) );
        memset ( result , 0 , sizeof(dword) * size );

        for (i = degree; i > 1; i--)
        {
            if (mp_getbit(x, size, i)) /* {1..} */
            {
                if (mp_getbit(x, size, i-1)) /* {11} */
                {
                    /* set result_i-1 = 1 */
                    mp_setbit (result, size, i-1, 1);
                    /* update the dividend x[]: */
                    mp_setbit (x, size, i-1, 0);
                }
                else
                {
                    /* set result_i-2 = 1 */
                    mp_setbit (result, size, i-2, 1);
                    if (mp_getbit (x, size, i-2)) /* {101} */
                        bit = 1;
                    else /* {100} */
                        bit = 0;

                    /* update the dividend x[]: */
                    mp_setbit (x, size, i-1, bit);
                    mp_setbit (x, size, i-2, (word)!bit);
                }
                mp_setbit (x, size, i  , 0);
            }
        }

        if (x[0] == 3)
        {
            /* sets the least significant bit of the quotient to 1 and ... */
            result[0] = result[0] | 0x0001;
            /* ... the remainder to zero */
            x[0] = 0;
        }

        remainder = (word)(x[0]);
        memcpy ( x , result , size * sizeof(dword) );
        free ( result );
    }
    return remainder;
}
