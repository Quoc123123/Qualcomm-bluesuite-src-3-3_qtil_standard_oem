/**********************************************************************
 *
 *  test.c
 *  
 *  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 **********************************************************************/

/* TEST FILE */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keygen_private.h"
#include <time.h>

const char * text_string = "This is a short test string.";

void get_randomness ( dword *randwords )
{
    int i;

    /* Get "random" double words using the clock and an ad-hoc algorithm */
    for (i = 0; i < 32; i++)
	randwords[0] ^= (dword)(time(NULL) << ((i & 16) + (randwords[0] >> 28)));

    for (i = 1; i < KEY_WIDTH_W ; i++)
	randwords[i] = ((dword)time(NULL) << ((i & 16) +
			(randwords[i] >> 28))) ^ randwords[i-1];
}

void test_encryption ( RSA_dKey * key )
{
    char plain_text [ KEY_WIDTH_W *sizeof(dword) ];
    char testc [ KEY_WIDTH_W *sizeof(dword) ];
    dword *test = (dword*) testc;
                     
    memset ( test , 0 , KEY_WIDTH_W *sizeof(dword) );
    memset ( plain_text , 0 , KEY_WIDTH_W *sizeof(dword) );
    memcpy ( plain_text , text_string , strlen ( text_string ) );
    memcpy ( test , plain_text , strlen (plain_text) );

    mp_exp ( test , key->key , KEY_WIDTH_W , &key->mod );
    mp_expFP ( test , KEY_WIDTH_W , 3 , &key->mod );

    if ( 0 == memcmp ( test , plain_text , KEY_WIDTH_W *sizeof(dword) ) )
        printf ("Enc/Dec successful.\n");
    else
        printf ("Enc/Dec failed.\n");
}

int main ( int argc , char ** argv )
{
    bool ok = TRUE;
    Prime_Candidate a , b ;
    RSA_dKey key;
    dword randomness[KEY_WIDTH_W];
    int count = 0;
    double tt;

    tt = (double)clock()/(double)CLOCKS_PER_SEC;

    get_randomness (randomness);
    initialise_pair_search(randomness);
    while ( count++ < 1000 )
    {
        get_prime_pair ( &a , &b );
        if ( generate_key ( &key , &a , &b ) )
	    test_encryption ( &key );
	else
	    printf ( "Key generation Failed.\n" );
	printf ( "Average time per key: %f" , ( (double)clock()/(double)CLOCKS_PER_SEC - tt ) / count );
    }
    finished_pair_search ();

    return ( ok ? 0 : 1 );
}
