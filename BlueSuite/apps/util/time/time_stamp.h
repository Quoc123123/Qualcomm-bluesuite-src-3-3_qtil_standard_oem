// Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// Simple time stamp
// JBS 3 October 2001
// Introduced for marking packets in WinPrimDebug

#ifndef TIME_STAMP_H

#define TIME_STAMP_H

#include "common/countedpointer.h"
#include <string>

class TimeStamp
{
public:
    TimeStamp(); // Records the time it was built
    TimeStamp(const TimeStamp &);
    TimeStamp &operator=(const TimeStamp &);
    ~TimeStamp();

    std::string hms() const; // HH:MM:SS (plus millisecs if available)
    std::string hmsu() const; //HH:MM:SS (plus microsecs if available)

    std::string ymdhm() const; // YYYY-MMM-DD HH:MM

    TimeStamp add_milliseconds(unsigned int msec) const;

    //  make TimeStamp from a system time
    static TimeStamp make(unsigned long);
private:
    class Impl;
    CountedPointer<Impl> ptr;
    TimeStamp(class Impl *);
};

#endif
