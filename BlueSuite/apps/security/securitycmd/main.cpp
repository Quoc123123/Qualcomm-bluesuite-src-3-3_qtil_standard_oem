/*******************************************************************************
 *
 *  main.cpp
 *
 *  Copyright (c) 2016-2021 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  This application signs and encrypts a firmware XUV file.
 *
 ******************************************************************************/

#define PORTABILITY_DEFINE_STRCASESTR /* Need strcasestr from portability.h */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <cassert>

#include <string>
#include <vector>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h> // for RAND_bytes
#include "securlib/securlib.h"
#ifdef _MSC_VER
#include <openssl/applink.c>
#endif

#include "secureappstore.h"
#include "keygen.h"

#include "cmdline/cmdline.h"
#include "engine/enginefw_interface.h"
#include "common/portability.h"

/* Application name text */
#define NAME_NEW "SecurityCmd"
#define NAME_OLD "SignCmd"

// Automatically add any non-class methods to the appropriate group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_APPLICATION

#define ENV_SECURITYCMD_XUVE "SECURITYCMD_XUVE"

#define OPERATION_LIST      "OPERATION_LIST"
#define CMD_KEYGEN_USBDBG   "createunlockkey"
#define CMD_KEYGEN_RSA      "creatersakey"
#define CMD_WRAP_KEY        "wrapkey"
#define CMD_WRAP_KEY_AR     "wrapkeyar"
#define CMD_CBCMAC          "createcbcmac"
#define CMD_HASH            "hash"
#define CMD_SIGN            "sign"
#define CMD_ENCRYPT         "encrypt"
#define CMD_SIGNENCRYPT     "signencrypt"
#define CMD_SCRAMBLEASPK    "scrambleaspk"
#define CMD_PEMTODFUKEY     "pem2dfukey"
#define OPT_COPYEXEC        "copyexec"

#define OPT_PRODUCT         "-product"
#define OPT_ENDIAN          "-endian"
#define OPT_ADDRSTART       "-address"
#define OPT_USEIMGHDR       "-useimageheader"
#define OPT_PEM             "-pem"
#define OPT_DFU             "-dfu"


enum PRODUCTS { PR_UE, PR_HYD };
enum ENDIAN   { EN_DEFAULT, EN_U16BE, EN_U16LE };
enum KEYTYPES { KY_PRV, KY_PUB };
enum KEYFORMAT { KF_TEXT, KF_PEM, KF_DFU };
enum OPERATIONS { OP_KEYGEN_UNLOCK, OP_KEYGEN_RSA, OP_HASH, OP_SIGN, OP_ENCRYPT, OP_SIGNENCRYPT, 
    OP_PEMTODFUKEY, OP_CBCMAC, OP_SCRAMBLEASPK, OP_WRAP_KEY, OP_WRAP_KEY_AR };

struct
{
    PRODUCTS product;
    /* Command */
    OPERATIONS Command;
    /* Mandatory parameters */
    std::string InFile;
    std::string OutFile;
    std::string SignKeyFile; /* reused for IV file */
    std::string EncrKeyFile;
    std::string PrvKeyFile;
    std::string PubKeyFile;
    /* scrambleaspk command parameters */
    KEYFORMAT ModKeyFormat;
    std::string Modulus;
    std::string Seed;
    std::string Aspk;
    uint64_t U64Address; /* Address of image for hyd encryption command */
    ENDIAN endian;
    /* type of key for key conversion to DFU key */
    KEYTYPES KeyType;
    /* key size and public exponent for RSA key generation */
    int KeySize;
    int PubExp;
    /* Optional parameters */
    /* copyexec flag, specific to sign command */
    bool CopyExec;
    /* global -address <addr> */
    bool HasAddress;             /* Flag indicate -address was specified */
    unsigned int Address;       /* Address of store header */
    /* global -useimageheader flag*/
    bool UseImageHeader;
} static CmdLineParams =
{
    /* CmdLineParams initial values */
    PR_UE,
    OP_SIGN,        /* command */
    "", "", "", "", "", "", /* Filenames */
    KF_TEXT, "","","",      /* scrambleaspk command parameters */
    0,              /* Address (hyd encryption) */
    EN_DEFAULT,     /* endian */
    KY_PRV,         /* key type */
    0,0,            /* key size and public exponent */
    false,          /* CopyExec flag */
    false, 0        /* HasAddress, Address */
};
bool gXuvBe = true; /* XUV file is UINT16 big endian */


/******************************************************************************
@brief Append OpenSSL error string to string.

@param[in] aStr     String to receive error string.

@return String with OpenSSL error appended.
*/
std::string AppendOsslError(std::string aStr)
{
    FUNCTION_DEBUG_SENTRY_RET(std::string, aStr);
    unsigned long err = ERR_get_error();
    if(err)
    {
        char buf[2048] = "";
        ERR_error_string_n(err, buf, sizeof(buf));
        if(strlen(buf))
        {
            aStr.append(" (");
            aStr.append(buf);
            aStr.append(")");
        }
    }
    return aStr;
}

/******************************************************************************
@brief Define command line syntax.

@param[in] aCmdLine The command line object to use
@param[in] aProduct A PRODUCTS value or -1 for unspecified
*/
void DefineCmdLine(CCmdLine& aCmdLine, int aProduct)
{
    FUNCTION_DEBUG_SENTRY;

    /* Tailor the help based on product */
    istring UeOnly("(Product UE only) ");
    istring HydOnly("(Product CDA only) ");
    if(PR_HYD == aProduct)
    {
        HydOnly = "";
    }
    else if(PR_UE == aProduct)
    {
        UeOnly = "";
    }

    /* -product option */
    aCmdLine.SetExpectedParam(OPT_PRODUCT, "Selects algorithms specific to product range", NOT_MANDATORY, NOT_HIDDEN);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "product",
        "Specify the product range. One of UE or CDA",
        MANDATORY);

    if(PR_HYD == aProduct || aProduct < 0)
    {
        /* USB Debug unlock key generate */
        aCmdLine.SetExpectedParam(CMD_KEYGEN_USBDBG, HydOnly + "Generate unlock key for USB Debug", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input key file", "The filename of the input key file", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output key file", "The filename of the output key file", MANDATORY);
    }

    if (PR_HYD == aProduct || aProduct < 0)
    {
        /* key bundle generate */
        aCmdLine.SetExpectedParam(CMD_WRAP_KEY, HydOnly + "Wrap key into a bundle suitable for secure provisioning", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input key file", "The filename of the input key file", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output key bundle file", "The filename of the output key bundle file", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input QCOM KeK file", "Filename of the Qualcomm-supplied input values (KeK/EncKeK/QcomIv)", MANDATORY);
    }

    if (PR_HYD == aProduct || aProduct < 0)
    {
        /* wrapped key bundle generate */
        aCmdLine.SetExpectedParam(CMD_WRAP_KEY_AR, HydOnly + "Wrap key into a bundle suitable for Anti-Replay provisioning", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input key file", "The filename of the input key file", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output key bundle file", "The filename of the output key bundle file", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input QCOM Kek/PaKeK file", "Filename of the Qualcomm-supplied input values (KeK/EncKeK/QcomIv/PaKeK/EncPaKeK)", MANDATORY);
    }

    /* hash command */
    aCmdLine.SetExpectedParam(CMD_HASH, "Hash an XUV image", NOT_MANDATORY, NOT_HIDDEN);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input XUV file", "The filename of the input XUV image", MANDATORY);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output XUV file", "The filename of the output XUV image", MANDATORY);

    if(PR_HYD == aProduct || aProduct < 0)
    {
        /* create CBC-MAC command */
        aCmdLine.SetExpectedParam(CMD_CBCMAC, HydOnly + "Create cipher block chaining message authentication code (CBC-MAC) of an XUV image", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input XUV file", "The filename of the input XUV image", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output XUV file", "The filename of the output XUV image", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "authentication key file", "The filename of the key file", MANDATORY);
    }

    /* sign command */
    aCmdLine.SetExpectedParam(CMD_SIGN, "Sign an XUV image", NOT_MANDATORY, NOT_HIDDEN);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input XUV file", "The filename of the input XUV image", MANDATORY);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output XUV file", "The filename of the output XUV image", MANDATORY);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "sign private key file", "The filename of the signing private key file", MANDATORY);
    if(PR_UE == aProduct || aProduct < 0)
    {
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "flags", UeOnly + "One of \"" OPT_COPYEXEC "\": " OPT_COPYEXEC " - Set the copy-and-execute flag in the app store", NOT_MANDATORY);
    }

    /* encrypt command */
    aCmdLine.SetExpectedParam(CMD_ENCRYPT, "Encrypt an XUV image", NOT_MANDATORY, NOT_HIDDEN);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input XUV file", "The filename of the input XUV image", MANDATORY);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output XUV file", "The filename of the output XUV image", MANDATORY);
    aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "encryption key file", "The filename of the encryption key file", MANDATORY);
    if(PR_HYD == aProduct || aProduct < 0)
    {
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "nonce file", HydOnly + "The filename of the initialisation vector (nonce) key file",
            PR_HYD == aProduct ? MANDATORY: NOT_MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_UNDECORATED_HEX, "address",
            HydOnly + "Specify the address (in hexadecimal) of the image within the flash device",
            PR_HYD == aProduct ? MANDATORY: NOT_MANDATORY);
    }

    if(PR_UE == aProduct || aProduct < 0)
    {
        /* signencrypt command */
        aCmdLine.SetExpectedParam(CMD_SIGNENCRYPT, UeOnly + "Sign and encrypt an XUV image", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input XUV file", "The filename of the input XUV image", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output XUV file", "The filename of the output XUV image", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "sign private key file", "The filename of the signing private key file", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "encryption key file", "The filename of the encryption key file", MANDATORY);
    }

    if(PR_HYD == aProduct || aProduct < 0)
    {
        /* scrambleaspk command */
        aCmdLine.SetExpectedParam(CMD_SCRAMBLEASPK, HydOnly + "Calculate a scrambled ASPK. Output is unsigned integer in hexadecimal", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "modulus", "The modulus (unsigned integer in hexadecimal)", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "seed", "The seed value (unsigned integer in hexadecimal)", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "aspk", "The Anti-Spoofing Private key (unsigned integer in base64 per RFC4648)", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output file", "The filename of the output file. Optional", NOT_MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, OPT_PEM "|" OPT_DFU,
            "The modulus argument is a path to a PEM or DFU public key file respectively. Optional", NOT_MANDATORY);

        /* pemtodfukey command */
        aCmdLine.SetExpectedParam(CMD_PEMTODFUKEY, HydOnly + "Convert an OpenSSL RSA key to DFU key format", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "key type",
            "Specify private or public key type. One of prv or pub", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "input key file", "The filename of the private/public key file in OpenSSL RSA PEM format", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "output key file", "The filename of the private/public key file in DFU key format", MANDATORY);
        aCmdLine.SetExpectedParamSynonym("pemtodfukey");

        /* Create RSA Private/Public keys*/
        aCmdLine.SetExpectedParam(CMD_KEYGEN_RSA, HydOnly + "Create an OpenSSL RSA private/public key", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, "size", "Size of key in bits. 1024 or 2048. Must match firmware support", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "exponent", "The public exponent. 3 or F4 (65537). Must match firmware support", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "private key file", "The filename of the private key", MANDATORY);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "public key file", "The filename of the public key. Optional", NOT_MANDATORY);
    }

    if(PR_UE == aProduct || aProduct < 0)
    {
        /* -address option */
        aCmdLine.SetExpectedParam(OPT_ADDRSTART, UeOnly + "Specify the address of the app store", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_UNDECORATED_HEX, "address",
            "Specify the address (in hexadecimal) of the store header of the app store",
            MANDATORY);

        /* -useimageheader option */
        aCmdLine.SetExpectedParam(OPT_USEIMGHDR, UeOnly + "Use the image header fields during"
            " hash calculation. Without this option the app id and store length field"
            " will be assumed to be zero for the hash calculation", NOT_MANDATORY, NOT_HIDDEN);
    }

    if(PR_HYD == aProduct || aProduct < 0)
    {
        /* -endian option */
        aCmdLine.SetExpectedParam(OPT_ENDIAN, HydOnly + "Specify the endian interpretation"
            " of XUV images. The default is big endian but may be modified using"
            " the " ENV_SECURITYCMD_XUVE " environment variable. " ENV_SECURITYCMD_XUVE "=L"
            " will set the default to little endian", NOT_MANDATORY, NOT_HIDDEN);
        aCmdLine.AddExpectedValue(DATA_TYPE_STRING, "endian", "One of B, BIG, L or LITTLE", MANDATORY);
    }

    /* Group commands. Order matches order in help text. */
    aCmdLine.CreateList(LIST_UNIQUE, OPERATION_LIST);
    if(PR_HYD == aProduct || aProduct < 0)
    {
        aCmdLine.AddToList(OPERATION_LIST, CMD_CBCMAC);
        aCmdLine.AddToList(OPERATION_LIST, CMD_KEYGEN_RSA);
        aCmdLine.AddToList(OPERATION_LIST, CMD_KEYGEN_USBDBG);
        aCmdLine.AddToList(OPERATION_LIST, CMD_WRAP_KEY);
        aCmdLine.AddToList(OPERATION_LIST, CMD_WRAP_KEY_AR);
    }
    aCmdLine.AddToList(OPERATION_LIST, CMD_ENCRYPT);
    aCmdLine.AddToList(OPERATION_LIST, CMD_HASH);
    if(PR_HYD == aProduct || aProduct < 0)
    {
        aCmdLine.AddToList(OPERATION_LIST, CMD_PEMTODFUKEY);
        aCmdLine.AddToList(OPERATION_LIST, CMD_SCRAMBLEASPK);
    }
    aCmdLine.AddToList(OPERATION_LIST, CMD_SIGN);
    if(PR_UE == aProduct || aProduct < 0)
    {
        aCmdLine.AddToList(OPERATION_LIST, CMD_SIGNENCRYPT);
    }

}

