/**********************************************************************
*
*  FILE   :  main.h
*
*            Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  Generic main unicode and non-unicode
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/result/include/unicode/main.h#1 $
*
***********************************************************************/

#ifndef UNICODE__MAIN_H
#define UNICODE__MAIN_H

/* Include header files */
#include "unicode/ichar.h"


#ifdef  __cplusplus
extern "C" {
#endif

/* The program using this library should define the following function */
int imain(int argc, ichar **argv);

#ifdef  __cplusplus
}
#endif


#endif
