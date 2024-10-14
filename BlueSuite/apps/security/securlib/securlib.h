/*******************************************************************************
 *
 *  securlib.h
 *
 *  Copyright (c) 2013-2021 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Holds declarations for a C-style interface library of functions which 
 *  transform plaintext byte streams (for example holding program imab).
 *
 ******************************************************************************/

#ifndef SECURLIB_H
#define SECURLIB_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include "rsa/crypt_public.h"
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef SECURLIB_EXPORT_ME
#define SECURLIB_API __declspec(dllexport)
#else
#define SECURLIB_API __declspec(dllimport)
#endif
#else
#define SECURLIB_API
#endif

const size_t ASPK_MODULUS_LENGTH = 256; /* number of bits */
const size_t ASPK_MODULUS_SIZE = ASPK_MODULUS_LENGTH / 8; /* number of bytes */

/* Functions using OpenSSL */

/******************************************************************************
@brief Initialise OpenSSL.

@note
Call this function before using any other OpenSSL functions.
*/
SECURLIB_API void OsslInit(void);

/******************************************************************************
@brief Finalise OpenSSL.

@note
Call this to free up resources.
*/
SECURLIB_API void OsslFin(void);

/******************************************************************************
@brief Perform XOR on apA and apB placing result in apResult (R = A XOR B).

@param[in]  apA         Pointer to first BIGNUM operand.
@param[in]  apB         Pointer to second BIGNUM operand.
@param[out] apResult    Pointer to BIGNUM to receive resulting value.

@return Success or error code. These are from or consistent with OpenSSL.
@retval     1   Success.
@retval     0   Failure.

@note
OpenSSL does not provide a BN_xor function.
OpenSSL uses big endian so binary values output from BN_bn2bin are big endian.
*/
SECURLIB_API int OsslBnXor(const BIGNUM *apA, const BIGNUM *apB, BIGNUM *apResult);

/******************************************************************************
@brief Calculate scrambled aspk.

@param[in]  apModulus       BIGNUM containing modulus.
@param[in]  apSeed          BIGNUM containing seed.
@param[in]  apAspk          BIGNUM containing aspk.
@param[out] apScrAspk       Pointer to BIGNUM to receive resulting value.

@return Success or error code. These are from or consistent with OpenSSL.
@retval     1   Success.
@retval     0   Failure.
*/
SECURLIB_API int OsslCalcScrAspk(const BIGNUM *apModulus, const BIGNUM *apSeed, const BIGNUM *apAspk, BIGNUM *apScrAspk);

/******************************************************************************
@brief Calculate scrambled aspk and output to FILE.
@param[in]  apOutput    FILE to print scrambled aspk.
@param[in]  apModulus   BIGNUM containing modulus.
@param[in]  apSeed      BIGNUM containing seed.
@param[in]  apAspk      BIGNUM containing aspk.

@return Success or error code. These are from or consistent with OpenSSL.
@retval     1   Success.
@retval     0   Failure.
*/
SECURLIB_API int OsslPrintScrAspk(FILE *apOutput, const BIGNUM *apModulus, const BIGNUM *apSeed, const BIGNUM *apAspk);

/******************************************************************************
@brief Convert base64 string to a binary unsigned int (big endian).
@param[in]  apBase64    base64 string. Maximum length is INT_MAX.
@param[out] apOutput    pointer to output buffer.
@param[in]  aOutputLen  size of output buffer. Maximum is INT_MAX.

@return Success or error code. These are from or consistent with OpenSSL. See return values of BIO_read
    for further information.
@retval    >0   Success, value is size of data.
@retval    0,-1 Failure.
@retval    -2   operation is not implemented.
*/
SECURLIB_API int OsslBase64ToBin(const char *apBase64, void *apOutput, size_t aOutputLen);

/******************************************************************************
@brief Convert base64 string to a BIGNUM.
@param[in]  apBase64    base64 string.
@param[out] apResult    Pointer to BIGNUM to receive resulting value.

@return Success or error code. These are from or consistent with OpenSSL.
@retval     1   Success.
@retval     0   Failure.
*/
SECURLIB_API int OsslBase64ToBn(const char *apBase64, BIGNUM *apResult);

