//**************************************************************************************************
//
//  MultiProgCmd.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  This file defines an example application for production line concurrent programming of multiple
//  Qualcomm CDA IC based products.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "spi/TestFlash.h"
#include "PtUtil.h"
#include "PtUi.h"
#include "common/globalversioninfo.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <bitset>
#include <map>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////

// Maximum number of devices
// Note that while the API supports up to 32, we have only tested up to 16.
static const size_t MAX_DEVICES = 16;

// Application return codes
static const int32 PT_CMD_ERROR = -2;
static const int32 PT_CMD_FAIL = -1;
static const int32 PT_CMD_PASS = 0;

// Command-line arguments
static const char* const HELP_OPT = "-help";
static const char* const TRANS_OPT = "-trans";
static const char* const DEV_MASK_OPT = "-devmask";
static const char* const VERIFY_OPT = "-verify";
static const char* const ENABLE_SECURITY_OPT = "-enable_security";
static const char* const DEVICE_ENCRYPTION_OPT = "-device_enc";

// Transport strings
static const char* const TRANS_STR_USBDBG = "USBDBG";
static const char* const TRANS_STR_TRB = "TRB";

static const char* const EXE_NAME = "MultiProgCmd";
#define CREATION_YEAR "2020"

// UI object
static CPtUi& gPtUi = CPtUi::Ref();

static string gImageFile;
static uint32 gDeviceMask = 0;
static uint32 gOpenedDeviceMask = 0;
static string gTransTypeStr = TRANS_STR_USBDBG;
static int32 gTfTransType = TFL_USBDBG;
static bool gVerify = false;
static map<size_t, string> gSecKeys; // Map of device index to security key/file.
static bool gDeviceEnc = false;

