//*******************************************************************************
//
//  UpgradeProtocol.h
//
//  Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class interface for sending Upgrade Protocol messages over HID
//
//*******************************************************************************

#ifndef UPGRADE_PROTOCOL_H
#define UPGRADE_PROTOCOL_H

#include "common/types.h"

#include <vector>


///
/// The device may record that part of the upgrade process was completed.
/// In the case of an interruption and restart of the process,
/// the device may specify from which point to resume the message exchange.
/// The host shall implement all of the resume points.
///
typedef enum
{
    UPGRADE_RESUME_POINT_START = 0,     // equivalent to not having a resume point
    UPGRADE_RESUME_POINT_PRE_VALIDATE,  // set when data transfer is complete
    UPGRADE_RESUME_POINT_PRE_REBOOT,    // set when validation of image is complete
    UPGRADE_RESUME_POINT_POST_REBOOT,   // device rebooted to use new image, but not yet committed
    UPGRADE_RESUME_POINT_POST_COMMIT    // new image has been comitted
} UpgradeResumePoint;

/// Upgrade Status as received in response to Connection Request
typedef enum
{
    UPGRADE_STATUS_SUCCESS = 0,         // Operation succeeded
    UPGRADE_STATUS_UNEXPECTED_ERROR,    // Operation failed
    UPGRADE_STATUS_ALREADY_CONNECTED_WARNING,       // Already connected
    UPGRADE_STATUS_IN_PROGRESS,         // Requested operation failed, an upgrade is in progress
    UPGRADE_STATUS_BUSY,                // UNUSED
    UPGRADE_STATUS_INVALID_POWER_STATE  // Invalid power management state
} UpgradeStatus;

typedef enum
{
    UPGRADE_START_REQ               =   0x01,
    UPGRADE_START_CFM               =   0x02,
    UPGRADE_DATA_BYTES_REQ          =   0x03,
    UPGRADE_DATA                    =   0x04,
    UPGRADE_ABORT_REQ               =   0x07,
    UPGRADE_ABORT_CFM               =   0x08,
    UPGRADE_TRANSFER_COMPLETE_IND   =   0x0B,
    UPGRADE_TRANSFER_COMPLETE_RES   =   0x0C,
    UPGRADE_PROCEED_TO_COMMIT       =   0x0E,
    UPGRADE_COMMIT_REQ              =   0x0F,
    UPGRADE_COMMIT_CFM              =   0x10,
    UPGRADE_ERROR_IND               =   0x11,
    UPGRADE_COMPLETE_IND            =   0x12,
    UPGRADE_SYNC_REQ                =   0x13,
    UPGRADE_SYNC_CFM                =   0x14,
    UPGRADE_START_DATA_REQ          =   0x15,
    UPGRADE_IS_VALIDATION_DONE_REQ  =   0x16,
    UPGRADE_IS_VALIDATION_DONE_CFM  =   0x17,
    UPGRADE_HOST_VERSION_REQ        =   0x19,
    UPGRADE_HOST_VERSION_CFM        =   0x1A,
    UPGRADE_ERROR_RES               =   0x1F,
} UpgradeProtocolOpCode;


