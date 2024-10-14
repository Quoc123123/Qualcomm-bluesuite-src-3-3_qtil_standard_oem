//**************************************************************************************************
//
//  AgilentEsaSpectrumAnalyser.cpp
//
//  Copyright (c) 2020 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class definition for an Agilent ESA Spectrum Analyser, part of an example application for
//  production test.
//
//**************************************************************************************************

#include "AgilentEsaSpectrumAnalyser.h"
#include <sstream>

using namespace std;

#ifdef NOGPIB
// Disable C4702 (Unreachable code) in this configuration otherwise there are
// lots of those due to CGpibInterface methods throwing "GPIB not implemented"
// exceptions.
#pragma warning( disable : 4702 )
#endif

////////////////////////////////////////////////////////////////////////////////

CAgilentEsaSpectrumAnalyser::CAgilentEsaSpectrumAnalyser(const std::string& aPortId)
: CSpectrumAnalyserInstrument(aPortId)
{
}

////////////////////////////////////////////////////////////////////////////////

CAgilentEsaSpectrumAnalyser::~CAgilentEsaSpectrumAnalyser()
{
}

////////////////////////////////////////////////////////////////////////////////

void CAgilentEsaSpectrumAnalyser::Initialise()
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

    // Switch off display for higher speed
    mGpibIf.Write("DISP:ENAB OFF");

    // Set sweep speed
    mGpibIf.Write("SENS:SWE:TIME 27.5ms");

    // Set the span to 300KHz (should be more than sufficient for the limits
    // of frequency deviation).
    mGpibIf.Write("FREQ:SPAN 300KHZ");

    // Setup peak search
    mGpibIf.Write("MARK:PEAK:SEAR:MODE MAX");
}

////////////////////////////////////////////////////////////////////////////////

uint32 CAgilentEsaSpectrumAnalyser::MeasureFrequencyHz()
{
    // Run peak search
    mGpibIf.Write("CALC:MARK1:MAX");

    // Query peak search result
    mGpibIf.Write("CALC:MARK1:X?");

    // Read result of peak search
    const string result = mGpibIf.Read();

    // Equipment returns the value in floating point format,
    // but as we're dealing with frequency in Hz, we will convert
    // to uint32.
    float64 frequency;
    istringstream resStr(result);
    if (!(resStr >> frequency))
    {
        ostringstream msg;
        msg << "Spectrum analyser peak search result \"" << result
            << "\" could not be converted to a number";
        throw CSpecAnException(msg.str());
    }

    return static_cast<uint32>(frequency);
}

////////////////////////////////////////////////////////////////////////////////

float64 CAgilentEsaSpectrumAnalyser::MeasurePowerDbm(float64 /*aExpMaxDbm*/)
{
    // Run peak search
    mGpibIf.Write("CALC:MARK1:MAX");

    // Query peak search result
    mGpibIf.Write("CALC:MARK1:Y?");

    // Read result of peak search
    const string result = mGpibIf.Read();

    float64 powerDbm;
    istringstream resStr(result);
    if (!(resStr >> powerDbm))
    {
        ostringstream msg;
        msg << "Spectrum analyser peak search result \"" << result
            << "\" could not be converted to a number";
        throw CSpecAnException(msg.str());
    }

    return powerDbm;
}

////////////////////////////////////////////////////////////////////////////////

void CAgilentEsaSpectrumAnalyser::SetCentreFrequency(uint32 aCentreFreqHz)
{
    ostringstream command;
    command << "FREQ:CENT " << aCentreFreqHz << "HZ";
    mGpibIf.Write(command.str());
}