/******************************************************************************
@brief Read public key from DFU file and extract required portion into a BIGNUM.
@param[in]  apFilename  The filename of the DFU public key file.
@param[out] apResult    Pointer to BIGNUM to receive resulting value.

@return Success or error code. These are from or consistent with OpenSSL.
@retval     1   Success.
@retval     0   Failure.
*/
SECURLIB_API int ExtractAspkModDfuKey(const char *apFilename, BIGNUM *apResult);

/******************************************************************************
@brief Read public key from PEM file and extract required portion into a BIGNUM.
@param[in]  apFilename  The filename of the PEM public key file.
@param[out] apResult    Pointer to BIGNUM to receive resulting value.

@return Success or error code. These are from or consistent with OpenSSL.
@retval     1   Success.
@retval     0   Failure.
*/
SECURLIB_API int ExtractAspkModPubKey(const char *apFilename, BIGNUM *apResult);


/******************************************************************************
@brief Read an OpenSSL private key file.

@param[in] apFilename   The filename

@return Pointer to EVP_PKEY structure.
@retval non NULL    EVP_PKEY structure containing private key.
@retval NULL        an error occurred.

@note
Must be freed with EVP_PKEY_free()
*/
SECURLIB_API EVP_PKEY *OsslReadPrvKeyFile(const char *apFilename);

/******************************************************************************
@brief Read an OpenSSL public key file.

@param[in] apFilename   The filename

@return Pointer to EVP_PKEY structure.
@retval non NULL    EVP_PKEY structure containing public key.
@retval NULL        an error occurred.

@note
Must be freed with EVP_PKEY_free()
*/
SECURLIB_API EVP_PKEY *OsslReadPubKeyFile(const char *apFilename);

/******************************************************************************
@brief Generate a RSA Key Pair (Private Key).

@param[in]  aBits       Size of key in bits.
@param[in]  aPubExp     Public exponent.

@return Pointer to key.
@retval NULL    error or failure from OpenSSL functions.

@note
Must be freed with EVP_PKEY_free()
*/
SECURLIB_API EVP_PKEY *OsslRsaGenKeyPair(int aBits, unsigned long aPubExp);

/******************************************************************************
@brief Write an OpenSSL private key file in PEM PKCS#8 format.

@param[in] apKey        The key to write.
@param[in] apFilename   The filename.

@return Success or error code. These are from or consistent with OpenSSL.
@retval 1   Success.
@retval 0   Failure.
*/
SECURLIB_API int OsslWritePrvKeyFile(EVP_PKEY *apKey, const char *apFilename);

/******************************************************************************
@brief Write an OpenSSL public key file in PEM PKCS#8 format.

@param[in] apKey        The key to write.
@param[in] apFilename   The filename.

@return Success or error code. These are from or consistent with OpenSSL.
@retval 1   Success.
@retval 0   Failure.
*/
SECURLIB_API int OsslWritePubKeyFile(EVP_PKEY *apKey, const char *apFilename);

/******************************************************************************
@brief Generate a RSA Key Pair (Private Key).

@param[in] aBits        Size of key in bits.
@param[in] aPubExp      Public exponent.
@param[in] apPrvKeyFile Filename of private key.
@param[in] apPubKeyFile Filename of public key (Optional, NULL to not write public key).

@return Success or error code.
@retval 1   Success.
@retval 0   Failure.

@note
Public key is not much good without private key. That's why only the public key is optional.
Public key can be extracted from private key.
*/
SECURLIB_API int OsslRsaGenKeyPairFiles(int aBits, unsigned long aPubExp, const char *apPrvKeyFile, const char *apPubKeyFile);

