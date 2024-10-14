/**********************************************************************
 *
 *  mp_exponentiation.h
 *  
 *  Copyright (c) 2001-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  To provide arbitrary precision exponentiation routines for use
 *  with big numbers!
 *
 **********************************************************************/

#ifndef __MP_EXPONENTIATION_H__
#define __MP_EXPONENTIATION_H__

void mp_exp (dword *x, dword *e, const word size, const dModulus *ms);
void mp_2exp (dword *ans, dword *e, const word size, const dModulus *ms);
void mp_expFP (dword x[], const dword size, const dword EXP, const dModulus *ms);

#endif
