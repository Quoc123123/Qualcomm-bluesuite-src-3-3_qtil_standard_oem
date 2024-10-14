/**********************************************************************
 *
 *  EngineFrameworkClr.h
 *  
 *  Copyright (c) 2013-2020 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 **********************************************************************/

#pragma once
#include "engine\enginefw_interface.h"
#include <vcclr.h>

using namespace System;

namespace QTIL { namespace HostTools { namespace Common { namespace EngineFrameworkClr
{

    // Disable warnings about non-standard extension used
#pragma warning( disable: 4482 )

    /// <summary>
    /// The levels of debug messages available.
    /// </summary>
    ///<remarks>This *MUST* be kept in step with EngineFrameworkCpp::DebugLevels</remarks>
    public enum class DebugLevel
    {
        EntryExit = DebugLevels::DEBUG_ENTRY_EXIT,   ///< Used internally to denote entry and exit to/from a method.
        Parameter,              ///< Used to express the value of an individual parameter to a method.
        Enhanced,               ///< Used to express debug output that would be useful to someone who is debugging a complicated problem.
        Basic,                  ///< Used to express debug output that would be useful to someone who is starting to debug the code.
        All                     ///< Used only when turning levels on and off; do not use when outputting individual lines of debug.
    };

    /// The levels of status messages available.
    public enum class StatusLevel
    {
        Info = StatusLevels::STATUS_INFO,        ///< Informational message which the user would find useful to know.
        Essential,              ///< Information that the user has specifically asked for.
        Warning,                ///< Something that should be brought to the attention of the user, but not fatal.
        Error,                  ///< Something that is a fatal error and the situation cannot be resolved.
        All                     ///< Used only when turning levels on and off; do not use when outputting individual lines of status.
    };

    /// <summary>
    /// The groups ...
    /// </summary>
    ///<remarks>This *MUST* be kept in step with EngineFrameworkCpp::GroupEnum</remarks>
    public enum class GroupEnum
    {
        // The groups that map directly to a specific LIB/DLL...
        // (where the library matches the binary output file unless otherwise stated)
        GROUP_ENUM_PTTRANSPORT_LIB = CMessageHandler::GroupEnum::GROUP_ENUM_PTTRANSPORT_LIB,
        GROUP_ENUM_UENERGY_LIB, ///< uEnergyCsEngine and uEnergyTest
        GROUP_ENUM_CMDLINE_LIB,
        GROUP_ENUM_A11EPROMCFGRW_LIB,
        GROUP_ENUM_CNDTEXT_LIB,
        GROUP_ENUM_CONFIGTOOLSCOREFRAMEWORK_LIB,
        GROUP_ENUM_CONFIGFWMANAGED_LIB,
        GROUP_ENUM_CURATORLIBRARY_LIB,
        GROUP_ENUM_CURATORWR_LIB,
        GROUP_ENUM_E2_LIB,
        GROUP_ENUM_FLASH_LIB,
        GROUP_ENUM_HYDTEXT_LIB,
        GROUP_ENUM_PSHELP_LIB,
        GROUP_ENUM_SQLITEMETADATAPROVIDER_LIB,
        GROUP_ENUM_TESTE2_LIB,
        GROUP_ENUM_TESTFLASH_LIB,
        GROUP_ENUM_TESTENGINE_LIB,
        GROUP_ENUM_VLINTLIBRARY_LIB,
        GROUP_ENUM_NVSENGINE_LIB,
        GROUP_ENUM_BCCMDTRANS_LIB,
        GROUP_ENUM_CHUNKSIOIMPL_LIB,
        GROUP_ENUM_KEYFILE_LIB,
        GROUP_ENUM_TRUNLOCK_LIB,
        GROUP_ENUM_HCITRANSPORT_LIB,
        GROUP_ENUM_PUNITEST_LIB,
        GROUP_ENUM_UNIPSLIB_LIB,
        GROUP_ENUM_HYDPARSER_LIB,
        GROUP_ENUM_PTAP_LIB,
        GROUP_ENUM_UNITEST_LIB,
        GROUP_ENUM_UNITESTHYD_LIB,
        GROUP_ENUM_COEXVALIDATOR_LIB,
        GROUP_ENUM_DFUIMAGE_LIB,
        GROUP_ENUM_HYDPROTOCOLS_LIB,
        GROUP_ENUM_FSIF_LIB,
        GROUP_ENUM_HYDEEPROM_LIB,
        GROUP_ENUM_HCIPACKER_LIB,
        GROUP_ENUM_ACLENGINE_LIB,
        GROUP_ENUM_ACCMD_LIB,
        GROUP_ENUM_DFUENGINE_LIB,
        GROUP_ENUM_UEOTAUIMG_LIB,
        GROUP_ENUM_UCI_PACKER_LIB,
        GROUP_ENUM_UCI_TRANSACTION_LIB,
        GROUP_ENUM_FASTPIPEENGINE_LIB,
        GROUP_ENUM_SECUREKEYENGINE_LIB,
        GROUP_ENUM_UENERGY_DEVICE_LIB,
        GROUP_ENUM_HID_DFU_LIB,
        GROUP_ENUM_QCTDOWNLOADENGINE_LIB,
        GROUP_ENUM_HYDISP_LIB,
        GROUP_ENUM_USER_PS_STORE_LIB,
        GROUP_ENUM_ISOENGINE_LIB,

