//**************************************************************************************************
//
//  PtSerialNum.h
//
//  Copyright (c) 2021-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Serial number class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_SERIAL_NUM_H
#define PT_SERIAL_NUM_H

#include <string>
#include <map>

class CPtSetup;

///
/// Production test serial number class.
///
class CPtSerialNum
{
public:

    ///
    /// Supported SN types.
    ///
    enum class SnType
    {
        DEC, // Decimal characters only
        HEX, // Hexadecimal characters only
        ALNUM // Alphanumeric characters only
    };

    ///
    /// Constructor.
    /// @param[in] aSerNum Serial number string.
    /// @param[in] aSetup The production test setup.
    /// @throws CPtException.
    ///
    CPtSerialNum(const std::string& aSerNum, const CPtSetup& aSetup);

    ///
    /// Destructor.
    ///
    ~CPtSerialNum() {};

    ///
    /// Gets the serial number type.
    /// @return The type of the serial number.
    ///
    SnType GetType() const { return mType; };

    ///
    /// Gets the serial number string.
    /// @return The serial number string.
    ///
    std::string ToString() const { return mSerNum; };

private:

    ///
    /// SN types map, mapping type strings to enumerated type values.
    ///
    typedef std::map<std::string, SnType> SnTypeMap;
    static const SnTypeMap mSupportedTypesMap;

    ///
    /// Serial number type.
    ///
    SnType mType;

    ///
    /// Serial number string.
    ///
    std::string mSerNum;

    ///
    /// Checks the length of the serial number against any requirement.
    /// @param[in] aSetup The production test setup.
    /// @throws CPtException.
    ///
    void CheckLength(const CPtSetup& aSetup);

    ///
    /// Checks the format of the serial number against the type.
    /// @param[in] aSetup The production test setup.
    /// @throws CPtException.
    ///
    void CheckFormat(const CPtSetup& aSetup);
};

#endif // PT_SERIAL_NUM_H
