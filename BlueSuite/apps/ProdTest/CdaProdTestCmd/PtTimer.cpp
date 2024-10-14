//**************************************************************************************************
//
//  PtTimer.cpp
//
//  Copyright (c) 2020 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Timer class definition, part of an example application for production test.
//
//**************************************************************************************************

#include "PtTimer.h"
#include "PtUi.h"
#include <sstream>

using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////

CPtTimer::CPtTimer(const std::string& aWhat)
    : mWhat(aWhat), mStartTime(steady_clock::now())
{
}

////////////////////////////////////////////////////////////////////////////////

CPtTimer::~CPtTimer()
{
    time_point<steady_clock> endTime = steady_clock::now();

    ostringstream msg;
    msg << mWhat << " duration = "
        << duration_cast<milliseconds>(endTime - mStartTime).count() << " ms";
    
    CPtUi::Ref().Write(msg.str(), false);
}
