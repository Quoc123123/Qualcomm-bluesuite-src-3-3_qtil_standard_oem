///////////////////////////////////////////////////////////////////////////////
//  FILENAME:    keyfile.h
//  
//  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//  
//  PURPOSE:     reading and writing files containing private keys for signing
//               firmware releases.
//  
///////////////////////////////////////////////////////////////////////////////
#ifndef DFU_KEYFILE_H
#define DFU_KEYFILE_H

#include <string>

#include "dfu_private.h"
#include "unicode/ichar.h"
extern "C"
{
#include "crypt_public.h"
}


bool writePrivateKey (const RSA_Key *aKey, const std::string &aFilename,  const std::string &aDescription);
bool writePrivateKey (const RSA_Key *aKey, const std::wstring &aFilename, const std::string &aDescription);
bool writePrivateKey (const RSA_Key *aKey, const char *aFilename,         const std::string &aDescription);

bool readPrivateKey (RSA_Key *aKey, const std::string &aFilename,  std::string &aDescription);
bool readPrivateKey (RSA_Key *aKey, const std::wstring &aFilename, std::string &aDescription);
bool readPrivateKey (RSA_Key *aKey, const char *aFilename,         std::string &aDescription);

#endif /* DFU_KEYFILE_H */