/******************************************************************************
@brief Process the command line
*/

bool CheckCmdLine(CCmdLine &aCmdLine)
{
    bool failure = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, failure);
    std::string product;
    std::string keytype;
    std::string opstring = aCmdLine.GetParamForList(OPERATION_LIST);
    const char *pOp = opstring.c_str();
#if 0
    printf("pOp:%s\n", pOp);
#endif

    /* Get the product */
    GetParameterResultEnum res = aCmdLine.GetParameterValue(OPT_PRODUCT, 1, product);
    if (res == GET_PARAMETER_SUCCESS)
    {
        if(0 == STRICMP(product.c_str(), "UE"))
        {
            CmdLineParams.product = PR_UE;
        }
        else if((0 == STRICMP(product.c_str(), "CDA")) || (0 == STRICMP(product.c_str(), "HYD")))
        {
            CmdLineParams.product = PR_HYD;
        }
        else
        {
            aCmdLine.OutputErrorMessage("Invalid product argument.");
            aCmdLine.PrintHelp();
            failure = true;
        }
    }
    else
    {   /* Default if not specified */
        CmdLineParams.product = PR_UE;

        MSG_HANDLER.NotifyStatus(STATUS_WARNING, OPT_PRODUCT " not specified, so defaulting to UE; please specify it in future.");
    }

    int paramidx = 1;
    /* neither CMD_PEMTODFUKEY nor CMD_KEYGEN_RSA nor CMD_SCRAMBLEASPK */
    if(STRICMP(pOp, CMD_PEMTODFUKEY) && STRICMP(pOp, CMD_KEYGEN_RSA) && STRICMP(pOp, CMD_SCRAMBLEASPK))
    {
        /* Get the input XUV filename */
        res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.InFile);
        if (res != GET_PARAMETER_SUCCESS)
        {
            aCmdLine.OutputErrorMessage("input file has not been supplied.");
            aCmdLine.PrintHelp();
            failure = true;
        }
        ++paramidx;
        /* Get the output XUV filename */
        res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.OutFile);
        if (res != GET_PARAMETER_SUCCESS)
        {
            aCmdLine.OutputErrorMessage("output file has not been supplied.");
            aCmdLine.PrintHelp();
            failure = true;
        }
        ++paramidx;
    }
    if(!failure)
    {
        if (0 == STRICMP(pOp, CMD_KEYGEN_USBDBG))
        {
            CmdLineParams.Command = OP_KEYGEN_UNLOCK;
        }
        else if (0 == STRICMP(pOp, CMD_WRAP_KEY))
        {
            CmdLineParams.Command = OP_WRAP_KEY;
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.SignKeyFile); // get KeK/encKeK/QcomIv file
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("KeK/encKeK/QcomIv file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
        }
        else if (0 == STRICMP(pOp, CMD_WRAP_KEY_AR))
        {
            CmdLineParams.Command = OP_WRAP_KEY_AR;
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.SignKeyFile); // get KeK/encKeK/QcomIv/PaKeK/encPaKeK file
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("KeK/encKeK/QcomIv/PaKeK/encPaKeK file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
        }
        else if(0 == STRICMP(pOp, CMD_HASH))
        {
            CmdLineParams.Command = OP_HASH;
        }
        else if(0 == STRICMP(pOp, CMD_SIGN))
        {
            CmdLineParams.Command = OP_SIGN;
            /* Get the signing key filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.SignKeyFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("sign private key file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            if(PR_UE == CmdLineParams.product)
            {
                std::string flag;
                /* Has the OPT_COPYEXEC option been specified? */
                GetParameterResultEnum cex = aCmdLine.GetParameterValue(CMD_SIGN, paramidx, flag);
                if(cex == GET_PARAMETER_SUCCESS)
                {
                    if(PR_UE == CmdLineParams.product && 0 == STRICMP(flag.c_str(), OPT_COPYEXEC))
                    {
                        CmdLineParams.CopyExec = true;
                    }
                    else
                    {
                        aCmdLine.OutputErrorMessage("invalid flag specified \"" + flag + "\"");
                        aCmdLine.PrintHelp();
                        failure = true;
                    }
                }
                ++paramidx;
            }
        }
        else if(0 == STRICMP(pOp, CMD_ENCRYPT))
        {
            CmdLineParams.Command = OP_ENCRYPT;
            /* Get the encryption filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.EncrKeyFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("encryption key file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            if(PR_HYD == CmdLineParams.product)
            {
                /* Get the IV (nonce) filename */
                res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.SignKeyFile);
                if (res != GET_PARAMETER_SUCCESS)
                {
                    aCmdLine.OutputErrorMessage("nonce file has not been supplied.");
                    aCmdLine.PrintHelp();
                    failure = true;
                }
                ++paramidx;
                /* Get address parameter */
                uint64_t val;
                res = aCmdLine.GetParameterValueAsU64(pOp, paramidx, val);
                if (res == GET_PARAMETER_SUCCESS)
                {
                    CmdLineParams.U64Address = val;
                }
                else
                {
                    aCmdLine.OutputErrorMessage("Invalid address argument.");
                    aCmdLine.PrintHelp();
                    failure = true;
                }
                ++paramidx;
            }
        }
        else if(0 == STRICMP(pOp, CMD_CBCMAC))
        {
            CmdLineParams.Command = OP_CBCMAC;
            /* Get the encryption filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.EncrKeyFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("key file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
        }
        else if(0 == STRICMP(pOp, CMD_SIGNENCRYPT))
        {
            CmdLineParams.Command = OP_SIGNENCRYPT;
            /* Get the signing key filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.SignKeyFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("sign private key file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            /* Get the encryption filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.EncrKeyFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("encryption key file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
        }
        else if(PR_HYD == CmdLineParams.product && 0 == STRICMP(pOp, CMD_SCRAMBLEASPK))
        {
            CmdLineParams.Command = OP_SCRAMBLEASPK;
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.Modulus);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("modulus has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.Seed);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("seed has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.Aspk);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("aspk has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            int args = 0;
            std::string arg1;
            std::string arg2;
            std::string format;
            /* Need to handle empty string parameters too */
            res = aCmdLine.GetParameterValue(pOp, paramidx, arg1);
            if (res != GET_PARAMETER_PARAM_NOT_GIVEN)
            {
                ++args;
                ++paramidx;
                res = aCmdLine.GetParameterValue(pOp, paramidx, arg2);
                if (res != GET_PARAMETER_PARAM_NOT_GIVEN)
                {
                    ++args;
                    ++paramidx;
                }
            }
            /* since we can't distinguish between non-exist arguments and empty strings we now
            need to drop them off the end if they're empty. */
            if(args >= 2 && arg2.empty())
            {
                args--;
                if(arg1.empty())
                {
                    args--;
                }
            }
            if(args > 0)
            {
                bool hasFormat = false;
                switch(args)
                {
                case 1:
                    if(arg1[0] == '-')
                    {
                        format = arg1;
                        hasFormat = true;
                    }
                    else
                    {
                        CmdLineParams.OutFile = arg1;
                    }
                    break;
                case 2:
                default:
                    CmdLineParams.OutFile = arg1;
                    format = arg2;
                    hasFormat = true;
                    if(arg1[0] == '-')
                    {
                        if(arg2[0] == '-')
                        {
                            aCmdLine.OutputErrorMessage("two flags specified.");
                            aCmdLine.PrintHelp();
                            failure = true;
                        }
                        else
                        {
                            CmdLineParams.OutFile = arg2;
                            format = arg1;
                        }
                    }
                    break;
                }
                if(!failure && hasFormat)
                {
                    /* check format flag is valid */
                    const char *pOpt = format.c_str();
                    if(STRICMP(OPT_PEM, pOpt) == 0)
                    {
                        CmdLineParams.ModKeyFormat = KF_PEM;
                    }
                    else if(STRICMP(OPT_DFU, pOpt) == 0)
                    {
                        CmdLineParams.ModKeyFormat = KF_DFU;
                    }
                    else
                    {
                        aCmdLine.OutputErrorMessage("invalid flag \"" + format + "\".");
                        aCmdLine.PrintHelp();
                        failure = true;
                    }
                }
            }
        }
        else if(PR_HYD == CmdLineParams.product && 0 == STRICMP(pOp, CMD_PEMTODFUKEY))
        {
            CmdLineParams.Command = OP_PEMTODFUKEY;
            /* Get the key conversion type */
            GetParameterResultEnum res = aCmdLine.GetParameterValue(CMD_PEMTODFUKEY, paramidx, keytype);
            if (res == GET_PARAMETER_SUCCESS)
            {
                if(0 == STRICMP(keytype.c_str(), "PRV"))
                {
                    CmdLineParams.KeyType = KY_PRV;
                }
                else if(0 == STRICMP(keytype.c_str(), "PUB"))
                {
                    CmdLineParams.KeyType = KY_PUB;
                }
                else
                {
                    aCmdLine.OutputErrorMessage("Invalid key type argument \"" + keytype + "\". Should be one of prv or pub.");
                    aCmdLine.PrintHelp();
                    failure = true;
                }
            }
            else
            {   /* Default if not specified */
                CmdLineParams.KeyType = KY_PRV;
            }
            ++paramidx;
            /* Get the input key filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.InFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("input file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            /* Get the output key filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.OutFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("output file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
        }
        else if(PR_HYD == CmdLineParams.product && 0 == STRICMP(pOp, CMD_KEYGEN_RSA))
        {
            CmdLineParams.Command = OP_KEYGEN_RSA;
            /* Get the key size type */
            GetParameterResultEnum res = aCmdLine.GetParameterValueAsInteger(CMD_KEYGEN_RSA, paramidx, CmdLineParams.KeySize);
            if (res != GET_PARAMETER_SUCCESS || (CmdLineParams.KeySize != 1024 && CmdLineParams.KeySize != 2048))
            {
                aCmdLine.OutputErrorMessage("Invalid key size argument. Should be one of 1024 or 2048.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            /* Get the key size type */
            std::string pubexp;
            CmdLineParams.PubExp = 0;
            res = aCmdLine.GetParameterValue(CMD_KEYGEN_RSA, paramidx, pubexp);
            if(res == GET_PARAMETER_SUCCESS)
            {
                if(0 == STRICMP(pubexp.c_str(), "3"))
                {   /* Fermat prime F0 = 3 */
                    CmdLineParams.PubExp = 3;
                }
                else if(0 == STRICMP(pubexp.c_str(), "F4"))
                {   /* Fermat prime F4 = 65537 */
                    CmdLineParams.PubExp = 65537;
                }
            }
            if(!CmdLineParams.PubExp)
            {
                aCmdLine.OutputErrorMessage("Invalid public exponent argument. Should be one of 3 or F4.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            /* Get the private key filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.PrvKeyFile);
            if (res != GET_PARAMETER_SUCCESS)
            {
                aCmdLine.OutputErrorMessage("private key file has not been supplied.");
                aCmdLine.PrintHelp();
                failure = true;
            }
            ++paramidx;
            /* Get the optional public key filename */
            res = aCmdLine.GetParameterValue(pOp, paramidx, CmdLineParams.PubKeyFile);
            ++paramidx;
        }
        else
        {
            aCmdLine.OutputErrorMessage("Command not available for this product.");
            aCmdLine.PrintHelp();
            failure = true;
        }
        /* Get address parameter */
        if(PR_UE == CmdLineParams.product)
        {
            int val;
            GetParameterResultEnum res = aCmdLine.GetParameterValueAsInteger(OPT_ADDRSTART, 1, val);
            if (res == GET_PARAMETER_SUCCESS)
            {
                if (OP_HASH == CmdLineParams.Command || OP_SIGN == CmdLineParams.Command ||
                    OP_ENCRYPT == CmdLineParams.Command || OP_SIGNENCRYPT == CmdLineParams.Command)
                {
                    CmdLineParams.HasAddress = true;
                    CmdLineParams.Address = val;
                }
                else
                {
                    aCmdLine.OutputErrorMessage("\"" OPT_ADDRSTART "\" not available with this command and/or this product.");
                    aCmdLine.PrintHelp();
                    failure = true;
                }
            }
            if (aCmdLine.GetFlagParameterValue(OPT_USEIMGHDR))
            {
                if (OP_HASH == CmdLineParams.Command || OP_SIGN == CmdLineParams.Command ||
                    OP_ENCRYPT == CmdLineParams.Command || OP_SIGNENCRYPT == CmdLineParams.Command)
                {
                    CmdLineParams.UseImageHeader = true;
                }
                else
                {
                    aCmdLine.OutputErrorMessage("\"" OPT_USEIMGHDR "\" not available with this command and/or this product.");
                    aCmdLine.PrintHelp();
                    failure = true;
                }
            }
        }
        if(PR_HYD == CmdLineParams.product)
        {
            std::string endian;
            GetParameterResultEnum res = aCmdLine.GetParameterValue(OPT_ENDIAN, 1, endian);
            if (res == GET_PARAMETER_SUCCESS)
            {
                if(0 == STRICMP(endian.c_str(), "L") || 0 == STRICMP(endian.c_str(), "LITTLE"))
                {
                    CmdLineParams.endian = EN_U16LE;
                }
                else if(0 == STRICMP(endian.c_str(), "B") || 0 == STRICMP(endian.c_str(), "BIG"))
                {
                    CmdLineParams.endian = EN_U16BE;
                }
                else
                {
                    aCmdLine.OutputErrorMessage("Invalid endian argument.");
                    aCmdLine.PrintHelp();
                    failure = true;
                }
            }
        }
    }
#if 0
    printf("Command %d\n", CmdLineParams.Command);
    printf("InFile %s\n", CmdLineParams.InFile.c_str());
    printf("OutFile %s\n", CmdLineParams.OutFile.c_str());
    printf("SignKeyFile %s\n", CmdLineParams.SignKeyFile.c_str());
    printf("EncrKeyFile %s\n", CmdLineParams.EncrKeyFile.c_str());
    printf("CopyExec %d\n", CmdLineParams.CopyExec);
    printf("HasAddress %d\n", CmdLineParams.HasAddress);
    printf("Address %08X\n", CmdLineParams.Address);
#endif
    return failure;
}

/******************************************************************************
@brief Preprocess command line for optional product flag stealthly.

@param[in] argc Number of arguments.
@param[in] argv Array of arguments including program name at [0].

@return Product enumeration value.
@retval PRODUCTS enum value.
@retval -1  Product flag was not specified.
 */
int PreprocessCmdLineForProduct(const int argc, const char *const argv[])
{
    int res = -1;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    int i;

    for (i = 1; i < argc; i++)
    {
        if(STRICMP(argv[i], OPT_PRODUCT) == 0)
        {
            i++;
            if(i < argc)
            {
                if(0 == STRICMP(argv[i], "UE"))
                {
                    res = PR_UE;
                }
                else if ((0 == STRICMP(argv[i], "CDA")) || (0 == STRICMP(argv[i], "HYD")))
                {
                    res = PR_HYD;
                }
            }
        }
    }
    return res;
}

/******************************************************************************
@brief Process the command line

@param[in] aCmdLine     A CCmdLine object.
@param[in] aProduct     A PRODUCTS enum value or -1 for unspecified.
*/
bool ProcessCmdLine(CCmdLine &aCmdLine, int aProduct)
{
    bool failure = true;
    FUNCTION_DEBUG_SENTRY_RET(bool, failure);

    DefineCmdLine(aCmdLine, aProduct);
    if (PARSE_SUCCESS == aCmdLine.Parse())
    {
        failure = CheckCmdLine(aCmdLine);
    }
    return failure;
}

/******************************************************************************
@brief Calculate hash using SHA256 on the XUV image outputting hash into an XUV image.

@param[out] aXuvImageHash   Output XUV image containing generated hash.
@param[in]  aXuvImageIn     Input XUV image.
@param[in]  aAddrFirst      Address of first word to be hashed.
@param[in]  aAddrLast       Address of last word to be hashed.

@note
The image is expected to be a contiguous block. If not the gaps will default to
0xFF.
*/
void
XuvImageHashSha256(xuv::image &aXuvImageHash, const xuv::image &aXuvImageIn,
    unsigned int aAddrFirst, unsigned int aAddrLast)
{
    FUNCTION_DEBUG_SENTRY;
    securlib::HashType hash;
    xuv::ByteBlockType block;

    /* copy app store into vector from XUV image (contiguous bytes, BIG endian) */
    block = ByteBlockFlattenU16(aXuvImageIn, aAddrFirst, aAddrLast, 0xFF, gXuvBe);
    (void) securlib::HashSha256(&hash, &block[0], block.size());
    /* Push the hash into the XUV image. */
    xuv::ByteBlockIncorporateU16(aXuvImageHash, 0, &hash.digest[0], &hash.digest[SHA256_DIGEST_LENGTH-1], gXuvBe);
}


/******************************************************************************
@brief Calculate hash using SHA256 on the XUV image outputting hash into an XUV image.
This function works on files.

@param[in]  apInFile    Input XUV image.
@param[in]  apOutFile   Output XUV image containing hash.
*/
enum {FXUVIMGHASH_SUCCESS, FXUVIMGHASH_ERR_READ_IMG, FXUVIMGHASH_ERR_IMG_EMPTY, FXUVIMGHASH_ERR_WRITE_IMG};
int FXuvImageHashSha256(const char *apInFile, const char *apOutFile)
{
    int res = FXUVIMGHASH_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    /* Read XUV file. */
    xuv::image image;
    if(xuv::READ_OK != xuv::Read(image, apInFile))
    {
        res = FXUVIMGHASH_ERR_READ_IMG;
    }
    else
    {
        if(image.data.empty())
        {
            res = FXUVIMGHASH_ERR_IMG_EMPTY;
        }
        else
        {
            /* Get address of first and last word */
            unsigned int AddrFirst = image.data.begin()->first;
            unsigned int AddrLast = image.data.rbegin()->first;
#if 0
            printf("AddrFirst %06X\n", AddrFirst);
            printf("AddrLast %06X\n", AddrLast);
#endif

            /* Calculate the hash */
            xuv::image hash;
            XuvImageHashSha256(hash, image, AddrFirst, AddrLast);

            /* Write the output XUV image. */
            if(xuv::WRITE_OK != xuv::Write(hash, apOutFile))
            {
                res = FXUVIMGHASH_ERR_WRITE_IMG;
            }
        }
    }
    return res;
}

/******************************************************************************
@brief Sign a message digest (hash) using RSA with PSS padding.

@param[out] aXuvSignature   Output XUV image containing generated signature.
@param[in]  aXuvImageIn     Input XUV image containing digest.
@param[in]  pKey            Private key.
@param[in]  aAddrFirst      Address of first word to be signed.
@param[in]  aAddrLast       Address of last word to be signed.

@note
The image is expected to be a contiguous block. If not the gaps will default to
0xFF.

If the signing fails the output image will be empty.
*/
void XuvImageSignRsaPss(xuv::image &aXuvSignature, const xuv::image &aXuvImageIn,
    EVP_PKEY *pKey, unsigned int aAddrFirst, unsigned int aAddrLast)
{
    FUNCTION_DEBUG_SENTRY;
    const size_t siglen = 1024;
    size_t rsiglen = siglen;
    unsigned char sig[siglen];
    /* copy image into vector from XUV image (contiguous bytes, BIG endian) */
    xuv::ByteBlockType block = ByteBlockFlattenU16(aXuvImageIn, aAddrFirst, aAddrLast, 0xFF, gXuvBe);
    if(OsslRsaPssSign(pKey, &block[0], block.size(), sig, &rsiglen) > 0)
    {
        if(rsiglen <= siglen)
        {
            /* Push the signature into the XUV image. */
            xuv::ByteBlockIncorporateU16(aXuvSignature, 0, &sig[0], &sig[rsiglen-1], gXuvBe);
        }
    }
}

/******************************************************************************
@brief Sign a message digest (hash) using RSA with PSS padding.
*/
enum {FXUVIMGSIGN_SUCCESS = 0, FXUVIMGSIGN_ERR_READ_KEY, FXUVIMGSIGN_ERR_READ_IMG, FXUVIMGSIGN_ERR_IMG_EMPTY, FXUVIMGSIGN_ERR_WRITE_IMG, FXUVIMGSIGN_ERR_SIGN_IMG};
int FXuvImageSignRsaPss(const char *apKeyFile, const char *apInFile, const char *apOutFile)
{
    int res = FXUVIMGSIGN_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    xuv::image image;
    xuv::image signature;

    /* Read the private key */
    EVP_PKEY *pKey = OsslReadPrvKeyFile(apKeyFile);
    if(!pKey)
    {
        res = FXUVIMGSIGN_ERR_READ_KEY;
    }

    /* Read XUV file. */
    if(!res && xuv::READ_OK != xuv::Read(image, apInFile))
    {
        res = FXUVIMGHASH_ERR_READ_IMG;
    }
    if(!res && !image.data.empty())
    {
        /* Get address of first and last word. Having variables here make it easier to debug. */
        unsigned int AddrFirst = image.data.begin()->first;
        unsigned int AddrLast = image.data.rbegin()->first;
        /* Calculate the signature */
        XuvImageSignRsaPss(signature, image, pKey, AddrFirst, AddrLast);
        if(signature.data.empty())
        {
            res = FXUVIMGSIGN_ERR_SIGN_IMG;
        }
    }
    else
    {
        res = FXUVIMGHASH_ERR_IMG_EMPTY;
    }

    /* Write the output XUV image. */
    if(!res && xuv::WRITE_OK != xuv::Write(signature, apOutFile))
    {
        res = FXUVIMGHASH_ERR_WRITE_IMG;
    }

    if(pKey)
    {
        EVP_PKEY_free(pKey);
    }
    return res;
}

/******************************************************************************
@brief Create CBC-MAC with given algorithm.

@param[in]  aAlgorithm      Algorithm (EVP_CIPHER). Shall be in CBC mode.
@param[out] aXuvCbcMac      Output XUV image containing generated CBC-MAC.
@param[in]  aXuvImageIn     Input XUV image containing digest.
@param[in]  apKey           Key.
@param[in]  aAddrFirst      Address of first word to be signed.
@param[in]  aAddrLast       Address of last word to be signed.
@param[in]  aPadding        Value to pass to EVP_CIPHER_CTX_set_padding().

@note
The image is expected to be a contiguous block. If not the gaps will default to
0xFF.

If the signing fails the output image will be empty.
*/
void XuvImageCreateCbcMac(const EVP_CIPHER *aAlgorithm, xuv::image &aXuvCbcMac,
    const xuv::image &aXuvImageIn, const unsigned char *apKey,
    unsigned int aAddrFirst, unsigned int aAddrLast, int aPadding)
{
    FUNCTION_DEBUG_SENTRY;
    const int KEYSIZE = EVP_CIPHER_key_length(aAlgorithm);
    unsigned char CbcMac[EVP_MAX_KEY_LENGTH];

    /* copy image into vector from XUV image (contiguous bytes, BIG endian) */
    if(KEYSIZE <= EVP_MAX_KEY_LENGTH)
    {
        xuv::ByteBlockType block = ByteBlockFlattenU16(aXuvImageIn, aAddrFirst, aAddrLast, 0xFF, gXuvBe);
        if(OsslCbcMac(aAlgorithm, CbcMac, &block[0], block.size(), apKey, aPadding) > 0)
        {
            /* Push the CBC-MAC into the XUV image. */
            xuv::ByteBlockIncorporateU16(aXuvCbcMac, 0, &CbcMac[0], &CbcMac[KEYSIZE-1], gXuvBe);
        }
    }
}

/******************************************************************************
@brief Create CBC-MAC with given algorithm.

@param[in]  aAlgorithm      Algorithm (EVP_CIPHER). Shall be in CBC mode.
@param[in]  apKeyFile       Key.
@param[in]  apInFile        Input XUV image.
@param[in]  apOutFile       Output XUV image containing hash.
@param[in]  aPadding        Value to pass to EVP_CIPHER_CTX_set_padding().
*/
enum {FXUVIMGCBCMAC_SUCCESS = 0, FXUVIMGCBCMAC_ERR_READ_KEY, FXUVIMGCBCMAC_ERR_READ_IMG,
    FXUVIMGCBCMAC_ERR_IMG_EMPTY, FXUVIMGCBCMAC_ERR_WRITE_IMG, FXUVIMGCBCMAC_ERR_IMG_NOT_MULTIPLE,
    FXUVIMGCBCMAC_ERR_CBCMAC_IMG};
int FXuvImageCreateCbcMac(const EVP_CIPHER *aAlgorithm, const char *apKeyFile,
    const char *apInFile, const char *apOutFile, int aPadding)
{
    int res = FXUVIMGCBCMAC_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    xuv::image image;
    xuv::image CbcMac;

    /* Read the key */
    securlib::Aes128KeyType AuthKey = {{0}};
    if(apKeyFile && strlen(apKeyFile))
    {
        if(securlib::ReadAes128KeyFile(AuthKey, apKeyFile))
        {
            res = FXUVIMGCBCMAC_ERR_READ_KEY;
        }
    }
    securlib::reverse8BitBytes(AuthKey.key, sizeof(AuthKey.key));

    /* Read XUV file. */
    if(!res && xuv::READ_OK != xuv::Read(image, apInFile))
    {
        res = FXUVIMGCBCMAC_ERR_READ_IMG;
    }
    if(!res)
    {
        if(!image.data.empty())
        {
            /* Get address of first and last word. Having variables here make it easier to debug. */
            unsigned int AddrFirst = image.data.begin()->first;
            unsigned int AddrLast = image.data.rbegin()->first;
            /* image size in bytes */
            size_t ImageSize = (AddrLast - AddrFirst + 1) * 2;
            size_t BlockSize = EVP_CIPHER_block_size(aAlgorithm);
            if((ImageSize % BlockSize) == 0)
            {
                /* Calculate the CBC-MAC */
                XuvImageCreateCbcMac(aAlgorithm, CbcMac, image, AuthKey.key, AddrFirst, AddrLast, aPadding);
                if(CbcMac.data.empty())
                {
                    res = FXUVIMGCBCMAC_ERR_CBCMAC_IMG;
                }
            }
            else
            {
                /* Image size is not a multiple of block size */
                res = FXUVIMGCBCMAC_ERR_IMG_NOT_MULTIPLE;
            }
        }
        else
        {
            res = FXUVIMGCBCMAC_ERR_IMG_EMPTY;
        }
    }

    /* Write the output XUV image. */
    if(!res && xuv::WRITE_OK != xuv::Write(CbcMac, apOutFile))
    {
        res = FXUVIMGCBCMAC_ERR_WRITE_IMG;
    }

    return res;
}

/******************************************************************************
@brief Encrypt XUV image with given AES-128 with custom CTR mode.

@param[out] aXuvImageOut    Encrypted output XUV image.
@param[in]  aXuvImageIn     Input XUV image containing digest.
@param[in]  apKey           Key for encryption. Shall be EVP_CIPHER_key_length(aType).
@param[in]  apIv            Initialisation vector. Shall be EVP_CIPHER_iv_length(aType).
@param[in]  aAddrFirst      Address of first word to be signed.
@param[in]  aAddrLast       Address of last word to be signed.
@param[in]  aAddress        Address in the flash device.

@return Success or error code.
@retval 1   Success.
@retval 0   Failure.

@note
See https://www.openssl.org/docs/man1.0.2/crypto/ to understand specific error
code for each function.

@note
The output image will be upto a block larger than input image.

The image is expected to be a contiguous block. If not the gaps will default to
0xFF.

If the encryption fails the output image will be empty.
*/
int XuvImageEncryptAes128Cctr(xuv::image &aXuvImageOut, const xuv::image &aXuvImageIn,
    const unsigned char *apKey, const unsigned char *apIv,
    unsigned int aAddrFirst, unsigned int aAddrLast, unsigned int aAddress)
{
    int res = 0; /* default to failure (to match OpenSSL error for these functions) */
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    size_t size;
    /* copy image into vector from XUV image (contiguous bytes, BIG endian) */
    xuv::ByteBlockType block = ByteBlockFlattenU16(aXuvImageIn, aAddrFirst, aAddrLast, 0xFF, gXuvBe);
    /* Create output buffer to receive encrypted image. */
    size = block.size() + EVP_MAX_BLOCK_LENGTH;
    xuv::ByteBlockType DataOut(size);
    /* Encrypt the application image. */
    if((res = OsslEncryptAes128Cctr(&DataOut[0], &size, &block[0], block.size(), apKey, apIv, aAddress)) > 0)
    {
        if(!DataOut.empty())
        {
            if(size < DataOut.size())
            {
                DataOut.resize(size);
            }
            xuv::ByteBlockIncorporateU16(aXuvImageOut, aAddrFirst, &DataOut.front(), &DataOut.back(), gXuvBe);
        }
    }
    return res;
}

/******************************************************************************
@brief Encrypt XUV image with given algorithm.

@param[in]  aAlgorithm      Algorithm (EVP_CIPHER). Shall be in CBC mode.
@param[in]  apKeyFile       File name that contains the encryption key.
@param[in]  apIvFile        File name that contains the initialisation vector (nonce).
@param[in]  apInFile        Input XUV image.
@param[in]  apOutFile       Output XUV image.
@param[in]  aAddress        Address in the flash device.

@return Success or error code.
@retval FXUVIMGENC_SUCCESS          
@retval FXUVIMGENC_ERR_READ_KEY     
@retval FXUVIMGENC_ERR_READ_IV      
@retval FXUVIMGENC_ERR_READ_IMG     
@retval FXUVIMGENC_ERR_IMG_EMPTY    
@retval FXUVIMGENC_ERR_WRITE_IMG    
@retval FXUVIMGENC_ERR_ENCRYPT_IMG  
*/
enum {FXUVIMGENC_SUCCESS = 0, FXUVIMGENC_ERR_READ_KEY, FXUVIMGENC_ERR_READ_IV, FXUVIMGENC_ERR_READ_IMG,
    FXUVIMGENC_ERR_IMG_EMPTY, FXUVIMGENC_ERR_WRITE_IMG, FXUVIMGENC_ERR_ENCRYPT_IMG};
int FXuvImageEncryptAes128Cctr(const char *apKeyFile, const char *apIvFile,
    const char *apInFile, const char *apOutFile, unsigned int aAddress)
{
    int res = FXUVIMGENC_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    xuv::image image;
    xuv::image imgout;
    securlib::Aes128KeyType key = {{0}};
    securlib::Aes128KeyType iv = {{0}};

    /* Read the key */
    if(!apKeyFile || !strlen(apKeyFile) || securlib::ReadAes128KeyFile(key, apKeyFile))
    {
        res = FXUVIMGENC_ERR_READ_KEY;
    }
    securlib::reverse8BitBytes(key.key, sizeof(key.key));
    /* Read the IV */
    if(!res)
    {
        if(!apIvFile || !strlen(apIvFile) || securlib::ReadAes128KeyFile(iv, apIvFile))
        {
            res = FXUVIMGENC_ERR_READ_IV;
        }
    }

    /* Read XUV file. */
    if(!res)
    {
        if(xuv::READ_OK != xuv::Read(image, apInFile))
        {
            res = FXUVIMGENC_ERR_READ_IMG;
        }
    }
    if(!res)
    {
        if(!image.data.empty())
        {
            /* Get address of first and last word. Having variables here make it easier to debug. */
            unsigned int AddrFirst = image.data.begin()->first;
            unsigned int AddrLast = image.data.rbegin()->first;
            /* Calculate the CBC-MAC */
            if(XuvImageEncryptAes128Cctr(imgout, image, key.key, iv.key, AddrFirst, AddrLast, aAddress) != 1 || imgout.data.empty())
            {
                res = FXUVIMGENC_ERR_ENCRYPT_IMG;
            }
        }
        else
        {
            res = FXUVIMGENC_ERR_IMG_EMPTY;
        }
    }

    /* Write the output XUV image. */
    if(!res)
    {
        if(xuv::WRITE_OK != xuv::Write(imgout, apOutFile))
        {
            res = FXUVIMGENC_ERR_WRITE_IMG;
        }
    }

    return res;
}

/******************************************************************************
@brief Convert hexadecimal string to BIGNUM and report errors to user.
@param[in]  aCmdLine    The command line object to use for outputting errors.
@param[in]  apHex       Hexadecimal string containing value to convert.
@param[out] apResult    Pointer to BIGNUM to receive resulting value.

@return success
@retval true    success
@retval false   error occurred. Error message was output to user.
*/
bool HexToBnReportErr(CCmdLine &aCmdline, const char *apHex, BIGNUM *apResult)
{
    bool success = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, success);
    if(apHex && apResult)
    {
        size_t slen = strlen(apHex);
        if(slen > 0)
        {
            size_t len = static_cast<size_t>(BN_hex2bn(&apResult, apHex));
            if((success = (0 == ERR_peek_last_error())))
            {   /* Check length processed matches string length. */
                if(!(success = (len == slen)))
                {   /* Explain to user which digit was invalid */
                    std::string s = "Invalid hexadecimal digit '" + std::string(1, apHex[len]) +
                        "' in \"" + apHex + "\"";
                    aCmdline.OutputErrorMessage(s);
                }
            }
            else
            {
                aCmdline.OutputErrorMessage(AppendOsslError("BN_hex2bn error"));
            }
        }
        else
        {
            aCmdline.OutputErrorMessage("Empty hexadecimal string");
        }
    }
    return success;
}

/******************************************************************************
@brief Convert base64 string to BIGNUM and report errors to user.
@param[in]  aCmdLine    The command line object to use for outputting errors.
@param[in]  apBase64    Base64 string containing value to convert.
@param[out] apResult    Pointer to BIGNUM to receive resulting value.

@return success
@retval true    success
@retval false   error occurred. Error message was output to user.

@note
OpenSSL implementation of base64 decoding is non-compliant so validate the string beforehand.
The RFC4648 non-conformance to prevent are:
It ignores the last group when the string length is not a multiple of four.
It allows the padding character to appear anywhere in the string and isn't ignored but interpreted
as "A" digit.
*/
bool Base64ToBnReportErr(CCmdLine &aCmdline, const char *apBase64, BIGNUM *apResult)
{
    bool success = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, success);
    if(apBase64 && apResult)
    {
        size_t slen = strlen(apBase64);
        size_t partial = slen % 4;
        /* count the padding character */
        size_t padding = 0;
        for(size_t i = 0; i < slen; ++i)
        {
            if(apBase64[i] == '=')
            {
                padding++;
            }
        }

        if(slen < 4 || partial > 0)
        {
            aCmdline.OutputErrorMessage("Invalid base64 string. Padding is mandatory. Length must be a multiple of four.");
        }
        else if(padding > 2)
        {
            aCmdline.OutputErrorMessage("Excessive padding in base64 string.");
        }
        else if((padding >= 1 && apBase64[slen-1] != '=') || (padding >= 2 && apBase64[slen-2] != '='))
        {
            aCmdline.OutputErrorMessage("Invalid padding in base64 string.");
        }
        else
        {
            int dataLen =  OsslBase64ToBn(apBase64, apResult) != 0;
            success = dataLen > 0 && 0 == ERR_peek_last_error();
            if(!success)
            {
                aCmdline.OutputErrorMessage(AppendOsslError("Base64 conversion error"));
            }
        }
    }
    return success;
}