typedef enum {
    UPGRADE_HOST_SUCCESS = 0,
    UPGRADE_HOST_OEM_VALIDATION_SUCCESS,

    UPGRADE_HOST_ERROR_INTERNAL_ERROR_DEPRECATED = 0x10,
    UPGRADE_HOST_ERROR_UNKNOWN_ID,
    UPGRADE_HOST_ERROR_BAD_LENGTH_DEPRECATED,
    UPGRADE_HOST_ERROR_WRONG_VARIANT,
    UPGRADE_HOST_ERROR_WRONG_PARTITION_NUMBER,

    UPGRADE_HOST_ERROR_PARTITION_SIZE_MISMATCH,
    UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_FOUND_DEPRECATED,
    UPGRADE_HOST_ERROR_PARTITION_OPEN_FAILED,
    UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DEPRECATED,
    UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_DEPRECATED,

    UPGRADE_HOST_ERROR_SFS_VALIDATION_FAILED,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_DEPRECATED,
    UPGRADE_HOST_ERROR_UPDATE_FAILED,
    UPGRADE_HOST_ERROR_APP_NOT_READY,

    UPGRADE_HOST_ERROR_LOADER_ERROR,
    UPGRADE_HOST_ERROR_UNEXPECTED_LOADER_MSG,
    UPGRADE_HOST_ERROR_MISSING_LOADER_MSG,

    UPGRADE_HOST_ERROR_BATTERY_LOW,
    UPGRADE_HOST_ERROR_INVALID_SYNC_ID,
    UPGRADE_HOST_ERROR_IN_ERROR_STATE,
    UPGRADE_HOST_ERROR_NO_MEMORY,
    UPGRADE_HOST_ERROR_SQIF_ERASE,

    // The remaining errors are grouped, each section starting at a fixed offset
    UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_PARSE = 0x30,
    UPGRADE_HOST_ERROR_BAD_LENGTH_TOO_SHORT,
    UPGRADE_HOST_ERROR_BAD_LENGTH_UPGRADE_HEADER,
    UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_HEADER,
    UPGRADE_HOST_ERROR_BAD_LENGTH_SIGNATURE,
    UPGRADE_HOST_ERROR_BAD_LENGTH_DATAHDR_RESUME,

    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_HEADERS = 0x38,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_UPGRADE_HEADER,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER1,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER2,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_DATA,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_FOOTER,
    UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_MEMORY,

    UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED = 0x40,
    UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_HEADER,

    // When sent, the error indicates that an upgrade could not be completed 
    // due to concerns about space in Persistent Store.  No other upgrade
    // activities will be possible until the device restarts.
    // This error requires a UPGRADE_HOST_ERRORWARN_RES response (following
    // which the library will cause a restart, if the VM application permits)

    UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_PS_SPACE,

    UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_MATCHING = 0x48,
    UPGRADE_HOST_ERROR_PARTITION_TYPE_TWO_DFU,

    UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_HEADER = 0x50,
    UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DATA,

    UPGRADE_HOST_ERROR_FILE_TOO_SMALL = 0x58,
    UPGRADE_HOST_ERROR_FILE_TOO_BIG,

    UPGRADE_HOST_ERROR_INTERNAL_ERROR_1 = 0x65, // 101 - Human readable decimal
    UPGRADE_HOST_ERROR_INTERNAL_ERROR_2,
    UPGRADE_HOST_ERROR_INTERNAL_ERROR_3,
    UPGRADE_HOST_ERROR_INTERNAL_ERROR_4,
    UPGRADE_HOST_ERROR_INTERNAL_ERROR_5,
    UPGRADE_HOST_ERROR_INTERNAL_ERROR_6,
    UPGRADE_HOST_ERROR_INTERNAL_ERROR_7,

    UPGRADE_HOST_WARN_APP_CONFIG_VERSION_INCOMPATIBLE = 0x80,
    UPGRADE_HOST_WARN_SYNC_ID_IS_DIFFERENT
} UpgradeProtocolErrorCode;

///
/// Constants
///

// HID report descriptor
typedef enum
{
    HID_REPORTID_COMMAND        = 3,
    HID_REPORTID_DATA_TRANSFER  = 5,
    HID_REPORTID_RESPONSE       = 6,
} HidReportId;

static const uint16 HID_CMD_CONNECTION_REQ = 0x02;
static const uint16 HID_CMD_DISCONNECT_REQ = 0x07;

// Size of HID header (1  byte Report Id and 1 byte Length)
// + UpgradeProtocolMsg Header (1 byte opcode and 2 byte length)
// + Data)
static const uint8 HEADER_SIZE = 5;

///
/// Class API for UpgradeProtocol functions
///
class CUpgradeProtocolMsg
{
public:

    CUpgradeProtocolMsg(uint8 aReportId, uint8 aReportLenBytes, uint8 aOpCode, uint16 alength = 0);
    ~CUpgradeProtocolMsg() {};

    CUpgradeProtocolMsg(const uint8 *apBuffer, uint8 aLength);

    void Serialise(uint8 *apBuffer, uint8 aLength) const;

    uint8 GetUpgradeMsgSize() const { return mUpgradeMsgSize; };

    uint8 GetOpCode() const { return mOpCode; };

    void SetLength(uint16 aLength) { mLength = aLength; };

    void SetUpgradeMsgSize(uint8 aUpgradeMsgSize) { mUpgradeMsgSize = aUpgradeMsgSize; };

protected:

    void SetDataUint8(uint8 aIndex, uint8 aValue);

    uint8 GetDataUint8(uint8 aIndex) const;

