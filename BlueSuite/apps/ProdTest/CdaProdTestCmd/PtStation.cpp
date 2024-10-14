//**************************************************************************************************
//
//  PtStation.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  ProdTest station class definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "PtStation.h"
#include "CdaDevice.h"
#include "CdaDevTests.h"
#include "CdaDtsDevice.h"
#include "CdaDtsDevTests.h"
#include "Equipment\AgilentEsaSpectrumAnalyser.h"
#include "Equipment\AgilentN4010ATester.h"
#include "Equipment\AnritsuMT8852BTester.h"
#include "Equipment\ReferenceEndpoint.h"
#include "ChargerDevice.h"
#include "ChargerTests.h"
#include "PtTimer.h"
#include "PtSerialNum.h"
#include "PtSetup.h"
#include "..\PtUtil.h"
#include "Test.h"
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace QTIL;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// CStation
////////////////////////////////////////////////////////////////////////////////

const CStation::SpecAnTypeMap CStation::mSpecAnTypeNameMap = {
    { "AG-ESA", SpecAnalyserType::AG_ESA },
    { "AG-N4010A", SpecAnalyserType::AG_N4010A },
    { "AN-MT8852B", SpecAnalyserType::AN_MT8852B },
    { "Q-REFEP", SpecAnalyserType::Q_REFEP }
};

const CStation::DutTypeMap CStation::mDutTypeNameMap = {
    { "CDA-PCB", DutType::CDA_PCB },
    { "CDA-DTS", DutType::CDA_DTS },
    { "CHARGER-PCB", DutType::CHARGER_PCB }
};

////////////////////////////////////////////////////////////////////////////////

CStation::CStation(const CPtSetup& aSetup)
    : mSetup(aSetup), mpDut(NULL), mUi(CPtUi::Ref()), mpSpecAn(NULL),
      mStopOnFail(true)
{
    Configure();
}

////////////////////////////////////////////////////////////////////////////////

CStation::~CStation()
{
    for each (CTest* pTest in mTests)
    {
        delete pTest;
    }

    delete mpDut;
    delete mpSpecAn;
}

////////////////////////////////////////////////////////////////////////////////

void CStation::Configure()
{
    mUi.SetQuiet(mSetup.GetValueNum<bool>("QuietOutput"));
    
    mLogDirPath = mSetup.GetValue("LogFileDir", true);
    // If directory doesn't exist, try to create it
    if (CreateDirectory(mLogDirPath.c_str(), NULL) == FALSE)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            ostringstream msg;
            msg << "Couldn't create log file directory \"" << mLogDirPath << "\"."
                << " Check intermediate directories exist and that permissions allow writing";
            throw CPtException(msg.str());
        }
    }

    mResultLogFilePath = mLogDirPath + "\\pt_results.txt";

    mStopOnFail = mSetup.GetValueNum<bool>("StopOnFail");

    CreateSpectrumAnalyser();

    DutType dutType = GetDutType();
    CreateDut(dutType);

    // This has to happen after creating equipment and DUT objects
    CreateTests(dutType);
}

////////////////////////////////////////////////////////////////////////////////