        // ADD NEW ENTRIES IMMEDIATELY BEFORE THIS LINE...
        // PLEASE READ THE NOTE ABOVE...

        // The generic groups to which code can belong...
        GROUP_ENUM_APPLICATION = CMessageHandler::GroupEnum::GROUP_ENUM_APPLICATION, ///< The actual application (i.e. the implementation
                                      ///< of main and closely related functions)
        GROUP_ENUM_PTTRANS_PLUGIN,    ///< PtTransport Plug-ins
        GROUP_ENUM_TEST_CODE,         ///< Test code (i.e. unit tests, regression tests etc.)
        GROUP_ENUM_UTILITY,           ///< Simple utility functions (often generic)

        /// @note Code cannot belong to the groups with "..._RSVD_..." in the
        /// name, but the enum values are used to manipulate messages.
        GROUP_ENUM_RSVD_ALL = CMessageHandler::GroupEnum::GROUP_ENUM_RSVD_ALL,    ///< Used to denote "all groups"
        GROUP_ENUM_RSVD_LOCAL         ///< Used to denote "just the current group"
    #ifdef EF_ALLOW_LEGACY_DEFAULT_GROUP
       ,GROUP_ENUM_RSVD_DEFAULT       ///< Legacy "everything else" group
    #endif
    };


    // Forward declarations
    class CUnmanagedMessageHandler;
    class CUnmanagedEngineObserver;




    public ref class StatusEventArgs
        : public EventArgs
    {
    public:
        StatusLevel^ Level;
        String^ Message;

        StatusEventArgs(StatusLevel^ level, String^ message)
            : EventArgs()
        {
            Level = level;
            Message = message;
        }
    };

    public ref class ProgressEventArgs
        : public EventArgs
    {
    public:
        int16 Value;

        ProgressEventArgs(int16 value)
            : EventArgs()
        {
            Value = value;
        }
    };

    public ref class DebugEventArgs
        : public EventArgs
    {
    public:
        DebugLevel^ Level;
        String^ Message;

        DebugEventArgs(DebugLevel^ level, String^ message)
            : EventArgs()
        {
            Level = level;
            Message = message;
        }
    };



