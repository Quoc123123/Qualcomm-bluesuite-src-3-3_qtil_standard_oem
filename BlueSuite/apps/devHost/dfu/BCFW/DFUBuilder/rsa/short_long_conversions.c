/**********************************************************************
 *
 *  short_long_conversions.c
 *  
 *  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To implement a few conversion routines from uint16 <-> uint32 
 *  arrays.
 *  The arrays are all LSW first.
 *
 **********************************************************************/

#include "short_long_conversions.h"
#include <stdlib.h>
#include <string.h>

dword join ( word msb, word lsb )
{
    return ( ( (dword) msb) << WORD_BITS ) + lsb ;
}

void split ( word * msb , word * lsb , dword value )
{
    *msb = (value >> WORD_BITS) & mod16;
    *lsb = value & mod16;
}

void array_widen  ( word  input  [/* size */] ,
                    dword output [/* size/2 */] ,
                    word  size )
{
    word i;
    for ( i = 0 ; i < size ; i += 2 )
        output [i>>1] = join ( input[i+1] , input[i] );
}

void array_narrow ( dword input  [/* size */] ,
                    word  output [/* size*2 */] ,
                    word  size )
{
    word i;
    for ( i = 0 ; i < size ; i ++ )
        split ( &output[ (i<<1) + 1 ] , &output[i<<1] , input[i] );
}

void modulus_widen  ( Modulus  *input , dModulus *output )
{
    array_widen ( input->M , output->M , KEY_WIDTH );
    /* need some way to widen M' */
    abort();
    array_widen ( input->R2NmodM , output->R2NmodM , KEY_WIDTH );
    array_widen ( input->RNmodM , output->RNmodM , KEY_WIDTH );
    array_widen ( input->one , output->one , KEY_WIDTH );
}

void modulus_narrow ( dModulus *input , Modulus  *output )
{
    array_narrow ( input->M , output->M , KEY_WIDTH_W );
    output->M_dash = input->M_dash & mod16;
    array_narrow ( input->R2NmodM , output->R2NmodM , KEY_WIDTH_W );
    array_narrow ( input->RNmodM , output->RNmodM , KEY_WIDTH_W );
    array_narrow ( input->one , output->one , KEY_WIDTH_W );
}


void key_widen  ( RSA_Key  *input , RSA_dKey *output )
{
    array_widen ( input->key , output->key , KEY_WIDTH );
    output->size = KEY_WIDTH_W;
    modulus_widen ( &(input->mod) , &(output->mod) );
}

void key_narrow ( RSA_dKey *input , RSA_Key  *output )
{
    array_narrow ( input->key , output->key , KEY_WIDTH_W );
    output->size = KEY_WIDTH;
    modulus_narrow ( &(input->mod) , &(output->mod) );
}


