////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  FILE:    stop_watch.h
//
//  AUTHOR:  ajh
//
//  DATE:    3 October 2001
//
//  PURPOSE: Simple duration measuring object.
//
//  CLASS:   StopWatch
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __STOP_WATCH_H__
#define __STOP_WATCH_H__

#ifndef _WIN32
// required by non-windows implementation
#include "common/countedpointer.h"
#include <string> // keep for now but in the future this could be removed if the non-windows builds do not require it
#endif

class StopWatch
{
public:
    StopWatch(); // Records the time it was built
    StopWatch(const StopWatch &);
    StopWatch &operator=(const StopWatch &);
    ~StopWatch();

    unsigned long duration(); // milliseconds since creation.
    unsigned long uduration();// microseconds since creation.

private:
    class Impl;
#ifdef _WIN32
    Impl * pImpl;
#else
    CountedPointer<Impl> ptr;
#endif
};

#endif