    public ref class MessageHandler
    {
    private:
        static bool                 mInDesignMode;

        CUnmanagedEngineObserver*   mEngineObserver;

        CUnmanagedMessageHandler*   mUnmanaged;

        static String^              mPreParseError;

        /// <summary>
        /// Emits a debug message.
        /// </summary>
        /// <param name="debugLevel">The level.</param>
        /// <param name="isEntry">Flag that this is a function entry message.</param>
        /// <param name="isExit">Flag that this is a function exit message.</param>
        /// <param name="text">The message text.</param>
        /// <param name="retVal">The return value, only used if isExit.</param>
        static void EmitDebugMessage(DebugLevel debugLevel, bool isEntry, bool isExit, String^ text, Object^ retVal);


    public:
        static MessageHandler();
        MessageHandler();
        ~MessageHandler();

        property String^ PreParseError
        {
            String^ get()
            {
                return mPreParseError;
            }
        }

        /////////////////////////////////////////////////////////////////////////

        delegate void StatusEventHandler(Object^ sender, StatusEventArgs^ e);

        event StatusEventHandler^ Status;

        void OnStatus(StatusLevels level, const std::string& message);


        delegate void ProgressEventHandler(Object^ sender, ProgressEventArgs^ e);

        event ProgressEventHandler^ Progress;

        void OnProgress(int16 progress);


        delegate void DebugEventHandler(Object^ sender, DebugEventArgs^ e);

        event DebugEventHandler^ Debug;

        void OnDebug(DebugLevels level, const std::string& message);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// The method to use to send progress indications of a specific percentage.
        /// </summary>
        /// <param name="value">The percentage level (a value between 0 and 100 inclusive).</param>
        static void NotifyProgress(uint16 value);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Records entry to a function.
        /// </summary>
        static void DebugEntry();

        /// <summary>
        /// Records exit from a function.
        /// </summary>
        void static DebugExit();

        /// <summary>
        /// Records exit from a function with a value.
        /// </summary>
        static void DebugExit(Object^ aRetVal);

        /// <summary>
        /// Records exit from a function with a formatted list of values.
        /// </summary>
        static void DebugExitFormat(String^ aFormat, ... cli::array<Object ^, 1>^ aArgs);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Records a simple debug message.
        /// </summary>
        /// <param name="debugLevel">The level.</param>
        /// <param name="text">A text.</param>
        static void NotifyDebug(DebugLevel debugLevel, String^ text);

        /// <summary>
        /// Records a formatted debug message.
        /// </summary>
        /// <param name="debugLevel">The level.</param>
        /// <param name="aFormat">The format.</param>
        /// <param name="args">The arguments.</param>
        static void NotifyDebug(DebugLevel debugLevel, String^ aFormat, ... cli::array<Object ^, 1>^ args);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// The method to use to send status messages.
        /// </summary>
        /// <param name="level">The status level; any value except STATUS_ALL.</param>
        /// <param name="text">The text to be sent.</param>
        static void NotifyStatus(StatusLevel level, String^ text);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Set the error message, replacing the current error message.
        /// </summary>
        /// <param name="errorCode">The error code number shown to the user, unless set to the value zero (in which case there is no concept of an error number).</param>
        /// <param name="text">The text to be sent.</param>
        /// <returns>true if the error message is stored, false otherwise.</returns>
        /// <remarks>The concept is that there is only one error message in force at any one time.</remarks>
        static bool SetErrorMsg(int16 errorCode, String^ text);

        /// <summary>
        /// Modify the error message, by prefixing the text to the start of the current error message.
        /// </summary>
        /// <param name="errorCode">Change the current error code in GROUP_ENUM_RSVD_LOCAL to the value specified, unless zero is specified in which case the current code is left alone.</param>
        /// <param name="text">The text to be prefixed.</param>
        /// <returns>true if the error message is stored, false otherwise.</returns>
        /// <remarks>The concept is that there is only one error message in force at any one time.</remarks>
        static bool ModifyErrorMsg(int16 errorCode, String^ text);

        /// <summary>
        /// Modify the error message, by prefixing the text to the start of the current error message.
        /// </summary>
        /// <param name="errorCode">Change the current error code to the value specified, unless zero is specified in which case the current code is left alone.</param>
        /// <param name="text">The text to be prefixed.</param>
        /// <param name="groupEnum">The group to use when searching for an existing error.
        /// Specify GROUP_ENUM_RSVD_LOCAL to look in the 'local' handler,
        /// GROUP_ENUM_RSVD_ALL to look in all handlers or a specific (other) enumeration
        /// to look in that specific handler. The new, modified error condition will only be assigned
        /// to the LOCAL group; this value is only used to determine the search criteria.</param>
        /// <returns>true if the error message is stored, false otherwise.</returns>
        /// <remarks>The concept is that there is only one error message in force at any one time.</remarks>
        static bool ModifyErrorMsg(int16 errorCode, String^ text, GroupEnum groupEnum);

        /// <summary>
        /// Determine whether error messages in GROUP_ENUM_RSVD_LOCAL are automatically transmitted to the user.
        /// </summary>
        /// <param name="newAutomaticDisplay">true if messages are to be automatically transmitted to the user, false otherwise.</param>
        /// <returns>The value of the flag when the method was called (if the group was not GROUP_ENUM_RSVD_ALL, in which case it is always true).</returns>
        static bool SetErrorAutomaticDisplay(bool newAutomaticDisplay);

        /// <summary>
        /// Determine whether error messages are automatically transmitted to the user.
        /// </summary>
        /// <param name="newAutomaticDisplay">true if messages are to be automatically transmitted to the user, false otherwise.</param>
        /// <param name="groupEnum">The group to which the modifications apply.</param>
        /// <returns>The value of the flag when the method was called (if the group was not GROUP_ENUM_RSVD_ALL, in which case it is always true).</returns>
        static bool SetErrorAutomaticDisplay(bool newAutomaticDisplay, GroupEnum groupEnum);

        /// <summary>
        /// Retrieve the last/current error message details for the current thread only in any group.
        /// </summary>
        /// <param name="errorCode">The code number of the error.</param>
        /// <param name="errorText">The text of the error.</param>
        /// <returns>true if there is a current error and the ouput parameters have been populated, false otherwise (in which case the ouput parameters remain unchanged).</returns>
        static bool LastError(int16% errorCode, String^% errorText);

        /// <summary>
        /// Retrieve the last/current error message details for the current thread only in any group.
        /// </summary>
        /// <param name="errorText">The text of the error.</param>
        /// <returns>The code number of the error.</returns>
        static int16 LastError(String^% errorText);

        /// <summary>
        /// Retrieve the last/current error message details for the current thread only.
        /// </summary>
        /// <param name="errorCode">The code number of the error.</param>
        /// <param name="errorText">The text of the error.</param>
        /// <param name="groupEnum">The group in which to search.
        /// Specify GROUP_ENUM_RSVD_LOCAL to look in the 'local' handler,
        /// GROUP_ENUM_RSVD_ALL to look in all handlers or a specific (other) enumeration
        /// to look in that specific handler.
        /// On exit (if the return code is true) it contains the group that has the error.</param>
        /// <returns>true if there is a current error and the ouput parameters have been populated, false otherwise (in which case the ouput parameters remain unchanged).</returns>
        static bool LastError(int16% errorCode, String^% errorText, GroupEnum% groupEnum);

        /// <summary>
        /// Retrieve the last/current error message details for the current thread only.
        /// </summary>
        /// <param name="errorText">The text of the error.</param>
        /// <param name="groupEnum">The group in which to search.
        /// Specify GROUP_ENUM_RSVD_LOCAL to look in the 'local' handler,
        /// GROUP_ENUM_RSVD_ALL to look in all handlers or a specific (other) enumeration
        /// to look in that specific handler.
        /// On exit (if the return code is true) it contains the group that has the error.</param>
        /// <returns>The code number of the error.</returns>
        static int16 LastError(String^% errorText, GroupEnum% groupEnum);

        /// <summary>
        /// Retrieve the last/current error message details.
        /// </summary>
        /// <param name="errorCode">The code number of the error.</param>
        /// <param name="errorText">The text of the error.</param>
        /// <param name="groupEnum">The group in which to search.
        /// Specify GROUP_ENUM_RSVD_LOCAL to look in the 'local' handler,
        /// GROUP_ENUM_RSVD_ALL to look in all handlers or a specific (other) enumeration
        /// to look in that specific handler.
        /// On exit (if the return code is true) it contains the group that has the error.</param>
        /// <param name="currentThreadOnly">true to only look at data for this current thread; false for all threads.</param>
        /// <returns>true if there is a current error and the ouput parameters have been populated, false otherwise (in which case the ouput parameters remain unchanged).</returns>
        static bool LastError(int16% errorCode, String^% errorText, GroupEnum% groupEnum, bool currentThreadOnly);

        /// <summary>
        /// Retrieve the last/current error message details.
        /// </summary>
        /// <param name="errorText">The text of the error.</param>
        /// <param name="groupEnum">The group in which to search.
        /// Specify GROUP_ENUM_RSVD_LOCAL to look in the 'local' handler,
        /// GROUP_ENUM_RSVD_ALL to look in all handlers or a specific (other) enumeration
        /// to look in that specific handler.
        /// On exit (if the return code is true) it contains the group that has the error.</param>
        /// <param name="currentThreadOnly">true to only look at data for this current thread; false for all threads.</param>
        /// <returns>The code number of the error.</returns>
        static int16 LastError(String^% errorText, GroupEnum% groupEnum, bool currentThreadOnly);

        /// <summary>
        /// Determine if there is a current error in GROUP_ENUM_RSVD_LOCAL in the current thread only.
        /// </summary>
        /// <returns>true if there is a current error, false otherwise.</returns>
        static bool IsErrorSet();

        /// <summary>
        /// Determine if there is a current error in the current thread only.
        /// </summary>
        /// <param name="groupEnum">The group to use when searching for an existing error.</param>
        /// <returns>true if there is a current error, false otherwise.</returns>
        static bool IsErrorSet(GroupEnum groupEnum);

        /// <summary>
        /// Determine if there is a current error.
        /// </summary>
        /// <param name="groupEnum">The group to use when searching for an existing error.</param>
        /// <param name="currentThreadOnly">true to only look at data for this current thread; false for all threads.</param>
        /// <returns>true if there is a current error, false otherwise.</returns>
        static bool IsErrorSet(GroupEnum groupEnum, bool currentThreadOnly);

        /// <summary>
        /// Determine if the specified debug level is enabled in any handler.
        /// </summary>
        /// <param name="debugLevel">The level for which the settings are to be determined.</param>
        /// <returns>true if the level is enabled in any handler, false otherwise.</returns>
        static bool IsDebugLevelEnabled(DebugLevel debugLevel);

        /// <summary>
        /// Clear the current error in GROUP_ENUM_RSVD_LOCAL.
        /// </summary>
        static void ClearError();


        /// <summary>
        /// Clear the current error.
        /// </summary>
        /// <param name="groupEnum">The group to which the modifications apply.</param>
        static void ClearError(GroupEnum groupEnum);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Set the current level at which debug messages are stored in GROUP_ENUM_RSVD_LOCAL.
        /// </summary>
        /// <param name="debugLevel">The level for which the settings are to be modified.</param>
        /// <param name="enable">true to enable the level(s), false to disable.</param>
        static void SetDebugLevel(DebugLevel debugLevel, bool enable);

        /// <summary>
        /// Set the current level at which debug messages are stored.
        /// </summary>
        /// <param name="debugLevel">The level for which the settings are to be modified.</param>
        /// <param name="enable">true to enable the level(s), false to disable.</param>
        /// <param name="groupEnum">The group enum to which the modifications apply.</param>
        static void SetDebugLevel(DebugLevel debugLevel, bool enable, GroupEnum groupEnum);

        /// <summary>
        /// Set the current level at which status messages are stored in GROUP_ENUM_RSVD_LOCAL.
        /// </summary>
        /// <param name="statusLevel">The level for which the settings are to be modified.</param>
        /// <param name="enable">true to enable the level(s), false to disable.</param>
        static void SetStatusLevel(StatusLevel statusLevel, bool enable);

        /// <summary>
        /// Set the current level at which status messages are stored.
        /// </summary>
        /// <param name="statusLevel">The level for which the settings are to be modified.</param>
        /// <param name="enable">true to enable the level(s), false to disable.</param>
        /// <param name="groupEnum">The group enum to which the modifications apply.</param>
        static void SetStatusLevel(StatusLevel statusLevel, bool enable, GroupEnum groupEnum);

        /////////////////////////////////////////////////////////////////////////

        // log4net-like interface

        /// <summary>
        /// Log a debug message object at Basic level.
        /// </summary>
        /// <param name="message">The message object to log.</param>
        static void DebugBasic(Object^ message);

        /// <summary>
        /// Log a debug message object at Basic level including the stack trace of the System.Exception passed as a parameter.
        /// </summary>
        /// <param name="message"/>The message object to log.</param>
        /// <param name="exception"/>The exception to log, including its stack trace.</param>
        static void DebugBasic(Object^ message, Exception^ exception);

        /// <summary>
        /// Log a formatted debug message string at Basic level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        static void DebugBasicFormat(String^ format, Object^ arg0);

        /// <summary>
        /// Log a formatted debug message string at Basic level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void DebugBasicFormat(String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted debug message string at Basic level.
        /// </summary>
        /// <param name="provider"/>A System.IFormatProvider that supplies culture-specific formatting information.</param>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void DebugBasicFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted debug message string at Basic level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        static void DebugBasicFormat(String^ format, Object^ arg0, Object^ arg1);

        /// <summary>
        /// Log a formatted debug message string at Basic level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        /// <param name="arg2"/>An Object to format.</param>
        static void DebugBasicFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2);


