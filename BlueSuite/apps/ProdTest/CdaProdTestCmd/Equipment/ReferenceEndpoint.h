//**************************************************************************************************
//
//  ReferenceEndpoint.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class declaration for a reference endpoint, part of an example application for
//  production test.
//
//**************************************************************************************************

#ifndef REFERENCE_ENDPOINT_H
#define REFERENCE_ENDPOINT_H

#include "SpectrumAnalyserInstrument.h"

///
/// Spectrum Analyser class for the reference endpoint
///
class CReferenceEndpoint : public CSpectrumAnalyserInstrument
{
public:
    ///
    /// Constructor.
    /// @param[in] aPortId Connection port identifier (USB device id, e.g. "\\\\.\\csr0").
    ///
    explicit CReferenceEndpoint(const std::string& aPortId);

    ///
    /// Destructor.
    ///
    virtual ~CReferenceEndpoint();

    ///
    /// Initialise the instrument.
    /// @throws CSpecAnException.
    ///
    void Initialise() override;

    ///
    /// Measure the frequency.
    /// @return The frequency in Hz.
    /// @throws CSpecAnException.
    ///
    uint32 MeasureFrequencyHz() override;

    ///
    /// Measure the power.
    /// @param[in] aExpMaxDbm The expected maximum input power in dBm.
    /// @return The power in dBm.
    /// @throws CSpecAnException.
    ///
    float64 MeasurePowerDbm(float64 aExpMaxDbm) override;

    ///
    /// Set the centre frequency for measurements.
    /// @param[in] aCentreFreqHz The centre frequency in Hz.
    /// @throws CSpecAnException.
    ///
    void SetCentreFrequency(uint32 aCentreFreqHz) override;

private:
    ///
    /// Number of register reads averaged to get an offset measurement.
    ///
    static const int32 SAMPLE_SIZE = 100;

    ///
    /// TestEngine handle for a reference endpoint device.
    ///
    uint32 mTeHandle;

    ///
    /// Centre frequency in Hz (stored in order to calculate frequency from
    /// offset measurement).
    ///
    uint32 mCentreFreqHz;
};

#endif // REFERENCE_ENDPOINT_H
