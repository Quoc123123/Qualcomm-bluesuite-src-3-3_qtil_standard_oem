/******************************************************************************
FILENAME:    crypt_private.h

Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     exponentiation routines used in rsa

******************************************************************************/
#include "crypt_public.h"

#ifndef __CRYPT_PRIVATE_H__
#define __CRYPT_PRIVATE_H__

void multiprec_exp (word *x, const word *e, const word wsize,
		    const Modulus *ms);

void multiprec_expFP (word *x, const dword EXP, const word wsize,
		      const Modulus *ms);

#endif