        /// <summary>
        /// Log a debug message object at Enhanced level.
        /// </summary>
        /// <param name="message"/>The message object to log.</param>
        static void DebugEnhanced(Object^ message);

        /// <summary>
        /// Log a debug message object at Enhanced level including the stack trace of the System.Exception passed as a parameter.
        /// </summary>
        /// <param name="message"/>The message object to log.</param>
        /// <param name="exception"/>The exception to log, including its stack trace.</param>
        static void DebugEnhanced(Object^ message, Exception^ exception);

        /// <summary>
        /// Log a formatted debug message string at Enhanced level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        static void DebugEnhancedFormat(String^ format, Object^ arg0);

        /// <summary>
        /// Log a formatted debug message string at Enhanced level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void DebugEnhancedFormat(String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted debug message string at Enhanced level.
        /// </summary>
        /// <param name="provider"/>A System.IFormatProvider that supplies culture-specific formatting information.</param>
        /// <param name=format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void DebugEnhancedFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted debug message string at Enhanced level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        static void DebugEnhancedFormat(String^ format, Object^ arg0, Object^ arg1);

        /// <summary>
        /// Log a formatted debug message string at Enhanced level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        /// <param name="arg2"/>An Object to format.</param>
        static void DebugEnhancedFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2);

        /////////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Log a status message object at Info level.
        /// </summary>
        /// <param name="message">The message object to log.</param>
        static void StatusInfo(Object^ message);

