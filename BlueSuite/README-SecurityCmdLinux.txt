
These instructions describe how to build and run SecurityCmd for Linux.


Prerequisites
-------------------------------------------------------------------------------
OpenSSL 1.1.1 (revision i or later) is required.


Preparation
-------------------------------------------------------------------------------
1. Unzip the source code package.
2. Obtain and build OpenSSL. See README-OpenSSL.txt for details.


Building
-------------------------------------------------------------------------------
1. Update the path in ./apps/security/openssl.path to point to the OpenSSL
    build e.g.: OPENSSL_TOP=$(HOME)/src/build-openssl-1.1.1i
    This path will be baked into the securitycmd binary RPATH. Consider if this
    is appropriate for the target platform.
2. cd to apps/make.
3. Run "make securitycmd".
4. securitycmd requires libcrypto.so.1.1 which is expected to be in the same
    directory as the securitycmd executable or the directory specified in step
    1. If necessary it can be copied into the result directory, e.g.:
    cp ${HOME}/src/build-openssl-1.1.1i/lib/libcrypto.so.1.1 ../../result/linux/bin


Running the tools
-------------------------------------------------------------------------------
cd to ../../result/linux/bin.

Run "./securitycmd -help" to display the usage details.