/******************************************************************************
@brief Sign a digest (hash) using OpenSSL RSA with PSS padding.

@param[in] pPrvKey      The RSA private key.
@param[in] apMd         The message digest to sign.
@param[in] aMdLen       The length of the message digest to sign.
@param[out] apSig       Pointer to the buffer to receive signature. May be NULL
                        to obtain the buffer size required in *apSigLen.
@param[in,out] apSigLen Pointer to value containing the maximum and receiving
                        the actual length of the buffer to receive signature.

@return Value indicating success or failure from OpenSSL functions.
@retval 1 or positive   Success.
@retval 0 or negative   failure.
@retval -2              operation is not supported.
@note
See https://www.openssl.org/docs/man1.0.2/crypto/ to understand specific error
code for each function.

@note
The key size dictates the block size in the algorithm.
ie. EVP_PKEY_size(pPrvKey)
*/
SECURLIB_API int OsslRsaPssSign(EVP_PKEY *pPrvKey, const unsigned char *apMd, const size_t aMdLen, unsigned char *apSig, size_t *apSigLen);

/******************************************************************************
@brief Encrypt block of data with given algorithm.

@param[in] aType        Algorithm (EVP_CIPHER).
@param[in] apOut        Pointer to output data.
@param[in] apOutSize    Pointer to variable to receive output data size.
@param[in] apIn         Pointer to input data.
@param[in] aInSize      Input data size.
@param[in] apKey        Key for encryption. Shall be EVP_CIPHER_key_length(aType).
@param[in] apIv         Initialisation vector. Shall be EVP_CIPHER_iv_length(aType).
@param[in] aPadding     Value to pass to EVP_CIPHER_CTX_set_padding().

@return Success or error code.
@retval 1   Success.
@retval 0   Failure.

@note
See https://www.openssl.org/docs/man1.0.2/crypto/ to understand specific error
code for each function.

@note
The amount of data written may be larger than the input so apOut should contain
sufficient room (aInSize + EVP_MAX_BLOCK_LENGTH).
*/
SECURLIB_API int OsslEncrypt(const EVP_CIPHER *aType, unsigned char *apOut, size_t *apOutSize,
    const unsigned char *apIn, size_t aInSize, const unsigned char *apKey, const unsigned char *apIv, int aPadding);

/******************************************************************************
@brief Calculate CBC-MAC with given algorithm.

@param[in] aType        Algorithm (EVP_CIPHER). Shall be in CBC mode.
@param[in] apOut        Pointer to output data. Shall be EVP_CIPHER_block_size(aType).
@param[in] apIn         Pointer to input data.
@param[in] aInSize      Input data size.
@param[in] apKey        Key for encryption. Shall be EVP_CIPHER_key_length(aType).
@param[in] aPadding     Value to pass to EVP_CIPHER_CTX_set_padding().

@return Success or error code.
@retval 1   Success.
@retval 0   Failure.

@note
See https://www.openssl.org/docs/man1.0.2/crypto/ to understand specific error
code for each function.

@note
The CBC-MAC is calculated by encrypting input image and taking the last block as the MAC.
This function has less memory requirements than the encrypt function. The output image
being the block size instead of one block larger than the input image.
*/
SECURLIB_API int OsslCbcMac(const EVP_CIPHER *aType, unsigned char *apOut,
    const unsigned char *apIn, size_t aInSize, const unsigned char *apKey, int aPadding);


/******************************************************************************
@brief Calculate the M_dash value
This is a derivative from the modulus but required input by firmware code.

    M_dash = -M^(-1) (mod 2^16)
    where
        M is the key modulus
        M^(-1) is the modular inverse of the key modulus.
        -M^(-1) is the negation of the modular inverse.
            In this case negation means the two's complement.
        mod 2^16 is the modulo.

@param[in] apRsa    An OpenSSL RSA object containing private or public key.

@return Pointer to BIGNUM.
@retval non NULL    BIGNUM containing M_dash.
@retval NULL        an error occurred.

@note
Must be freed with BN_free()
*/
SECURLIB_API BIGNUM *OsslCalcMdash(const RSA *apRsa);

