/*******************************************************************************
 *
 *  securlib.cpp
 *
 *  Copyright (c) 2016-2021 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Holds definitions for a C-style interface library of functions which
 *  transform plaintext byte streams (for example holding program imab).
 *
 ******************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <memory.h>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#include "engine/enginefw_interface.h"

#ifndef SECURLIB_EXPORT_ME
#define SECURLIB_EXPORT_ME
#endif
#include "securlib.h"
#include "rsa/keyfile.h"
#include "keyfile/keyfile.h"
#include <common/types_t.h>

// Automatically add any non-class methods to the appropriate group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_APPLICATION


void OsslInit(void)
{
    FUNCTION_DEBUG_SENTRY;
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    ENGINE_load_builtin_engines();
    RAND_poll();
    if(RAND_status() < 1)
    {
        fprintf(stderr, "WARNING: Random number generator has been seeded insufficiently.\n");;
    }
}


void OsslFin(void)
{
    FUNCTION_DEBUG_SENTRY;
    OBJ_cleanup();
    EVP_cleanup();
    ENGINE_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_remove_thread_state(NULL);
    RAND_cleanup();
    ERR_free_strings();
}


int OsslBnXor(const BIGNUM *apA, const BIGNUM *apB, BIGNUM *apResult)
{
    int result = 0; /* default to failure */
    FUNCTION_DEBUG_SENTRY_RET(int, result);
    if(apA && apB && apResult)
    {
        /* Calculate size of widest number. Binary values must be same size to perform XOR. */
        int sizea = BN_num_bytes(apA);
        int sizeb = BN_num_bytes(apB);
        int size = sizea;
        if(sizeb > size)
        {
            size = sizeb;
        }
        /* allocate memory for both binary values. */
        unsigned char *pBuf = new unsigned char[2 * size];
        if(pBuf)
        {
            unsigned char *pA = pBuf;
            unsigned char *pB = pBuf + size;
            /* BN_bn2binpad returns size or -1 (buffer too small) */
            int res = BN_bn2binpad(apA, pA, size);
            if (res == size)
            {
                res = BN_bn2binpad(apB, pB, size);
            }
            if (res == size)
            {
                /* perform XOR with binary values */
                for(int i = 0; i < size; i++)
                {
                    pA[i] ^=  pB[i];
                }
                /* Convert binary back to a BIGNUM in r */
                result = BN_bin2bn(pA, size, apResult) != NULL;
            }
            else /* PROGRAMMING ERRORS */
            {
                /* res != size
                This is a programming error in this function or potentially a bug
                in BN_bn2binpad.
                res == OSSL_ERR_TOOSMALL: buffer is too small for number.
                */
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "ERROR: res != size (%d != %d)", res, size);
                assert(0);
            }
            delete[] pBuf;
        }
    }
    return result;
}

int OsslCalcScrAspk(const BIGNUM *apModulus, const BIGNUM *apSeed, const BIGNUM *apAspk, BIGNUM *apScrAspk)
{
    int result = 0; /* default to failure */
    FUNCTION_DEBUG_SENTRY_RET(int, result);
    if(apModulus && apSeed && apAspk && apScrAspk)
    {
        if((result = OsslBnXor(apModulus, apSeed, apScrAspk)))
        {
            result = OsslBnXor(apAspk, apScrAspk, apScrAspk);
        }
    }
    return result;
}


int OsslBase64ToBin(const char *apBase64, void *apOutput, size_t aOutputLen)
{
    int dataLen = -1;
    FUNCTION_DEBUG_SENTRY_RET(int, dataLen);
    size_t len = strlen(apBase64);
    assert(len <= INT_MAX);
    assert(aOutputLen <= INT_MAX);
    BIO *pB64 = BIO_new(BIO_f_base64());
    if(pB64)
    {
        BIO *pBio = BIO_new_mem_buf(apBase64, static_cast<int>(len));
        if(pBio)
        {
            pBio = BIO_push(pB64, pBio);
            BIO_set_flags(pBio, BIO_FLAGS_BASE64_NO_NL);
            dataLen = BIO_read(pBio, apOutput, static_cast<int>(aOutputLen));
            BIO_free_all(pBio);
        }
        else
        {
            BIO_free(pB64);
        }
    }
    return dataLen;
}