/******************************************************************************
@brief Scramble ASPK
@param[in] aCmdLine     The command line object to use for outputting errors.
@param[in] apModulus    Hexadecimal string containing modulus.
@param[in] apSeed       Hexadecimal string containing seed.
@param[in] apAspk       Base64 string containing aspk.

@return success
@retval true    success
@retval false   error occurred. Error message was output to user.
*/
static bool ScrambleAspk(CCmdLine &aCmdline, const char *apModulus, const char *apSeed, const char *apAspk,
    KEYFORMAT aModKeyFormat, const char *apOutFile)
{
    bool good = true;
    FUNCTION_DEBUG_SENTRY_RET(bool, good);
    BIGNUM *pBnModulus = BN_new();
    BIGNUM *pBnSeed = BN_new();
    BIGNUM *pBnAspk = BN_new();
    good &= pBnModulus && pBnSeed && pBnAspk;
    /* The intention is to validate all three modulus, seed and aspk reporting errors before exiting
    due to any error. */
    good &= HexToBnReportErr(aCmdline, apSeed, pBnSeed);
    good &= Base64ToBnReportErr(aCmdline, apAspk, pBnAspk);
    switch(aModKeyFormat)
    {
    case KF_PEM:
        if(ExtractAspkModPubKey(apModulus, pBnModulus) <= 0)
        {
            good = false;
            aCmdline.OutputErrorMessage(AppendOsslError("Failed to read public key"));
        }
        break;
    case KF_DFU:
        if(ExtractAspkModDfuKey(apModulus, pBnModulus) <= 0)
        {
            good = false;
            aCmdline.OutputErrorMessage(AppendOsslError("Failed to read dfu key"));
        }
        break;
    default:
    case KF_TEXT:
        good &= HexToBnReportErr(aCmdline, apModulus, pBnModulus);
        break;
    }
    if (good)
    {
        if(apOutFile == NULL || apOutFile[0] == '\0')
        {
            good = OsslPrintScrAspk(stdout, pBnModulus, pBnSeed, pBnAspk) > 0;
            puts("");
        }
        else
        {
            FILE *pFile;
            if((pFile = fopen(apOutFile, "wt")))
            {
                good = OsslPrintScrAspk(pFile, pBnModulus, pBnSeed, pBnAspk) > 0;
                fclose(pFile);
            }
            else
            {
                good = false;
                aCmdline.OutputErrorMessage("Failed to open output file");
            }
        }
    }
    if (pBnAspk)
    {
        BN_free(pBnAspk);
    }
    if (pBnSeed)
    {
        BN_free(pBnSeed);
    }
    if (pBnModulus)
    {
        BN_free(pBnModulus);
    }
    return good;
}