        /// <summary>
        /// Log a formatted status message string at Info level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        static void StatusInfoFormat(String^ format, Object^ arg0);

        /// <summary>
        /// Log a formatted status message string at Info level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusInfoFormat(String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Info level.
        /// </summary>
        /// <param name="provider"/>A System.IFormatProvider that supplies culture-specific formatting information.</param>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusInfoFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Info level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        static void StatusInfoFormat(String^ format, Object^ arg0, Object^ arg1);

        /// <summary>
        /// Log a formatted status message string at Info level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        /// <param name="arg2"/>An Object to format.</param>
        static void StatusInfoFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2);

        /// <summary>
        /// Log a status message object at Essential level.
        /// </summary>
        /// <param name="message">The message object to log.</param>
        static void StatusEssential(Object^ message);

        /// <summary>
        /// Log a formatted status message string at Essential level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        static void StatusEssentialFormat(String^ format, Object^ arg0);

        /// <summary>
        /// Log a formatted status message string at Essential level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusEssentialFormat(String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Essential level.
        /// </summary>
        /// <param name="provider"/>A System.IFormatProvider that supplies culture-specific formatting information.</param>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusEssentialFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Essential level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        static void StatusEssentialFormat(String^ format, Object^ arg0, Object^ arg1);

        /// <summary>
        /// Log a formatted status message string at Essential level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        /// <param name="arg2"/>An Object to format.</param>
        static void StatusEssentialFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2);