int OsslBase64ToBn(const char *apBase64, BIGNUM *apResult)
{
    int result = 0; /* default to failure */
    FUNCTION_DEBUG_SENTRY_RET(int, result);
    if(apBase64 && apResult)
    {
        /* calculate buffer size (over estimate) */
        size_t len = 3 * (strlen(apBase64) / 4) + 3;
        unsigned char *pBuf = new unsigned char[len];
        if(pBuf)
        {
            /* convert to binary */
            int dataLen = OsslBase64ToBin(apBase64, pBuf, len);
            if(dataLen > 0)
            {
                /* convert binary to BIGNUM */
                result = BN_bin2bn(pBuf, dataLen, apResult) != NULL;
            }
            delete[] pBuf;
        }
    }
    return result;
}


int ExtractAspkModPubKey(const char *apFilename, BIGNUM *apResult)
{
    int result = 0; /* default to failure */
    FUNCTION_DEBUG_SENTRY_RET(int, result);
    if(apFilename && apResult)
    {
        EVP_PKEY *pKey = OsslReadPubKeyFile(apFilename);
        if(pKey)
        {
            RSA *pRsa = EVP_PKEY_get1_RSA(pKey);
            if(pRsa)
            {
                const BIGNUM *pRsaMod; /* internal value owned by RSA object */
                BIGNUM *pMod;
                RSA_get0_key(pRsa, &pRsaMod, NULL, NULL);
                if((pMod = BN_dup(pRsaMod)) != NULL)
                {
                    /* truncate to required width */
                    result = BN_mask_bits(pMod, ASPK_MODULUS_LENGTH);
                    if(result)
                    {
                        result = BN_copy(apResult, pMod) != NULL;
                    }
                    BN_free(pMod);
                }
                RSA_free(pRsa);
            }
            EVP_PKEY_free(pKey);
        }
    }
    return result;
}


int ExtractAspkModDfuKey(const char *apFilename, BIGNUM *apResult)
{
    int result = 0; /* default to failure */
    FUNCTION_DEBUG_SENTRY_RET(int, result);
    if (apFilename && strlen(apFilename) && apResult)
    {
        RSA_Key pubKey;
        std::string description;
        result = !securlib::ReadRsaPrivateKey(&pubKey, apFilename, description);
        if(result)
        {
            unsigned char pBuf[ASPK_MODULUS_SIZE];
            const size_t words = ASPK_MODULUS_SIZE / sizeof(pubKey.mod.M[0]);
            for(size_t i = 0, b = ASPK_MODULUS_SIZE-2; i < words; ++i, b-=2)
            {
                pBuf[b] = pubKey.mod.M[i] >> 8;
                pBuf[b+1] = pubKey.mod.M[i] & 0xFF;
            }
            /* convert binary to BIGNUM */
            result = BN_bin2bn(pBuf, ASPK_MODULUS_SIZE, apResult) != NULL;
        }
    }
    return result;
}


int OsslPrintScrAspk(FILE *apOutput, const BIGNUM *apModulus, const BIGNUM *apSeed, const BIGNUM *apAspk)
{
    int result = 0; /* default to failure */
    FUNCTION_DEBUG_SENTRY_RET(int, result);
    if(apOutput && apModulus && apSeed && apAspk)
    {
        BIGNUM *pResult = BN_new();
        if(pResult)
        {
            if((result = OsslCalcScrAspk(apModulus, apSeed, apAspk, pResult)))
            {
                result = BN_print_fp(apOutput, pResult);
            }
            BN_free(pResult);
        }
    }
    return result;
}


EVP_PKEY *OsslReadPrvKeyFile(const char *apFilename)
{
    FILE *pFile = NULL;
    EVP_PKEY *pKey = NULL;
    FUNCTION_DEBUG_SENTRY_RET(EVP_PKEY*, pKey);

    if((pFile = fopen(apFilename, "rt")))
    {
        pKey = PEM_read_PrivateKey(pFile, NULL, NULL, NULL);
        fclose(pFile);
    }
    return pKey;
}