/******************************************************************************
@brief Convert OpenSSL RSA Private/Public key to DFU key format.

@param[in] aPrv     Zero for public key otherwise private key.
@param[in] apPemKey Filename of OpenSSL RSA Key in PEM format.
@param[in] apDfuKey Filename of DFU key output file.

@return Indicate success or error code.
@retval CONVERTPKEYDFU_SUCCESS      Success;
@retval CONVERTPKEYDFU_READ_FAIL    Reading the input key file failed. Includes
                                    IO errors and format errors from OpenSSL.
@retval CONVERTPKEYDFU_WRITE_FAIL   Writing the output key file failed.
@retval CONVERTPKEYDFU_ERR_KEYTYPE  The input key is not a RSA key.
@retval CONVERTPKEYDFU_ERR_PRVKEY   The input key is not a private key.
@retval CONVERTPKEYDFU_ERR_PUBKEY   The input key is not a public key.
*/
enum
{   CONVERTPKEYDFU_SUCCESS, CONVERTPKEYDFU_READ_FAIL, CONVERTPKEYDFU_WRITE_FAIL,
    CONVERTPKEYDFU_ERR_KEYTYPE, CONVERTPKEYDFU_ERR_PRVKEY, CONVERTPKEYDFU_ERR_PUBKEY
};
int ConvertPkeyDfu(int aPrv, const char *apPemKey, const char *apDfuKey)
{
    int res = CONVERTPKEYDFU_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    EVP_PKEY *pKey = aPrv ? OsslReadPrvKeyFile(apPemKey) : OsslReadPubKeyFile(apPemKey);
    if(pKey)
    {
        if(EVP_PKEY_RSA == EVP_PKEY_type(EVP_PKEY_id(pKey)))
        {
            FILE *pFile;
            if((pFile = fopen(apDfuKey, "wt")))
            {
                OsslPrintPkeyDfu(pFile, aPrv, pKey);
                fclose(pFile);
            }
            else
            {
                res = CONVERTPKEYDFU_WRITE_FAIL;
            }
        }
        else
        {
            res = CONVERTPKEYDFU_ERR_KEYTYPE;
        }
        EVP_PKEY_free(pKey);
    }
    else
    {   /*  Separate out expecting private or public key errors.
            I got this test from OpenSSL source for function PEM_bytes_read_bio.
            There is no documentation on PEM_R_NO_START_LINE. What it means is
            the expected start line is wrong:
                "-----BEGIN PRIVATE KEY-----" for a private key,
                "-----BEGIN PUBLIC KEY-----" for a public key.
        */
        if(ERR_GET_REASON(ERR_peek_error()) == PEM_R_NO_START_LINE)
        {
            res = aPrv ? CONVERTPKEYDFU_ERR_PRVKEY : CONVERTPKEYDFU_ERR_PUBKEY;
        }
        else
        {
            res = CONVERTPKEYDFU_READ_FAIL;
        }
    }
    return res;
}

