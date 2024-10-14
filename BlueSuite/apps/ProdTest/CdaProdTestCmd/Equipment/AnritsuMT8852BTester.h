//**************************************************************************************************
//
//  AnritsuMT8852BTester.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class declaration for an Anritsu MT8852B tester, part of an example application for production 
//  test.
//
//**************************************************************************************************

#ifndef ANRITSU_MT8852B_TESTER_H
#define ANRITSU_MT8852B_TESTER_H

#include "SpectrumAnalyserInstrument.h"
#include "GpibInterface.h"

///
/// Spectrum Analyser class for the Anritsu MT8852B
///
class CAnritsuMT8852BTester : public CSpectrumAnalyserInstrument
{
public:
    ///
    /// Constructor.
    /// @param[in] aPortId Connection port identifier (GPIB primary address).
    ///
    explicit CAnritsuMT8852BTester(const std::string& aPortId);

    ///
    /// Destructor.
    ///
    virtual ~CAnritsuMT8852BTester();

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
    /// GPIB interface object used for communication.
    ///
    CGpibInterface mGpibIf;

    ///
    /// Centre frequency in Hz (stored in order to calculate frequency from
    /// offset measurement).
    ///
    uint32 mCentreFreqHz;
};

#endif // ANRITSU_MT8852B_TESTER_H
