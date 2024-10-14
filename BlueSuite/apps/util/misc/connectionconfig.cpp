//**************************************************************************************************
//
//  connectionconfig.cpp
//
//  Implements the ConnectionConfig class encapsulating connection configuration parameters.
//
//  Copyright (c) 2017-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//**************************************************************************************************

#include "connectionconfig.h"
#include "stringutil.h"

#include "engine/enginefw_interface.h"
#include "spi/spi_common.h"

#include <map>

namespace qtil { namespace misc
{
//**************************************************************************************************
//
//  ConnectionString
//
//**************************************************************************************************

    std::string ConnectionConfig::CorrectSpiTrans(const std::string& aSpiTrans)
    {
        std::string correctedStr(aSpiTrans);

        if (aSpiTrans == "USBDBGTC")
        {
            correctedStr = "USBDBG";
        }

        return correctedStr;
    }

//**************************************************************************************************

    ConnectionConfig::ConnectionConfig()
        :
        mConnectionString(),
        mConnType(CONN_INVALID),
        mPortName()
    {
        MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_UTILITY);
        FUNCTION_DEBUG_SENTRY;
    }

//**************************************************************************************************

    ConnectionConfig::ConnectionConfig(
        const std::string& aConnectionString)
        :
        mConnectionString(),
        mConnType(CONN_INVALID),
        mPortName()
    {
        MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_UTILITY);
        FUNCTION_DEBUG_SENTRY;

        Parse(aConnectionString);
    }

//**************************************************************************************************

    ConnectionConfig::ConnectionConfig(
        const std::string& aSpiTrans,
        const std::string& aSpiPort)
        :
        mConnectionString(),
        mConnType(CONN_INVALID),
        mPortName()
    {
        MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_UTILITY);
        FUNCTION_DEBUG_SENTRY;

        std::stringstream ss;
        ss <<
            SPITRANS << "=" << aSpiTrans << " " <<
            SPIPORT << "=" << aSpiPort;

        Parse(ss.str());
    }

//**************************************************************************************************

    ConnectionConfig::~ConnectionConfig()
    {
        FUNCTION_DEBUG_SENTRY;
    }

//**************************************************************************************************

    void ConnectionConfig::Parse(
        const std::string& aConnectionString)
    {
        FUNCTION_DEBUG_SENTRY;

        std::stringstream error;

        mConnectionString = aConnectionString;

        std::map<std::string, std::string> options;
        bool ok = stringutil::StringToMap(stringutil::ToUpper(mConnectionString), '=', ' ', options);

        if (ok)
        {
            std::map<std::string, std::string>::iterator it = options.find(SPITRANS);
            if (it != options.end())
            {
                //
                // "SPITRANS=USB" is used for a USB SPI (babel) debug connection (using PtUsbSpi pttransport plugin).
                // "SPITRANS=LPT" is used for a LPT SPI debug connection (using SpiLpt pttransport plugin).
                // "SPITRANS=SDIO|SDIOCE|SDIOCEV5|SDIOEMB" is used for SDIO debug connections (using PtSdio* pttransport plugins).
                // "SPITRANS=TRB" is used for a USB TRB debug connection (using PtTrb).
                // "SPITRANS=TRBTC" is used for a USB TRB debug connection utilising TOOLCMD protocol (using PtToolCmd).
                // "SPITRANS=USBDBG" is used for a Low Cost Debug connection (using PtToolCmd).
                // "SPITRANS=USBDBGTC" is used for a Low Cost Debug connection (using PtToolCmd). Retained for backwards compatibility only.
                // "SPITRANS=USBCC" is used for a USB Charger Comms debug connection (using PtToolCmd).
                // "SPITRANS=PTAPTC" is used for a connection to CSRC9xxx.
                // "SPITRANS=ADBBT" is used for Android Debug Bridge <-> Bluetooth connection (using PtToolCmd).
                //
                if (it->second.compare("USB") == 0 ||
                    it->second.compare("LPT") == 0)
                {
                    mConnType = CONN_SPI;
                }
                else if (it->second.compare("SDIO") == 0 ||
                         it->second.compare("SDIOCE") == 0 ||
                         it->second.compare("SDIOCEV5") == 0 ||
                         it->second.compare("SDIOEMB") == 0)
                {
                    mConnType = CONN_SDIO;
                }
                else if (it->second.compare("TRB") == 0 ||
                         it->second.compare("TRBTC") == 0)
                {
                    mConnType = CONN_TRB;
                }
                else if (it->second.compare("USBDBG") == 0 ||
                         it->second.compare("USBDBGTC") == 0)
                {
                    mConnType = CONN_USBDBG;

                    // Change "USBDBGTC" to the preferred term "USBDBG" in the stored transport string
                    static const std::string DEPRECATED_STR("USBDBGTC");
                    const std::string::size_type pos = mConnectionString.find(DEPRECATED_STR);
                    if (pos != std::string::npos)
                    {
                        // Remove the last 2 chars ("TC")
                        mConnectionString.erase(pos + DEPRECATED_STR.size() - 2, 2);
                    }
                }
                else if (it->second.compare("USBCC") == 0)
                {
                    mConnType = CONN_USBCC;
                }
                else if (it->second.compare("PTAPTC") == 0)
                {
                    mConnType = CONN_PTAP;
                }
                else if (it->second.compare("ADBBT") == 0)
                {
                    mConnType = CONN_ADBBT;
                }
                else
                {
                    error << SPITRANS << " option in connection string \"" << mConnectionString << "\" is unsupported";
                    MSG_HANDLER.SetErrorMsg(0, error.str());
                    ok = false;
                }
            }
            else
            {
                error << "No " << SPITRANS << " option in connection string \"" << mConnectionString << "\"";
                MSG_HANDLER.SetErrorMsg(0, error.str());
                ok = false;
            }

            if (ok)
            {
                it = options.find(SPIPORT);
                if (it != options.end())
                {
                    if (!it->second.empty())
                    {
                        mPortName = it->second;
                    }
                    else
                    {
                        error << "No value for " << SPIPORT << " option";
                        MSG_HANDLER.SetErrorMsg(0, error.str());
                    }
                }
                else
                {
                    error << "No " << SPIPORT << " option in connection string \"" << mConnectionString << "\"";
                    MSG_HANDLER.SetErrorMsg(0, error.str());
                }
            }
        }
        else
        {
            error << "Unable to parse connection string \"" << mConnectionString << "\"";
            MSG_HANDLER.SetErrorMsg(0, error.str());
        }
    }

//**************************************************************************************************

    std::string ConnectionConfig::ConnectionString() const
    {
        FUNCTION_DEBUG_SENTRY_RET(std::string, mConnectionString);

        return mConnectionString;
    }

//**************************************************************************************************

    ConnectionConfig::ConnType ConnectionConfig::ConnectionType() const
    {
        FUNCTION_DEBUG_SENTRY_RET(ConnType, mConnType);

        return mConnType;
    }

//**************************************************************************************************

    std::string ConnectionConfig::PortName() const
    {
        FUNCTION_DEBUG_SENTRY_RET(std::string, mPortName);

        return mPortName;
    }

//**************************************************************************************************
}}
