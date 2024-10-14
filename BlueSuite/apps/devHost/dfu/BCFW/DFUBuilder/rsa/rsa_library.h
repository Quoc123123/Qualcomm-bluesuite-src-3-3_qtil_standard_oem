////////////////////////////////////////////////////////////////////////////////
//
//  FILE     :  rsa_library.h
//
//  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE  :  C++ wrapper for hostside rsa encryption
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __RSA_LIBRARY_H__
#define __RSA_LIBRARY_H__

#include "dfu_private.h"
#include "unicode/ichar.h"

bool rsa_sign ( const istring &file ,
                uint16 * blockStart ,
                uint16 * blockEnd ,
                std::string &description );

bool rsa_decrypt ( const istring &file ,
                   uint16 * blockStart ,
                   uint16 * blockEnd );

#endif
