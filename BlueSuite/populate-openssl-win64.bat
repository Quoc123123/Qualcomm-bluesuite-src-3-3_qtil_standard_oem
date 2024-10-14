@rem This batch file populates a BlueSuite source tree with the files from a
@rem OpenSSL 64-bit build.

@rem OSSL and BLUS will need editing to point to the correct directories.
@rem OSSL is the build output directory of the OpenSSL 64-bit build.
@rem BLUS is the BlueSuite source release directory to populate.
set "OSSL=C:\src\build-openssl-1.1.1i-win64"
set "BLUS=C:\sourceReleaseBlueSuite3xWindows-1.2.3.4"

@echo off

if not exist "%OSSL%" (
    echo %OSSL% does not exist. Update OSSL to be your OpenSSL 64-bit Windows build.
    goto :EOF
)
if not exist "%BLUS%" (
    echo %BLUS% does not exist. Update BLUS to be your BlueSuite source code directory.
    goto :EOF
)

set "INCL=%BLUS%\apps\3rd"
set "TARG=%BLUS%\result\x86win64"

rem common include files
for %%f in (aes.h, asn1.h, asn1_mac.h, asn1err.h, asn1t.h, async.h, asyncerr.h, bio.h,
    bioerr.h, blowfish.h, bn.h, bnerr.h, buffer.h, buffererr.h, camellia.h, cast.h,
    cmac.h, cms.h, cmserr.h, comp.h, comperr.h, conf.h, conf_api.h, conferr.h, crypto.h,
    cryptoerr.h, ct.h, cterr.h, des.h, dh.h, dherr.h, dsa.h, dsaerr.h, dtls1.h, e_os2.h,
    ebcdic.h, ec.h, ecdh.h, ecdsa.h, ecerr.h, engine.h, engineerr.h, err.h, evp.h,
    evperr.h, hmac.h, idea.h, kdf.h, kdferr.h, lhash.h, md2.h, md4.h, md5.h, mdc2.h,
    modes.h, obj_mac.h, objects.h, objectserr.h, ocsp.h, ocsperr.h, opensslv.h,
    ossl_typ.h, pem.h, pem2.h, pemerr.h, pkcs12.h, pkcs12err.h, pkcs7.h, pkcs7err.h,
    rand.h, rand_drbg.h, randerr.h, rc2.h, rc4.h, rc5.h, ripemd.h, rsa.h, rsaerr.h,
    safestack.h, seed.h, sha.h, srp.h, srtp.h, ssl.h, ssl2.h, ssl3.h, sslerr.h, stack.h,
    store.h, storeerr.h, symhacks.h, tls1.h, ts.h, tserr.h, txt_db.h, ui.h, uierr.h,
    whrlpool.h, x509.h, x509_vfy.h, x509err.h, x509v3.h, x509v3err.h
) do (
    xcopy "%OSSL%\include\openssl\%%f" "%INCL%\openssl\include\common\openssl\" /Y /I /F
)
rem platform specific include files (applink and opensslconf.h)
for %%f in (applink.c, opensslconf.h) do (
    xcopy "%OSSL%\include\openssl\%%f" "%INCL%\openssl\include\win64\openssl\" /Y /I /F
)
for %%i in (Debug, Release) do (
    rem lib files
    for %%f in (libcrypto.lib, libssl.lib) do (
        xcopy "%OSSL%\lib\%%f" "%TARG%\lib\%%i\" /Y /I /F
    )
    rem dll files
    for %%f in (libcrypto-1_1-x64.dll, libssl-1_1-x64.dll) do (
        xcopy "%OSSL%\bin\%%f" "%TARG%\bin\%%i\" /Y /I /F
    )
)