EVP_PKEY *OsslReadPubKeyFile(const char *apFilename)
{
    FILE *pFile = NULL;
    EVP_PKEY *pKey = NULL;
    FUNCTION_DEBUG_SENTRY_RET(EVP_PKEY*, pKey);

    if((pFile = fopen(apFilename, "rt")))
    {
        pKey = PEM_read_PUBKEY(pFile, NULL, NULL, NULL);
        fclose(pFile);
    }
    return pKey;
}


int OsslWritePrvKeyFile(EVP_PKEY *apKey, const char *apFilename)
{
    int res = 0; /* To match OpenSSL error for these functions */
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    if(apKey)
    {
        FILE *pFile = fopen(apFilename, "w");
        if(pFile)
        {
            res = PEM_write_PKCS8PrivateKey(pFile, apKey, NULL, NULL, 0, NULL, NULL);
            fclose(pFile);
        }
    }
    return res;
}


int OsslWritePubKeyFile(EVP_PKEY *apKey, const char *apFilename)
{
    int res = 0; /* To match OpenSSL error for these functions */
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    if(apKey)
    {
        FILE *pFile = fopen(apFilename, "w");
        if(pFile)
        {
            res = PEM_write_PUBKEY(pFile, apKey);
            fclose(pFile);
        }
    }
    return res;
}


EVP_PKEY *OsslRsaGenKeyPair(int aBits, unsigned long aPubExp)
{
    BIGNUM   *pPubExp = BN_new();
    RSA      *pRsa    = RSA_new();
    EVP_PKEY *pKey    = EVP_PKEY_new();
    int res = 0;
    FUNCTION_DEBUG_SENTRY_RET(EVP_PKEY*, pKey);

    if(pRsa && pPubExp && pKey)
    {
        if((res = BN_set_word(pPubExp, aPubExp)) > 0)
        {
            if((res = RSA_generate_key_ex(pRsa, aBits, pPubExp, NULL)) > 0)
            {
                res = EVP_PKEY_set1_RSA(pKey, pRsa);
            }
        }
    }
    if(pRsa)
    {
        RSA_free(pRsa);
    }
    if(pPubExp)
    {
        BN_free(pPubExp);
    }
    if(res <= 0)
    {
        EVP_PKEY_free(pKey);
        pKey = NULL;
    }
    return pKey;
}


int OsslRsaGenKeyPairFiles(int aBits, unsigned long aPubExp, const char *apPrvKeyFile, const char *apPubKeyFile)
{
    int res = 0;
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    EVP_PKEY *pKey = OsslRsaGenKeyPair(aBits, aPubExp);
    if(pKey && apPrvKeyFile)
    {
        if((res = OsslWritePrvKeyFile(pKey, apPrvKeyFile)) > 0)
        {
            if(apPubKeyFile && strlen(apPubKeyFile))
            {
                res = OsslWritePubKeyFile(pKey, apPubKeyFile);
            }
        }
    }
    if(pKey)
    {
        EVP_PKEY_free(pKey);
    }
    return res;
}


int OsslRsaPssSign(EVP_PKEY *pPrvKey, const unsigned char *apMd, const size_t aMdLen, unsigned char *apSig, size_t *apSigLen)
{
    int res = 0; /* To match OpenSSL error for these functions */
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    /* md is a SHA-256 digest in this example. */

    if(pPrvKey && apMd && apSigLen)
    {
        EVP_MD_CTX *pMdCtx;
        if((pMdCtx = EVP_MD_CTX_create()))
        {
            EVP_PKEY_CTX *pKeyCtx;
            if((res = EVP_DigestSignInit(pMdCtx, &pKeyCtx, EVP_sha256(), NULL, pPrvKey)) > 0)
            {
                if((res = EVP_PKEY_CTX_set_rsa_padding(pKeyCtx, RSA_PKCS1_PSS_PADDING)) > 0)
                {
                    if((res = EVP_PKEY_CTX_set_rsa_pss_saltlen(pKeyCtx, -2)) > 0)
                    {
                        if((res = EVP_DigestSignUpdate(pMdCtx, apMd, aMdLen)) > 0)
                        {
                            /* Determine buffer length */
                            size_t siglen = 0;
                            if((res = EVP_DigestSignFinal(pMdCtx, NULL, &siglen)) > 0)
                            {
                                if(apSig && *apSigLen >= siglen) /* output buffer big enough? */
                                {
                                    res = EVP_DigestSignFinal(pMdCtx, apSig, &siglen);
                                }
                                *apSigLen = siglen;
                            }
                        }
                    }
                }
            }
            EVP_MD_CTX_destroy(pMdCtx);
        }
    }
    return res;
}


