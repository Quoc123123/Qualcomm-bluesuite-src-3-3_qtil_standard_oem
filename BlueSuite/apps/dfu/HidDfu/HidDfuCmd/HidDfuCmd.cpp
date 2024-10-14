//*******************************************************************************
//
//  HidDfuCmd.cpp
//
//  Copyright (c) 2012-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Command line application for Device Firmware Update using HID driver
//
//*******************************************************************************

#include <stdarg.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

#include "HidDfuDll/HidDfu.h"
#include "common/globalversioninfo.h"
#include "time\hi_res_clock.h"

//////////////////////////////////////////////////////////////////////////////

typedef enum
{
    COMMAND_INVALID,
    COMMAND_HELP,
    COMMAND_UPGRADE,
    COMMAND_BACKUP,
    COMMAND_UPGRADEBIN
} Command;

static int32 RunCommand(Command aCommand, const std::string& aFilename, uint8 aResetAfter);
static uint16 GetHexValue(const char *apCommandStr, const char *apValueStr);
static void ShowUsageMessage(const char *apFormat, ...);
static void ShowVersionMessage(const uint16 aDeviceCount, const char *apMessage, bool aCheckMatch);
static void CheckNumArguments(int aArgc, int aArgIndex, int aExpectedArguments, const char *apCommandStr);
static void ErrorInsufficientArguments(int aGivenArguments, int aExpectedArguments, const char *apCommandStr);
static void CheckFirstOccurrence(bool aTooMany, const char *apOption);
static void Usage();

//////////////////////////////////////////////////////////////////////////////

static const char *gpExeName = "HidDfuCmd";

static const int NUM_COMMAND_ARGUMENTS    = 5; // VID, PID, usage, usagePage, fileName

static const char *COMMAND_BACKUP_STR       = "backup";
static const char *COMMAND_UPGRADE_STR      = "upgrade";
static const char *COMMAND_UPGRADE_BIN_STR  = "upgradebin";

// Define leading character for options
static const char OPTION_MARKER_CH        = '-';

static const char *OPTION_NO_RESET_STR    = "-noreset";

// Update/Backup all devices
static const char *OPTION_ALL_STR    = "-all";

// Ask for help
static const char *OPTION_HELP1_STR    = "-help";
static const char *OPTION_HELP2_STR    = "-?";

static const int HIDDFUCMD_SUCCESS = 0;
static const int HIDDFUCMD_ERROR   = 1;
static const int HIDDFUCMD_ABORT   = 2;