    void SetDataUint16(uint8 aIndex, uint16 aValue);

    uint16 GetDataUint16(uint8 aIndex) const;

    void SetDataUint32(uint8 aIndex, uint32 aValue);

    uint32 GetDataUint32(uint8 aIndex) const;

    void SetData(uint8 aIndex, const uint8 * apBuffer, uint8 aLength);

private:

    // HID header
    uint8 mReportId;        ///< HID report descriptor
    uint8 mUpgradeMsgSize;  ///< Includes OpCode, Length and data

    // Upgrade Protocol header
    uint8 mOpCode;          ///< 8-bit opcode value
    uint16 mLength;         ///< 16-bit value of number of bytes in Data field, in big-endian format, i.e. MSB, LSB.

    std::vector<uint8> mData;
};

///
/// Sent by the host at any time to restart the upgrade process.
///
class CUpgradeSyncReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeSyncReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_SYNC_REQ, 4) {};
    ~CUpgradeSyncReq() {};

    void SetFileIdentifier(uint32 aFileIdentifier);
};

///
/// Sent by the device.
///
class CUpgradeSyncCfm : public CUpgradeProtocolMsg
{
public:
    CUpgradeSyncCfm(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_SYNC_CFM, 6) {};
    CUpgradeSyncCfm(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeSyncCfm() {};

    uint8 GetResumePoint() const        { return GetDataUint8(0); };    // UpgradeResumePoint
    uint32 GetFileIdentifier() const    { return GetDataUint32(1); };   // Unique upgrade file identifier – 4 bytes
    uint8 GetProtocolVersion() const    { return GetDataUint8(5); };
};

///
/// Sent by the host.
///
class CUpgradeStartReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeStartReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_START_REQ) {};
    ~CUpgradeStartReq() {};
};

///
/// Sent by the device.
/// Setting the actual battery level in the ‘Battery level’ field is optional. 
/// It can be set to 0, but the length of the message remains at 3
///
class CUpgradeStartCfm : public CUpgradeProtocolMsg
{
public:
    CUpgradeStartCfm(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_START_CFM, 2) {};
    CUpgradeStartCfm(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeStartCfm() {};

    uint8 GetStatus() const         { return GetDataUint8(0); };    // 0x00 – Success 0x01 – Failure
    uint8 GetBatteryLevel() const   { return GetDataUint8(1); };    // Device battery level in mV
};

///
/// Sent by the host to initiate data transfer.
///
class CUpgradeStartDataReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeStartDataReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_START_DATA_REQ) {};
    ~CUpgradeStartDataReq() {};
};

///
/// Sent by the device to request the upgrade file data from the host.
///
class CUpgradeDataBytesReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeDataBytesReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_DATA_BYTES_REQ, 8) {};
    CUpgradeDataBytesReq(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeDataBytesReq() {};

    uint32 GetNumberOfBytes() const { return GetDataUint32(0); };
    uint32 GetStartOffset() const   { return GetDataUint32(4); };
};

///
/// Sent by the host.
///
class CUpgradeDataReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeDataReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_DATA) {};
    ~CUpgradeDataReq() {};

    void SetMoreData(uint8 aMoreData); // 0 – There is more data in the upgrade file 1 – Last packet of data from the upgrade file
    void SetImageData(const uint8 *apImageData, uint8 aLength);
};

///
/// Sent by the host.
///
class CUpgradeHostVersionReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeHostVersionReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_HOST_VERSION_REQ) {};
    ~CUpgradeHostVersionReq() {};
};

///
/// Sent by the device.
///
class CUpgradeHostVersionCfm : public CUpgradeProtocolMsg
{
public:
    CUpgradeHostVersionCfm(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_HOST_VERSION_CFM, 6) {};
    CUpgradeHostVersionCfm(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeHostVersionCfm() {};

    uint16 GetVersionMajor() const { return GetDataUint16(0); };
    uint16 GetVersionMinor() const { return GetDataUint16(2); };
    uint16 GetConfigVersion() const { return GetDataUint16(4); };
};

///
/// Sent by the host to check for completion of the validation process.
/// Responses are UPGRADE_IS_VALIDATION_DONE_CFM or UPGRADE_TRANSFER_COMPLETE_IND.
///
class CUpgradeIsValidationDoneReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeIsValidationDoneReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_IS_VALIDATION_DONE_REQ) {};
    ~CUpgradeIsValidationDoneReq() {};
};

