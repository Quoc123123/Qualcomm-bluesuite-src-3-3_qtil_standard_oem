////////////////////////////////////////////////////////////////////////////////
//
//  FILE     :  rsa_library.cpp
//
//  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE  :  C++ wrapper for hostside rsa encryption
//
////////////////////////////////////////////////////////////////////////////////

#include "rsa_library.h"
#include "keyfile.h"
extern "C"
{
#include "crypt_public.h"
}
#include <fstream>

bool rsa_sign ( const istring &file ,
                uint16 * blockStart ,
                uint16 * blockEnd ,
                std::string &description )
{
    RSA_Key privateKey;
    return readPrivateKey ( &privateKey, file , description )
        && ( blockEnd - blockStart == privateKey.size )
        && crypt_sign ( blockStart , &privateKey );
}

bool rsa_decrypt ( const istring &file ,
                   uint16 * blockStart ,
                   uint16 * blockEnd )
{
    RSA_Key privateKey;
    std::string tmp;
    return readPrivateKey ( &privateKey , file.c_str() , tmp )
        && ( blockEnd - blockStart == privateKey.size )
        && crypt_decrypt ( blockStart , &privateKey );
}

