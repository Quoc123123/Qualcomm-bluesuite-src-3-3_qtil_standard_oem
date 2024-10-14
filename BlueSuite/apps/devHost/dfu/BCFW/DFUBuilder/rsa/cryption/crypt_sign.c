/******************************************************************************
FILENAME:    crypt_sign.c

Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     rsa encryption routine for firmware signing

******************************************************************************/
#include "crypt_private.h"

int crypt_sign ( uint16 * block , const RSA_Key * key )
{
    multiprec_exp( block , key->key , key->size , &(key->mod) );
    return TRUE;
}

