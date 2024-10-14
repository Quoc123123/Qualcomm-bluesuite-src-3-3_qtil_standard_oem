/**********************************************************************
 *
 *  short_long_conversions.h
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To provide a few conversion routines from uint16 <-> uint32 arrays.
 *  The arrays are all LSW first.
 *
 **********************************************************************/

#include "crypt_public.h"
#include "keygen_public.h"

#ifndef _SHORT_LONG_CONVERSIONS_H_
#define _SHORT_LONG_CONVERSIONS_H_

dword join ( word msb, word lsb );
void split ( word * msb , word * lsb , dword value );

void array_widen  ( word  input  [/* size */] ,
		    dword output [/* size/2 */] ,
		    word  size );
void array_narrow ( dword input  [/* size */] ,
		    word  output [/* size*2 */] ,
		    word  size );

void modulus_widen  ( Modulus  *input , dModulus *output );
void modulus_narrow ( dModulus *input , Modulus  *output );

void key_widen  ( RSA_Key  *input , RSA_dKey *output );
void key_narrow ( RSA_dKey *input , RSA_Key  *output );

#endif
