//**************************************************************************************************
//
//  PtStation.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  ProdTest station class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_STATION_H
#define PT_STATION_H

#include "PtUi.h"
#include <string>
#include <vector>
#include <map>

class CSpectrumAnalyserInstrument;
class CDut;
class CPtSetup;
class CPtSerialNum;
class CTest;

///
/// Station base class
///
class CStation
{
public:
    ///
    /// Constructor.
    /// @param[in] aSetup The production test setup.
    ///
    explicit CStation(const CPtSetup& aSetup);

    ///
    /// Destructor.
    ///
    ~CStation();

    ///
    /// Configures the station.
    ///
    void Configure();

    ///
    /// Runs the tests.
    /// @param[in] aDutSerialNum The serial number of the DUT.
    /// @param[in] aDutAddress The address of the DUT (e.g. BT address).
    /// @return true if the tests passed, false otherwise.
    /// @throws CPtException.
    ///
    bool RunTests(const CPtSerialNum& aDutSerialNum, const std::string& aDutAddress);

    ///
    /// Gets the station setup.
    /// @return Station setup object.
    ///
    const CPtSetup& GetSetup() const { return mSetup; };

    ///
    /// Gets the device under test.
    /// @return DUT object.
    ///
    CDut* GetDut() const { return mpDut; };

    ///
    /// Gets the spectrum analyser instrument.
    /// @return Pointer to a spectrum analyser.
    ///
    CSpectrumAnalyserInstrument* GetSpectrumAnalyser() const { return mpSpecAn; };

    ///
    /// Gets the ordered list of named tests to run.
    /// @return Test names.
    /// @throws CPtException.
    ///
    std::vector<std::string> GetTestNames() const;

    ///
    /// Gets whether tests should stop after the first fail or continue.
    /// @return true if tests should stop after a fail, false if they should continue.
    ///
    bool StopOnFail() const { return mStopOnFail; };

private:

    ///
    /// Supported DUT types
    ///
    enum class DutType
    {
        UNKNOWN,
        CDA_PCB,
        CDA_DTS,
        CHARGER_PCB
    };

    ///
    /// DUT type map type.
    ///
    typedef std::map<std::string, DutType> DutTypeMap;

    ///
    /// Map of DUT names to type ids.
    ///
    static const DutTypeMap mDutTypeNameMap;

    ///
    /// Spectrum analysers
    ///
    enum class SpecAnalyserType
    {
        UNKNOWN,
        AG_ESA,
        AG_N4010A,
        AN_MT8852B,
        Q_REFEP
    };

    ///
    /// Spectrum Analyser type map type.
    ///
    typedef std::map<std::string, SpecAnalyserType> SpecAnTypeMap;

    ///
    /// Map of spectrum analyser names to type ids.
    ///
    static const SpecAnTypeMap mSpecAnTypeNameMap;

    ///
    /// Copy constructor.
    ///
    CStation(const CStation&);

    ///
    /// Assignment operator.
    ///
    CStation& operator=(const CStation&);

    ///
    /// Creates the spectrum analyser instrument object.
    ///
    void CreateSpectrumAnalyser();

    ///
    /// Creates the test objects.
    /// @param[in] aDutType The DUT type.
    ///
    void CreateTests(DutType aDutType);

    ///
    /// Creates the DUT object.
    /// @param[in] aType The DUT type to create.
    ///
    void CreateDut(DutType aType);

    ///
    /// Generates the Log file path.
    /// @param[in] aDutSerialNumber Serial number of the DUT.
    /// @param[in] aTimestamp The test timestamp to include in the file name.
    ///
    std::string GenerateLogFilePath(const std::string& aDutSerialNumber,
        const std::string& aTimestamp);

    ///
    /// Gets the spectrum analyser type identifier from the settings.
    /// @return The spectrum analyser type.
    /// @throws CPtException.
    ///
    SpecAnalyserType GetSpecAnalyserType() const;

    ///
    /// Gets the DUT type identifier from the settings.
    /// @return The DUT type.
    /// @throws CPtException.
    ///
    DutType GetDutType() const;

    ///
    /// Get the names.
    /// @param[in] aNameTotypeMap Map of strings to an enum type.
    /// @return Names as a comma-separated string.
    ///
    template <typename T>
    std::string GetNames(T& aNameTotypeMap) const;

    ///
    /// Updates the result log file with the result for the DUT.
    /// @param[in] aDutSerialNumber Serial number of the DUT.
    /// @param[in] aResult The result.
    /// @param[in] aTimestamp The timestamp for the test.
    ///
    void UpdateResultLog(const std::string& aDutSerialNumber,
        const std::string& aResult, const std::string& aTimestamp);

    ///
    /// The setup object (production test configuration).
    ///
    const CPtSetup& mSetup;

    ///
    /// The Device Under Test object.
    ///
    CDut* mpDut;

    ///
    /// The user interface object.
    /// Private as derived classes should use the protected functions for UI interaction.
    ///
    CPtUi& mUi;

    ///
    /// Spectrum analyser instrument.
    /// 
    CSpectrumAnalyserInstrument* mpSpecAn;

    ///
    /// Test collection.
    ///
    std::vector<CTest*> mTests;

    ///
    /// Log file directory path.
    ///
    std::string mLogDirPath;

    ///
    /// Result log file path.
    ///
    std::string mResultLogFilePath;

    ///
    /// Whether tests stop after the first failure or not.
    ///
    bool mStopOnFail;
};


#endif // PT_STATION_H
