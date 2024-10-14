//**************************************************************************************************
//
//  AnritsuMT8852BTester.cpp
//
//  Copyright (c) 2020-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class definition for an Anritsu MT8852B tester, part of an example application for production 
//  test.
//
//**************************************************************************************************

#include "AnritsuMT8852BTester.h"
#include "../../PtUtil.h"
#include <sstream>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace QTIL;

#ifdef NOGPIB
// Disable C4702 (Unreachable code) in this configuration otherwise there are
// lots of those due to CGpibInterface methods throwing "GPIB not implemented"
// exceptions.
#pragma warning( disable : 4702 )
#endif

////////////////////////////////////////////////////////////////////////////////

CAnritsuMT8852BTester::CAnritsuMT8852BTester(const std::string& aPortId)
: CSpectrumAnalyserInstrument(aPortId), mCentreFreqHz(0)
{
}

////////////////////////////////////////////////////////////////////////////////

CAnritsuMT8852BTester::~CAnritsuMT8852BTester()
{
}

////////////////////////////////////////////////////////////////////////////////

void CAnritsuMT8852BTester::Initialise()
{
    int32 primaryGpibAddr;
    istringstream portStr(mPortId);
    if (!(portStr >> primaryGpibAddr) || !portStr.eof())
    {
        ostringstream msg;
        msg << "Spectrum analyser port ID parameter \"" << mPortId
            << "\" is not a valid GPIB address";
        throw CSpecAnException(msg.str());
    }

    // Open connection
    mGpibIf.Open(primaryGpibAddr);

    // Reset the instrument
    mGpibIf.Write("*RST");
    mGpibIf.Write("*CLS");

    // Select RF analyser
    mGpibIf.Write("OPMD CWMEAS");

    // Use auto-ranging for input power
    mGpibIf.Write("SYSCFG CONFIG,RANGE,AUTO");
}

////////////////////////////////////////////////////////////////////////////////

uint32 CAnritsuMT8852BTester::MeasureFrequencyHz()
{
    if (mCentreFreqHz == 0)
    {
        throw CSpecAnException("Centre frequency has not been set");
    }

    // Allow settling time before taking the reading
    this_thread::sleep_for(milliseconds(100));

    // read CW offset result
    mGpibIf.Write("CWRESULT FREQOFF");

    string result = mGpibIf.Read();
    
    // Trim newline
    PtUtil::TrimStringEnd(result);

    // Equipment returns the value in E notation, e.g.:
    // "-7.13e+004" = -71300 Hz.
    float64 freqOffsetHz;
    istringstream resStr(result);
    if (!(resStr >> scientific >> freqOffsetHz))
    {
        ostringstream msg;
        msg << "Spectrum analyser frequency offset result \"" << result
            << "\" could not be converted to a number";
        throw CSpecAnException(msg.str());
    }

    return (mCentreFreqHz + static_cast<uint32>(freqOffsetHz));
}

////////////////////////////////////////////////////////////////////////////////

float64 CAnritsuMT8852BTester::MeasurePowerDbm(float64 /*aExpMaxDbm*/)
{
    if (mCentreFreqHz == 0)
    {
        throw CSpecAnException("Centre frequency has not been set");
    }

    // Using auto-ranging, configured during initialisation, so not setting the
    // expected range here, but allow time for auto-ranging before taking the
    // measurement.
    this_thread::sleep_for(milliseconds(500));

    // read power result
    mGpibIf.Write("CWRESULT POWER");

    const string result = mGpibIf.Read();

    // Equipment returns the value in E notation, e.g.:
    // "7.40e+000" = 7.4 dBm.
    float64 powerDbm;
    istringstream resStr(result);
    if (!(resStr >> scientific >> powerDbm))
    {
        ostringstream msg;
        msg << "Spectrum analyser power result \"" << result
            << "\" could not be converted to a number";
        throw CSpecAnException(msg.str());
    }

    return powerDbm;
}

////////////////////////////////////////////////////////////////////////////////

void CAnritsuMT8852BTester::SetCentreFrequency(uint32 aCentreFreqHz)
{
    // CWMEAS FREQ,<freq>,<gate width>
    // Fixing the gate width to the default of 3ms.
    ostringstream command;
    command << "CWMEAS FREQ," << aCentreFreqHz << ",3e-3";
    mGpibIf.Write(command.str());

    mCentreFreqHz = aCentreFreqHz;
}
