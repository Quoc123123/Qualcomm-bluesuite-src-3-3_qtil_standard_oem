/*******************************************************************************

FILE: create_key.c

Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:  To provide routines to create RSA keys for use with signing.

$Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/devHost/dfu/BCFW/DFUBuilder/rsa/keygeneration/create_key.c#1 $

*******************************************************************************/

#define RSA_REQUIRED_DEGREE (1023)

#include "keygen_private.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Prime_Candidate * nextPrime ( Prime_Candidate * p )
{
    return (Prime_Candidate *) p->next;
}

Prime_Candidate * newPrime ()
{
    Prime_Candidate *p;
    p = malloc ( sizeof (Prime_Candidate) );
    memset ( p , 0 , sizeof (Prime_Candidate) );
    p->degree = 511;
    return p;
}

void deleteChain ( Prime_Candidate * p )
{
    Prime_Candidate *tbd = p;
    while ( tbd )
    {
        p = nextPrime (tbd);
        free (tbd);
        tbd = p;
    }
}

Prime_Candidate * test_multiply ( Prime_Candidate * test , Prime_Candidate * list )
{
    dword result [ KEY_WIDTH_W + 1 ];
    dword scratch [ KEY_WIDTH_W + 1 ];
    dword degree;

    Prime_Candidate * ptr = list;
    while ( ptr )
    {
        memcpy ( result , ptr->prob_prime , (KEY_WIDTH_W + 1) * sizeof(dword) );
        mp_multiply ( result , test->prob_prime ,
                      KEY_WIDTH_W + 1 , KEY_WIDTH_W + 1 , scratch );

        degree = mp_degree ( result , KEY_WIDTH_W + 1 );

        if ( degree == RSA_REQUIRED_DEGREE )
            return ptr;

        ptr = nextPrime (ptr );
    }
    return NULL;
}

static Prime_Candidate * top = NULL;

void eliminate_used_prime ( Prime_Candidate * used )
{
    if ( top == used )
        top = nextPrime ( top );
    else
    {
        Prime_Candidate * list = top;
        while ( list )
        {
            if ( list->next == used )
                list->next = list->next->next;
            list = nextPrime (list);
        }
    }
}

void copy_prime ( Prime_Candidate * to , Prime_Candidate * from )
{
    to->degree = from->degree;
    memcpy ( to->prob_prime , from->prob_prime ,
             sizeof(dword)*( KEY_WIDTH_W + 1 ) );
}

void initialise_pair_search ( const dword *random_data )
{
    init_rand_seed ( random_data );
    deleteChain ( top );
}

void finished_pair_search ()
{
    deleteChain ( top );
}

void get_prime_pair ( Prime_Candidate * one ,
                      Prime_Candidate * two )
{
    bool done = FALSE;
    word size = 17;
    Prime_Candidate * current;
    Prime_Candidate * companion;

    int prime_count = 0;
    current = newPrime();

    while ( !done && prime_count++ < 100 )
    {
        /* get a random number */
        while (!strong_prime (current->prob_prime, (word)size,
                             &current->degree, 1 ) )
            ;

        companion = test_multiply ( current , top );
        if ( companion )
        {
            /*
            **  remove the companion from the list
            */
            eliminate_used_prime ( companion );
            /*
            **  copy then into the provided containers
            */
            copy_prime ( one, companion );
            copy_prime ( two, current );
            /*
            **  current has null next, companion points to the part
            **  of the list which used to hand from it.
            **  delete both by hanging current from companion.
            */
            companion->next = current;
            deleteChain ( companion );
            done = TRUE;
        }
        else
        {
            current->next = top;
            top = current;
            current = newPrime ();
        }
    }
    if ( !done )
        assert ( ("Found no primes" , 0 ) );
}

bool generate_key ( RSA_dKey * key,
                    Prime_Candidate * one ,
                    Prime_Candidate * two )
{
    bool ok = TRUE;
    dword modulus[KEY_WIDTH_W + 1];
    dword degree = 0, remainder;
    dword scratch[KEY_WIDTH_W + 1];
    dword temp   [KEY_WIDTH_W + 1];

    /* calculate the modulus */
    memcpy ( modulus , one->prob_prime , (KEY_WIDTH_W + 1) * sizeof(dword) );
    mp_multiply ( modulus , two->prob_prime ,
                  KEY_WIDTH_W + 1 , KEY_WIDTH_W + 1 , scratch );

    degree = mp_degree ( modulus , KEY_WIDTH_W + 1 );

    ok = ( degree == RSA_REQUIRED_DEGREE );

    if ( ok )
    {
        /*
        **  generate the modulus, and its hangers on.
        */
        memcpy ( key->mod.M , modulus , KEY_WIDTH_W * sizeof(dword) );

        /* M_dash = -M^-1 mod 2^32 */
        key->mod.M_dash = get_inv_ofM (key->mod.M[0]);

        /* RNmodM = 2^(32*KEYWIDTH) mod M \equiv 2^(32*KEYWIDTH) - M
                                          \equiv 0 - M mod 2^(32*KEYWIDTH) */
        memset ( key->mod.RNmodM , 0 , KEY_WIDTH_W *sizeof(dword) );
        mp_sub ( key->mod.RNmodM , key->mod.M , KEY_WIDTH_W );

        /* R2NmodM = 2^(2*32*KEYWIDTH) mod M */
        get_R2modM32 (key->mod.R2NmodM, key->mod.M, KEY_WIDTH_W );

        /* one = 1...  well, what did you expect? */
        memset ( key->mod.one , 0 , KEY_WIDTH_W * sizeof(dword) );
        key->mod.one[0] = 1;

        key->size = KEY_WIDTH_W;
        /*
        **  generate the private key = 3^(-1) mod M.
        */
        memcpy   ( scratch , one->prob_prime, (KEY_WIDTH_W+1)*sizeof(dword) );
        mp_subWC ( scratch , KEY_WIDTH_W + 1, 1 );
        memcpy   ( temp    , two->prob_prime, (KEY_WIDTH_W+1)*sizeof(dword) );
        mp_subWC ( temp    , KEY_WIDTH_W + 1, 1 );

        /* scratch = (p-1)(q-1) */
        mp_multiply ( scratch , temp ,
                      KEY_WIDTH_W + 1 , KEY_WIDTH_W + 1 , modulus );
        memcpy ( temp , scratch , (KEY_WIDTH_W+1)*sizeof(dword) );

        /* scratch = floor ( (p-1)(q-1)/3 ) */
        remainder = mp_divideby3 ( scratch , KEY_WIDTH_W + 1 );

        /* (p-1)(q-1) = 1 mod 3, by construction */
        if ( remainder == 1 )
        {
            /* d = (p-1)(q-1) - ( (p-1)(q-1) - 1 )/3 */
            mp_sub ( temp , scratch , KEY_WIDTH_W  + 1 );
            memcpy ( key->key , temp , KEY_WIDTH_W * sizeof(dword) );
        }
        else
            ok = FALSE;
    }
    return ok;
}
