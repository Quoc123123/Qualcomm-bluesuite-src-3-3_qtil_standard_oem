 /**********************************************************************
  *
  *  performance_measurement.h
  *
  *  Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
  *  All Rights Reserved.
  *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
  *
  *  Performance measurement object for logging duration of operations.
  *
  ***********************************************************************/

#ifndef __PERFORMANCE_MEASUREMENT_H__
#define __PERFORMANCE_MEASUREMENT_H__

// Usage notes:
//  To log durations in functions, include this file and define
//  PERFORMANCE_MEASUREMENT in the relevant code file (or project settings).
//
//  Use the PERFORMANCE_CHECK_START macro to start the performance check 
//  (arguments are an object name and a string describing the operation 
//  being measured). The start time is printed to stdout.
//
//  When the object created with PERFORMANCE_CHECK_START goes out of scope,
//  the destructor prints the finish time to stdout. Alternatively, the 
//  finish time can be logged explicitly using PERFORMANCE_CHECK_FINISH
//  (takes object name argument).
//
//  PERFORMANCE_CHECK_MARK can be used to time-stamp intermediate log points.
//  

#ifndef PERFORMANCE_MEASUREMENT

#define PERFORMANCE_CHECK_START(obj, tag)
#define PERFORMANCE_CHECK_FINISH(obj)
#define PERFORMANCE_CHECK_MARK(obj)

#else

#define PERFORMANCE_CHECK_START(obj, tag) CPerformanceMeasurement obj(tag);
#define PERFORMANCE_CHECK_FINISH(obj) obj.FinishTime();
#define PERFORMANCE_CHECK_MARK(obj) obj.MarkTime();

#include "stop_watch.h"

class CPerformanceMeasurement 
    : public StopWatch
{
public:
    CPerformanceMeasurement(const char* apTag);
    ~CPerformanceMeasurement();

    void MarkTime();
    void FinishTime();

private:
    bool mFinished;
    const char* mpTag;
    unsigned int mMark;
};

// Inline methods
inline CPerformanceMeasurement::CPerformanceMeasurement(const char* apTag)
    : mFinished(false), mpTag(apTag), mMark(0), StopWatch()
{
    printf("[TIMING] %s {start}\n", mpTag);
}

inline CPerformanceMeasurement::~CPerformanceMeasurement()
{
    // If we didn't already finish, do it now
    if (!mFinished)
    {
        FinishTime();
    }
}

inline void CPerformanceMeasurement::MarkTime()
{
    printf("[TIMING] %s {#%u} %ums\n", mpTag, mMark++, duration());
}

inline void CPerformanceMeasurement::FinishTime()
{
    printf("[TIMING] %s {finish} %ums\n", mpTag, duration());
    mFinished = true;
}

#endif // PERFORMANCE_MEASUREMENT

#endif // __PERFORMANCE_MEASUREMENT_H__

