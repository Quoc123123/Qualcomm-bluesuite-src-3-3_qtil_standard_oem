//**************************************************************************************************
//
//  PtTimer.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Timer class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_TIMER_H
#define PT_TIMER_H

#include <string>
#include <chrono>

///
/// Production test timer class.
///
class CPtTimer
{
public:
    ///
    /// Constructor: Stores the start time.
    /// @param[in] aWhat String indicating what's being timed.
    ///
    explicit CPtTimer(const std::string& aWhat);

    ///
    /// Destructor: Logs the Duration since construction.
    ///
    ~CPtTimer();

private:
    ///
    /// What's being timed.
    ///
    std::string mWhat;

    ///
    /// The start time.
    ///
    std::chrono::time_point<std::chrono::steady_clock> mStartTime;
};

#endif // PT_TIMER_H