BIGNUM *OsslCalcMdash(const RSA *apRsa)
{
    BIGNUM *r = BN_new();
    FUNCTION_DEBUG_SENTRY_RET(BIGNUM *, r);
    int res = 0; /* To match OpenSSL error for these functions */
    const unsigned long modulo = 0x10000; /* modulo is 2^16 */
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *mod = BN_new();
    res = BN_set_word(mod, modulo);
    if(res > 0 && apRsa && ctx && r && mod)
    {
        const BIGNUM *pRsaMod; /* internal value owned by RSA object */
        RSA_get0_key(apRsa, &pRsaMod, NULL, NULL);
        if(BN_mod_inverse(r, pRsaMod, mod, ctx))
        {
            /* get two's complement */
            res = BN_sub(r, mod ,r);
        }
    }
    if(mod)
    {
        BN_free(mod);
    }
    if(ctx)
    {
        BN_CTX_free(ctx);
    }
    if(res <= 0)
    {
        BN_free(r);
        r = NULL;
    }
    return r;
}


BIGNUM *OsslCalcRNmodM(const RSA *apRsa)
{
    BIGNUM *r = BN_new();
    FUNCTION_DEBUG_SENTRY_RET(BIGNUM*, r);
    int res = 0; /* To match OpenSSL error for these functions */
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *keysize = BN_new();
    if(apRsa && ctx && keysize && r)
    {
        res = BN_set_bit(keysize, 8*RSA_size(apRsa)); /* Key size in bits */
        if(res)
        {
            const BIGNUM *pRsaMod; /* internal value owned by RSA object */
            RSA_get0_key(apRsa, &pRsaMod, NULL, NULL);
            res = BN_sub(r, keysize, pRsaMod);
        }
    }
    if(keysize)
    {
        BN_free(keysize);
    }
    if(ctx)
    {
        BN_CTX_free(ctx);
    }
    if(!res)
    {
        BN_free(r);
        r = NULL;
    }
    return r;
}


BIGNUM *OsslCalcR2NmodM(const RSA *apRsa)
{
    BIGNUM *r = BN_new();
    FUNCTION_DEBUG_SENTRY_RET(BIGNUM*, r);
    int res = 0; /* To match OpenSSL error for these functions */
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *keysize = BN_new();
    if(apRsa && ctx && keysize && r)
    {
        res = BN_set_bit(keysize, 2*8*RSA_size(apRsa));  /* 2N where N is key size for RSA */
        if(res)
        {
            const BIGNUM *pRsaMod; /* internal value owned by RSA object */
            RSA_get0_key(apRsa, &pRsaMod, NULL, NULL);
            res = BN_mod(r, keysize, pRsaMod, ctx);
        }
    }
    if(keysize)
    {
        BN_free(keysize);
    }
    if(ctx)
    {
        BN_CTX_free(ctx);
    }
    if(!res)
    {
        BN_free(r);
        r = NULL;
    }
    return r;
}


void OsslPrintBnDfu(FILE *apOutput, int aSize, const BIGNUM *apN)
{
    FUNCTION_DEBUG_SENTRY;
    typedef uint16_t word_t;
    if(aSize >= 0)
    {
        if(apN)
        {
            int buflen = BN_num_bytes(apN);
            if(aSize > buflen)
            {
                buflen = aSize;
            }
            buflen += buflen % sizeof(word_t);
            unsigned char *pBuf = new unsigned char[buflen];
            if(pBuf)
            {
                int res = BN_bn2binpad(apN, pBuf, buflen);
                if(res == buflen)
                {
                    fputc('@', apOutput);
                    /* output in little endian format */
                    for(int i = buflen; i > 0; /* in body */)
                    {
                        i -= sizeof(word_t);
                        fprintf(apOutput, " %x", pBuf[i] << 8 | pBuf[i+1]);
                    }
                    fputc('\n', apOutput);
                }
                else /* PROGRAMMING ERRORS */
                {
                    /* res != size
                    This is a programming error in this function or potentially a bug
                    in BN_bn2binpad.
                    res == OSSL_ERR_TOOSMALL: buffer is too small for number.
                    */
                    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "ERROR: res != buflen (%d != %d)", res, buflen);
                    assert(0);
                }
                delete[] pBuf;
            }
        }
    }
    else
    {
        /* aSize outside valid range */
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "ERROR: aSize invalid. %d >= 0 must be true", aSize);
        assert(0);
    }
}


