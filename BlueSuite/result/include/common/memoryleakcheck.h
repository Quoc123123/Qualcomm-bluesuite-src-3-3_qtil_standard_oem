/*******************************************************************************
 *
 *  memoryleakcheck.h
 *
 *  Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 ******************************************************************************/

// Use another memory leak detector... Temporarily disable it for now...
#if 0

// Use the FORCE_MLC_ON and FORCE_MLC_OFF macros if required to control the definition of
// the macros for "new" and "DEBUG_NEW". This is only necessary if a third party (Eg. AFX)
// has a class method called "new()".

// Define this macro immediately before including this file to force memory leak checking ON
#ifdef FORCE_MLC_ON
#undef MEMORY_LEAK_CHECK_H
#undef FORCE_MLC_ON
#endif

// Define this macro immediately before including this file to force memory leak checking OFF
#ifdef  FORCE_MLC_OFF
#define MEMORY_LEAK_CHECK_H
#undef  FORCE_MLC_OFF
#undef  DEBUG_NEW
#undef  new
#endif


//
// This file is used to invoke the MSVC memory leak code.
// This file **MUST** be the very first #include in the source file,
// i.e. before *everything* else, including STDAFX.H or WINDOWS.H
//
// Include this header file in every source code that is to be checked.
// In the short-term, this can be achieved by modifying the project
// settings to force include the header file...
// Properties -> Configiuration Properties -> C/C++ -> Advanced -> Force includes
// ...but this should not be checked in; it is better to explicitly
// include the header file if the source code is to be checked in.
//
// For any given executable unit, there must be at least one call to
// the initialisation function; it is best to wrap that within a check
// for USE_MS_LEAK_DETECTOR, Eg.
//     int main(int argc, char* argv[])
//     {
//         ...
//     #ifdef USE_MS_LEAK_DETECTOR
//         InitialiseMemoryChecks();
//     #endif
//         ...
//     }
// The function definition is in "memoryleakcheckimpl.h".
//
// HOW TO USE THIS MEMORY LEAK CHECKING FUNCTIONALITY:
// Include the "memoryleakcheckimpl.h" header file at the top of the source code
// that contains the 'main' in the execution unit (and call the function as
// described above). Include the "memoryleakcheck.h" file at the top of every
// other source file in the project.
//

#undef USE_MS_LEAK_DETECTOR
#if ( defined(DEBUG) && defined(WIN32) && !defined(UNDER_CE) )
#define USE_MS_LEAK_DETECTOR 1
#endif

#ifdef USE_MS_LEAK_DETECTOR
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <iostream>
#undef  DEBUG_NEW
#undef  new
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#endif
