//**************************************************************************************************
//
//  SpectrumAnalyserInstrument.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Spectrum Analyser Instrument class declaration, part of an example application for
//  production test.
//
//**************************************************************************************************

#ifndef SPECTRUM_ANALYSER_INSTRUMENT_H
#define SPECTRUM_ANALYSER_INSTRUMENT_H

#include "common/types.h"
#include "PtException.h"

///
/// Spectrum analyser exception class
///
class CSpecAnException : public CPtException
{
public:
    ///
    /// Constructor
    /// @param[in] aMessage The exception message.
    ///
    explicit CSpecAnException(const std::string& aMessage) : CPtException(aMessage) {};
};

///
/// Abstract class for something allowing (restricted) spectrum analyser functionality.
///
class CSpectrumAnalyserInstrument
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CSpectrumAnalyserInstrument() {};

    ///
    /// Initialise the instrument.
    /// @throws CSpecAnException.
    ///
    virtual void Initialise() = 0;

    ///
    /// Measure the frequency.
    /// @return The frequency in Hz.
    /// @throws CSpecAnException.
    ///
    virtual uint32 MeasureFrequencyHz() = 0;

    ///
    /// Measure the power.
    /// @param[in] aExpMaxDbm The expected maximum input power in dBm.
    /// @return The power in dBm.
    /// @throws CSpecAnException.
    ///
    virtual float64 MeasurePowerDbm(float64 aExpMaxDbm) = 0;

    ///
    /// Set the centre frequency for measurements.
    /// @param[in] aCentreFreqHz The centre frequency in Hz.
    /// @throws CSpecAnException.
    ///
    virtual void SetCentreFrequency(uint32 aCentreFreqHz) = 0;

protected:
    ///
    /// Constructor.
    /// @param[in] aPortId Connection port identifier.
    ///
    explicit CSpectrumAnalyserInstrument(const std::string& aPortId) : mPortId(aPortId) { };

    ///
    /// The instrument connection port identifier.
    ///
    const std::string mPortId;
};

#endif // SPECTRUM_ANALYSER_INSTRUMENT_H
