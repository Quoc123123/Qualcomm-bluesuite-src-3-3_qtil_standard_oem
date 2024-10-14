/******************************************************************************
FILENAME:    crypt_decrypt.c

Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:     rsa decryption routine for firmware signing

******************************************************************************/
#include "crypt_private.h"

int crypt_decrypt ( uint16 * block , const RSA_Key * key )
{
    multiprec_expFP ( block, 3, key->size, &(key->mod) );
    return TRUE;
}

