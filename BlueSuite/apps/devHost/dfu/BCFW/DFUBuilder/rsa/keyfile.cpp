///////////////////////////////////////////////////////////////////////////////
//
//  FILENAME:    keyfile.cpp
//  
//  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//  
//  PURPOSE:     reading and writing files containing private keys for signing
//               firmware releases.  Always uses a 16 bit scheme.
//  
//               Wrappers are provided for flexibility so the library can be reused easily.
//
///////////////////////////////////////////////////////////////////////////////
#include "keyfile.h"
#include <fstream>
#include <iostream>
#include <limits.h>

#ifdef DEBUG
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT 0 &&
#endif

///////////////////////////////////////////////////////////////////////////////
//  FILE FORMAT:
//
// Encoding is ASCII.
//
//  --startoffile--
//  Comment...
//  ... any number of lines ...
//  comment.
//  @exponent uint16[64]
//  @Modulus  uint16[64]
//  @M'       uint16
//  @R^2n     uint16[64]
//  @R^n      uint16[64]
//  --endoffile--
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
@brief Write a DFU key to file wrapper for std::string (and istring)
*/
bool writePrivateKey (const RSA_Key *aKey, const std::string &aFilename, const std::string &aDescription)
{
    return writePrivateKey(aKey, aFilename.c_str(), aDescription);
}

/******************************************************************************
@brief Write a DFU key to file wrapper for std::wstring (and istring)
*/
bool writePrivateKey (const RSA_Key *aKey, const std::wstring &aFilename, const std::string &aDescription)
{
    return writePrivateKey(aKey, inarrow_filename(aFilename).c_str(), aDescription);
}

/******************************************************************************
@brief Write a DFU key to file

@param[in]  aKey            the RSA key
@param[in]  aFilename       the filename of the key file.
@param[in]  aDescription    the comments found in the key file.

@return true on success

*/
bool writePrivateKey (const RSA_Key *aKey, const char *aFilename, const std::string &aDescription)
{
    bool ok = aKey && strlen(aFilename);
    std::ofstream os (aFilename, std::ios::binary | std::ios::out );

    if ( ok && os )
    {
        if ( !aDescription.empty() )
        {
            os << inarrow(aDescription) << std::endl;
        }
        os.setf (std::ios::hex | std::ios::uppercase , std::ios::basefield );
        os << "@";
        int i = 0;
        for ( i = 0 ; os && i < KEY_WIDTH ; ++i )
        {
            os << " " << aKey->key[i];
        }
        os << std::endl << "@";
        for ( i = 0 ; os && i < KEY_WIDTH ; ++i )
        {
            os << " " << aKey->mod.M[i];
        }
        os << std::endl << "@ " << aKey->mod.M_dash << std::endl << "@";
        for ( i = 0 ; os && i < KEY_WIDTH ; ++i )
        {
            os << " " << aKey->mod.R2NmodM[i];
        }
        os << std::endl << "@";
        for ( i = 0 ; os && i < KEY_WIDTH ; ++i )
        {
            os << " " << aKey->mod.RNmodM[i];
        }
        os << std::endl;
    }
    else
    {
        ok = false;
    }
    return ok;
}


/******************************************************************************
@brief Read a DFU key from file wrapper function for std::string (and istring)
*/
bool readPrivateKey (RSA_Key *aKey, const std::string &aFilename, std::string &aDescription)
{
    return readPrivateKey(aKey, aFilename.c_str(), aDescription);
}

/******************************************************************************
@brief Read a DFU key from file wrapper function for std::wstring (and istring)
*/
bool readPrivateKey (RSA_Key *aKey, const std::wstring &aFilename, std::string&aDescription)
{
    return readPrivateKey(aKey, inarrow_filename(aFilename).c_str(), aDescription);
}

/******************************************************************************
@brief Read a DFU key from file

@param[out] aKey            the RSA key
@param[in]  aFilename       the filename of the key file.
@param[out] aDescription    the comments found in the key file.

@return true on success

*/
bool readPrivateKey (RSA_Key *aKey, const char *aFilename, std::string &aDescription)
{
    bool ok = aKey && strlen(aFilename) && aDescription.empty();

    if ( ok )
    {
        std::ifstream is(aFilename, std::ios::in | std::ios::binary);
        while ( is && is.peek() != '@' )
        {
            std::string line;
            std::getline ( is, line );
            aDescription += line + '\n';
        }
        if ( !is )
        {
            ok =  false;
        }

        if ( ok )
        {
            is.setf (std::ios::hex , std::ios::basefield );
            int i = 0;
            // skip the @
            if (is)
            {
                is.ignore();
                for ( i = 0 ; is && i < KEY_WIDTH ; ++i )
                {
                    is >> aKey->key[i];
                }
            }
            else
                ok = false;

            if (ok && is)
            {
                is.ignore( INT_MAX, '@' );
                for ( i = 0 ; is && i < KEY_WIDTH ; ++i )
                {
                    is >> aKey->mod.M[i];
                }
            }
            else
            {
                ok = false;
            }

            if (ok && is)
            {
                is.ignore( INT_MAX , '@');
                is >> aKey->mod.M_dash;
            }
            else
            {
                ok = false;
            }

            if (ok && is)
            {
                is.ignore( INT_MAX , '@');
                for ( i = 0 ; is && i < KEY_WIDTH ; ++i )
                {
                    is >> aKey->mod.R2NmodM[i];
                }
            }
            else
            {
                ok = false;
            }

            if (ok && is)
            {
                is.ignore( INT_MAX , '@');
                for ( i = 0 ; is && i < KEY_WIDTH ; ++i )
                {
                    is >> aKey->mod.RNmodM[i];
                }
            }
            else
            {
                ok = false;
            }
            
            if (ok && is)
            {
                aKey->size = KEY_WIDTH;
                aKey->mod.one[0] = 1;
                for ( i = 1 ; i < KEY_WIDTH ; ++i )
                {
                    aKey->mod.one[i] = 0;
                }
            }
            else
            {
                ok = false;
            }
        }
    }
    return ok;
}