        /// <summary>
        /// Log a status message object at Warning level.
        /// </summary>
        /// <param name="message">The message object to log.</param>
        static void StatusWarning(Object^ message);

        /// <summary>
        /// Log a formatted status message string at Warning level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        static void StatusWarningFormat(String^ format, Object^ arg0);

        /// <summary>
        /// Log a formatted status message string at Warning level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusWarningFormat(String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Warning level.
        /// </summary>
        /// <param name="provider"/>A System.IFormatProvider that supplies culture-specific formatting information.</param>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusWarningFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Warning level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        static void StatusWarningFormat(String^ format, Object^ arg0, Object^ arg1);

        /// <summary>
        /// Log a formatted status message string at Warning level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        /// <param name="arg2"/>An Object to format.</param>
        static void StatusWarningFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2);

        /// <summary>
        /// Log a status message object at Error level.
        /// </summary>
        /// <param name="message">The message object to log.</param>
        static void StatusError(Object^ message);

        /// <summary>
        /// Log a formatted status message string at Error level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        static void StatusErrorFormat(String^ format, Object^ arg0);

        /// <summary>
        /// Log a formatted status message string at Error level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusErrorFormat(String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Error level.
        /// </summary>
        /// <param name="provider"/>A System.IFormatProvider that supplies culture-specific formatting information.</param>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="args"/>An Object array containing zero or more Objects to format.</param>
        static void StatusErrorFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object ^, 1>^ args);

        /// <summary>
        /// Log a formatted status message string at Error level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        static void StatusErrorFormat(String^ format, Object^ arg0, Object^ arg1);

        /// <summary>
        /// Log a formatted status message string at Error level.
        /// </summary>
        /// <param name="format"/>A String containing zero or more format items.</param>
        /// <param name="arg0"/>An Object to format.</param>
        /// <param name="arg1"/>An Object to format.</param>
        /// <param name="arg2"/>An Object to format.</param>
        static void StatusErrorFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2);

        /////////////////////////////////////////////////////////////////////////

    };

    /////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////

    class CUnmanagedMessageHandler
    {
    private:
        gcroot<MessageHandler^>     mParent;

    public:
        CUnmanagedMessageHandler(MessageHandler^ parent);

        virtual void OnProgress(int16 progress);

        virtual void OnStatus(StatusLevels level, const std::string& response);

        virtual void OnDebug(DebugLevels level, const std::string& response);
    };

    /////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////

    class CUnmanagedEngineObserver
        : public CMessageHandlerObserver
    {
    private:
        CUnmanagedMessageHandler*               mObserver;

    public:
        CUnmanagedEngineObserver();
        ~CUnmanagedEngineObserver();

        virtual void NotifyText(uint32 aRsvdForFuture, CMessageHandler::MessageType major, uint32 minor, const char* str);

        virtual void NotifyNumber(uint32 aRsvdForFuture, CMessageHandler::MessageType major, uint32 minor, int32 number);

        void SetObserver(CUnmanagedMessageHandler* ob);
    };

    /////////////////////////////////////////////////////////////////////////

} } } }