/******************************************************************************
@brief Calculate the RNmodM value
This is a derivative from the modulus but required input by firmware code.

There are a number of ways it can bve calculated:
    RNmodM = 2^W mod M
    RNmodM = 2^W - M
    RNmodM = 0 - M mod 2^W
    where
        W is the key size in bits (ie. 1024 for RSA1024)
        M is the key modulus
        ^ is raise to the power of (and not the XOR operator)

    Using OpenSSL BIGNUMs the fastest is using subtraction (2^W - M).

@param[in] apRsa    An OpenSSL RSA object containing private or public key.

@return Pointer to BIGNUM.
@retval non NULL    BIGNUM containing RNmodM.
@retval NULL        an error occurred.

@note
Must be freed with BN_free()
*/
SECURLIB_API BIGNUM *OsslCalcRNmodM(const RSA *apRsa);

/******************************************************************************
@brief Calculate the R2NmodM value
This is a derivative from the modulus but required input by firmware code.

    R2NmodM = R^(2W) mod M
    where
        W is the key size (ie. 1024 for RSA1024)
        M is the key modulus
        2W is 2 multiply W
        ^ is raise to the power of (and not the XOR operator)

@param[in] apRsa    An OpenSSL RSA object containing private or public key.

@return Pointer to BIGNUM.
@retval non NULL    BIGNUM containing R2NmodM.
@retval NULL        an error occurred.

@note
Must be freed with BN_free()
*/
SECURLIB_API BIGNUM *OsslCalcR2NmodM(const RSA *apRsa);

/******************************************************************************
@brief Print a BIGNUM in DFU key format.

@param[in] apOutput The output file stream.
@param[in] aSize    Size of the BIGNUM as binary (in number of octets).
                    Valid range: 0 <= aSize <= INT_MAX
@param[in] apN      BIGNUM to output.
*/
SECURLIB_API void OsslPrintBnDfu(FILE *apOutput, int aSize, const BIGNUM *apN);

/******************************************************************************
@brief Print a Public/Private key in DFU key format.

@param[in] apOutput The output file stream.
@param[in] aPrv     True for private key.
@param[in] apKey    Public/Private key to convert and output.
*/
SECURLIB_API void OsslPrintPkeyDfu(FILE *apOutput, int aPrv, EVP_PKEY *apKey);


/******************************************************************************
@brief Encrypt block of data with using AES-128 with custom CTR mode.

@param[in] apOut        Pointer to output data.
@param[in] apOutSize    Pointer to variable to receive output data size.
@param[in] apIn         Pointer to input data.
@param[in] aInSize      Input data size.
@param[in] apKey        Key for encryption. Shall be EVP_CIPHER_key_length(EVP_aes_128_ecb()).
@param[in] apIv         Initialisation vector. Shall be EVP_CIPHER_iv_length(EVP_aes_128_ecb()).
@param[in] aAddress     Address in the flash device.

@return Success or error code.
@retval 1   Success.
@retval 0   Failure.

@note
See https://www.openssl.org/docs/man1.0.2/crypto/ to understand specific error
code for each function.

@note
The amount of data written may be larger than the input so apOut should contain
sufficient room (aInSize + EVP_MAX_BLOCK_LENGTH).
*/
SECURLIB_API int OsslEncryptAes128Cctr(unsigned char *apOut, size_t *apOutSize,
    const unsigned char *apIn, size_t aInSize,
    const unsigned char *apKey, const unsigned char *apIv,
    uint32_t aAddress);

#ifdef __cplusplus
}
#endif


namespace securlib {

    const size_t SIGNATURE_LENGTH = 128;
    const size_t AES_KEY_LENGTH = 16; /* octets, 128 bits */

    SECURLIB_API void reverse8BitBytes(unsigned char* words, const unsigned num_bytes);

    /* Define type for hash. */
    typedef struct { unsigned char digest[SHA256_DIGEST_LENGTH]; } HashType;
    typedef struct { unsigned char signature[SIGNATURE_LENGTH]; } SignatureType;
    typedef struct { unsigned char key[AES_KEY_LENGTH]; } Aes128KeyType;
    typedef std::vector<Aes128KeyType> Aes128KeysType;

    /******************************************************************************
    @brief Calculate SHA256 hash.

    @param[out] apHash    Calculated hash.
    @param[in] apData     Input data.
    @param[in] aDataSize  Input data size.

    @return Indicates success or failure from OpenSSL.
    @retval     1   Success.
    @retval     0   Failure.
    */
    SECURLIB_API int HashSha256(securlib::HashType *apHash, const void *apData, size_t aDataSize);