bool CStation::RunTests(const CPtSerialNum& aDutSerialNum, const std::string& aDutAddress)
{
    static const string PASS_STR = "PASSED";
    static const string FAIL_STR = "FAILED";
    static const string ERROR_STR = "ERROR";

    bool result = true;

    // Save the start timestamp for logging
    time_t timePoint = chrono::system_clock::to_time_t(chrono::system_clock::now());
    ostringstream timeStr;
    timeStr << put_time(localtime(&timePoint), "%Y%m%d_%H%M%S");
    string startTimestamp = timeStr.str();

    // Don't include these in the try-catch used to log errors to the results
    // log, as any errors are really setup issues, and happen before we have
    // created the test log.
    mpDut->SetSerialNum(&aDutSerialNum);
    mpDut->SetAddress(aDutAddress);

    try
    {
        // Start file logging
        mUi.SetLogFile(GenerateLogFilePath(aDutSerialNum.ToString(), startTimestamp));

        CPtTimer testTimer("Total test");
        ostringstream msg;
        msg << "Testing SN " << aDutSerialNum.ToString();
        mUi.Write(msg.str());

        // Connect to DUT
        mUi.Write("Trying to connect...", false);
        mpDut->Connect();
        mpDut->PreTestActions();

        // Run post-connection checks
        for (vector<CTest*>::const_iterator iTest = mTests.begin();
            iTest != mTests.end(); ++iTest)
        {
            (*iTest)->PostConnectionCheck();
        }

        // Run the tests
        for (vector<CTest*>::const_iterator iTest = mTests.begin();
            iTest != mTests.end() && (result || !mStopOnFail);
            ++iTest)
        {
            result = (*iTest)->Execute() && result;
        }

        mpDut->PostTestActions(result);
        mpDut->Disconnect();

        ostringstream statusMsg;
        statusMsg << "Tests for SN " << aDutSerialNum.ToString() << " ";
        if (result)
        {
            statusMsg << PASS_STR;
        }
        else
        {
            statusMsg << FAIL_STR;
        }

        // Report final status (using separators to make clear it is the final status)
        mUi.Write("", true);
        const string separator(80, '*');
        mUi.WriteStatus(separator, result);
        mUi.WriteStatus(statusMsg.str(), result);
        mUi.WriteStatus(separator, result);

        UpdateResultLog(aDutSerialNum.ToString(), (result ? PASS_STR : FAIL_STR),
            startTimestamp);
    }
    catch (exception)
    {
        UpdateResultLog(aDutSerialNum.ToString(), ERROR_STR, startTimestamp);
        throw;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CStation::GetTestNames() const
{
    return mSetup.GetValueList("Tests", true);
}

////////////////////////////////////////////////////////////////////////////////

void CStation::CreateSpectrumAnalyser()
{
    static const string PORT_ID_SETTING_NAME = "SpecAnPortId";

    SpecAnalyserType type = GetSpecAnalyserType();

    if (type == SpecAnalyserType::Q_REFEP)
    {
        mpSpecAn = new CReferenceEndpoint(mSetup.GetValue(PORT_ID_SETTING_NAME, true));
    }
    else if (type == SpecAnalyserType::AG_ESA)
    {
        mpSpecAn = new CAgilentEsaSpectrumAnalyser(mSetup.GetValue(PORT_ID_SETTING_NAME, true));
    }
    else if (type == SpecAnalyserType::AG_N4010A)
    {
        mpSpecAn = new CAgilentN4010ATester(mSetup.GetValue(PORT_ID_SETTING_NAME, true));
    }
    else if (type == SpecAnalyserType::AN_MT8852B)
    {
        mpSpecAn = new CAnritsuMT8852BTester(mSetup.GetValue(PORT_ID_SETTING_NAME, true));
    }

    if (mpSpecAn != NULL)
    {
        // Initialise the spectrum analyser
        mpSpecAn->Initialise();
    }
}

////////////////////////////////////////////////////////////////////////////////

void CStation::CreateTests(DutType aDutType)
{
    if (aDutType == DutType::CDA_PCB)
    {
        mTests = CCdaDevTestFactory::CreateTests(*this);
    }
    else if (aDutType == DutType::CDA_DTS)
    {
        mTests = CCdaDtsDevTestFactory::CreateTests(*this);
    }
    else if (aDutType == DutType::CHARGER_PCB)
    {
        mTests = CChargerTestFactory::CreateTests(*this);
    }
    else
    {
        // Shouldn't get here, type should already have been checked
        throw CPtException("Unsupported DUT type");
    }
}

////////////////////////////////////////////////////////////////////////////////

void CStation::CreateDut(DutType aType)
{
    if (aType == DutType::CDA_PCB)
    {
        mpDut = new CCdaDevice(mSetup);
    }
    else if (aType == DutType::CDA_DTS)
    {
        mpDut = new CCdaDtsDevice(mSetup);
    }
    else if (aType == DutType::CHARGER_PCB)
    {
        mpDut = new CChargerDevice(mSetup);
    }
    else
    {
        // Shouldn't get here, type should already have been checked
        throw CPtException("Unsupported DUT type");
    }
}

////////////////////////////////////////////////////////////////////////////////

string CStation::GenerateLogFilePath(const string& aDutSerialNumber,
    const std::string& aTimestamp)
{
    ostringstream filePath;
    filePath << mLogDirPath << "\\ptlog_sn" << aDutSerialNumber << "_"
             << aTimestamp << ".txt";

    return filePath.str();
}

////////////////////////////////////////////////////////////////////////////////

CStation::SpecAnalyserType CStation::GetSpecAnalyserType() const
{
    static const string TYPE_SETTING_NAME = "SpectrumAnalyser";

    SpecAnalyserType specAnType = SpecAnalyserType::UNKNOWN;

    string specAnName = mSetup.GetValue(TYPE_SETTING_NAME, false);
    if (!specAnName.empty())
    {
        PtUtil::ToUpper(specAnName);
        SpecAnTypeMap::const_iterator iSpecAn = mSpecAnTypeNameMap.find(specAnName);

        if (iSpecAn != mSpecAnTypeNameMap.end())
        {
            specAnType = iSpecAn->second;
        }
        else
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TYPE_SETTING_NAME << "\" can be one of: "
                << GetNames(mSpecAnTypeNameMap);
            throw CPtException(msg.str());
        }
    }

    return specAnType;
}

////////////////////////////////////////////////////////////////////////////////

CStation::DutType CStation::GetDutType() const
{
    static const string DUT_TYPE_SETTING = "DutType";

    string deviceType = mSetup.GetValue(DUT_TYPE_SETTING, true);
    PtUtil::ToUpper(deviceType);

    DutTypeMap::const_iterator iDut = mDutTypeNameMap.find(deviceType);
    DutType dutType = DutType::UNKNOWN;
    if (iDut != mDutTypeNameMap.end())
    {
        dutType = iDut->second;
    }
    else
    {
        ostringstream msg;
        msg << "Configuration setting \"" << DUT_TYPE_SETTING << "\" can be one of: "
            << GetNames(mDutTypeNameMap);
        throw CPtException(msg.str());
    }

    return dutType;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::string CStation::GetNames(T& aNameTotypeMap) const
{
    ostringstream names;
    for (T::const_iterator it = aNameTotypeMap.begin();
        it != aNameTotypeMap.end();
        ++it)
    {
        names << it->first;
        if (it != prev(aNameTotypeMap.end(), 1))
        {
            names << ", ";
        }
    }

    return names.str();
}

////////////////////////////////////////////////////////////////////////////////

void CStation::UpdateResultLog(const std::string& aDutSerialNumber,
    const std::string& aResult, const std::string& aTimestamp)
{
    ofstream resultFile(mResultLogFilePath, ios_base::app);
    if (!resultFile.good())
    {
        // Don't throw - could be writing error result as result of an exception
        ostringstream msg;
        msg << "Result log file \"" << mResultLogFilePath << "\" cannot be opened for append";
        mUi.WriteError(msg.str());
    }

    resultFile << aDutSerialNumber << "    "
        << aResult << "    "
        << aTimestamp << endl;
}
