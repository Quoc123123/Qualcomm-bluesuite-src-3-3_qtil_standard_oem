//**************************************************************************************************
//
//  ReferenceEndpoint.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class definition for a reference endpoint, part of an example application for
//  production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ReferenceEndpoint.h"
#include "hci\TestEngine.h"
#include <sstream>
#include <iomanip>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

CReferenceEndpoint::CReferenceEndpoint(const std::string& aPortId)
: CSpectrumAnalyserInstrument(aPortId),
  mTeHandle(TE_INVALID_HANDLE_VALUE), mCentreFreqHz(0)
{
}

////////////////////////////////////////////////////////////////////////////////

CReferenceEndpoint::~CReferenceEndpoint()
{
    if (mTeHandle != TE_INVALID_HANDLE_VALUE)
    {
        // Stop radiotest mode
        radiotestPause(mTeHandle);

        closeTestEngine(mTeHandle);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CReferenceEndpoint::Initialise()
{
    mTeHandle = openTestEngine(USB, mPortId.c_str(), 0, 5000, 1000);
    if (mTeHandle == TE_INVALID_HANDLE_VALUE)
    {
        ostringstream msg;
        msg << "Failed to connect to Reference Endpoint \"" << mPortId << "\"";
        throw CSpecAnException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

uint32 CReferenceEndpoint::MeasureFrequencyHz()
{
    uint32 frequencyHz;

    if (mTeHandle == TE_INVALID_HANDLE_VALUE)
    {
        throw CSpecAnException("Reference Endpoint not connected");
    }
    else if (mCentreFreqHz == 0)
    {
        throw CSpecAnException("Centre frequency has not been set");
    }

    float64 offsetPpm;
    if (get_freq_offset(mTeHandle, &offsetPpm, SAMPLE_SIZE) == TE_OK)
    {
        const float64 offsetHz = ((float64)mCentreFreqHz * offsetPpm) / 1000000;

        frequencyHz = static_cast<uint32>(mCentreFreqHz + offsetHz);
    }
    else
    {
        throw CSpecAnException("Reference Endpoint get_freq_offset failed");
    }

    return frequencyHz;
}

////////////////////////////////////////////////////////////////////////////////

float64 CReferenceEndpoint::MeasurePowerDbm(float64 /*aExpMaxDbm*/)
{
    static const uint16 RSSI_SAMPLE_SIZE = 10;

    if (mTeHandle == TE_INVALID_HANDLE_VALUE)
    {
        throw CSpecAnException("Reference Endpoint not connected");
    }
    else if (mCentreFreqHz == 0)
    {
        throw CSpecAnException("Centre frequency has not been set");
    }

    const uint16 freqMhz = static_cast<uint16>(mCentreFreqHz / 1000000);
    float64 powerDbm;

    if (radiotestRxstart2(mTeHandle, freqMhz, 0, 0, RSSI_SAMPLE_SIZE) == TE_OK)
    {
        uint16 rssiSamples[RSSI_SAMPLE_SIZE];
        // Approx. 10 samples per second, plus some headroom
        int32 timeoutMs = 500 + (100 * RSSI_SAMPLE_SIZE);
        if (hqGetRssi(mTeHandle, timeoutMs, RSSI_SAMPLE_SIZE, rssiSamples) != TE_OK)
        {
            throw CSpecAnException("Reference Endpoint hqGetRssi failed");
        }

        // Take the mean of the measured RSSI values
        uint32 rssiSum = 0;
        for (size_t i = 0; i < RSSI_SAMPLE_SIZE; i++)
        {
            rssiSum += rssiSamples[i];
        }
        float64 averageChipRssi = static_cast<float64>(rssiSum) / RSSI_SAMPLE_SIZE;

        // Convert to power using reference endpoint calibration data
        if (refEpGetRssiDbm(mTeHandle, freqMhz, averageChipRssi, &powerDbm) != TE_OK)
        {
            ostringstream msg;
            msg << "Reference Endpoint refEpGetRssiDbm failed with frequency = " << freqMhz
                << " MHz, chip RSSI value = " << fixed << setprecision(2) << averageChipRssi;
            throw CSpecAnException(msg.str());
        }

    }
    else
    {
        throw CSpecAnException("Reference Endpoint radiotestRxstart2 failed");
    }

    return powerDbm;
}

////////////////////////////////////////////////////////////////////////////////

void CReferenceEndpoint::SetCentreFrequency(uint32 aCentreFreqHz)
{
    if (mTeHandle == TE_INVALID_HANDLE_VALUE)
    {
        throw CSpecAnException("Reference Endpoint not connected");
    }

    // Required to get the radio into the right state for offset measurements
    const uint16 freqMhz = static_cast<uint16>(aCentreFreqHz / 1000000);
    if (radiotestRxdata1(mTeHandle, freqMhz, 0, 0) != TE_OK)
    {
        throw CSpecAnException("Reference Endpoint radiotestRxdata1 failed");
    }

    mCentreFreqHz = aCentreFreqHz;
}
