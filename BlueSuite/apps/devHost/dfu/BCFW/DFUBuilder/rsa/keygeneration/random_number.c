/******************************************************************************
FILENAME:    random_number.c

Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     To implement random number generation procedures.

******************************************************************************/

#include "keygen_private.h"
#include <stdlib.h>
#include <string.h>

uint64 ModsumFP32;
uint64 Mod_FermatPrimes[FPSIZE];
uint64 Mod_sum32[PSIZE];

const dModulus PRNG1024_M_str32 =
{

 /*M [Wn] = */            { 0x9ba88705, 0xbf1ece51, 0x63087e94, 0x64599287,
                            0x3bf47dc4, 0x1176166f, 0x789bdc47, 0xe53e4b9a,
                            0x5bed0e2c, 0xeed1e497, 0xf275fca7, 0x5086f047,
                            0xcee49559, 0x0df48b0e, 0xea485cba, 0x11c972af,
                            0x22793dcb, 0x983f2d2b, 0x70cfa26b, 0x0d652b17,
                            0x24b31288, 0x4aee2a67, 0xe1c8fe60, 0x955d1619,
                            0x92f3101e, 0x52a3a304, 0x908bf98a, 0xeead8c9e,
                            0x74a47339, 0x67ec227f, 0x7de8f826, 0xbb44b60d },

 /* M_dash = */           0xb832773b,

 /* word R2NmodR = */     { 0x6a651dcd, 0xb8eb5432, 0x28749b9c, 0x341b2fd3,
                            0xc1d6d586, 0xaec35af4, 0x59ca093b, 0x95ee6f75,
                            0x1622791b, 0x177a142b, 0x729aed3a, 0xb5a95e57,
                            0xb900a7a5, 0xc5f14924, 0x9d7b303f, 0x9a9719a0,
                            0x22cb20be, 0xf9c2bead, 0xcded311c, 0xb01052b7,
                            0xb11d6d76, 0x097ac754, 0x4d87f4cf, 0xa4b6bc05,
                            0x71c3950c, 0xd1e30b78, 0x1dc6ecdc, 0x0375352a,
                            0x332d36e6, 0x7b8c5140, 0x1fce972e, 0x27c7598f },

 /* word RNmodM [Wn] = */ { 0x645778fa, 0x40e131ae, 0x9cf7816b, 0x9ba66d78,
                            0xc40b823b, 0xee89e990, 0x876423b8, 0x1ac1b465,
                            0xa412f1d3, 0x112e1b68, 0x0d8a0358, 0xaf790fb8,
                            0x311b6aa6, 0xf20b74f1, 0x15b7a345, 0xee368d50,
                            0xdd86c234, 0x67c0d2d4, 0x8f305d94, 0xf29ad4e8,
                            0xdb4ced77, 0xb511d598, 0x1e37019f, 0x6aa2e9e6,
                            0x6d0cefe1, 0xad5c5cfb, 0x6f740675, 0x11527361,
                            0x8b5b8cc6, 0x9813dd80, 0x821707d9, 0x44bb49f3 },

 /* one */                { 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000, 0x00000001 }
};



/* Define exponents to be used in the random generator function */
/* (chosen to be Fermat primes)                                 */

/*                      Exp[0] = 3      Exp[1] = 17     Exp[2] = 65537   */
const dword Exp[3] = { ((1 << 1) + 1), ((1 << 4) + 1), ((1L << 16) + 1)};

static dword Xn[KEY_WIDTH_W];

void init_rand_seed ( const dword* random_data )
{
    memcpy ( Xn , random_data , KEY_WIDTH_W * sizeof (dword) );
    random_number32 ( Xn, KEY_WIDTH_W );
}

/*******************************************************************/
/* random_number ( ) places a random number in  the array          */
/* output[ ] of length outlen. The value of outlen must be <= Wn/2 */
/*******************************************************************/

void random_number32 ( dword rand_output[/* rand_size */], word rand_size )
{
    word size;
    dword temp[KEY_WIDTH_W];
    const dModulus *ms;

    /* The constant PRNG1024_M_str32 must contain a 1024-bit long modulus */
    /* and associated constants to perform the Montgomery multiplication. */
    ms = &PRNG1024_M_str32;

    size = (word)KEY_WIDTH_W;
    if (rand_size > size)
    rand_size = size;

    /* Xn[ ] = Xn[ ] + 1;  */
    mp_addWC (Xn, size, 1);

    /* (Xn[ ] + 1)^K2 mod M */
    mp_expFP (Xn, size, Exp[K2], ms);

    /* Yn[ ] = Xn[ ] */
    memcpy (temp, Xn, sizeof(dword)*size);

    /* Yn = (Xn[ ])^K1 mod M = f(Xn[ ]) */
    mp_expFP (temp, size, Exp[K1], ms);

    /* rand_output[0:outlen-1] = Yn[0:outlen-1] */
    memcpy (rand_output, temp, sizeof(dword)*rand_size);
}

void get_nbit_odd_rand32 (dword odd_rand[/*size*/], word size, dword degree )
{
    dword randnum[KEY_WIDTH_W];
    word nsize, maxsize, remainder;
    dword mask;

    nsize = (word)(degree/DWORD_BITS + 1);
    maxsize = (word)KEY_WIDTH_W;

    /* odd_rand is set to zero. It will hold n-2 random bits */
    /* (the most and least significant bit are always 1).    */
    memset(odd_rand, 0, sizeof(dword)*size);

    if (size > maxsize)
    {
        size = maxsize;
        if (nsize > size)
        nsize = size;
        degree = nsize*DWORD_BITS-1;
    }

    /* get a random number with the maximum allowed size, in uint32 */
    random_number32 (randnum, maxsize);

    /* set it to be odd */
    randnum[0] |= (dword)1;

    /* copy nsize dwords from randnum into odd_rand */
    memcpy(odd_rand, randnum, sizeof(dword)*nsize);

    remainder = (degree + 1) % DWORD_BITS;

    if (remainder)
    {
        mask = mod32;
        mask >>= (DWORD_BITS - remainder);
        odd_rand[nsize - 1] &= mask;
        /* set the most significant bit to 1: */
        odd_rand[nsize - 1] |= ((dword)1 << (remainder - 1));
    }
    else
        odd_rand[nsize - 1] |= ((dword)1 << (DWORD_BITS - 1));
}

