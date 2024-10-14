/**********************************************************************
*
*  FILE   :  inarrow.cpp
*
*            Copyright (c) 2001-2022 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  unicode widening and narrowing functions.
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/unicode/inarrow.cpp#1 $
*
***********************************************************************/

/* Include library header file */

#include "ichar.h"

/* Include system header files */
#ifdef WIN32
#include <Windows.h> // for definition of WideCharToMultiByte/MultiByteToWideChar
#endif

/*
    C callable equivalent functions
*/

char *iinarrowdupW(const wchar_t *apString)
{
    return istrdupA(inarrow(apString).c_str());
}

wchar_t *iccoercedupW(const char *apString)
{
    return istrdupW(iwiden(apString).c_str());
}


/*
    String conversions.
*/

#ifdef WIN32
/* for converting from wchar_t[] to UTF-8[] */
std::string Utf16To8String(const wchar_t *pWideIn)
{
    // Edge cases: no input or zero length input - return the empty string
    if ((pWideIn == NULL) || (*pWideIn == 0))
    {
        return std::string("");
    }

    // From UTF-16 to UTF-8 codepage, can hold any valid UNICODE sequence (and replaces any invalid sequences with U+FFFD)
    // which means octetsRequired will always be valid
    size_t octetsRequired = WideCharToMultiByte(CP_UTF8, 0, pWideIn, -1, NULL, 0, NULL, NULL);
    struct Buff { char *buff; explicit Buff(size_t n) : buff(new char[n]) {} ~Buff() { delete[] buff; } } buff(octetsRequired);
    WideCharToMultiByte(CP_UTF8, 0, pWideIn, -1, buff.buff, static_cast<int>(octetsRequired), NULL, NULL);

    return std::string(buff.buff, octetsRequired);
}

/* for converting from UTF-8[] to wchar_t based string */
std::wstring Utf8To16String(const char *pCharIn)
{
    // Edge cases: no input or zero length input - return the empty string
    if ((pCharIn == NULL) || (*pCharIn == 0))
    {
        return std::wstring(L"");
    }

    // From UTF-8 codepage to UTF-16, can hold any valid UNICODE sequence (and replaces any invalid sequences with U+FFFD)
    // which means wcharsRequired will always be valid
    size_t wcharsRequired = MultiByteToWideChar(CP_UTF8, 0, pCharIn, -1, NULL, 0);
    struct Buff { wchar_t *buff; explicit Buff(size_t n) : buff(new wchar_t[n]) {} ~Buff() { delete[] buff; } } buff(wcharsRequired);
    MultiByteToWideChar(CP_UTF8, 0, pCharIn, -1, buff.buff, static_cast<int>(wcharsRequired));

    return std::wstring(buff.buff, wcharsRequired);
}
#endif

std::wstring iwiden(const std::string &aString)
{
    const char *a = aString.data();
    size_t need = mbstowcs(0, a, aString.size());
    if ( need != (size_t) -1 )
    {
		struct Buff { wchar_t *buff; explicit Buff(size_t n) : buff(new wchar_t[n]) {} ~Buff() { delete [] buff; } } buff(need);
        mbstowcs(buff.buff, a, aString.size());
        return std::wstring(buff.buff, need);
    }
    else
    {
        // the api call has failed.  Lets do something stright forward.
        std::wstring rv;
        for ( size_t i = 0 ; i < aString.size() ; ++i )
            rv += (wchar_t)aString[i];
        return rv;
    }
}

std::string inarrow(const std::wstring &aString)
{
    const wchar_t *w = aString.data();
    size_t need = wcstombs(0, w, aString.size());
    if ( need != (size_t) -1 )
    {
		struct Buff { char *buff; explicit Buff(size_t n) : buff(new char[n]) {} ~Buff() { delete [] buff; } } buff(need);
        wcstombs(buff.buff, w, aString.size());
        return std::string(buff.buff, need);
    }
    else
    {
        // the api call has failed.  Lets do something stright forward.
        std::string rv;
        for ( size_t i = 0 ; i < aString.size() ; ++i )
            rv += (char)(aString[i] & 0xFF);
        // this may result in unprintable characters being generated.  Tough.
        return rv;
    }
}



/*
    Filename conversions to allow use with fstream constructors etc.
    This may need to be platform dependent, but currently just use the
    platform independent conversions defined above. If this does need
    to be done then only two functions (marked below) need changing.
*/

std::wstring iwiden_filename(const std::string &aFilename)
{
    // Platform specific widening
    return iwiden(aFilename);
}

std::string inarrow_filename(const std::wstring &aFilename)
{
    // Platform specific narrowing
    return inarrow(aFilename);
}

