/**********************************************************************
 *
 *  marshal.h
 *  
 *  Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 **********************************************************************/

#include <string>

using namespace System;

namespace marshal 
{ 
	template <typename T> 
	T to(String^ str);


	/// <summary>
	/// Converts the specified String^ to wchar_t*.
	/// </summary>
	/// <param name="str">The string to convert.</param>
	/// <returns></returns>
	template<> 
	extern wchar_t* to(String^ str);

	/// <summary>
	/// Converts the specified String^ to const char*.
	/// </summary>
	/// <param name="str">The string to convert.</param>
	/// <returns></returns>
	template<>
	extern char* to(String^ str);

	/// <summary>
	/// Converts the specified String^ to std::string.
	/// </summary>
	/// <param name="str">The string to convert.</param>
	/// <returns></returns>
	template<>
	extern std::string to(String^ str);

	/// <summary>
	/// Converts the specified const String^ to const std::string.
	/// </summary>
	/// <param name="str">The string to convert.</param>
	/// <returns></returns>
	template<>
	extern const std::string to(String^ const str);
} 