void OsslPrintPkeyDfu(FILE *apOutput, int aPrv, EVP_PKEY *apKey)
{
    FUNCTION_DEBUG_SENTRY;
    RSA *pRsa = EVP_PKEY_get1_RSA(apKey);
    if(pRsa)
    {
        /* internal values owned by RSA object */
        const BIGNUM *pRsaMod;
        const BIGNUM *pRsaPrvExp;
        const BIGNUM *pRsaPubExp;
        RSA_get0_key(pRsa, &pRsaMod, &pRsaPubExp, &pRsaPrvExp);
        if(aPrv)
        {   /* private exponent */
            OsslPrintBnDfu(apOutput, EVP_PKEY_size(apKey), pRsaPrvExp);
        }
        else
        {   /* public exponent */
            OsslPrintBnDfu(apOutput, EVP_PKEY_size(apKey), pRsaPubExp);
        }
        OsslPrintBnDfu(apOutput, EVP_PKEY_size(apKey), pRsaMod);
        BIGNUM *r = OsslCalcMdash(pRsa);
        if(r)
        {
            OsslPrintBnDfu(apOutput, sizeof(uint16_t), r);
            BN_free(r);
        }
        r = OsslCalcR2NmodM(pRsa);
        if(r)
        {
            OsslPrintBnDfu(apOutput, EVP_PKEY_size(apKey), r);
            BN_free(r);
        }
        r = OsslCalcRNmodM(pRsa);
        if(r)
        {
            OsslPrintBnDfu(apOutput, EVP_PKEY_size(apKey), r);
            BN_free(r);
        }
        RSA_free(pRsa);
    }
}


int OsslEncrypt(const EVP_CIPHER *apType, unsigned char *apOut, size_t *apOutSize,
    const unsigned char *apIn, size_t aInSize,
    const unsigned char *apKey, const unsigned char *apIv, int aPadding)
{
    int res = 0; /* default to failure (to match OpenSSL error for these functions) */
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    /* Create and initialise the context */
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(ctx)
    {
        if((res = EVP_EncryptInit_ex(ctx, apType, NULL, apKey, apIv)))
        {
            (void) EVP_CIPHER_CTX_set_padding(ctx, aPadding);
            unsigned char *pBlkOut = apOut;
            size_t OutSize = 0;
            const size_t BLKSIZE = INT_MAX;
            size_t i;
            for(i = 0; i < aInSize; i += BLKSIZE)
            {
                size_t remainder = aInSize - i;
                const unsigned char *pBlkIn = apIn + i;
                int len = 0;
                if(remainder > BLKSIZE)
                {
                    remainder = BLKSIZE;
                }
                res = EVP_EncryptUpdate(ctx, pBlkOut, &len, pBlkIn, static_cast<int>(remainder));
                pBlkOut += len;
                OutSize += len;
                if(!res)
                {
                    break;
                }
                
            }
            if(res)
            {
                int len = 0;
                if((res = EVP_EncryptFinal_ex(ctx, pBlkOut, &len)))
                {
                    if(apOutSize)
                    {
                        *apOutSize = OutSize + len;
                    }
                }
            }
        }
        EVP_CIPHER_CTX_free(ctx);
    }
    return res;
}


