#ifndef MEMORY_LEAK_CHECK_IMPL_H
#define MEMORY_LEAK_CHECK_IMPL_H

//
// memoryleakcheckimpl.h
//
// Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// See this included file for details of how to use this functionality
#include "memoryleakcheck.h"

#ifdef USE_MS_LEAK_DETECTOR
// This must be called once (or more) for each executable unit.
void InitialiseMemoryChecks()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
}
#endif

#endif
