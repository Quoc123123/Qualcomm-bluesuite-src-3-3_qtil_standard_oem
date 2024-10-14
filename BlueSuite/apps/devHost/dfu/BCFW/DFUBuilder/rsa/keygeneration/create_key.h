/*******************************************************************************

FILE: create_key.h

Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.

PURPOSE:  To provide routines to create RSA keys for use with signing.

$Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/devHost/dfu/BCFW/DFUBuilder/rsa/keygeneration/create_key.h#1 $

*******************************************************************************/

#ifndef __KEYGEN_H__
#define __KEYGEN_H__

Prime_Candidate * newPrime             ( void );
void              deleteChain          ( Prime_Candidate * p );
Prime_Candidate * nextPrime            ( Prime_Candidate * p );
Prime_Candidate * test_multiply        ( Prime_Candidate * test ,
                                         Prime_Candidate * list );
void              eliminate_used_prime ( Prime_Candidate * used );
void              copy_prime           ( Prime_Candidate * to ,
                                         Prime_Candidate * from );

#endif