    /******************************************************************************
    @brief Sign a hash with private key.
    
    @param[in] apPrivateKey Private used to sign.
    @param[in] apHash       Input hash.
    @param[out] apSignature The output signature.
    */
    SECURLIB_API void SignHash(const RSA_Key *apPrivateKey, const securlib::HashType *apHash,
        securlib::SignatureType *apSignature);

    /******************************************************************************
    @brief Encrypt using AES 128 CBC (to match firmware)

    @param[in] aDataOut     Output data block.
    @param[in] aDataIn      Input data block to be encrypted.
    @param[in] aDataSize    Size of data of aDataIn and aDataOut.
    @param[in] aKey         Key to encrypt with.

    @return Indicates success or failure from OpenSSL.
    @retval     1   Success.
    @retval     0   Failure.

    @note
    With initialisation vector, byte swapping and reversing to match firmware.
    */
    SECURLIB_API int  Encrypt(unsigned char *apDataOut, const unsigned char *apDataIn, int aDataSize, Aes128KeyType aKey);

    /******************************************************************************
    @brief Encrypt using AES 128 CBC

    @param[in] apDataOut    Output data block.
    @param[in] apDataIn     Input data block to be encrypted.
    @param[in] aDataSize    Size of data of aDataIn and aDataOut.
    @param[in] apKey        Key to encrypt with
    @param[in] apIv         Initialisation vector
    @return                 Indicates success or failure from OpenSSL.
    @retval    1            success.
    @retval    0            failure.
    */
    SECURLIB_API int  EncryptAes128Cbc(unsigned char * aDataOut, const unsigned char *aDataIn,
        int aDataSize, const Aes128KeyType *aKey, const Aes128KeyType *aIv);

    /******************************************************************************
    @brief Read DFU (RSA) key from file

    @param[out] apKey           RSA key
    @param[in]  apFilename      Filename of key file.
    @param[in]  aDescription    Comments from the key file.

    @return true on failure

    @note
    Wrapper to pull the function into the namespace and export from the DLL
    */
    SECURLIB_API bool ReadRsaPrivateKey(RSA_Key *apKey, const char *apFilename, std::string &aDescription);

    /******************************************************************************
    @brief Read AES128 key from file

    @param[out] aKey        AES128 key
    @param[in]  apFilename  Filename of key file.

    @return true on failure

    @note
    The order of bytes return are the same in the file. The translation is due to KeyFile module.
    */
    SECURLIB_API bool ReadAes128KeyFile(Aes128KeyType& aKey, const char *apFilename);

    /******************************************************************************
    @brief Read one or more LE octet ordered AES128 keys from a BE octet ordered file

    @param[out] aKeys       AES128 key collection
    @param[in]  apFilename  Filename of keys file.

    @return true on failure

    @note
    The order of bytes returned are 128bit LittleEndian in memory, from 128bit BigEndian in file.
    */
    SECURLIB_API bool ReadLeAes128KeysFromBeFile(Aes128KeysType& aKeys, const char *apFilename);

    /******************************************************************************
    @brief Write AES128 key from file

    @param[in]  aKey        AES128 key
    @param[in]  apFilename  Filename of key file.

    @return true on failure

    @note
    The order of bytes are the same in the file.
    */
    SECURLIB_API bool WriteAes128KeyFile(const Aes128KeyType *apKey, const char *apFilename);

    /******************************************************************************
    @brief Persist one or more LE octet ordered AES128 keys to file, arranging as BE octet order

    @param[in]  aKeys       AES128 key collection
    @param[in]  apFilename  Filename of keys file.

    @return true on failure

    @note
    The order of bytes written are to 128bit BigEndian in file, from 128bit LittleEndian in memory.
    */
    SECURLIB_API bool WriteLeAes128KeysToBeFile(const Aes128KeysType& aKeys, const char *apFilename);

} /* namespace securlib */

#endif /* SECURLIB_H */
