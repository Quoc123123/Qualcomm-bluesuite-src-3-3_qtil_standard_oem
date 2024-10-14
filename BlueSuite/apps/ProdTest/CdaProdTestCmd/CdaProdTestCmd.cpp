//**************************************************************************************************
//
//  CdaProdTestCmd.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  This file defines an example application for production line test of Qualcomm CDA IC based
//  products.
//
//**************************************************************************************************

#include "common/types.h"
#include "common/globalversioninfo.h"
#include "PtException.h"
#include "PtUi.h"
#include "PtUtil.h"
#include "PtSetup.h"
#include "PtSerialNum.h"
#include "PtStation.h"
#include <string>
#include <iostream>
#include <sstream>

using namespace QTIL;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

// Application return codes
static const int32 PT_CMD_ERROR = -2;
static const int32 PT_CMD_FAIL = -1;
static const int32 PT_CMD_PASS = 0;

// Default setup file
static const char* const DEFAULT_SETUP_FILE = "ptsetup.txt";

// Command-line arguments
static const char* const HELP_OPT = "-help";
static const char* const BD_ADDR_OPT = "-bdaddr";
static const char* const DEV_SN_OPT = "-sernum";
static const char* const SETUP_OPT = "-setup";

static const char* const EXE_NAME = "CdaProdTestCmd";
#define CREATION_YEAR "2020"

// UI object
static CPtUi& gPtUi = CPtUi::Ref();

static string gSerNum;
static string gBdAddress;
static string gSetupFile;

////////////////////////////////////////////////////////////////////////////////
// Print the usage information.
////////////////////////////////////////////////////////////////////////////////
static void Usage(void)
{
    const string EXE_NAME_STR(EXE_NAME);

    cout << "Qualcomm CDA Product Production Test Tool" << endl;
    cout << endl;
    cout << "Usage:" << endl << endl
         << "    " << EXE_NAME_STR << " [" << HELP_OPT << "] | (" << DEV_SN_OPT << " <ser_num> [" << BD_ADDR_OPT << " <bd_addr>]" << endl
         << "    " << string(EXE_NAME_STR.length(), ' ') << " [" << SETUP_OPT << " <file_path>])" << endl;
    cout << endl;
    cout << "    " << HELP_OPT << endl
         << "        Shows the usage information and exits." << endl;
    cout << endl;
    cout << "    " << DEV_SN_OPT << " <ser_num>" << endl
         << "        Indicates the serial number of the device." << endl
         << "        <ser_num> Serial number as a string, e.g. \"463551\"." << endl
         << "        The required length and format are specified in the setup file." << endl;
    cout << endl;
    cout << "    " << BD_ADDR_OPT << " <bd_addr>" << endl
         << "        Indicates the Bluetooth device address to set (valid only if a " << endl
         << "        Bluetooth address file is not specified in the setup file)." << endl
         << "        <bd_addr> Bluetooth address as a string of 12 hex digits with optional" << endl
         << "        \"0x\" prefix, e.g. \"0x00025b123456\"." << endl;
    cout << endl;
    cout << "    " << SETUP_OPT << " <file_path>" << endl
        << "        Path to a setup file. If not specified, the file \"" << DEFAULT_SETUP_FILE << "\" must be" << endl
        << "        present in the working directory." << endl
        << "        <file_path> Setup file path." << endl;
    cout << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Parse the command line arguments.
// Returns PT_CMD_ERROR on error, PT_CMD_PASS on success.
////////////////////////////////////////////////////////////////////////////////
static int ParseArgs(int argc, char* argv[])
{
    int retVal = PT_CMD_PASS;

    for (int argIndex = 1; argIndex < argc && retVal == PT_CMD_PASS; ++argIndex)
    {
        string arg = argv[argIndex];

        if (arg.size() < 2 || arg[0] != '-')
        {
            retVal = PT_CMD_ERROR;
        }
        else
        {
            // Convert to lower case
            PtUtil::ToLower(arg);

            if (arg == HELP_OPT)
            {
                Usage();
                exit(PT_CMD_ERROR);
            }
            else
            {
                if (argc < argIndex + 2)
                {
                    retVal = PT_CMD_ERROR;
                }
                else
                {
                    ++argIndex;
                    if (arg == DEV_SN_OPT)
                    {
                        gSerNum = argv[argIndex];
                    }
                    else if (arg == BD_ADDR_OPT)
                    {
                        gBdAddress = argv[argIndex];
                    }
                    else if (arg == SETUP_OPT)
                    {
                        gSetupFile = argv[argIndex];
                    }
                    else
                    {
                        retVal = PT_CMD_ERROR;
                    }
                }
            }
        }
    }

    if (retVal == PT_CMD_ERROR || gSerNum.empty())
    {
        if (retVal == PT_CMD_ERROR)
        {
            gPtUi.WriteError("Invalid argument provided");
        }
        else if (gSerNum.empty())
        {
            ostringstream msg;
            msg << DEV_SN_OPT << " must be specified";
            gPtUi.WriteError(msg.str());
            
            retVal = PT_CMD_ERROR;
        }
        
        gPtUi.Write("");
        Usage();
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// Command-line application entry point.
// Returns one of the PT_CMD* exit codes defined in this file.
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
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

    if (retVal == PT_CMD_PASS)
    {
        if (gSetupFile.empty() && !PtUtil::FileExists(DEFAULT_SETUP_FILE))
        {
            ostringstream msg;
            msg << "If " << SETUP_OPT << " is not specified, the file \""
                << DEFAULT_SETUP_FILE << "\" must be present in the working directory";
            gPtUi.WriteError(msg.str());
            retVal = PT_CMD_ERROR;
        }
        else
        {
            try
            {
                // Load the setup
                CPtSetup setup(gSetupFile.empty() ? DEFAULT_SETUP_FILE : gSetupFile);

                CPtSerialNum serNum(gSerNum, setup);

                CStation station(setup);
                bool pass = station.RunTests(serNum, gBdAddress);
                if (!pass)
                {
                    retVal = PT_CMD_FAIL;
                }
            }
            catch (const CPtException& ex)
            {
                // An error (not a test fail) has occurred
                ostringstream msg;
                msg << ex.what() << endl;
                msg << "Tests ABORTED";
                gPtUi.WriteError(msg.str());
                retVal = PT_CMD_ERROR;
            }
            catch (const exception& ex)
            {
                // Shouldn't get any exceptions outside of our defined classes,
                // so if we do, flag as unexpected.
                ostringstream msg;
                msg << "Unexpected internal exception: " << ex.what() << endl;
                msg << "Tests ABORTED";
                gPtUi.WriteError(msg.str());
                retVal = PT_CMD_ERROR;
            }
        }
    }

    return retVal;
}
