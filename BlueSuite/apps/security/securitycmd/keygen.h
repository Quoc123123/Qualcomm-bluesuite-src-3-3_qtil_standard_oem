/*******************************************************************************
*
*   Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.
*   All Rights Reserved.
*   Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*   This module generates keys for USB debug unlocking.
*
*******************************************************************************/
#include "securlib/securlib.h"

int KeyGenUsbDebugUnlock(const securlib::Aes128KeyType *aKeyIn, securlib::Aes128KeyType *apKeyOut);

/* Result of KeyGenUsbDebugUnlockFile */
enum {
    KEY_GEN_USB_DBG_SUCCESS = 0,
    KEY_GEN_USB_DBG_READ_FAIL,
    KEY_GEN_USB_DBG_WRITE_FAIL,
    KEY_GEN_USB_DBG_ENCRYPT_FAIL
};
int KeyGenUsbDebugUnlockFile(const char *apFileIn, const char *aFileOut);
