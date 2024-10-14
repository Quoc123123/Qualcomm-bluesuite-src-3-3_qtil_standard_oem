//**************************************************************************************************
//
//  AgilentN4010ATester.cpp
//
//  Copyright (c) 2020 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class definition for an Agilent N4010A tester, part of an example application for production 
//  test.
//
//**************************************************************************************************

#include "AgilentN4010ATester.h"
#include <sstream>

using namespace std;

#ifdef NOGPIB
// Disable C4702 (Unreachable code) in this configuration otherwise there are
// lots of those due to CGpibInterface methods throwing "GPIB not implemented"
// exceptions.
#pragma warning( disable : 4702 )
#endif

////////////////////////////////////////////////////////////////////////////////

CAgilentN4010ATester::CAgilentN4010ATester(const std::string& aPortId)
: CSpectrumAnalyserInstrument(aPortId), mCentreFreqHz(0)
{
}

////////////////////////////////////////////////////////////////////////////////

CAgilentN4010ATester::~CAgilentN4010ATester()
{
}

////////////////////////////////////////////////////////////////////////////////

void CAgilentN4010ATester::Initialise()
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
    mGpibIf.Write("INST:SEL 'RFA'");
}

////////////////////////////////////////////////////////////////////////////////

uint32 CAgilentN4010ATester::MeasureFrequencyHz()
{
    if (mCentreFreqHz == 0)
    {
        throw CSpecAnException("Centre frequency has not been set");
    }

    // Trigger the measurement
    mGpibIf.Write("INIT");

    // read CW offset result
    mGpibIf.Write("FETCH:FOFF?");

    const string result = mGpibIf.Read();

    // Equipment returns the value in floating point format,
    // but as we're dealing with frequency in Hz, we will convert
    // to uint32.
    float64 freqOffsetHz;
    istringstream resStr(result);
    if (!(resStr >> freqOffsetHz))
    {
        ostringstream msg;
        msg << "Spectrum analyser frequency offset result \"" << result
            << "\" could not be converted to a number";
        throw CSpecAnException(msg.str());
    }

    return (mCentreFreqHz + static_cast<uint32>(freqOffsetHz));
}

////////////////////////////////////////////////////////////////////////////////

float64 CAgilentN4010ATester::MeasurePowerDbm(float64 aExpMaxDbm)
{
    if (mCentreFreqHz == 0)
    {
        throw CSpecAnException("Centre frequency has not been set");
    }

    // Set the power range (expects an integer)
    ostringstream cmd;
    cmd << "SENS:POW:RANG " << static_cast<uint16>(aExpMaxDbm);
    mGpibIf.Write(cmd.str());

    // Trigger the measurement
    mGpibIf.Write("INIT");

    // read power result
    mGpibIf.Write("FETCH:APOW?");

    const string result = mGpibIf.Read();

    float64 powerDbm;
    istringstream resStr(result);
    if (!(resStr >> powerDbm))
    {
        ostringstream msg;
        msg << "Spectrum analyser power result \"" << result
            << "\" could not be converted to a number";
        throw CSpecAnException(msg.str());
    }

    return powerDbm;
}

////////////////////////////////////////////////////////////////////////////////

void CAgilentN4010ATester::SetCentreFrequency(uint32 aCentreFreqHz)
{
    ostringstream command;
    command << "SENS:FREQ:CENT " << aCentreFreqHz << "HZ";
    mGpibIf.Write(command.str());

    mCentreFreqHz = aCentreFreqHz;
}
