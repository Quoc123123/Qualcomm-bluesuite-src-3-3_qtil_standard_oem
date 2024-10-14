//**************************************************************************************************
//
//  connectionconfig.h
//
//  Implements the ConnectionConfig class encapsulating connection configuration parameters.
//
//  Copyright (c) 2017-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//**************************************************************************************************

#ifndef CONNECTIONCONFIG_H
#define CONNECTIONCONFIG_H

#include "common/types.h"

#include <string>

namespace qtil { namespace misc
{
//**************************************************************************************************
//
//  ConnectionConfig
//
//**************************************************************************************************

    ///
    /// Encapsulates connection configuration parameters
    ///
    class ConnectionConfig
    {
    public:

        ///
        /// Supported connection types
        ///
        enum ConnType
        {
            CONN_INVALID,
            CONN_SPI,
            CONN_SDIO,
            CONN_PTAP,
            CONN_TRB,
            CONN_USBDBG,
            CONN_USBCC,
            CONN_ADBBT
        };

        ///
        /// Correct the given SPITRANS value if necessary (if a deprecated term).
        /// No checks are made on the validity of the given SPITRANS value.
        /// @param[in] aSpiTrans the SPITRANS value for the connection.
        /// @return aSpiTrans string if no correction is necessary, otherwise corrected SPTRANS value.
        ///
        static std::string CorrectSpiTrans(const std::string& aSpiTrans);

        ///
        /// Constructor.
        ///
        ConnectionConfig();

        ///
        /// Constructor.
        /// @param[in] aConnectionString the string specifying the connection parameters.
        ///
        ConnectionConfig(
            const std::string& aConnectionString);

        ///
        /// Constructor.
        /// @param[in] aSpiTrans the SPITRANS value for the connection.
        /// @param[in] aSpiPort the SPIPORT value for the connection.
        ///
        ConnectionConfig(
            const std::string& aSpiTrans,
            const std::string& aSpiPort);

        ///
        /// Destructor.
        ///
        virtual ~ConnectionConfig();

        ///
        /// Gets the connection configuration string.
        /// @return Connection configuration string.
        ///
        std::string ConnectionString() const;

        ///
        /// Gets the connection type.
        /// @return Connection type.
        ///
        ConnType ConnectionType() const;

        ///
        /// Gets the port name.
        /// @return Port name.
        ///
        std::string PortName() const;

    private:

        ///
        /// Parses a connection string.
        /// @param[in] aConnectionString the connection string to parse.
        ///
        void Parse(
            const std::string& aConnectionString);

        std::string mConnectionString; ///< The connection configuration string.
        ConnType mConnType;            ///< The connection type.
        std::string mPortName;         ///< The port name.
    };

//**************************************************************************************************
}}
#endif // CONNECTIONCONFIG_H
