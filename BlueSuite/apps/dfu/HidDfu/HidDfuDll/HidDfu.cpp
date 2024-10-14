//*******************************************************************************
//
//  HidDfu.cpp
//
//  Copyright (c) 2012-2018 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implements the HidDfu API (Device Firmware Update using HID driver)
//
//*******************************************************************************

#include "HidDfu.h"
#include "HidDfuEngine.h"
#include "common/dynamicversioninfo.h"

// Internal engine object
static CHidDfuEngine gHidDfuEngine;

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuGetFirmwareVersions(char* versionString,
                                           uint16* maxLength, uint8 checkMatch)
{
    return gHidDfuEngine.GetFirmwareVersion(versionString, maxLength, (checkMatch != 0));
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuGetVersion(uint16* major, uint16* minor,
                                   uint16* release, uint16* build)
{
    if(0 != major)
    {
        *major = VERSION_APP_MAJOR;
    }
    if(0 != minor)
    {
        *minor = VERSION_APP_MINOR;
    }
    if(0 != release)
    {
        *release = VERSION_APP_REVISION;
    }
    if(0 != build)
    {
        *build = VERSION_APP_BUILD;
    }

    return HIDDFU_ERROR_NONE;
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuConnect(uint16 vid, uint16 pid, uint16 usage,
                                uint16 usagePage, uint16* count)
{
    return gHidDfuEngine.ConnectDevice(vid, pid, usage, usagePage, count);
}

////////////////////////////////////////////////////////////////////////////////

HIDDFU_API(uint8) hidDfuGetFailedDevicesCount(void)
{
    return gHidDfuEngine.GetFailedDevicesCount();
}

////////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuDisconnect(void)
{
    return gHidDfuEngine.DisconnectDevice();
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuBackup(const char* fileName, uint8 resetAfter)
{
    return gHidDfuEngine.Backup(fileName, resetAfter);
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuUpgrade(const char* fileName, uint8 resetAfter)
{
    return gHidDfuEngine.Upgrade(fileName, resetAfter);
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuUpgradeBin(const char* fileName)
{
    return gHidDfuEngine.UpgradeBin(fileName);
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuStop(uint16 waitForStopMs)
{
    return gHidDfuEngine.StopOperation(waitForStopMs);
}

//////////////////////////////////////////////////////////////////////////////
HIDDFU_API(int32) hidDfuResetDevice(void)
{
    return gHidDfuEngine.ResetDevice();
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(uint8) hidDfuGetProgress(void)
{
    return gHidDfuEngine.GetProgress();
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuGetResult(void)
{
    return gHidDfuEngine.GetResult();
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(const char*) hidDfuGetLastError(void)
{
    return gHidDfuEngine.GetLastError();
}

//////////////////////////////////////////////////////////////////////////////

HIDDFU_API(int32) hidDfuSendCommand(const uint8* data, uint32 length)
{
    return gHidDfuEngine.WriteToDevice(data, length);
}

//////////////////////////////////////////////////////////////////////////////
