/*******************************************************************************
*
*   Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.
*   All Rights Reserved.
*   Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*   This module generates keys for USB debug unlocking.
*
*******************************************************************************/
#include "engine/enginefw_interface.h"
#include "keygen.h"

// Automatically add any non-class methods to the appropriate group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_APPLICATION

/******************************************************************************
@brief Generate unlock key for USB Debug.

@param[in] apKeyIn      Key used to generate unlock key.
@param[in] apKeyOut     Pointer to receive generated unlock key.

@return    Indicates success or failure from OpenSSL.
@retval    1            success.
@retval    0            failure.
*/
int
KeyGenUsbDebugUnlock(const securlib::Aes128KeyType *apKeyIn, securlib::Aes128KeyType *apKeyOut)
{
    FUNCTION_DEBUG_SENTRY;

    using namespace securlib;
    Aes128KeyType Data = {{0}};
    Aes128KeyType InitVecKey = {{0}};
    return EncryptAes128Cbc(apKeyOut->key, Data.key, AES_KEY_LENGTH, apKeyIn, &InitVecKey);
}

/******************************************************************************
@brief Generate unlock key from key file for USB Debug.

@param[in] apFileIn     Filename of input key file.
@param[in] apFileOut    Filename of unlock key file.

@return     Indicates success or error code:
@retval     KEY_GEN_USB_DBG_SUCCESS         Success
@retval     KEY_GEN_USB_DBG_READ_FAIL       Error occurred while reading the input key file.
@retval     KEY_GEN_USB_DBG_WRITE_FAIL      Error occurred while writing the unlock key file.
@retval     KEY_GEN_USB_DBG_ENCRYPT_FAIL    Error was returned from OpenSSL functions while encrypting.
*/
int
KeyGenUsbDebugUnlockFile(const char *apFileIn, const char *apFileOut)
{
    int res = KEY_GEN_USB_DBG_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    securlib::Aes128KeyType EncKey = {{0}};
    securlib::Aes128KeyType UnlockKey = {{0}};
    if(securlib::ReadAes128KeyFile(EncKey, apFileIn))
    {
        res = KEY_GEN_USB_DBG_READ_FAIL;
    }
    else
    {
        securlib::reverse8BitBytes(EncKey.key, sizeof(EncKey.key));
        if (0 == KeyGenUsbDebugUnlock(&EncKey, &UnlockKey))
        {
            res = KEY_GEN_USB_DBG_ENCRYPT_FAIL;
        }
        else
        {
            if(securlib::WriteAes128KeyFile(&UnlockKey, apFileOut))
            {
                res = KEY_GEN_USB_DBG_WRITE_FAIL;
            }
        }
    }
    return res;
}
