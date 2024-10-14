/*******************************************************************************
*
*   Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.
*   All Rights Reserved.
*   Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*   This module signs and encrypts a firmware XUV file specific to uenergy app store.
*
*******************************************************************************/
#include "securlib/securlib.h"
#include "xuvreader.h"


void XuvImageHash(xuv::image &aXuvImage, securlib::HashType &apHash, unsigned int aAddrFirst,
    unsigned int aAddrLast, unsigned int aAddrHash, bool aZeroHeader);
void XuvImageHashSign(xuv::image &aXuvImage, const securlib::HashType apHash, const RSA_Key *apSignPrvKey,
    unsigned int aAddrSignature);
bool XuvImageEncrypt(xuv::image &aXuvImage, securlib::Aes128KeyType aEncKey,
    unsigned int aAddrFirst, unsigned int aAddrLast);

/* Result of SecureAppStoreImage */
enum {
    SECUREAPPSTOREIMG_SUCCESS = 0, 
    SECUREAPPSTOREIMG_ERR_ENCRYPT,
    SECUREAPPSTOREIMG_ERR_NO_APP,
    SECUREAPPSTOREIMG_ERR_TOO_SMALL,
    SECUREAPPSTOREIMG_ERR_RESERVED_BITS
};
int SecureAppStoreImage(xuv::image &aXuvImage, RSA_Key *apSignPrvKey,
    securlib::Aes128KeyType *apEncKey, unsigned int *apAddress, bool aCopyExec, bool aZeroHeader);

/* Result of FSecureAppStoreImage */
enum {
    FSECUREAPPSTOREIMG_SUCCESS = 0,
    /* Leave gap for SECUREAPPSTOREIMG_ values that may also be returned */
    FSECUREAPPSTOREIMG_ERR_READ_ENCKEY = 100,
    FSECUREAPPSTOREIMG_ERR_READ_SIGKEY,
    FSECUREAPPSTOREIMG_ERR_READ_IMG,
    FSECUREAPPSTOREIMG_ERR_IMG_EMPTY,
    FSECUREAPPSTOREIMG_ERR_WRITE_IMG,
};
int FSecureAppStoreImage(const char *apInFile, const char *apSignKeyFile,
    const char *apEncrKeyFile, const char *apOutFile,
    unsigned int *apAddress, bool aCopyExec, bool aZeroHeader);