/******************************************************************************
@brief Generate a wrapped key bundle for secure deployment of OEM key to device.

@param[in] apKeyIn      Filename of the OEM key.
@param[in] apBundleOut  Filename of key bundle to create.
@param[in] apQcomIn     Filename of Qualcomm KeK keys file.

@return Indicate success or error code.
@retval KEYBUNDLE_SUCCESS           Success;
@retval KEYBUNDLE_ERR_READ_KEY      Reading the OEM input key file failed.
(Includes IO errors and format errors.)
@retval KEYBUNDLE_ERR_READ_QCOM     Reading the Qualcomm keys input file failed.
(Includes IO errors and format errors.)
@retval KEYBUNDLE_WRITE_FAIL        Writing the output bundle file failed.
@retval KEYBUNDLE_OSSLIB_FAIL       The OSSLIB functions failed.
*/
enum
{
    KEYBUNDLE_SUCCESS, KEYBUNDLE_ERR_READ_KEY, KEYBUNDLE_ERR_READ_QCOM, KEYBUNDLE_WRITE_FAIL,
    KEYBUNDLE_OSSLIB_FAIL
};
int CreateWrappedKeyBundle(const char *apKeyIn, const char *apBundleOut, const char *apQcomIn)
{
    int res = KEYBUNDLE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    // Positions of Qualcomm values in the KeK qdata file
    const int IDX_KEK = 0;
    const int IDX_ENCKEK = 1;
    const int IDX_QCOMIV = 2;

    // declare something that will be holding the values when read
    securlib::Aes128KeysType qData;
    securlib::Aes128KeyType oemKey = { { 0 } };
    securlib::Aes128KeyType oemNonceMic = { { 0 } };
    securlib::Aes128KeyType oemNonceEnc = { { 0 } };

    if (!apKeyIn || !apKeyIn[0] || securlib::ReadAes128KeyFile(oemKey, apKeyIn))
    {   // missing input key filename, or cannot read key contents for another reason
        res = KEYBUNDLE_ERR_READ_KEY;
    }
    else if (!apQcomIn || !apQcomIn[0])
    {   // missing input qdata filename
        res = KEYBUNDLE_ERR_READ_QCOM;
    }
    else if (securlib::ReadLeAes128KeysFromBeFile(qData, apQcomIn))
    {   // cannot get the Qualcomm supplied key data
        res = KEYBUNDLE_ERR_READ_QCOM;
    }
    else if ((qData.size() != 3) && (qData.size() != 5))
    {   // expect 3 or 5 values in QDATA - {KeK, EncKeK, QCIV}[{PaKeK, EncPaKeK}]
        res = KEYBUNDLE_ERR_READ_QCOM;
    }
    else if ((RAND_bytes(oemNonceMic.key, securlib::AES_KEY_LENGTH) != 1) || (RAND_bytes(oemNonceEnc.key, securlib::AES_KEY_LENGTH) != 1))
    {   // The OpenSSL failed to generate random numbers
        res = KEYBUNDLE_OSSLIB_FAIL;
    }
    else
    {
        // fixup the octet order of key to match a consistent PC little-endian order
        securlib::reverse8BitBytes(oemKey.key, sizeof(oemKey.key));

        // Secure provisioning Bundle file consists of:
        // OemNonceEnc - the nonce used to encode OemWrappedKey from oemKey
        // OemNonceMic - the nonce used while calculating the overall MIC for the bundle
        // QCOMIV - Qualcomm supplied value (IV used to make encKeK)
        // EncKeK - Qualcomm supplied value
        // OemWrappedKey - result of KeK + OemNonceEnc encoding the oemKey
        // KeyBundleMic - the AES128CBC for {QCOMIV, OemNonceMIC, OemNonceEnc, EncKek, OemWrappedKey}

        // encode the OEM key (AES128ctr(key=KeK, IV=NonceEnc, Text=oemKey))
        securlib::Aes128KeyType oemEncKey = { { 0 } };
        size_t sizeOfKey = securlib::AES_KEY_LENGTH;
        const int INITIAL_CTR_VALUE = 0;
        if (OsslEncryptAes128Cctr(oemEncKey.key, &sizeOfKey, oemKey.key, sizeOfKey, qData[IDX_KEK].key, oemNonceEnc.key, INITIAL_CTR_VALUE) > 0)
        {
            // In OsslEncryptAes128Cctr it references the IV(oemNonceEnc.key in this case) from hi to lo array index...
            // ...and as the Curator implementation references from lo to hi array index,
            // we need to reverse this one value before putting it in the bundle that will be used by Curator.
            securlib::reverse8BitBytes(oemNonceEnc.key, sizeof(oemNonceEnc.key));

            // construct the bundle over which to generate the MIC
            std::vector<unsigned char> oemBundle;
            oemBundle.insert(oemBundle.end(), qData[IDX_QCOMIV].key, qData[IDX_QCOMIV].key + securlib::AES_KEY_LENGTH);
            oemBundle.insert(oemBundle.end(), oemNonceMic.key, oemNonceMic.key + securlib::AES_KEY_LENGTH);
            oemBundle.insert(oemBundle.end(), oemNonceEnc.key, oemNonceEnc.key + securlib::AES_KEY_LENGTH);
            oemBundle.insert(oemBundle.end(), qData[IDX_ENCKEK].key, qData[IDX_ENCKEK].key + securlib::AES_KEY_LENGTH);
            oemBundle.insert(oemBundle.end(), oemEncKey.key, oemEncKey.key + securlib::AES_KEY_LENGTH);

            // construct the key for performing the MIC (from oemNonceMic and KeK)...
            securlib::Aes128KeyType macIv = { { 0 } };
            for (size_t i = 0; i < sizeof(oemNonceMic.key); ++i)
            {
                macIv.key[i] = oemNonceMic.key[i] ^ qData[IDX_KEK].key[i];
            }
            // ... and use that key to calculate the MIC via AES128MAC
            const int PADDING_VALUE = 0;
            securlib::Aes128KeyType keyBundleMic = { { 0 } };
            if (OsslCbcMac(EVP_aes_128_cbc(), keyBundleMic.key, &oemBundle[0], oemBundle.size(), macIv.key, PADDING_VALUE) > 0)
            {
                // add keyBundleMic to final list of items for file
                // and write the AES128keysfile
                securlib::Aes128KeysType fileKeys;
                fileKeys.push_back(oemNonceEnc);
                fileKeys.push_back(oemNonceMic);
                fileKeys.push_back(qData[IDX_QCOMIV]);
                fileKeys.push_back(qData[IDX_ENCKEK]);
                fileKeys.push_back(oemEncKey);
                fileKeys.push_back(keyBundleMic);
                if (WriteLeAes128KeysToBeFile(fileKeys, apBundleOut))
                {
                    res = KEYBUNDLE_WRITE_FAIL;
                }
            }
            else
            {
                res = KEYBUNDLE_OSSLIB_FAIL;
            }
        }
        else
        {
            res = KEYBUNDLE_OSSLIB_FAIL;
        }
    }

    return res;
}

