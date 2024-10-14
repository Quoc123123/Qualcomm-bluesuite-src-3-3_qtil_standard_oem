//*******************************************************************************
//
//  HidDfuDeviceLoader.h
//
//  Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for CHidDfuDeviceLoader class.
//
//*******************************************************************************

#ifndef HID_DFU_DEVICE_LOADER_H
#define HID_DFU_DEVICE_LOADER_H

#include "HidDfuDevice.h"
#include "HidDfuErrorMsg.h"
#include "common/types.h"

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <string>

///
/// Class API for HidDfu Device (Loader specific functions).
///
class CHidDfuDeviceLoader : public CHidDfuDevice
{
public:
    CHidDfuDeviceLoader();
    CHidDfuDeviceLoader(HANDLE aDeviceHandle, USHORT aFeatureReportLength);
    ~CHidDfuDeviceLoader();

    ///
    /// Upgrade the device.
    /// @return The result.
    ///
    virtual int32 DoDeviceUpgrade();

    ///
    /// Backup the device.
    /// @return The result.
    ///
    virtual int32 DoDeviceBackup();

    ///
    /// Get the firmware version from the device.
    /// @param[out] aVersionMajor Version Major
    /// @param[out] aVersionMinor Version Minor
    /// @param[out] aConfigVersion Config Version
    /// @return The result.
    ///
    virtual int32 GetDevFirmwareVersion(
        uint16 &aVersionMajor, uint16 &aVersionMinor, uint16 &aConfigVersion);

    ///
    /// Initialise.
    /// @return The result.
    ///
    virtual int32 Initialise();

    ///
    /// Send command to DFU mode BlueCore to carry out a reset, resulting 
    /// in the device exiting DFU mode.
    /// @param[in] aFromOperationThread Indicates whether the call is from
    /// an operation thread or another source. If set to false, the function
    /// will fail if an operation thread is active, therefore it must be set
    /// to true if called from an operation thread.
    /// @return The result.
    ///
    virtual int32 ResetDevice(bool aFromOperationThread);

    ///
    /// Writes to the connected device.
    /// @param[in] apBuffer Write buffer.
    /// @param[in] aLength Length of the write buffer in bytes.
    /// @return The result.
    ///
    virtual int32 WriteToDevice(const uint8* apBuffer, uint32 aLength);

private:

    // File suffix structure.
    // The DFU suffix is defined as offsets from the end of the file, which
    // is why the fields are listed in the opposite of expected order and
    // why the "DFU" string for ucDfuSignature is reversed
    // 
    // uint16 bcdDevice         0x0001      (not used) This can be used to hold the stack software build integer
    // uint16 idProduct         0xFFFE      USB Product ID
    // uint16 idVendor          0x0A12      USB Vendor ID
    // uint16 bcdDfu            0x0100      DFU specification number (1.0)
    // uint8 ucDfuSignature[3]  "UFD"       DFU Signature string
    // uint8 bLength            0x10        DFU Suffix length
    // uint32 dwCRC                         Added in code, not a constant
    struct DfuFileSuffix
    {
        uint16  bcdDevice;
        uint16  idProduct;
        uint16  idVendor;
        uint16  bcdDfu;
        uint8   ucDfuSignature[3];
        uint8   bLength;
        uint32  dwCrc;
    };

    /// Standard DFU file suffix data (that expected from a DFU file)
    static const DfuFileSuffix STANDARD_DFU_SUFFIX;

    /// Length of file suffix without CRC (because the CRC calculation doesn't include itself)
    static const size_t DFU_SUFFIX_WITHOUT_CRC_LEN = sizeof(DfuFileSuffix) - sizeof(uint32);

    // Various class constants
    static const size_t DFU_HID_FROM_HOST_HEADER_SIZE = 6;
    static const size_t DFU_HID_TO_HOST_HEADER_SIZE = DFU_HID_FROM_HOST_HEADER_SIZE;

    static const uint8 DFU_HID_REPORTID_UPGRADE = 1;
    static const uint8 DFU_HID_REPORTID_STATUS = 2;
    static const uint8 DFU_HID_REPORTID_STATE = 3;
    static const uint8 DFU_B_REQUEST = 1;

    static const uint32 DFU_HID_BYTE_MULTIPLIER = 0x100;
    static const uint32 DFU_HID_WORD_MULTIPLIER = 0x10000;

    static const uint8 DFU_CMD_CLRSTATUS = 4;
    static const uint8 DFU_CMD_RESET = 255;

    static const size_t DFU_HID_REPORTID_STATUS_SIZE = 7; ///< 6 bytes of the status plus report ID
    static const size_t DFU_HID_REPORTID_STATE_SIZE = 2; ///< 1 byte of state and the report ID

