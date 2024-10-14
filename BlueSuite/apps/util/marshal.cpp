/**********************************************************************
 *
 *  marshal.cpp
 *  
 *  Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 **********************************************************************/

#include "marshal.h"

#include <vcclr.h>	// for PtrToStringChars

#include <string>

using namespace System;
using namespace System::Runtime::InteropServices;

using namespace marshal;

/// <summary>
/// Converts the specified System::String^ to wchar_t*.
/// </summary>
/// <param name="str">The string to convert.</param>
/// <returns></returns>
template<> 
wchar_t* marshal::to(String^ str) 
{ 
	pin_ptr<const wchar_t> cpwc = PtrToStringChars(str); 

	int len = str->Length + 1; 
	wchar_t* s = new wchar_t[len]; 
	wcscpy_s(s, len, cpwc); 

	return s; 
} 

/// <summary>
/// Converts the specified System::String^ to const char*.
/// </summary>
/// <param name="str">The string to convert.</param>
/// <returns></returns>
template<>
char* marshal::to(String^ str)
{
	char* s = nullptr;

	if(!String::IsNullOrEmpty(str))
	{
        IntPtr p = Marshal::StringToHGlobalAnsi(str);

		int len = str->Length + 1; 
		s = new char[len]; 
		strcpy_s(s, len, (char *)p.ToPointer()); 

		Marshal::FreeHGlobal(p);
	}
	else
	{
		s = "";
	}

    return s;
}

/// <summary>
/// Converts the specified System::String^ to std::string.
/// </summary>
/// <param name="str">The string to convert.</param>
/// <returns></returns>
template<>
std::string marshal::to(String^ str)
{
#ifdef UNICODE__ICHAR_WIDE
    IntPtr p = Marshal::StringToHGlobalUni(str);
#else
    IntPtr p = Marshal::StringToHGlobalAnsi(str);
#endif
	std::string s((const char*) p.ToPointer());
    Marshal::FreeHGlobal(p);
    
    return s;
}

/// <summary>
/// Converts the specified const System::String^ to const std::string.
/// </summary>
/// <param name="str">The string to convert.</param>
/// <returns></returns>
template<>
const std::string marshal::to(String^ const str)
{
#ifdef UNICODE__ICHAR_WIDE
    IntPtr p = Marshal::StringToHGlobalUni(str);
#else
    IntPtr p = Marshal::StringToHGlobalAnsi(str);
#endif
	const std::string s((const char*) p.ToPointer());
    Marshal::FreeHGlobal(p);
    
    return s;
}