/******************************************************************************
@brief Generate a wrapped key bundle for anti-replay deployment of OEM key to device.

@param[in] apKeyIn      Filename of the OEM key.
@param[in] apBundleOut  Filename of wrapped key bundle to create.
@param[in] apQcomIn     Filename of Qualcomm KeK/PaKeK keys file.

@return Indicate success or error code.
@retval KEYBUNDLEAR_SUCCESS         Success;
@retval KEYBUNDLEAR_ERR_READ_KEY    Reading the OEM input key file failed.
(Includes IO errors and format errors.)
@retval KEYBUNDLEAR_ERR_READ_QCOM   Reading the Qualcomm keys input file failed.
(Includes IO errors and format errors.)
@retval KEYBUNDLEAR_WRITE_FAIL      Writing the output wrapped bundle file failed.
@retval KEYBUNDLEAR_OSSLIB_FAIL     The OSSLIB functions failed.
*/
enum
{
    KEYBUNDLEAR_SUCCESS, KEYBUNDLEAR_ERR_READ_KEY, KEYBUNDLEAR_ERR_READ_QCOM, KEYBUNDLEAR_WRITE_FAIL,
    KEYBUNDLEAR_OSSLIB_FAIL
};
int CreateAntiReplayWrappedKeyBundle(const char *apKeyIn, const char *apBundleOut, const char *apQcomIn)
{
    int res = KEYBUNDLEAR_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);

    // positions of Qualcomm values in the KeK/PaKeK qdata file
    const int IDX_KEK = 0;
    const int IDX_ENCKEK = 1;
    const int IDX_QCOMIV = 2;
    const int IDX_PAKEK = 3;
    const int IDX_ENCPAKEK = 4;

    // declare something that will be holding those values when read
    securlib::Aes128KeysType qData;
    securlib::Aes128KeyType oemKey = { { 0 } };
    securlib::Aes128KeyType oemNonceMic = { { 0 } };
    securlib::Aes128KeyType oemNonceEnc = { { 0 } };

    if (!apKeyIn || !apKeyIn[0] || securlib::ReadAes128KeyFile(oemKey, apKeyIn))
    {   // missing input key filename, or cannot read key contents for another reason
        res = KEYBUNDLEAR_ERR_READ_KEY;
    }
    else if (!apQcomIn || !apQcomIn[0])
    {   // missing input qdata filename
        res = KEYBUNDLEAR_ERR_READ_QCOM;
    }
    else if (securlib::ReadLeAes128KeysFromBeFile(qData, apQcomIn))
    {   // cannot get the Qualcomm supplied key data
        res = KEYBUNDLEAR_ERR_READ_QCOM;
    }
    else if (qData.size() != 5)
    {   // expect all 5 values in QDATA - {KeK, EncKeK, QCIV}{PaKeK, EncPaKeK}
        res = KEYBUNDLEAR_ERR_READ_QCOM;
    }
    else if ((RAND_bytes(oemNonceMic.key, securlib::AES_KEY_LENGTH) != 1) || (RAND_bytes(oemNonceEnc.key, securlib::AES_KEY_LENGTH) != 1))
    {   // The OpenSSL failed to generate random numbers
        res = KEYBUNDLE_OSSLIB_FAIL;
    }
    else
    {
        // fixup the octet order of key to match a consistent PC little-endian order
        securlib::reverse8BitBytes(oemKey.key, sizeof(oemKey.key));

        // Anti-Replay Bundle file consists of:
        // OemNonceEnc - the nonce used to encode OemWrappedKey from oemKey
        // OemNonceMic - the nonce used while calculating the overall MIC for the bundle
        // QCOMIV - Qualcomm supplied value (IV used to make encKeK)
        // EncKeK - Qualcomm supplied value
        // EncPaKeK - Qualcomm supplied value
        // OemWrappedKey - result of KeK + OemNonceEnc encoding the oemKey
        // KeyBundleMic - the AES128CBC for {OemNonceMIC, OemNonceEnc, QCOMIV, EncPAKek, EncKek, OemWrappedKey}
        // PaKeK - Qualcomm supplied value needed during anti-replay protocol key send from this bundle

        // encode the OEM key (AES128ctr(key=KeK, IV=NonceEnc, Text=oemKey))
        securlib::Aes128KeyType oemEncKey = { { 0 } };
        size_t sizeOfKey = securlib::AES_KEY_LENGTH;
        const int INITIAL_CTR_VALUE = 0;
        if (OsslEncryptAes128Cctr(oemEncKey.key, &sizeOfKey, oemKey.key, sizeOfKey, qData[IDX_KEK].key, oemNonceEnc.key, INITIAL_CTR_VALUE) > 0)
        {
            // In OsslEncryptAes128Cctr it references the IV(oemNonceEnc.key in this case) from hi to lo array index...
            // ...and as the Curator implementation references from lo to hi array index,
            // we need to reverse this one value before putting it in the bundle that will be used by Curator.
            securlib::reverse8BitBytes(oemNonceEnc.key, sizeof(oemNonceEnc.key));

            // construct the bundle over which to generate the MIC
            std::vector<unsigned char> wrapBundle;
            wrapBundle.insert(wrapBundle.end(), oemNonceMic.key, oemNonceMic.key + securlib::AES_KEY_LENGTH);
            wrapBundle.insert(wrapBundle.end(), oemNonceEnc.key, oemNonceEnc.key + securlib::AES_KEY_LENGTH);
            wrapBundle.insert(wrapBundle.end(), qData[IDX_QCOMIV].key, qData[IDX_QCOMIV].key + securlib::AES_KEY_LENGTH);
            wrapBundle.insert(wrapBundle.end(), qData[IDX_ENCPAKEK].key, qData[IDX_ENCPAKEK].key + securlib::AES_KEY_LENGTH);
            wrapBundle.insert(wrapBundle.end(), qData[IDX_ENCKEK].key, qData[IDX_ENCKEK].key + securlib::AES_KEY_LENGTH);
            wrapBundle.insert(wrapBundle.end(), oemEncKey.key, oemEncKey.key + securlib::AES_KEY_LENGTH);

            // construct the key for performing the MIC (from oemNonceMic and KeK)...
            securlib::Aes128KeyType macIv = { { 0 } };
            for (size_t i = 0; i < sizeof(oemNonceMic.key); ++i)
            {
                macIv.key[i] = oemNonceMic.key[i] ^ qData[IDX_KEK].key[i];
            }
            // ... and use that key to calculate the MIC via AES128MAC
            const int PADDING_VALUE = 0;
            securlib::Aes128KeyType keyBundleMic = { { 0 } };
            if (OsslCbcMac(EVP_aes_128_cbc(), keyBundleMic.key, &wrapBundle[0], wrapBundle.size(), macIv.key, PADDING_VALUE) > 0)
            {
                // add keyBundleMic to final list of items for file
                // and write the AES128keysfile
                securlib::Aes128KeysType fileKeys;
                fileKeys.push_back(oemNonceEnc);
                fileKeys.push_back(oemNonceMic);
                fileKeys.push_back(qData[IDX_QCOMIV]);
                fileKeys.push_back(qData[IDX_ENCPAKEK]);
                fileKeys.push_back(qData[IDX_ENCKEK]);
                fileKeys.push_back(oemEncKey);
                fileKeys.push_back(keyBundleMic);
                fileKeys.push_back(qData[IDX_PAKEK]);
                if (WriteLeAes128KeysToBeFile(fileKeys, apBundleOut))
                {
                    res = KEYBUNDLEAR_WRITE_FAIL;
                }
            }
            else
            {
                res = KEYBUNDLEAR_OSSLIB_FAIL;
            }
        }
        else
        {
            res = KEYBUNDLEAR_OSSLIB_FAIL;
        }
    }

    return res;
}