////////////////////////////////////////////////////////////////////////////////
// Print the usage information.
////////////////////////////////////////////////////////////////////////////////
static void Usage()
{
    cout << "Qualcomm CDA Multi-device programming tool" << endl;
    cout << endl;
    cout << "Usage:" << endl << endl
         << "    " << EXE_NAME << " " << HELP_OPT << " | (<image_file> [" << TRANS_OPT << " <trans>] [" << DEV_MASK_OPT << " <dev_mask>]" << endl
         << "    [" << VERIFY_OPT << "] [" << ENABLE_SECURITY_OPT << " <key_file>] [" << DEVICE_ENCRYPTION_OPT << "])" << endl;
    cout << endl;
    cout << "    " << HELP_OPT << endl
         << "        Shows the usage information and exits." << endl;
    cout << endl;
    cout << "    <image_file> The path of an image file (*.xuv or *.hex) to burn to the devices." << endl;
    cout << endl;
    cout << "    " << TRANS_OPT << endl
         << "        Indicates the transport to use. If unspecified, the default is " << TRANS_STR_USBDBG << "." << endl
         << "        <trans> Transport type to use, e.g. \"" << TRANS_STR_USBDBG << "\" or \"" << TRANS_STR_TRB << "\"." << endl;
    cout << endl;
    cout << "    " << DEV_MASK_OPT << endl
         << "        Indicates the device mask specifying which of up to 16 devices to" << endl
         << "        program. If not specified, all detected devices will be programmed." << endl
         << "        <dev_mask> Device mask specified as a 16bit hex value (optional '0x'" << endl
         << "        prefix)." << endl;
    cout << endl;
    cout << "    " << VERIFY_OPT << endl
         << "        Enables verification after programming." << endl;
    cout << endl;
    cout << "    " << ENABLE_SECURITY_OPT << endl
         << "        Indicates that security (encryption) should be enabled using the specified" << endl
         << "        key file(s). If one file is specified, all devices will have the same key" << endl
         << "        programmed. Multiple files can be specified - in this case the number of" << endl
         << "        files specified must match the number of specified devices (" << DEV_MASK_OPT << endl
         << "        must be specified). The first file given will be programmed to the lowest" << endl
         << "        device index set in the mask, second file to the next highest device index" << endl
         << "        set, and so on." << endl
         << "        <key_file> One or more key files (comma-separated)." << endl;
    cout << endl;
    cout << "    " << DEVICE_ENCRYPTION_OPT << endl
         << "        Indicates that the device will be used to encrypt the image. Supported only" << endl
         << "        when " << ENABLE_SECURITY_OPT << " is specified. Causes a device reset before" << endl
         << "        programming so that the device can be used to encrypt the image." << endl;
    cout << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Parse the command line arguments.
// Returns PT_CMD_ERROR on error, PT_CMD_PASS on success.
////////////////////////////////////////////////////////////////////////////////
static int ParseArgs(int argc, char* argv[])
{
    int retVal = PT_CMD_PASS;

    int argIndex = 1;
    string arg;
    string errMsg;

    // Deal with mandatory argument
    if (argc < 2)
    {
        retVal = PT_CMD_ERROR;
    }
    else
    {
        arg = argv[argIndex];

        if (arg == HELP_OPT)
        {
            Usage();
            exit(PT_CMD_ERROR);
        }
        else if (arg.at(0) == '-')
        {
            retVal = PT_CMD_ERROR;
        }
        else
        {
            gImageFile = arg;
            ++argIndex;

            // Check image exists here to fail early
            if (!PtUtil::FileExists(gImageFile))
            {
                ostringstream msg;
                msg << "Specified image file \"" << gImageFile
                    << "\" cannot be read";
                errMsg = msg.str();
                retVal = PT_CMD_ERROR;
            }
        }
    }

    // Deal with optional arguments
    vector<string> secKeys;

    while (argIndex < argc && retVal == PT_CMD_PASS)
    {
        arg = argv[argIndex];

        // All optional arguments start with '-'
        if (arg.size() < 2 || arg[0] != '-')
        {
            retVal = PT_CMD_ERROR;
        }
        else
        {
            // Convert to lower case
            PtUtil::ToLower(arg);

            if (arg == TRANS_OPT)
            {
                gTransTypeStr = argv[++argIndex];
                // Convert to upper case
                PtUtil::ToUpper(gTransTypeStr);

                if (gTransTypeStr == TRANS_STR_USBDBG)
                {
                    gTfTransType = TFL_USBDBG;
                }
                else if (gTransTypeStr == TRANS_STR_TRB)
                {
                    gTfTransType = TFL_TRB;
                }
                else
                {
                    ostringstream msg;
                    msg << "Invalid value \"" << gTransTypeStr
                        << "\" passed for \"" << arg << "\" argument";
                    errMsg = msg.str();
                    retVal = PT_CMD_ERROR;
                }
            }
            else if (arg == DEV_MASK_OPT)
            {
                uint16 val = 0;
                istringstream iss(argv[++argIndex]);
                if (iss.str().find('-') != string::npos || 
                    !(iss >> hex >> val) || !iss.eof() || val == 0)
                {
                    ostringstream msg;
                    msg << "Invalid value \"" << argv[argIndex]
                        << "\" passed for \"" << arg
                        << "\" argument. Must be a non-zero 16bit hex value.";
                    errMsg = msg.str();
                    retVal = PT_CMD_ERROR;
                }
                else
                {
                    gDeviceMask = val;
                }
            }
            else if (arg == VERIFY_OPT)
            {
                gVerify = true;
            }
            else if (arg == ENABLE_SECURITY_OPT)
            {
                secKeys = QTIL::PtUtil::SplitString(argv[++argIndex], ",");
            }
            else if (arg == DEVICE_ENCRYPTION_OPT)
            {
                gDeviceEnc = true;
            }
            else
            {
                retVal = PT_CMD_ERROR;
            }
        }

        ++argIndex;
    }

    if (retVal == PT_CMD_PASS)
    {
        if (secKeys.size() > 1 && gDeviceMask == 0)
        {
            ostringstream msg;
            msg << DEV_MASK_OPT
                << " must be specified with a non-zero value when per-device security keys are specified";
            errMsg = msg.str();
            retVal = PT_CMD_ERROR;
        }
        else if (secKeys.empty() && gDeviceEnc)
        {
            ostringstream msg;
            msg << DEVICE_ENCRYPTION_OPT << " is not supported unless " << ENABLE_SECURITY_OPT << " is also set";
            errMsg = msg.str();
            retVal = PT_CMD_ERROR;
        }
        else if (!secKeys.empty())
        {
            const bitset<MAX_DEVICES> devMaskBits(gDeviceMask);
            if (secKeys.size() > 1 && devMaskBits.count() != secKeys.size())
            {
                ostringstream msg;
                msg << "If multiple key files are specified, the number of devices specified for "
                    << DEV_MASK_OPT << " (" << devMaskBits.count()
                    << ") must equal the number of security keys specified for " << ENABLE_SECURITY_OPT
                    << " (" << secKeys.size() << ")";
                errMsg = msg.str();
                retVal = PT_CMD_ERROR;
            }
            
            if (retVal == PT_CMD_PASS)
            {
                for (string file : secKeys)
                {
                    // Check exists here to fail early
                    if (!PtUtil::FileExists(file))
                    {
                        ostringstream msg;
                        msg << "Specified security key file \"" << file
                            << "\" cannot be read";
                        errMsg = msg.str();
                        retVal = PT_CMD_ERROR;
                        break;
                    }
                }

                if (retVal == PT_CMD_PASS)
                {
                    // Map deviceIndex to security key file
                    if (secKeys.size() == 1)
                    {
                        // One key used for all
                        gSecKeys.insert(pair<size_t, string>(0, secKeys[0]));
                    }
                    else
                    {
                        for (size_t devIndex = 0, fileIndex = 0; devIndex < MAX_DEVICES; ++devIndex)
                        {
                            if ((1 << devIndex) & gDeviceMask)
                            {
                                gSecKeys.insert(pair<size_t, string>(devIndex, secKeys[fileIndex]));
                                ++fileIndex;
                            }
                        }
                    }
                }
            }
        }
    }

    if (retVal == PT_CMD_ERROR)
    {
        if (!errMsg.empty())
        {
            gPtUi.WriteError(errMsg);
        }
        else
        {
            // General error message
            gPtUi.WriteError("Invalid argument provided");
        }
        gPtUi.Write(""); // Separate usage with a blank line
        Usage();
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// Get the device mask for all connected devices.
// Returns PT_CMD_ERROR on error, PT_CMD_PASS on success.
////////////////////////////////////////////////////////////////////////////////
static int GetConnectedDevicesMask()
{
    int retVal = PT_CMD_PASS;

    uint16 maxLen(256);
    uint16 count(0);
    char* pPortsStr = new char[maxLen];
    char* pTransStr = new char[maxLen]; // The transport option strings (e.g. "SPITRANS=TRB SPIPORT=1")

    int32 status = flmGetAvailablePorts(&maxLen, pPortsStr, pTransStr, &count);
    if (status != TFL_OK && maxLen != 0)
    {
        // Not enough space - resize the storage
        pPortsStr = new char[maxLen];
        pTransStr = new char[maxLen];
        status = flmGetAvailablePorts(&maxLen, pPortsStr, pTransStr, &count);
    }

    vector<size_t> portIds;
    if (status == TFL_OK && count > 0)
    {
        // Split up the comma separated strings of transport options
        vector<string> trans = PtUtil::SplitString(pTransStr, ",");

        const string EXP_PORT_STR = "SPIPORT=";
        string expTransStr = "SPITRANS=";
        expTransStr += gTransTypeStr;

        // Filter out non-matching transports and get the port IDs
        for (vector<string>::iterator iTrans = trans.begin(); iTrans != trans.end();)
        {
            if (iTrans->find(expTransStr) == string::npos)
            {
                iTrans = trans.erase(iTrans);
            }
            else
            {
                // Need to get the port IDs rather than rely on the count
                // as the IDs may be mapped in a mapping file, and therefore
                // could be sparse.
                string::size_type pos = iTrans->find(EXP_PORT_STR);
                if (pos != string::npos)
                {
                    pos += EXP_PORT_STR.length();
                    istringstream iss(iTrans->substr(pos));
                    size_t portId;
                    if (iss >> portId)
                    {
                        portIds.push_back(portId);
                    }
                }
                ++iTrans;
            }
        }

        count = static_cast<uint16>(portIds.size());
    }

    delete[] pPortsStr;
    delete[] pTransStr;

    if (status != TFL_OK)
    {
        gPtUi.WriteError("Failed to get available transports");
        retVal = PT_CMD_ERROR;
    }
    else if (count == 0)
    {
        ostringstream msg;
        msg << "No available " << gTransTypeStr << " transports / devices found";
        gPtUi.WriteError(msg.str());
        retVal = PT_CMD_ERROR;
    }
    else if (count > MAX_DEVICES)
    {
        ostringstream msg;
        msg << "Devices found (" << count << ") exceeds maximum ("
             << MAX_DEVICES << ")";
        gPtUi.WriteError(msg.str());
    }
    else
    {
        for (vector<size_t>::const_iterator iPortId = portIds.begin();
            iPortId != portIds.end();
            ++iPortId)
        {
            // Need to subtract one from the port ID to get the bit shift
            // value as port IDs for TRB and USBDBG start from 1.
            gDeviceMask = gDeviceMask | (1 << (*iPortId - 1));
        }

        ostringstream msg;
        msg << "Number of " << gTransTypeStr << " transports / devices found = "
             << count << endl;
        msg << "Device mask = 0x" << hex << gDeviceMask;
        gPtUi.Write(msg.str());
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// Setup the programming operation.
// Returns one of the PT_CMD* exit codes defined in this file.
////////////////////////////////////////////////////////////////////////////////
int Setup()
{
    static const uint16 SUBSYS_ID_APPS = 4;

    int retVal = PT_CMD_PASS;

    if (flmReadProgramFiles(gImageFile.c_str()) != TFL_OK)
    {
        gPtUi.WriteError("Failed to read image file");
        retVal = PT_CMD_ERROR;
    }

    if (retVal == PT_CMD_PASS && flmSetFlashType(gDeviceMask, TFL_TYPE_SQIF) != TFL_OK)
    {
        gPtUi.WriteError("Failed to set flash type");
        retVal = PT_CMD_ERROR;
    }

    if (retVal == PT_CMD_PASS && flmSetSubsysChipSel(gDeviceMask, SUBSYS_ID_APPS, 0) != TFL_OK)
    {
        gPtUi.WriteError("Failed to set subsystem and chip select");
        retVal = PT_CMD_ERROR;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// Enables device security, adjusting gDeviceMask for any failures.
// aResetAfter: If true devices are reset after security is enabled.
// Returns a bitmask specifying any failed devices.
////////////////////////////////////////////////////////////////////////////////
uint32 EnableSecurity(bool aResetAfter)
{
    static const uint32 ENCRYPTION_ONLY_OPT = 1;

    uint32 failedDevs = 0;

    gPtUi.Write("Enabling device security...");

    uint32 singleDeviceMask;
    for (size_t devIndex = 0; devIndex < MAX_DEVICES; ++devIndex)
    {
        singleDeviceMask = (1 << devIndex);
        if (singleDeviceMask & gDeviceMask)
        {
            bool fail = false;

            // If code is running from flash, we can't enable security (at least for secure provisioning).
            // Do a flash identify to get into the right state.
            uint32 sectors, sizeMbits, manId, devId;
            if (flmGetDeviceFlashInfoEx(static_cast<uint32>(devIndex), &sectors, &sizeMbits, &manId, &devId) != TFL_OK)
            {
                ostringstream msg;
                msg << "Failed to get flash info for device index " << devIndex;
                gPtUi.Write(msg.str());
                fail = true;
            }

            // Write the key
            const string keyFile = (gSecKeys.size() == 1 ? gSecKeys[0] : gSecKeys[devIndex]);
            if (!fail && flmSetSecurityKey(singleDeviceMask, keyFile.c_str()) != TFL_OK)
            {
                ostringstream msg;
                msg << "Failed to write security key for device index " << devIndex;
                gPtUi.Write(msg.str());
                fail = true;
            }

            // Set the enable bit
            if (!fail && flmEnableSecurity(singleDeviceMask, ENCRYPTION_ONLY_OPT) != TFL_OK)
            {
                ostringstream msg;
                msg << "Failed to enable security key for device index " << devIndex;
                gPtUi.Write(msg.str());
                fail = true;
            }

            if (fail)
            {
                failedDevs |= singleDeviceMask;
            }
        }
    }

    if (failedDevs != 0)
    {
        ostringstream msg;
        msg << "Failed to enable security for device(s) (0x" << hex << failedDevs << dec << ")";
        gPtUi.Write(msg.str());

        gDeviceMask &= ~failedDevs;
    }

    if (aResetAfter && gDeviceMask != 0)
    {
        // Reset needed to activate security on the device - close causes reset
        flmClose(gOpenedDeviceMask);

        // Wait for reset to complete, inc. reenumeration for USBDBG
        gPtUi.Write("Waiting for device reset...");
        if (gTfTransType == TFL_USBDBG)
        {
            Sleep(12000);
        }
        else // using TRB, does not disconnect but will take a minimum time to boot
        {
            Sleep(1000);
        }

        if (flmOpen(gDeviceMask, 0, gTfTransType) != TFL_OK)
        {
            const uint32 openErrDevs = gDeviceMask & flmGetBitErrorField();
            ostringstream msg;
            msg << "Failed to re-open device(s) (0x" << hex << openErrDevs << dec << ")";
            gPtUi.Write(msg.str());

            failedDevs |= openErrDevs;
            gDeviceMask &= ~failedDevs;
        }

        gOpenedDeviceMask = gDeviceMask; // Save this so we can close all opened devices.
    }

    if (gDeviceMask == 0)
    {
        gPtUi.WriteError("All devices failed security enable process");
    }
    else
    {
        if (failedDevs != 0)
        {
            ostringstream maskMsg;
            maskMsg << "Adjusted device mask = 0x" << hex << gDeviceMask;
            gPtUi.Write(maskMsg.str());
        }
    }

    return failedDevs;
}

////////////////////////////////////////////////////////////////////////////////
// Starts the programming operation.
// Returns one of the PT_CMD* exit codes defined in this file.
////////////////////////////////////////////////////////////////////////////////
int StartProgramming()
{
    int retVal = PT_CMD_PASS;

    // Start the programming operation
    // No need for reset after programming as reset happens when we close the connection anyway
    if (flmProgramSpawn(gDeviceMask, 0, (gVerify ? 1 : 0), 0) != TFL_OK)
    {
        gPtUi.WriteError("Failed to spawn flash programming thread");
        retVal = PT_CMD_ERROR;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// Waits for the operation to complete.
////////////////////////////////////////////////////////////////////////////////
void WaitForCompletion()
{
    // These chararcters are cycled through to show a spinning "wheel" while
    // programming is proceeding. This indicates that there is activity even
    // if the progress percentage isn't moving for some time.
    static const vector<char> ACTIVE_INDICATORS = { '\\', '|', '/', '-' };

    size_t devicesRunning;
    size_t progLoopCount = 0;

    do
    {
        int32 cumulativeProgress = 0;
        devicesRunning = 0;
        size_t numDevices = 0;
        for (uint32 devIndex = 0; devIndex < MAX_DEVICES; ++devIndex)
        {
            // Only check the progress if the device is in the mask
            if ((gDeviceMask >> devIndex) & 1)
            {
                ++numDevices;
                int32 progress = flmGetDeviceProgress(devIndex);
                cumulativeProgress += progress;
                if (progress < 100)
                {
                    ++devicesRunning;
                }
            }
        }

        // Show the combined (average) progress for all devices
        char activeIndicator = ACTIVE_INDICATORS.at(progLoopCount % ACTIVE_INDICATORS.size());
        ostringstream progMsg;
        progMsg << "Programming " << activeIndicator << " Progress = " << (cumulativeProgress / numDevices) << "% "; // Space following to overwrite duplicate % if the progress restarts.
        gPtUi.UpdateLine(progMsg.str());

        this_thread::sleep_for(seconds(1));
        ++progLoopCount;
    } while (devicesRunning > 0);

    // End progress line
    gPtUi.Write("");
}

////////////////////////////////////////////////////////////////////////////////
// Report the results of the operation.
// aNonProgFailures: Bitmask of non-programming failures.
// Returns one of the PT_CMD* exit codes defined in this file.
////////////////////////////////////////////////////////////////////////////////
int ReportResults(uint32 aNonProgFailures)
{
    int retVal = PT_CMD_PASS;

    // Check the status
    int32 error = flmGetLastError();
    uint32 errDevs = gDeviceMask & flmGetBitErrorField();
    ostringstream statusMsg;
    if (error == TFL_OK && aNonProgFailures == 0 && errDevs == 0)
    {
        statusMsg << "PASS: Successfully programmed all devices:";
    }
    else
    {
        retVal = PT_CMD_FAIL;

        // One or more devices failed
        if (errDevs == gDeviceMask)
        {
            statusMsg << "FAIL: Failed to program all devices (error code = "
                      << error << "):";
        }
        else
        {
            statusMsg << "FAIL (partial): Status for each device:";
        }
    }

    gPtUi.WriteStatus(statusMsg.str(), retVal == PT_CMD_PASS);

    // List passed devices
    for (size_t devIndex = 0; devIndex < MAX_DEVICES; ++devIndex)
    {
        if (((gDeviceMask & ~errDevs) >> devIndex) & 1)
        {
            ostringstream msg;
            msg << "    dev" << devIndex << " PASS";
            gPtUi.WriteStatus(msg.str(), true);
        }
    }

    // List failed devices
    errDevs |= aNonProgFailures;
    for (size_t devIndex = 0; devIndex < MAX_DEVICES; ++devIndex)
    {
        if ((errDevs >> devIndex) & 1)
        {
            ostringstream msg;
            msg << "    dev" << devIndex << " FAIL";
            gPtUi.WriteStatus(msg.str(), false);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// Command-line application entry point.
// Returns one of the PT_CMD* exit codes defined in this file.
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Show everything
    gPtUi.SetQuiet(false);
    
    // Print app version information
    ostringstream appInfoMsg;
    appInfoMsg << EXE_NAME << ", version " << VERSION_APP_STR << endl;
    // If the special build string is set then show the date of the build
    if (strlen(VERSION_SPECIAL_BUILD) > 0)
    {
        appInfoMsg << VERSION_SPECIAL_BUILD << endl;
    }
    appInfoMsg << (strcmp(CREATION_YEAR, VERSION_YEAR) == 0 ? VERSION_COPYRIGHT_NO_START_YEAR :
                                                              VERSION_COPYRIGHT_START_YEAR(CREATION_YEAR));
    appInfoMsg << endl;
    gPtUi.Write(appInfoMsg.str(), false);

    int retVal = ParseArgs(argc, argv);

    // Count the duration from here on
    auto startTime = steady_clock::now();

    // If the optional device mask hasn't been provided,
    // get the mask for all connected devices.
    if (retVal == PT_CMD_PASS && gDeviceMask == 0)
    {
        retVal = GetConnectedDevicesMask();
    }

    if (retVal == PT_CMD_PASS)
    {
        // Need to track failures for operations where failures are not reported by flmGetBitErrorField(),
        // e.g. connection opening and security operations, any other non-threaded operations.
        uint32 nonProgFailures = 0;
        // Open connections to all specified devices
        if (flmOpen(gDeviceMask, 0, gTfTransType) != TFL_OK)
        {
            nonProgFailures = gDeviceMask & flmGetBitErrorField();
            ostringstream msg;
            msg << "Failed to open device(s) (0x" << hex << nonProgFailures << dec << ")";
            gPtUi.Write(msg.str());

            gDeviceMask &= ~nonProgFailures;
            if (gDeviceMask == 0)
            {
                gPtUi.WriteError("All devices failed to open");
                retVal = PT_CMD_FAIL;
            }
            else
            {
                // We'll continue to try and program the successfully opened devices
                ostringstream maskMsg;
                maskMsg << "Adjusted device mask = 0x" << hex << gDeviceMask;
                gPtUi.Write(maskMsg.str());
            }
        }

        gOpenedDeviceMask = gDeviceMask; // Save this so we can close all opened devices.

        if (retVal == PT_CMD_PASS)
        {
            retVal = Setup();
        }

        if (retVal == PT_CMD_PASS && !gSecKeys.empty())
        {
            // If using device encryption, reset is required after writing the security settings to make them active.
            nonProgFailures |= EnableSecurity(gDeviceEnc);
            if (gDeviceMask == 0)
            {
                retVal = PT_CMD_FAIL;
            }
        }

        if (retVal == PT_CMD_PASS)
        {
            retVal = StartProgramming();
            if (retVal == PT_CMD_PASS)
            {
                WaitForCompletion();

                retVal = ReportResults(nonProgFailures);
            }
        }

        // Closing causes device reset
        flmClose(gOpenedDeviceMask);

        auto endTime = steady_clock::now();
        ostringstream timeMsg;
        timeMsg << "Duration = " << duration_cast<seconds>(endTime - startTime).count()
                << " seconds";
        gPtUi.Write(timeMsg.str(), false);
    }

    return retVal;
}