    // Status values returned by DFU_GETSTATUS request.
    // These are defined in the USB Device Firmware Upgrade Specification 1.0.
    static const uint8 DFU_STATUS_OK                = 0x00;    // No error condition present
    //static const uint8 DFU_STATUS_ERR_TARGET        = 0x01;    // File is not targeted fo use by this device
    //static const uint8 DFU_STATUS_ERR_FILE          = 0x02;    // File is for this device but fails some vendor-specific verification test
    //static const uint8 DFU_STATUS_ERR_WRITE         = 0x03;    // Device is unable to write memory
    //static const uint8 DFU_STATUS_ERR_ERASE         = 0x04;    // Memory erase function failed
    //static const uint8 DFU_STATUS_ERR_CHECK_ERASED  = 0x05;    // Memory erase check failed
    //static const uint8 DFU_STATUS_ERR_PROG          = 0x06;    // Program memory function failed
    //static const uint8 DFU_STATUS_ERR_VERIFY        = 0x07;    // Programmed memory failed verification
    //static const uint8 DFU_STATUS_ERR_ADDRESS       = 0x08;    // Cannot program memory due to received address that is out of range
    //static const uint8 DFU_STATUS_ERR_NOTDONE       = 0x09;    // Received DFU_DNLOAD with wLength = 0, but device does not think that it has the data yet
    static const uint8 DFU_STATUS_ERR_FIRMWARE      = 0x0A;    // Device's firmware is corrupt. It cannot return to run-time (non DFU) operation
    //static const uint8 DFU_STATUS_ERR_VENDOR        = 0x0B;    // iString indicates vendor specific error
    //static const uint8 DFU_STATUS_ERR_USBR          = 0x0C;    // Device detected unexpected USB reset signalling
    //static const uint8 DFU_STATUS_ERR_POR           = 0x0D;    // Device detected unexpected power on reset
    //static const uint8 DFU_STATUS_ERR_UNKNOWN       = 0x0E;    // Something went wrong, but the device does not know what it was
    //static const uint8 DFU_STATUS_ERR_STALLEDPKT    = 0x0F;    // Device stalled in an unexpected request

    // Values for DFU state
    static const uint8 DFU_STATE_APP_IDLE                = 0;
    //static const uint8 DFU_STATE_APP_DETACH              = 1;
    static const uint8 DFU_STATE_DFU_IDLE                = 2;
    //static const uint8 DFU_STATE_DFU_DNLOAD_SYNC         = 3;
    static const uint8 DFU_STATE_DFU_DNBUSY              = 4;
    static const uint8 DFU_STATE_DFU_DNLOAD_IDLE         = 5;
    //static const uint8 DFU_STATE_DFU_MANIFEST_SYNC       = 6;
    //static const uint8 DFU_STATE_DFU_MANIFEST            = 7;
    //static const uint8 DFU_STATE_DFU_MANIFEST_WAIT_RESET = 8;
    //static const uint8 DFU_STATE_DFU_UPGRADE_IDLE        = 9;
    //static const uint8 DFU_STATE_DFU_ERROR               = 10;

    /// Max wait period for GetStatus (the chip tells us how long to wait)
    DWORD mBwPollTimeout;

    /// Feature report length
    USHORT mFeatureReportLength;

    ///
    /// Writes the DFU write header to a data buffer.
    /// @param[out] apBuffer Data buffer.
    /// @param[in] aWValue Block number for DFU download.
    /// @param[in] aWLength Total amount of bytes to be written.
    ///
    void AddWriteHeader(uint8 * apBuffer, uint16 aWValue, uint16 aWLength);

    ///
    /// Checks whether a DFU upgrade has completed successfully or not.
    /// @param[in] aWaitForFullTimeout true to wait for the full device timeout before 
    /// getting the status, false to try early, then wait for the full timeout if busy.
    /// @return The result.
    ///
    int32 CheckUpgradeComplete(bool aWaitForFullTimeout = false);

    ///
    /// Clears the device status
    /// @return The result.
    ///
    int32 ClearDeviceStatus();

    ///
    /// Sends a download request to the device with the payload length set 
    /// to zero, in order to signal that there is no more upgrade data. 
    /// This is described in the USB Device Firmware Upgrade Specification.
    /// Checks that the upgrade completed successfully.
    /// @param[in,out] apBuffer Write buffer.
    /// @param[in] aBlockNumber DFU block number.
    /// @return The result.
    ///
    int32 EndUpgrade(uint8 *apBuffer, uint16 aBlockNumber);

    ///
    /// Gets the DFU file length from a file data buffer.
    /// @param[out] apBuffer Data buffer.
    /// @return The file length.
    ///
    uint32 GetDfuFileLength(const uint8 *apBuffer) const;

    ///
    /// Gets the current DFU status from the device.
    /// @param[out] apStatus DFU status value (one of DFU_STATUS_*).
    /// @param[out] apState DFU state value (one of DFU_STATE_*).
    /// @return The result.
    ///
    int32 GetStatus(uint8* apStatus, uint8* apState);

    ///
    /// Reads information from connected device.
    /// @param[out] apBuffer Data read buffer.
    /// @param[out] aBufferLength Data read buffer length (number of bytes).
    /// @param[out] apReplyLength Length of data read from device (number of bytes).
    /// @return The result.
    ///
    int32 HidReportIdBackup(uint8* apBuffer, uint16 aBufferLength, uint16 *apReplyLength);

    ///
    /// Writes DFU information to the connected device. Adds the write 
    /// header, and adjusts the data left to write and block number values
    /// after writing the given block.
    /// @param[in] aPayloadLength Payload length in bytes.
    /// @param[in,out] apBuffer Write buffer.
    /// @param[in,out] apDataLeftToWrite DFU data left to write.
    /// @param[in,out] apBlockNumber DFU block number.
    /// @return The result.
    ///
    int32 HidSetFeatureWithHeader(int32 aPayloadLength, uint8* apBuffer, 
        int64* apDataLeftToWrite, uint16* apBlockNumber);
};

#endif // #ifndef HID_DFU_DEVICE_LOADER_H