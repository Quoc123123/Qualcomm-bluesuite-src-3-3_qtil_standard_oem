//*******************************************************************************
//
//  UpgradeProtocol.cpp
//
//  Copyright (c) 2016-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for UpgradeProtocol class.
//
//*******************************************************************************

#include "UpgradeProtocol.h"

#include <assert.h>
#include "engine/enginefw_interface.h"

#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_HID_DFU_LIB

////////////////////////////////////////////////////////////////////////////////

CUpgradeProtocolMsg::CUpgradeProtocolMsg(const uint8 aReportId, uint8 aReportLenBytes,
        const uint8 aOpCode, const uint16 aLength)
        : mReportId(aReportId), mOpCode(aOpCode), mLength(aLength)
{
    FUNCTION_DEBUG_SENTRY;

    mUpgradeMsgSize = static_cast<uint8>(sizeof(aOpCode) + sizeof(aLength) + aLength);

    size_t upgradeDataSize = aReportLenBytes - HEADER_SIZE;

    // Set data size to 'Upgrade Data Size' and initialise to 0's
    mData.resize(upgradeDataSize, 0);
}

CUpgradeProtocolMsg::CUpgradeProtocolMsg(const uint8 *apBuffer, const uint8 aLength)
{
    FUNCTION_DEBUG_SENTRY;

    // Report ID
    mReportId = apBuffer[0];

    // Size
    mUpgradeMsgSize = apBuffer[1];

    // Opcode (8-bits)
    mOpCode = apBuffer[2];

    // Length (16-bits MSB, LSB)
    mLength = apBuffer[4];
    mLength = (apBuffer[3] << 8) & mLength;

    // Data
    size_t upgradeDataSize = aLength - HEADER_SIZE;
    // Set data size to 'Upgrade Data Size' and initialise to 0's
    mData.resize(upgradeDataSize, 0);

    for (uint8 i = 0; i < upgradeDataSize; ++i)
    {
        mData.at(i) = apBuffer[i + 5];
    }
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeProtocolMsg::Serialise(uint8 *apBuffer, uint8 aLength) const
{
    FUNCTION_DEBUG_SENTRY;

    // Report ID
    apBuffer[0] = mReportId;

    // Size
    apBuffer[1] = mUpgradeMsgSize;

    // Opcode (8-bits)
    apBuffer[2] = mOpCode;

    // Length (16-bits MSB, LSB)
    apBuffer[3] = (mLength >> 8) & 0xFF;;
    apBuffer[4] = mLength & 0xFF;

    // Data
    uint8 upgradeDataSize = aLength - HEADER_SIZE;
    for (uint8 i = 0; i < upgradeDataSize; ++i)
    {
        apBuffer[i + 5] = mData.at(i);
    }

    MSG_HANDLER_NOTIFY_DEBUG_BUFFER(DEBUG_BASIC, "UpgradeProtocolMsg[...]", apBuffer, upgradeDataSize);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeProtocolMsg::SetData(const uint8 aIndex, const uint8* apBuffer, uint8 aLength)
{
    FUNCTION_DEBUG_SENTRY;

    // Data
    for (uint8 i = 0; i < aLength; ++i)
    {
        mData.at(aIndex + i) = apBuffer[i];
    }
}

////////////////////////////////////////////////////////////////////////////////

uint8 CUpgradeProtocolMsg::GetDataUint8(const uint8 aIndex) const
{
    uint8 retVal = 0;
    FUNCTION_DEBUG_SENTRY_RET(uint8, retVal);

    retVal = mData.at(aIndex);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

uint16 CUpgradeProtocolMsg::GetDataUint16(const uint8 aIndex) const
{
    uint16 retVal = 0;
    FUNCTION_DEBUG_SENTRY_RET(uint16, retVal);

    retVal = mData.at(aIndex + 1);
    retVal = (mData.at(aIndex) << 8) | retVal;

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

uint32 CUpgradeProtocolMsg::GetDataUint32(const uint8 aIndex) const
{
    uint32 retVal = 0;
    FUNCTION_DEBUG_SENTRY_RET(uint32, retVal);

    retVal = mData.at(aIndex + 3);
    retVal = (mData.at(aIndex + 2) << 8) | retVal;
    retVal = (mData.at(aIndex + 1) << 16) | retVal;
    retVal = (mData.at(aIndex) << 24) | retVal;

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeProtocolMsg::SetDataUint8(const uint8 aIndex, const uint8 aValue)
{
    FUNCTION_DEBUG_SENTRY;

    mData.at(aIndex) = aValue & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeProtocolMsg::SetDataUint16(const uint8 aIndex, const uint16 aValue)
{
    FUNCTION_DEBUG_SENTRY;

    mData.at(aIndex) = (aValue >> 8) & 0xFF;
    mData.at(aIndex + 1) = aValue & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeProtocolMsg::SetDataUint32(const uint8 aIndex, const uint32 aValue)
{
    FUNCTION_DEBUG_SENTRY;

    mData.at(aIndex) = (aValue >> 24) & 0xFF;
    mData.at(aIndex + 1) = (aValue >> 16) & 0xFF;
    mData.at(aIndex + 2) = (aValue >> 8) & 0xFF;
    mData.at(aIndex + 3) = aValue & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeCommitCfm::SetAction(const uint8 aAction)
{
    FUNCTION_DEBUG_SENTRY;

    SetDataUint8(0, aAction);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeDataReq::SetMoreData(const uint8 aMoreData)
{
    FUNCTION_DEBUG_SENTRY;

    SetDataUint8(0, aMoreData);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeDataReq::SetImageData(const uint8 *apImageData, uint8 aLength)
{
    FUNCTION_DEBUG_SENTRY;

    // "More data" flag goes before data, so write data at index 1
    SetData(1, apImageData, aLength);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeErrorRes::SetErrorCode(const uint16 aErrorCode)
{
    FUNCTION_DEBUG_SENTRY;

    SetDataUint16(0, aErrorCode);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeProceedToCommit::SetAction(const uint8 aAction)
{
    FUNCTION_DEBUG_SENTRY;

    SetDataUint8(0, aAction);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeSyncReq::SetFileIdentifier(const uint32 aFileIdentifier)
{
    FUNCTION_DEBUG_SENTRY;

    SetDataUint32(0, aFileIdentifier);
}

////////////////////////////////////////////////////////////////////////////////

void CUpgradeTransferCompleteRes::SetAction(const uint8 aAction)
{
    FUNCTION_DEBUG_SENTRY;

    SetDataUint8(0, aAction);
}

////////////////////////////////////////////////////////////////////////////////