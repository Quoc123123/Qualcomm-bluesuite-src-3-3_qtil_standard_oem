/**********************************************************************
 *
 *  mp_exponentiation.c
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To implement arbitrary precision exponentiation routines for use
 *  with big numbers!
 *
 **********************************************************************/

#include "keygen_private.h"
#include <stdlib.h>
#include <string.h>

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
/* INPUTS: x[size] - multiprecison base . x[0] is the       */
/*                    LSWord, x[size-1] is the MSWord.      */
/*         e[size] - multiprecision exponent. e[0] is       */
/*                    the LSWord, e[size-1] is the MSWord.  */
/*         size - size (in words) of both the base and the  */
/*                 exponent.                                */
/*         ms    - dModulus from where the modulo ms->M[Wn] */
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

void mp_exp (dword *x, dword *e, const word size, const dModulus *ms)
{
  dword degree, wdegree;
  int i, rem;
  dword *A, *temp;

  /* Get the degree of x[] */
  degree = mp_degree (x, size);

  if (degree > (size * DWORD_BITS - 1 ))
     /* If this is true it implies the base x[] is zero: */
     return;

  /* Get the degree of e[] */
  degree = mp_degree (e, size);


  if (degree > (size * DWORD_BITS - 1))
  /* If this is true it implies the exponent e[] is zero: */
  {
     memcpy( x, &(ms->one[0]), sizeof(dword)*size);
     return;
  }
  else if (degree == 0)
     /* If this is true it implies the exponent e[ ] is one: */
     return;

  /* Allocates memory for temporary storage needed in this function */
  A    = (dword *)calloc(size, sizeof(dword));
  temp = (dword *)calloc(size + 1, sizeof(dword));

  /* The degree in words of the exponent e[] */
  wdegree = (dword) (degree/DWORD_BITS);

  /* Number of significant bits (minus 1) */
  /* in the MSWord of the exponent e[]    */
  rem = degree - wdegree*DWORD_BITS;

  /* initializes A with R mod M */
  memcpy(A, ms->RNmodM, sizeof(dword)*size);

  /* x' = x*R (mod M) */
  mp_mont_mult (x, ms->R2NmodM, ms, temp, size);

  for (i = wdegree; i >  - 1; i--)
  {
     while ( rem > -1)
     {
        mp_mont_mult (A, A, ms, temp, size);  /* A = A*A*R^-1 mod M */

        if ((e[i] >> rem) & 1)
           mp_mont_mult (A, x, ms, temp, size); /* A = A*x'*R^-1 mod M */

        rem--;
     }

     rem = DWORD_BITS-1;
  }

  /* A = A*R^-1 mod M */
  mp_mont_mult (A, &(ms->one[0]), ms, temp, size);

  /* The output result x[] = A[] = x[]^e[] */
  memcpy(x, A, sizeof(dword)*size);

  free ((void *) A);
  free ((void *) temp);
}


/* ans = 2^e mod ms */
void mp_2exp (dword *ans, dword *e, const word size, const dModulus *ms)
{
  dword degree, wdegree;
  int i, j, rem;
  dword *A, *temp;
  bool msbOfA;

  /* Get the degree of e[] */
  degree = mp_degree (e, size);

  if ((degree > (size * DWORD_BITS - 1))/* If this is true it implies the exponent e[] is zero: */
      || (degree == 0)) /* If this is true it implies the exponent e[ ] is one: */
  {
     memcpy( ans, &(ms->one[0]), sizeof(dword)*size);
     if (degree == 0)
        ans[0] = 2;
     return;
  }

  /* Allocates memory for temporary storage needed in this function */
  A    = (dword *)calloc(size, sizeof(dword));
  temp = (dword *)calloc(size + 1, sizeof(dword));

  /* The degree in words of the exponent e[] */
  wdegree = (dword) (degree/DWORD_BITS);

  /* Number of significant bits (minus 1) */
  /* in the MSWord of the exponent e[]    */
  rem = degree - wdegree*DWORD_BITS;

  /* initializes A with R^N mod M */
  memcpy(A, &(ms->RNmodM[0]), sizeof(dword)*size);

  for (i = wdegree; i >  - 1; i--)
  {
     while ( rem > -1)
     {
        mp_mont_mult (A, A, ms, temp, size);  /* A = A*A*R^-1 mod M */

        if ((e[i] >> rem) & 1)
        {
           msbOfA = (A[size-1] >> (DWORD_BITS - 1)) == 1;

           /* A = A*2 */
           /*multiprec_Lshift32 (A, 1, size);*/
           for(j = size-1; j > 0; j--)
              A[j] = (A[j] << 1) | (A[j-1] >> (DWORD_BITS - 1));
           A[0] = A[0] << 1;

           if (msbOfA || mp_gte(A, ms->M, size))
              mp_sub (A, ms->M, size);
           /* At this point we must have A = A*2 mod M */
        }

        rem--;
     }

     rem = DWORD_BITS-1;
  }


  /* A = A*R^-1 mod M */
  mp_mont_mult (A, &(ms->one[0]), ms, temp, size);

  /* The output result x[] = A[] = x[]^e[] */
  memcpy(ans, A, sizeof(dword)*size);

  free ((void *) A);
  free ((void *) temp);
}

/********************************************************/
/* mp_expFP ()                                          */
/* Performs modular exponentiation of a multi-precision */
/* unsigned integer if the exponent is 2 or the Fermat  */
/* prime 3, 17 or 65535                                 */
/*                                                      */
/* INPUTS: Base x[],                                    */
/*         wsize = size in words of x[], x[0] is the    */
/*                 LSWord, x[wsize-1] is the MSWord     */
/*         EXP = constant 2, 3, 17 or 65535             */
/*         *ms = the structure dModulus where the       */
/*         modulus ms->M[wsize] is obtained from        */
/*                                                      */
/* OUTPUT: x = x^EXP mod M                              */
/********************************************************/

void mp_expFP (dword x[], const dword size, const dword EXP, const dModulus *ms)
{
    dword i;
    dword *A, *temp;

    if (EXP == 0)
        memcpy( x, &(ms->one[0]), sizeof(dword)*size);
    else if ((EXP == 3) || (EXP == 17) ||(EXP == 65537))
    {    /* the exponent is a Fermat prime (2^n + 1): */
        A    = (dword *)calloc(size, sizeof(dword));
        temp = (dword *)calloc(size + 1, sizeof(dword));

        /* initializes A with R^2N mod M */
        memcpy( A, &(ms->R2NmodM[0]), sizeof(dword)*size);

        mp_mont_mult  (A, x, ms, temp, (word)size); /* = x*R^N mod M */

        for (i = (EXP >> 1); i; i >>= 1 )
            mp_mont_mult  (A, A, ms, temp, (word)size); /* = (x^(2^i))*R^N mod M */

        mp_mont_mult  (x, A, ms, temp, (word)size); /* = x^EXP mod M */

        free(A);
        free(temp);
    }
    else if (EXP == 2)
    {
        temp = (dword *)calloc(size + 1, sizeof(dword));
        mp_mont_mult  (x, x, ms, temp, (word)size); /* x = x^2 * R^-1 mod M */
        mp_mont_mult  (x, &(ms->R2NmodM[0]), ms, temp, (word)size); /* x = x^2 mod M */
        free(temp);
    }
}

