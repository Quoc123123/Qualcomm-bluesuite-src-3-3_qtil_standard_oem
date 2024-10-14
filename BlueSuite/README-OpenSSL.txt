
These instructions describe how to build OpenSSL and copy the necessary files
into a BlueSuite 3.x source code release.


Prerequisites
-------------------------------------------------------------------------------
1. (For Windows only) Visual Studio 2019 (as per BlueSuite source release
    requirements).
2. NASM (Netwide Assembler from https://www.nasm.us). Use a recent "stable" build.
    For Windows use appropriate installer to match version of Windows e.g.:
    nasm-2.14.02-installer-x64.exe or nasm-2.14.02-installer-x86.exe.
3. Perl (as per OpenSSL minimum requirements. See NOTES.PERL in the OpenSSL tarball).


Instructions for obtaining and authenticating OpenSSL
-------------------------------------------------------------------------------
1. The required version of OpenSSL is 1.1.1 (revision i or later). Download
    from https://www.openssl.org/source/ the openssl-1.1.1i.tar.gz tarball,
    the SHA256 checksum and the PGP signature files.
2. Check SHA256 check matches using sha256sum (or other means) e.g.:
    sha256sum -c <(echo "$(<openssl-1.1.1i.tar.gz.sha256) *openssl-1.1.1i.tar.gz")
3. Check signature matches. e.g.:
    gpg --verify openssl-1.1.1i.tar.gz.asc openssl-1.1.1i.tar.gz
4. Check the fingerprint matches the signee's fingerprint on
    https://www.openssl.org/community/omc.html


Instructions for building OpenSSL
-------------------------------------------------------------------------------
The authoritative instructions are included in the OpenSSL tarball (in the
files "INSTALL", "NOTES.WIN", "NOTES.UNIX" and "README").
The following example instructions have been derived from those instructions
for OpenSSL 1.1.1i.
For Windows 10 and later releases, the tar application may be found within
the Windows\System32 folder. Tar is also available in MinGW/MSYS and Cygwin.

For Windows 32-bit:
    1. Extract (e.g. using 7-zip or tar) the tarball into
        a platform specific directory, e.g.
        C:\src\openssl-1.1.1i-win32
    2. Open "x86 Native Tools Command Prompt for VS 2019" or
        "x64_x86 Cross Tools Command Prompt for VS 2019" from
        Start menu >
            Visual Studio 2019
    3. Build specifying a build directory (with --prefix and --openssldir
        specifying non-default directories) e.g.:
            a. cd C:\src\openssl-1.1.1i-win32
            b. perl Configure VC-WIN32 --prefix=C:\src\build-openssl-1.1.1i-win32 --openssldir=C:\src\build-openssl-1.1.1i-win32-ssl
            c. nmake
            d. nmake install
            e. nmake test

For Windows 64-bit:
    1. Extract (e.g. using 7-zip or tar) the tarball into
        a platform specific directory, e.g.
        C:\src\openssl-1.1.1i-win64
    2. Open "x64 Native Tools Command Prompt for VS 2019" or
        "x86_x64 Cross Tools Command Prompt for VS 2019" from
        Start menu >
            Visual Studio 2019
    3. Build specifying a build directory (with --prefix and --openssldir
        specifying non-default directories) e.g.:
            a. cd C:\src\openssl-1.1.1i-win64
            b. perl Configure VC-WIN64A --prefix=C:\src\build-openssl-1.1.1i-win64 --openssldir=C:\src\build-openssl-1.1.1i-win64-ssl
            c. nmake
            d. nmake install
            e. nmake test

For Linux:
    1. Create and change to a suitable directory e.g.:
        a. mkdir -p "${HOME}/src"
        b. cd "${HOME}/src"
    2. Copy the downloaded openssl-1.1.1i.tar.gz tarball into the directory e.g:
            cp /your/download/path/openssl-1.1.1i.tar.gz .
    3. Extract the openssl-1.1.1i.tar.gz tarball using tar e.g:
            tar xvf openssl-1.1.1i.tar.gz
    4. Build specifying a build directory (with --prefix specifying a
        non-default directory) e.g.:
            a. cd "${HOME}/src/openssl-1.1.1i"
            b. LDFLAGS=" -Wl,-rpath='\$\$ORIGIN/../lib' " ./config --prefix="${HOME}/src/build-openssl-1.1.1i"
            c. make
            d. make install
            e. make test


Instructions for copying OpenSSL into the BlueSuite source tree
-------------------------------------------------------------------------------
For Windows:
    1. Edit the following batch files as necessary in order to set the
        locations of the OpenSSL files and BlueSuite source tree (as per the
        instructions in the batch files):
            populate-openssl-win32.bat
            populate-openssl-win64.bat
    2. Run the batch files listed in step 1.

For Linux:
    1. Copy libcrypto.so.1.1 into the result directory, e.g.:
        cp ${HOME}/src/build-openssl-1.1.1i/lib/libcrypto.so.1.1 ${HOME}/src/sourceReleaseBlueSuite3xWindows-3.x.y.z/result/linux/bin