int OsslCbcMac(const EVP_CIPHER *aType, unsigned char *apOut,
    const unsigned char *apIn, size_t aInSize, const unsigned char *apKey,
    int aPadding)
{
    int res = 0; /* default to failure (to match OpenSSL error for these functions) */
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    const size_t BLKSIZE = EVP_CIPHER_block_size(aType);
    const unsigned char iv[EVP_MAX_IV_LENGTH] = {0};

    if(EVP_CIPHER_mode(aType) == EVP_CIPH_CBC_MODE)
    {
        /* Create and initialise the context */
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if(ctx)
        {
            if((res = EVP_EncryptInit_ex(ctx, aType, NULL, apKey, iv)))
            {
                (void) EVP_CIPHER_CTX_set_padding(ctx, aPadding);
                for(size_t i = 0; i < aInSize; i += BLKSIZE)
                {
                    size_t remainder = aInSize - i;
                    const unsigned char *pBlkIn = apIn + i;
                    int len = 0;
                    if(remainder > BLKSIZE)
                    {
                        remainder = BLKSIZE;
                    }
                    res = EVP_EncryptUpdate(ctx, apOut, &len, pBlkIn, static_cast<int>(remainder));
                    if(!res)
                    {
                        break;
                    }
                    
                }
                if(res)
                {
                    int len = 0;
                    if((res = EVP_EncryptFinal_ex(ctx, apOut, &len)))
                    {
                        res = ((size_t)len <= BLKSIZE);
                    }
                }
            }
            EVP_CIPHER_CTX_free(ctx);
        }
    }
    return res;
}


int OsslEncryptAes128Cctr(unsigned char *apOut, size_t *apOutSize,
    const unsigned char *apIn, size_t aInSize,
    const unsigned char *apKey, const unsigned char *apIv,
    uint32_t aAddress)
{
    int res;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    const EVP_CIPHER *cipher = EVP_aes_128_ecb();
    size_t OutSize = 0;
    aAddress /= 16;
    size_t i;
    unsigned char ctr[EVP_MAX_BLOCK_LENGTH] = {0};
    const size_t BLKSIZE = EVP_CIPHER_block_size(cipher);

    /* Put address into counter ordering as little endian */
    ctr[0] = aAddress & 0xFF;
    ctr[1] = (aAddress >> 8) & 0xFF;
    ctr[2] = (aAddress >> 16) & 0xFF;
    ctr[3] = (aAddress >> 24) & 0xFF;
    /* Iterate over BLKSIZE size chunks */
    for(i = 0; i < aInSize; i += BLKSIZE)
    {
        size_t j;
        size_t outlen;
        unsigned char in[EVP_MAX_BLOCK_LENGTH];
        unsigned char out[EVP_MAX_BLOCK_LENGTH];
        size_t remainder = aInSize - i;
        if(remainder > BLKSIZE)
        {
            remainder = BLKSIZE;
        }
        /* Combine (XOR) the counter and IV */
        for(j = 0; j < BLKSIZE; ++j)
        {
            in[j] = ctr[j] ^ apIv[BLKSIZE-1-j];
        }
        outlen = BLKSIZE;
        res = OsslEncrypt(cipher, out, &outlen, in, BLKSIZE, apKey, apIv, 0);
        if(res != 1)
        {
            break;
        }
        OutSize += remainder;
        /* XOR the plain text with cipher output */
        for(j = 0; j < remainder; ++j)
        {
            apOut[i+j] = out[j] ^ apIn[i+j];
        }
        /* Increment the counter considering counter as little endian */
        for(j = 0; j < BLKSIZE; ++j)
        {
            ctr[j]++;
            if(ctr[j])
            {
                break;
            }
        }
    }
    if(apOutSize)
    {
        *apOutSize = OutSize;
    }
    return res;
}


namespace securlib {