// ***************************************************************************

int UeProcessCmd(CCmdLine &cmdline)
{
    int res = EXIT_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    switch (CmdLineParams.Command)
    {
    case OP_HASH:
    case OP_SIGN:
    case OP_ENCRYPT:
    case OP_SIGNENCRYPT:
        {
            const char *pEncrKeyFile = (CmdLineParams.EncrKeyFile.empty()) ? NULL: CmdLineParams.EncrKeyFile.c_str();
            const char *pSignKeyFile = (CmdLineParams.SignKeyFile.empty()) ? NULL: CmdLineParams.SignKeyFile.c_str();
            unsigned int *pAddress = CmdLineParams.HasAddress ? &CmdLineParams.Address: NULL;
            res = FSecureAppStoreImage(CmdLineParams.InFile.c_str(), pSignKeyFile, pEncrKeyFile,
                CmdLineParams.OutFile.c_str(), pAddress, CmdLineParams.CopyExec, !CmdLineParams.UseImageHeader);
            switch (res)
            {
            case FSECUREAPPSTOREIMG_SUCCESS:
                break;
            case SECUREAPPSTOREIMG_ERR_ENCRYPT:
                cmdline.OutputErrorAndFailMessages("Encrypting image");
                break;
            case SECUREAPPSTOREIMG_ERR_NO_APP:
                cmdline.OutputErrorAndFailMessages("App store not present in file " + CmdLineParams.InFile);
                break;
            case SECUREAPPSTOREIMG_ERR_TOO_SMALL:
                cmdline.OutputErrorAndFailMessages("Expected data is missing. This suggests the image"
                    " is invalid or specify app store address with " OPT_ADDRSTART " <address>.");
                break;
            case SECUREAPPSTOREIMG_ERR_RESERVED_BITS:
                cmdline.OutputErrorAndFailMessages("Reserved bits set. This suggests the image"
                    " is invalid or specify app store address with " OPT_ADDRSTART " <address>.");
                break;
            case FSECUREAPPSTOREIMG_ERR_READ_ENCKEY:
                cmdline.OutputErrorAndFailMessages("Reading encryption key file " + CmdLineParams.EncrKeyFile);
                break;
            case FSECUREAPPSTOREIMG_ERR_READ_SIGKEY:
                cmdline.OutputErrorAndFailMessages("Reading signing key file " + CmdLineParams.SignKeyFile);
                break;
            case FSECUREAPPSTOREIMG_ERR_READ_IMG:
                cmdline.OutputErrorAndFailMessages("Reading XUV file " + CmdLineParams.InFile);
                break;
            case FSECUREAPPSTOREIMG_ERR_IMG_EMPTY:
                cmdline.OutputErrorAndFailMessages("No data in XUV file " + CmdLineParams.InFile);
                break;
            case FSECUREAPPSTOREIMG_ERR_WRITE_IMG:
                cmdline.OutputErrorAndFailMessages("Writing XUV file " + CmdLineParams.OutFile);
                break;
            default:
                cmdline.OutputErrorAndFailMessages("Unknown error");
                break;
            }
            res = (FSECUREAPPSTOREIMG_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        }
        break;
    default:
        /* Should not arrive here unless there a programming error. */
        cmdline.OutputErrorAndFailMessages("Unexpected operation error");
        res = EXIT_FAILURE;
    }
    return res;
}

int HydProcessCmd(CCmdLine &cmdline)
{
    int res = EXIT_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    if(CmdLineParams.endian == EN_DEFAULT)
    {
        const char *pEndian = getenv(ENV_SECURITYCMD_XUVE); /* XUV endianness L or B */
        gXuvBe = !(pEndian && strlen(pEndian) == 1 && tolower(pEndian[0]) =='l');
    }
    else
    {
        gXuvBe = (CmdLineParams.endian != EN_U16LE);
    }
    if(!cmdline.IsQuiet() &&
        OP_PEMTODFUKEY != CmdLineParams.Command && OP_KEYGEN_UNLOCK != CmdLineParams.Command &&
        OP_WRAP_KEY != CmdLineParams.Command && OP_WRAP_KEY_AR != CmdLineParams.Command &&
        OP_KEYGEN_RSA != CmdLineParams.Command && OP_SCRAMBLEASPK != CmdLineParams.Command)
    {   /* only applicable to XUV files */
        printf("U16%s endian mode (for XUV files)\n", gXuvBe? "BE": "LE");
    }
    switch (CmdLineParams.Command)
    {
    case OP_HASH:
        switch((res = FXuvImageHashSha256(CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str())))
        {
        case FXUVIMGHASH_SUCCESS:
            break;
        case FXUVIMGHASH_ERR_READ_IMG:
            cmdline.OutputErrorAndFailMessages("Reading XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGHASH_ERR_IMG_EMPTY:
            cmdline.OutputErrorAndFailMessages("No data in XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGHASH_ERR_WRITE_IMG:
            cmdline.OutputErrorAndFailMessages("Writing XUV file " + CmdLineParams.OutFile);
            break;
        }
        res = (FXUVIMGHASH_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        break;

    case OP_SIGN:
        res = FXuvImageSignRsaPss(CmdLineParams.SignKeyFile.c_str(), CmdLineParams.InFile.c_str(),
            CmdLineParams.OutFile.c_str());
        switch(res)
        {
        case FXUVIMGSIGN_SUCCESS:
            break;
        case FXUVIMGSIGN_ERR_READ_KEY:
            cmdline.OutputErrorAndFailMessages("Reading key file " + CmdLineParams.SignKeyFile);
            break;
        case FXUVIMGSIGN_ERR_READ_IMG:
            cmdline.OutputErrorAndFailMessages("Reading XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGSIGN_ERR_IMG_EMPTY:
            cmdline.OutputErrorAndFailMessages("No data in XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGSIGN_ERR_SIGN_IMG:
            cmdline.OutputErrorAndFailMessages(AppendOsslError("Signing failed")); /* Error from OpenSSL */
            break;
        case FXUVIMGSIGN_ERR_WRITE_IMG:
            cmdline.OutputErrorAndFailMessages("Writing XUV file " + CmdLineParams.OutFile);
            break;
        }
        res = (FXUVIMGSIGN_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        break;

    case OP_CBCMAC:
        res = FXuvImageCreateCbcMac(EVP_aes_128_cbc(), CmdLineParams.EncrKeyFile.c_str(),
            CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str(), 0);
        switch(res)
        {
        case FXUVIMGCBCMAC_SUCCESS:
            break;
        case FXUVIMGCBCMAC_ERR_READ_KEY:
            /* Let's not output a second error if keyfile lib already has */
            if(!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
            {
                cmdline.OutputErrorAndFailMessages("Reading key file " + CmdLineParams.EncrKeyFile);
            }
            else
            {
                cmdline.OutputFinalMessage();
                res = EXIT_FAILURE;
            }
            break;
        case FXUVIMGCBCMAC_ERR_READ_IMG:
            cmdline.OutputErrorAndFailMessages("Reading XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGCBCMAC_ERR_IMG_EMPTY:
            cmdline.OutputErrorAndFailMessages("No data in XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGCBCMAC_ERR_IMG_NOT_MULTIPLE:
            cmdline.OutputErrorAndFailMessages("Image is not a multiple of 8 words in XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGCBCMAC_ERR_WRITE_IMG:
            cmdline.OutputErrorAndFailMessages("Writing XUV file " + CmdLineParams.OutFile);
            break;
        case FXUVIMGCBCMAC_ERR_CBCMAC_IMG:
            cmdline.OutputErrorAndFailMessages(AppendOsslError("CBC-MAC creation failed")); /* Error from OpenSSL */
            break;
        }
        res = (FXUVIMGCBCMAC_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        break;

    case OP_ENCRYPT:
        res = FXuvImageEncryptAes128Cctr(CmdLineParams.EncrKeyFile.c_str(),
            CmdLineParams.SignKeyFile.c_str(),
            CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str(), (unsigned int)CmdLineParams.U64Address);
        switch(res)
        {
        case FXUVIMGENC_SUCCESS:
            break;
        case FXUVIMGENC_ERR_READ_KEY:
            /* Let's not output a second error if keyfile lib already has */
            if(!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
            {
                cmdline.OutputErrorAndFailMessages("Reading key file " + CmdLineParams.EncrKeyFile);
            }
            else
            {
                cmdline.OutputFinalMessage();
                res = EXIT_FAILURE;
            }
            break;
        case FXUVIMGENC_ERR_READ_IV:
            cmdline.OutputErrorAndFailMessages("Reading IV file " + CmdLineParams.SignKeyFile);
            break;
        case FXUVIMGENC_ERR_READ_IMG:
            cmdline.OutputErrorAndFailMessages("Reading XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGENC_ERR_IMG_EMPTY:
            cmdline.OutputErrorAndFailMessages("No data in XUV file " + CmdLineParams.InFile);
            break;
        case FXUVIMGENC_ERR_WRITE_IMG:
            cmdline.OutputErrorAndFailMessages("Writing XUV file " + CmdLineParams.OutFile);
            break;
        case FXUVIMGENC_ERR_ENCRYPT_IMG:
            cmdline.OutputErrorAndFailMessages(AppendOsslError("Encryption failed")); /* Error from OpenSSL */
            break;
        }
        res = (FXUVIMGENC_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        break;

    case OP_KEYGEN_RSA:
        if(OsslRsaGenKeyPairFiles(CmdLineParams.KeySize, CmdLineParams.PubExp,
            CmdLineParams.PrvKeyFile.c_str(), CmdLineParams.PubKeyFile.c_str()) < 1)
        {
            cmdline.OutputErrorAndFailMessages(AppendOsslError("Key creation failed or IO error."));
            res =EXIT_FAILURE;
        }
        break;

    case OP_SCRAMBLEASPK:
        /* ScrambleAspk outputs it's own error messages */
        if(!ScrambleAspk(cmdline, CmdLineParams.Modulus.c_str(), CmdLineParams.Seed.c_str(),
            CmdLineParams.Aspk.c_str(), CmdLineParams.ModKeyFormat, CmdLineParams.OutFile.c_str()))
        {
            cmdline.OutputFinalMessage();
            res =EXIT_FAILURE;
        }
        break;

    case OP_PEMTODFUKEY:
        switch((res = ConvertPkeyDfu(KY_PRV == CmdLineParams.KeyType, CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str())))
        {
        case CONVERTPKEYDFU_SUCCESS:
            break;
        case CONVERTPKEYDFU_READ_FAIL:
            cmdline.OutputErrorAndFailMessages(AppendOsslError("Reading key file " + CmdLineParams.InFile));
            break;
        case CONVERTPKEYDFU_WRITE_FAIL:
            cmdline.OutputErrorAndFailMessages(AppendOsslError("Writing key file " + CmdLineParams.OutFile));
            break;
        case CONVERTPKEYDFU_ERR_KEYTYPE:
        case CONVERTPKEYDFU_ERR_PRVKEY:
        case CONVERTPKEYDFU_ERR_PUBKEY:
            if(KY_PRV == CmdLineParams.KeyType)
            {
                cmdline.OutputErrorAndFailMessages("Wrong key type, expecting RSA private key in \"" + CmdLineParams.InFile + "\"");
            }
            else
            {
                cmdline.OutputErrorAndFailMessages("Wrong key type, expecting RSA public key in \"" + CmdLineParams.InFile + "\"");
            }
            break;
        }
        res = (CONVERTPKEYDFU_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        break;

    case OP_KEYGEN_UNLOCK:
        switch((res = KeyGenUsbDebugUnlockFile(CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str())))
        {
        case KEY_GEN_USB_DBG_SUCCESS:
            break;
        case KEY_GEN_USB_DBG_READ_FAIL:
            /* Let's not output a second error if keyfile lib already has */
            if(!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
            {
                cmdline.OutputErrorAndFailMessages("Reading key file " + CmdLineParams.InFile);
            }
            else
            {
                cmdline.OutputFinalMessage();
                res = EXIT_FAILURE;
            }
            break;
        case KEY_GEN_USB_DBG_WRITE_FAIL:
            cmdline.OutputErrorAndFailMessages("Writing key file " + CmdLineParams.OutFile);
            break;
        case KEY_GEN_USB_DBG_ENCRYPT_FAIL:
            cmdline.OutputErrorAndFailMessages("encrypting key");
            break;
        }
        res = (KEY_GEN_USB_DBG_SUCCESS == res) ? EXIT_SUCCESS: EXIT_FAILURE;
        break;

    case OP_WRAP_KEY:
        switch (res = CreateWrappedKeyBundle(CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str(), CmdLineParams.SignKeyFile.c_str()))
        {
        case KEYBUNDLE_SUCCESS:
            break;
        case KEYBUNDLE_ERR_READ_KEY:
            /* Let's not output a second error if keyfile lib already has */
            if (!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
            {
                cmdline.OutputErrorAndFailMessages("Reading key file " + CmdLineParams.InFile);
            }
            else
            {
                cmdline.OutputFinalMessage();
                res = EXIT_FAILURE;
            }
            break;
        case KEYBUNDLE_ERR_READ_QCOM:
            /* Let's not output a second error if keyfile lib already has */
            if (!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
            {
                cmdline.OutputErrorAndFailMessages("Reading QCOM file " + CmdLineParams.SignKeyFile);
            }
            else
            {
                cmdline.OutputFinalMessage();
                res = EXIT_FAILURE;
            }
            break;
        case KEYBUNDLE_WRITE_FAIL:
            cmdline.OutputErrorAndFailMessages("Writing key bundle file " + CmdLineParams.OutFile);
            break;
        case KEYBUNDLE_OSSLIB_FAIL:
            cmdline.OutputErrorAndFailMessages("OSSLIB cannot calculate/Encrypt using AES128");
            break;
        }
        res = (KEYBUNDLE_SUCCESS == res) ? EXIT_SUCCESS : EXIT_FAILURE;
        break;

    case OP_WRAP_KEY_AR:
        switch(res = CreateAntiReplayWrappedKeyBundle(CmdLineParams.InFile.c_str(), CmdLineParams.OutFile.c_str(), CmdLineParams.SignKeyFile.c_str()))
        {
            case KEYBUNDLEAR_SUCCESS:
                break;
            case KEYBUNDLEAR_ERR_READ_KEY:
                /* Let's not output a second error if keyfile lib already has */
                if (!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
                {
                    cmdline.OutputErrorAndFailMessages("Reading key file " + CmdLineParams.InFile);
                }
                else
                {
                    cmdline.OutputFinalMessage();
                    res = EXIT_FAILURE;
                }
                break;
            case KEYBUNDLEAR_ERR_READ_QCOM:
                /* Let's not output a second error if keyfile lib already has */
                if (!MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_KEYFILE_LIB, true))
                {
                    cmdline.OutputErrorAndFailMessages("Reading QCOM PaKeK file " + CmdLineParams.SignKeyFile);
                }
                else
                {
                    cmdline.OutputFinalMessage();
                    res = EXIT_FAILURE;
                }
                break;
            case KEYBUNDLEAR_WRITE_FAIL:
                cmdline.OutputErrorAndFailMessages("Writing key bundle file " + CmdLineParams.OutFile);
                break;
            case KEYBUNDLEAR_OSSLIB_FAIL:
                cmdline.OutputErrorAndFailMessages("OSSLIB cannot calculate/Encrypt using AES128");
                break;
        }
        res = (KEYBUNDLEAR_SUCCESS == res) ? EXIT_SUCCESS : EXIT_FAILURE;
        break;

    default:
        /* Should not arrive here unless there a programming error. */
        cmdline.OutputErrorAndFailMessages("Unexpected operation error");
        res = EXIT_FAILURE;
        break;
    }
    return res;
}

/******************************************************************************
@brief main function for signing and encryption tool.
*/
int main(int argc, char** argv)
{
    int res = EXIT_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(int, res);
    OsslInit();
    /* flush otherwise OpenSSL failure message appears in wrong place */
    fflush(stdout);
    fflush(stderr);

    /* Choose application name based on executable filename (argv[0]) */
    const char *pName = NAME_NEW;
    if(argc > 0)
    {
        if(strcasestr(argv[0], NAME_OLD))
        {
            pName = NAME_OLD;
        }
    }

    int product = PreprocessCmdLineForProduct(argc, argv);

    CCmdLine cmdline(pName, "Signing and Encrypting Tool.",
        "This tool is for signing and encrypting XUV images for supported Qualcomm devices.",
        "2016", argc, argv);
    if(ProcessCmdLine(cmdline, product))
    {
        /* Command line is badly formed. */
        res = EXIT_FAILURE;
    }
    else
    {
        switch(CmdLineParams.product)
        {
        case PR_UE:
            if(UeProcessCmd(cmdline) == EXIT_FAILURE)
            {
                res = EXIT_FAILURE;
            }
            break;
        case PR_HYD:
            if(HydProcessCmd(cmdline) == EXIT_FAILURE)
            {
                res = EXIT_FAILURE;
            }
            break;
        default:
            /* Should not arrive here unless there a programming error. */
            cmdline.OutputErrorAndFailMessages("Unexpected product error");
            res = EXIT_FAILURE;
        }
    }

    OsslFin();
    if(EXIT_SUCCESS == res && !cmdline.IsQuiet())
    {
        res = cmdline.OutputFinalMessage();
    }
    return res;
}