///
/// Sent by the device to indicate that the validation process is in progress.
/// This message is optional.
///
class CUpgradeIsValidationDoneCfm : public CUpgradeProtocolMsg
{
public:
    CUpgradeIsValidationDoneCfm(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_IS_VALIDATION_DONE_CFM, 1) {};
    CUpgradeIsValidationDoneCfm(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeIsValidationDoneCfm() {};

    // Time in ms the host should wait before sending the next UPGRADE_IS_VALIDATION_DONE_REQ message
    uint8 GetDelayTime() { return GetDataUint8(0); };
};

///
/// Sent by the device to indicate that the device is ready to apply the upgrade.
///
class CUpgradeTransferCompleteInd : public CUpgradeProtocolMsg
{
public:
    CUpgradeTransferCompleteInd(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_TRANSFER_COMPLETE_IND) {};
    CUpgradeTransferCompleteInd(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeTransferCompleteInd() {};
};

///
/// Sent by the host.
///
class CUpgradeTransferCompleteRes : public CUpgradeProtocolMsg
{
public:
    CUpgradeTransferCompleteRes(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_TRANSFER_COMPLETE_RES, 1) {};
    ~CUpgradeTransferCompleteRes() {};

    void SetAction(uint8 aAction); // 0x00 – Proceed 0x01 – Do not proceed 
};

///
/// Sent by the host.
/// Responses are UPGRADE_COMMIT_REQ or UPGRADE_COMPLETE_IND.
///
class CUpgradeProceedToCommit : public CUpgradeProtocolMsg
{
public:
    CUpgradeProceedToCommit(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_PROCEED_TO_COMMIT, 1) {};
    ~CUpgradeProceedToCommit() {};

    void SetAction(uint8 aAction); // 0x00 – Proceed 0x01 – Do not proceed 
};

///
/// Sent by the device. 
/// This message is optional.
///
class CUpgradeCommitReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeCommitReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_COMMIT_REQ) {};
    CUpgradeCommitReq(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeCommitReq() {};
};

///
/// Sent by the host.
///
class CUpgradeCommitCfm : public CUpgradeProtocolMsg
{
public:
    CUpgradeCommitCfm(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_COMMIT_CFM, 1) {};
    ~CUpgradeCommitCfm() {};

    void SetAction(uint8 aAction); // 0x00 – Proceed 0x01 – Do not proceed 
};

///
/// Sent by the device to indicate that the upgrade process is complete.
///
class CUpgradeCompleteInd : public CUpgradeProtocolMsg
{
public:
    CUpgradeCompleteInd(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_COMPLETE_IND) {};
    CUpgradeCompleteInd(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeCompleteInd() {};
};

///
/// Sent by the device in response to any host message.
///
class CUpgradeErrorInd : public CUpgradeProtocolMsg
{
public:
    CUpgradeErrorInd(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_ERROR_IND, 2) {};
    CUpgradeErrorInd(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeErrorInd() {};

    uint16 GetErrorCode() const { return GetDataUint16(0); };    // Error codes are implementation dependent
};

///
/// Sent by the host.
///
class CUpgradeErrorRes : public CUpgradeProtocolMsg
{
public:
    CUpgradeErrorRes(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_ERROR_RES, 2) {};
    ~CUpgradeErrorRes() {};

    void SetErrorCode(const uint16 aErrorCode); // Error code confirmation
};

///
/// Sent by the host and can be sent at any time.
///
class CUpgradeAbortReq : public CUpgradeProtocolMsg
{
public:
    CUpgradeAbortReq(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_ABORT_REQ) {};
    ~CUpgradeAbortReq() {};
};

///
/// Sent by the device.
///
class CUpgradeAbortCfm : public CUpgradeProtocolMsg
{
public:
    CUpgradeAbortCfm(uint8 aReportLenBytes) : CUpgradeProtocolMsg(HID_REPORTID_DATA_TRANSFER,
            aReportLenBytes, UPGRADE_ABORT_CFM) {};
    CUpgradeAbortCfm(const uint8 *apBuffer, uint8 aLength) : CUpgradeProtocolMsg(apBuffer, aLength) {};
    ~CUpgradeAbortCfm() {};
};

#endif // #ifndef UPGRADE_PROTOCOL_H