    int HashSha256(securlib::HashType *apHash, const void *apData, size_t aDataSize)
    {
        /* Create hash with sha256 */
        SHA256_CTX sha256;
        int res = SHA256_Init(&sha256);
        FUNCTION_DEBUG_SENTRY_RET(int, res);

        res &= SHA256_Update(&sha256, apData, aDataSize);
        res &= SHA256_Final(apHash->digest, &sha256);
        #if 0
        puts("Sha256");
        for(unsigned int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        {
            printf("%02x", apHash->digest[i]);
        }
        puts("\n");
        #endif
        return res;
    }

    void SignHash(const RSA_Key *apPrivateKey, const securlib::HashType *apHash, securlib::SignatureType *apSignature)
    {
        FUNCTION_DEBUG_SENTRY;
    #if 0
        printf("securlib::Sign %u:", SHA256_DIGEST_LENGTH);
        for(size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        {
            printf("%02X ", apHash->digest[i]);
        }
        puts("");
    #endif
        /* Data to be signed must be 128 octets long, so repeat hash 4 times. */
        for(size_t i = 0; i < securlib::SIGNATURE_LENGTH; ++i)
        {
            apSignature->signature[i] = apHash->digest[SHA256_DIGEST_LENGTH -1 - (i % SHA256_DIGEST_LENGTH)];
        }
        /* Force data to be numerically less than modulus. */
        apSignature->signature[securlib::SIGNATURE_LENGTH-1] = 0;
        apSignature->signature[securlib::SIGNATURE_LENGTH-2] = 0;
    #if 0
        printf("hash %u:", securlib::SIGNATURE_LENGTH);
        for(int i = 0; i < securlib::SIGNATURE_LENGTH; ++i)
        {
            printf("%02X ", apSignature->signature[i]);
        }
        puts("");
        printf("Size of PrvKey %u\n", apPrivateKey->size);
    #endif
        ::crypt_sign((uint16*)(&apSignature->signature[0]), apPrivateKey);
    #if 1
        std::reverse(&apSignature->signature[0], apSignature->signature + securlib::SIGNATURE_LENGTH);
    #endif
    #if 0
        printf("after hash %u:", securlib::SIGNATURE_LENGTH);
        for(int i = 0; i < securlib::SIGNATURE_LENGTH; ++i)
        {
            printf("%02X ", aSignature->signature[i]);
        }
        puts("");
    #endif
    }

    //! Reverse the 8bit bytes in their block
    void reverse8BitBytes(unsigned char* words, const unsigned num_bytes)
    {
        FUNCTION_DEBUG_SENTRY;
        assert(0 == num_bytes % 2);

        unsigned char tmp;
        unsigned i = 0;
        const unsigned half = num_bytes / 2;

        for(i = 0; i < half; ++i )
        {
            unsigned opposite_word_ix = (num_bytes - 1) - i;
            tmp = words[i];
            words[i] = words[opposite_word_ix];
            words[opposite_word_ix] = tmp;
        }
    }

    //! Swap the 8bit bytes in their (16bit) words
    void byteSwap(unsigned char* bytes, const size_t num_bytes)
    {
        FUNCTION_DEBUG_SENTRY;
        size_t i = 0;
        for(i = 0; i < num_bytes; i += 2)
        {
            unsigned char tmp = bytes[i];
            bytes[i] = bytes[i+1];
            bytes[i+1] = tmp;
        }
    }


    int Encrypt(unsigned char * aDataOut, const unsigned char *aDataIn, int aDataSize, Aes128KeyType aKey)
    {
        int retval = 0;
        FUNCTION_DEBUG_SENTRY_RET(int, retval);
        Aes128KeyType iv = {{0}};

        reverse8BitBytes(aKey.key, AES_KEY_LENGTH);
        retval = EncryptAes128Cbc(aDataOut, aDataIn, aDataSize, &aKey, &iv);
        byteSwap(aDataOut, aDataSize);

        return retval;
    }


    int EncryptAes128Cbc(unsigned char * apDataOut, const unsigned char *apDataIn,
        int aDataSize, const Aes128KeyType *apKey, const Aes128KeyType *apIv)
    {
        int retval = 0;
        FUNCTION_DEBUG_SENTRY_RET(int, retval);

        /* Create and initialise the context */
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if(ctx)
        {
            retval = EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, apKey->key, apIv->key);
            if(1 == retval)
            {
                int len = 0;
                retval = EVP_EncryptUpdate(ctx, apDataOut, &len, apDataIn, aDataSize);
            }

            EVP_CIPHER_CTX_free(ctx);
        }
        return retval;
    }


    bool ReadRsaPrivateKey(RSA_Key *apKey, const char *apFilename, std::string &aDescription)
    {
        FUNCTION_DEBUG_SENTRY;
        return !::readPrivateKey(apKey, apFilename, aDescription);
    }


    bool ReadAes128KeyFile(Aes128KeyType& aKey, const char *apFilename)
    {
        bool failure = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, failure);
        CKeyFile keyFile(apFilename);
        std::vector<uint16> keyBuf;
        CKeyFile::KeyfileStatusEnum res = keyFile.GetKeyFromFile(keyBuf);
        if(res == CKeyFile::KEYFILE_SUCCESS && AES_KEY_LENGTH == keyBuf.size()*2)
        {
            /* Copy keyBuf (uint16[8] big endian) into aKey.key (uchar[16]) correcting uint16 byte order */
            for(size_t i = 0; i < AES_KEY_LENGTH; ++i)
            {
                aKey.key[i] = ((unsigned char *)&keyBuf[0])[i ^ 1];
            }
            failure = false;
        }
        return failure;
    }