// For CSRA681xx, QCC302x-8x and QCC512x-8x devices usagePage must be 0xFF00.
static const int32 USAGE_PAGE_APP = 0xFF00;

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    int retVal = HIDDFUCMD_SUCCESS;

    gpExeName = argv[0];
    
    // Print app version information
    std::cout << gpExeName << ", version " << VERSION_APP_STR << std::endl;

    // If the special build string is set then show the date of the build
    if (strlen(VERSION_SPECIAL_BUILD) > 0)
    {
        std::cout << VERSION_SPECIAL_BUILD << std::endl;
    }
    std::cout << VERSION_COPYRIGHT_START_YEAR("2012") << std::endl << std::endl;

    Command command = COMMAND_INVALID;
    std::string commandStr;
    bool restartAfter = true; // Default behaviour is to reset after completing the operation
    bool operateAll = false; // Default behaviour is to query the user

    int argIndex = 1; // First application argument

    if (argc > 1)
    {
        // Process the command line options
        while(argIndex < argc && argv[argIndex][0] == OPTION_MARKER_CH)
        {
            std::string arg = argv[argIndex];
            std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);

            if (arg.compare(OPTION_NO_RESET_STR) == 0)
            {
                ++argIndex;
                CheckFirstOccurrence(restartAfter == false, OPTION_NO_RESET_STR);
                restartAfter = false;
            }
            else if (arg.compare(OPTION_ALL_STR) == 0)
            {
                ++argIndex;
                CheckFirstOccurrence(operateAll == true, OPTION_ALL_STR);
                operateAll = true;
            }
            else if (arg.compare(OPTION_HELP1_STR) == 0 ||
                     arg.compare(OPTION_HELP2_STR) == 0)
            {
                command = COMMAND_HELP;
                Usage();
            }
            else
            {
                ShowUsageMessage("\"%s\" invalid option", argv[argIndex]);
            }
        }

        if (argIndex < argc)
        {
            // Process the command
            commandStr = argv[argIndex];
            // Ensure command is all lower case
            std::transform(commandStr.begin(), commandStr.end(), commandStr.begin(), ::tolower);

            if (commandStr.compare(COMMAND_BACKUP_STR) == 0)
            {
                command = COMMAND_BACKUP;
            }
            else if (commandStr.compare(COMMAND_UPGRADE_STR) == 0)
            {
                command = COMMAND_UPGRADE;
            }
            else if (commandStr.compare(COMMAND_UPGRADE_BIN_STR) == 0)
            {
                command = COMMAND_UPGRADEBIN;

                // Upgrade Binary process restarts as part of the upgrade
                // so a second restart is not needed
                restartAfter = false;
            }
        }
    }

    if (command == COMMAND_HELP)
    {
        // Do nothing
    }
    else if (command == COMMAND_INVALID)
    {
        ShowUsageMessage("Invalid command");
    }
    else
    {
        // Process the command arguments
        CheckNumArguments(argc, argIndex, NUM_COMMAND_ARGUMENTS, commandStr.c_str());

        // Move to first command parameter argument
        ++argIndex;

        // Get the command parameters, incrementing the index along the way
        const uint16 vid = GetHexValue(commandStr.c_str(), argv[argIndex++]);
        const uint16 pid = GetHexValue(commandStr.c_str(), argv[argIndex++]);
        const uint16 usage = GetHexValue(commandStr.c_str(), argv[argIndex++]);
        const uint16 usagePage = GetHexValue(commandStr.c_str(), argv[argIndex++]);
        const std::string fileName = argv[argIndex];

        int32 errVal = HIDDFU_ERROR_NONE;

        uint16 count = 0;

        if ((command == COMMAND_UPGRADEBIN) && (usagePage != USAGE_PAGE_APP))
        {
            retVal = HIDDFUCMD_ABORT;
            std::cout << "Error: usagePage should be 0x" << std::hex << std::uppercase << USAGE_PAGE_APP 
                    << " for upgradebin command (CSRA681xx, QCC302x-8x and QCC512x-8x devices)" << std::endl;
        }
        else
        {
            // Connect to the HID Devices
            errVal = hidDfuConnect(vid, pid, usage, usagePage, &count);

            // Upgrade/Backup all devices ?
            if (errVal != HIDDFU_ERROR_NONE)
            {
                std::cout << "Error connecting: " << hidDfuGetLastError() << std::endl;
                retVal = HIDDFUCMD_ABORT;
            }
            else if ((count > 0) && !(operateAll))
            {
                char userChoice = '\0';
                while (userChoice != 'y' && userChoice != 'Y' &&
                    userChoice != 'n' && userChoice != 'N')
                {
                    std::cout << "Found " << count << " devices. Do you want to "
                        << commandStr.c_str() << " all? (y/n) ";
                    std::cin >> userChoice;
                }

                if (userChoice == 'n' || userChoice == 'N')
                {
                    retVal = HIDDFUCMD_ABORT;
                    std::cout << "Aborted" << std::endl;
                }
            }
        }

        if (retVal == HIDDFUCMD_SUCCESS)
        {
            if (command == COMMAND_UPGRADEBIN)
            {
                ShowVersionMessage(count, "before upgrade", false);
            }

            // Run the command
            errVal = RunCommand(command, fileName, restartAfter);

            // Report command completion status
            if ((errVal == HIDDFU_ERROR_OPERATION_PARTIAL_SUCCESS)
                    || (errVal == HIDDFU_ERROR_OP_PARTIAL_SUCCESS_NO_RESPONSE))
            {

                std::cout << "\rError running " << commandStr.c_str() << " command: "
                    << "error on " << static_cast<uint16>(hidDfuGetFailedDevicesCount()) << " device(s)." << std::endl;

                if ((command == COMMAND_UPGRADEBIN)
                        && (errVal == HIDDFU_ERROR_OPERATION_PARTIAL_SUCCESS))
                {
                    ShowVersionMessage(count, "after upgrade", true);
                }
            }
            else if (errVal != HIDDFU_ERROR_NONE)
            {
                std::cout << "\rError running " << commandStr.c_str() << " command: "
                          << hidDfuGetLastError() << std::endl;
            }
            else
            {
                std::cout << "\r" << commandStr.c_str() << " succeeded" << std::endl;

                if (restartAfter)
                {
                    std::cout << "Device reset succeeded" << std::endl;
                }

                if (command == COMMAND_UPGRADEBIN)
                {
                    ShowVersionMessage(count, "after upgrade", true);
                }
            }

            // Disconnect
            int32 disconnectResult = hidDfuDisconnect();
            if (disconnectResult != HIDDFU_ERROR_NONE)
            {
                std::cout << "Error disconnecting: " << hidDfuGetLastError() << std::endl;
                errVal = disconnectResult;
            }

            // Set the application return code
            if (errVal != HIDDFU_ERROR_NONE)
            {
                retVal = HIDDFUCMD_ERROR;
            }
        }
        else if (retVal != HIDDFUCMD_ABORT)
        {
            // Connection failed, report the error
            std::cout << "Error: " << hidDfuGetLastError() << std::endl;
            retVal = HIDDFUCMD_ERROR;
        }
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// Runs and command, reports progress
//////////////////////////////////////////////////////////////////////////////
static int32 RunCommand(Command aCommand, const std::string& aFilename, uint8 aResetAfter)
{
    int32 retVal = HIDDFU_ERROR_NONE;

    // Run the command
    if (aCommand == COMMAND_UPGRADE)
    {
        retVal = hidDfuUpgrade(aFilename.c_str(), aResetAfter); 
    }
    else if (aCommand == COMMAND_BACKUP)
    {
        retVal = hidDfuBackup(aFilename.c_str(), aResetAfter);
    }
    else if (aCommand == COMMAND_UPGRADEBIN)
    {
        retVal = hidDfuUpgradeBin(aFilename.c_str());
    }

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Wait for completion, notify progress
        uint8 progress;

        uint8 isWaitMsgPrinted = false;

        while((progress = hidDfuGetProgress()) != 100)
        {
            if (isWaitMsgPrinted == false)
            {
                if ((progress == PROGRESS_REBOOT_VALUE) && (aCommand == COMMAND_UPGRADEBIN))
                {   // The sequence of Upgrading CSRA681xx, QCC302x-8x and QCC512x-8x devices with binary image
                    // necessitates restart after the image has been copied, the device is reconnected to
                    // after a short delay, so display a relevant message for the restart duration.
                    isWaitMsgPrinted = true;
                    std::cout << "\r" << "Device(s) rebooting, waiting for " << RESTART_DELAY_SEC << " seconds..." << std::endl;
                }
                else
                {   // Display the progress
                    std::cout << "\r" << static_cast<uint16>(progress) << "% completed" << std::flush;
                }
            }

            HiResClockSleepMilliSec(500);
            retVal = hidDfuGetResult();
        }
    }

    // Check again for result incase the above while loop did not execute
    if (retVal == HIDDFU_ERROR_NONE)
    {
        retVal = hidDfuGetResult();
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// Converts a hex number, making sure it is in valid format
//////////////////////////////////////////////////////////////////////////////
static uint16 GetHexValue(const char *apCommandStr, const char *apValueStr)
{
    char *pErr;
    const uint16 val = (uint16)strtoul(apValueStr, &pErr, 16);
    if (*pErr != '\0')
    {
        // A conversion error has occurred
        ShowUsageMessage("\"%s\" value must be hexadecimal, got \"%s\"", apCommandStr, apValueStr);
    }
    return val;
 }

//////////////////////////////////////////////////////////////////////////////
// Display a message followed by the usage information
//////////////////////////////////////////////////////////////////////////////
static void ShowUsageMessage(const char *apFormat, ...)
{
    std::cout << "========================================================================" << std::endl;
    std::cout << "== Error: ";

    va_list argptr;
    va_start(argptr, apFormat);
    vprintf(apFormat, argptr);
    va_end(argptr);

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    Usage();
}

//////////////////////////////////////////////////////////////////////////////
// Display device firmware version information
//////////////////////////////////////////////////////////////////////////////
static void ShowVersionMessage(const uint16 aDeviceCount, const char *apMessage, bool aCheckMatch)
{
    static const size_t COL_SIZE = 20;

    // Calculate length to allocate memory for -
    //      uint16 versionMajor, uint16 versionMinor, uint16 configVersion, ...
    // A uint16 will take maximum 5 characters, so for 3 uint16's allocate 15 character
    // and each uint16 number is followed by a comma/semicolon so allocate another 3 for 3 uint16 numbers
    // which makes it a total of 18 character for 1 device.
    const uint16 multiplier = 18;
    // Allocate an additional byte for NULL character.
    uint16 length = (aDeviceCount * multiplier) + 1;

    char *pVersionBuffer = new char[length];
    memset(pVersionBuffer, 0, length);

    // Get the firmware version
    int32 retVal = hidDfuGetFirmwareVersions(pVersionBuffer, &length, aCheckMatch);

    std::stringstream s;

    // Test if versions match
    if (aCheckMatch == true && retVal == HIDDFU_ERROR_VERSION_MISMATCH)
    {
        s << std::endl;
        s << "--------------------------------------------------------------------------";
        s << std::endl;
        s << "WARNING: Version mismatch on the devices upgraded.";
        s << std::endl;
        s << "--------------------------------------------------------------------------";
        s << std::endl;
    }

    if (((retVal == HIDDFU_ERROR_NONE) || (retVal == HIDDFU_ERROR_VERSION_MISMATCH)) 
            && (pVersionBuffer != NULL))
    {
        // Header row
        s << std::endl << std::endl;
        s << "DEVICE INFORMATION (" << apMessage << ")";
        s << std::endl;
        s << "--------------------------------------------------------------------------";
        s << std::endl;
        s << std::setw(COL_SIZE) << std::left << "Device Number";
        s << std::setw(COL_SIZE) << std::left << "Version Major";
        s << std::setw(COL_SIZE) << std::left << "Version Minor";
        s << std::setw(COL_SIZE) << std::left << "Config Version";
        s << std::endl;
        s << "--------------------------------------------------------------------------";
        s << std::endl;

        int counter = 0;
        char *pDevToken = strtok(pVersionBuffer, ",");

        // Print version
        while (pDevToken != NULL)
        {
            // Insert device number initially, and
            // after printing set of 3 numbers reperesenting version information
            if ((counter == 0) || (counter % 3 == 0))
            {
                s << std::setw(COL_SIZE) << std::left << (counter/3) + 1;
            }

            s << std::setw(COL_SIZE) << std::left << pDevToken;
            pDevToken = strtok(NULL, ",;");

            // After printing set of 3 numbers reperesenting version information,
            // insert a new line
            if ((counter + 1) % 3 == 0)
            {
                s << std::endl;
            }
            counter++;
        }

        s << std::endl << std::endl;
        std::cout << s.str();
    }
    delete[] pVersionBuffer;
}

//////////////////////////////////////////////////////////////////////////////
// Checks the correct number of arguments have been given for a command
//////////////////////////////////////////////////////////////////////////////
static void CheckNumArguments(int aArgc, int aArgIndex, int aExpectedArguments, const char *apCommandStr)
{
    const int remainingArguments = aArgc - aArgIndex;
    if (remainingArguments != aExpectedArguments + 1)
    {
        ErrorInsufficientArguments(remainingArguments - 1, aExpectedArguments, apCommandStr);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Report insufficient arguments, usage and abort
//////////////////////////////////////////////////////////////////////////////
static void ErrorInsufficientArguments(int aGivenArguments, int aExpectedArguments, const char *apCommandStr)
{
    ShowUsageMessage("\"%s\" takes %d arguments, got %d", apCommandStr, aExpectedArguments, aGivenArguments);
}

//////////////////////////////////////////////////////////////////////////////
// Report a command line option as being specified more than once, output 
// usage and abort.
//////////////////////////////////////////////////////////////////////////////
static void CheckFirstOccurrence(bool aTooMany, const char *apOption)
{
    if (aTooMany)
    {
        ShowUsageMessage("\"%s\" should be specified only once", apOption);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Output usage information
//////////////////////////////////////////////////////////////////////////////
static void Usage()
{
    std::cout << gpExeName << "  [-noreset] backup|upgrade|upgradebin <vid> <pid> <usage> <usagePage> " << std::endl;
    std::cout << "                                     <fileName>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -noreset - Prevents device reset before exit." << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "    backup - Performs a backup." << std::endl;
    std::cout << "    upgrade - Performs an upgrade." << std::endl;
    std::cout << "    upgradebin - Performs an upgrade with a binary file." << std::endl;
    std::cout << "Command Arguments:" << std::endl;
    std::cout << "    vid - USB Vendor ID." << std::endl;
    std::cout << "    pid - USB Product ID." << std::endl;
    std::cout << "    usage - USB Usage value. Can be set to zero to ignore." << std::endl;
    std::cout << "    usagePage - USB Usage Page value. Can be set to zero to ignore." << std::endl;
    std::cout << "    fileName - DFU file name." << std::endl;
    std::cout << "Note:" << std::endl;
    std::cout << "    The options -noreset, backup and upgrade are only applicable to BlueCore ICs." << std::endl;
    std::cout << "    The option upgradebin is only applicable to CSRA681xx, QCC302x-8x and QCC512x-8x devices." << std::endl;
    std::cout << "    For upgradebin the following apply:" << std::endl;
    std::cout << "        usage should be 0x1 for QCC304x-8x and QCC514x-8x devices." << std::endl;
    std::cout << "        usagePage should be 0x" << std::hex << std::uppercase << USAGE_PAGE_APP << "." << std::endl;
    std::cout << std::endl;

    // Always exit program on usage
    exit(HIDDFUCMD_ERROR);
}
