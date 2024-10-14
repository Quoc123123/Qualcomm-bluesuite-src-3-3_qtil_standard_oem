/**********************************************************************
 *
 *  mp_arithmatic.h
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To provide arbitrary precision arithmatic routine for use with 
 *  big numbers!
 *
 **********************************************************************/

#ifndef __MP_ARITHMATIC_H__
#define __MP_ARITHMATIC_H__

void mp_add        ( dword *p , const dword *m , const dword size );
void mp_addWC      ( dword *p , const dword size , const dword WC );
void mp_sub        ( dword *p , const dword *m , const dword size );
void mp_subWC      ( dword *p , const dword size , const dword WC );
bool mp_iszero     ( const dword *x , const dword size);
bool mp_isequal    ( const dword *x , const dword *y, const word size);
bool mp_gte        ( const dword *x , const dword *y, const dword size);
void mp_Lshift     ( dword *x , const word tbits, const word size);
void mp_Rshift     ( dword *x , const word tbits, const word size);
dword mp_degree    ( const dword *x , const dword size);
word mp_getbit     ( dword *x, const dword size, const dword bitnumber);
void mp_setbit     ( dword *x, const dword size, const dword bitnumber,
                     const word bitvalue);
void mp_multiply   ( dword *y , dword *x,
                     const word y_size, const word x_size, dword *w);
void mp_multiplyWC ( dword *y , const word y_size, const dword WC, dword *w);
void mp_mont_mult  ( dword *A, const dword *B, const dModulus *ms,
                     dword P[/* size + 1 */], const word size);
word mp_divideby3  ( dword *x , const dword size );

#endif