    bool ReadLeAes128KeysFromBeFile(Aes128KeysType& aKeys, const char *apFilename)
    {
        bool failure = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, failure);
        CKeyFile keyFile(apFilename);
        std::vector<CKeyFile::KeyValue_t> keys;
        CKeyFile::KeyfileStatusEnum keyStatus = keyFile.GetKeysFromFile(keys);
        if (keyStatus == CKeyFile::KEYFILE_SUCCESS)
        {
            for (size_t key = 0; key < keys.size(); ++key)
            {
                Aes128KeyType oneKey = { {0} };
                CKeyFile::KeyValue_t& keyAsWords = keys[key];
                for (size_t i = 0; i < keyAsWords.size(); ++i)
                {   // given "1234..." key[0] is 0x1234 so work backwards and push Lo octet followed by Hi octet
                    size_t idx = i * 2;
                    oneKey.key[idx] = (keyAsWords[keyAsWords.size() - i - 1] & 0xFF);
                    oneKey.key[idx+1] = ((keyAsWords[keyAsWords.size() - i - 1] >> 8) & 0xFF);
                }
                if (keyAsWords.size() * 2 == AES_KEY_LENGTH)
                {
                    aKeys.push_back(oneKey);
                }
                else
                {
                    failure = true;
                    break;
                }
            }
        }
        else
        {
            failure = true;
        }
        return failure;
    }


    bool WriteAes128KeyFile(const Aes128KeyType *apKey, const char *apFilename)
    {
        bool failure = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, failure);
        CKeyFile keyFile(apFilename);
        std::vector<uint16> keyBuf;

        /* Pack the key into KeyBuf (uint16[8] big endian) */
        keyBuf.resize(AES_KEY_LENGTH / sizeof(uint16));
        for(size_t i = 0, j = 0; j < AES_KEY_LENGTH; i++, j+=2)
        {
            keyBuf[i] = apKey->key[j+1] | (apKey->key[j] << 8);
        }
        CKeyFile::KeyfileStatusEnum res = keyFile.WriteKeyToOutputFile(keyBuf, apFilename);
        failure = (res != CKeyFile::KEYFILE_SUCCESS);
        return failure;
    }


    bool WriteLeAes128KeysToBeFile(const Aes128KeysType& aKeys, const char *apFilename)
    {
        bool failure = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, failure);
        CKeyFile keyFile(apFilename);
        std::vector<CKeyFile::KeyValue_t> keys;

        CKeyFile::KeyValue_t oneKey;
        for (size_t key = 0; key < aKeys.size(); ++key)
        {
            Aes128KeyType keyBuf = aKeys[key];
            securlib::reverse8BitBytes(keyBuf.key, sizeof(keyBuf.key));

            oneKey.clear();
            for (size_t i = 0; i < AES_KEY_LENGTH; i+=2)
            {
                oneKey.push_back((keyBuf.key[i]<<8) + keyBuf.key[i+1]);
            }
            keys.push_back(oneKey);
        }
        CKeyFile::KeyfileStatusEnum res = keyFile.WriteKeysToOutputFile(keys, apFilename);
        failure = (res != CKeyFile::KEYFILE_SUCCESS);
        return failure;
    }

} /* namespace securlib */
