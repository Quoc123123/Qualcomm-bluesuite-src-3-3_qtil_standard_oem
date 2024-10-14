/**********************************************************************
 *
 *  cmdline.cpp
 *
 *  Copyright (c) 2010-2022 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Command line parsing class
 *
 ***********************************************************************/


#include <errno.h>
#include <assert.h>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <list>
#include "cmdline.h"
#include "stdarg.h"
#include "engine/enginefw_interface.h"
#include "misc/stringutil.h"

#if defined(WIN32) && !defined(WINCE)
#include "misc/registry.h"
#include "misc/sysutil.h"
#endif

using namespace std;

// Avoid unused variable warnings where a temporary variable is used only for an assert.
#ifdef DEBUG
 #define ASSERT(expression) assert(expression)
#else
 #define ASSERT(expression) (void)(expression)
#endif

// If unit testing is enabled override ASSERT
#if UNIT_TESTING
extern void unittest_assert(const int result, const char* const expression,
                        const char * const file, const int line);
#undef ASSERT
#define ASSERT(expression) \
    unittest_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif // UNIT_TESTING

#define HELP_INDENT        (4)  //!< Number of spaces to indent help
#define HELP_DISPLAY_WIDTH (79) //!< Maximum length of each line of help


#define AUTO_CHAIN_LIST_NAME         II("auto_chain")

static const int DEFAULT_UNTERMINATED_LIST_START_INDEX = -1;

// Automatically add any non-class methods to the CMDLINE group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_CMDLINE_LIB

static iistringstream gTempFormatStr;

// the following are used outwith the class in global matching functions

static istring gMatchItem;               //!< Global string that is setup when using find_if on lists

static int gRequiredFloatingIndex = 0;         //!< Incrementing index when retrieving floating parameters
static int gCurrentFloatingIndex  = 0;         //!< Current floating parameter

/**
 *  Returns a string representing info about the parameter name / synonym  - intended for use in error / debug messages.
 *  If no synonym was supplied, the additional text is "<param main name>".
 *  Otherwise, the text is of the form "<synonym> (synonym of <param main name>)".
 *  @param[in] aParam The parameter
 *  @return information text for parameter
 */
istring GetParamNameOutputText(const ParamStruct &aParam);

/////////////////////////////////////////////////////////////////////////////

CCmdLine::CCmdLine
(
    const istring& aProgramName,
    const istring& aProgramTitle,
    const istring& aProgramDescription,
    const istring& aCopyrightStartYear,
    int     aArgc,
    ichar*  apArgv[]
)
    :
    mCurrentChainedCmdIndex(DEFAULT_UNTERMINATED_LIST_START_INDEX),
    mNumChainedCommands(0),
    mProgramName(aProgramName),
    mProgramTitle(aProgramTitle),
    mProgramDescription(aProgramDescription),
    mCopyrightStartYear(aCopyrightStartYear),
    mProgramArgc(aArgc),
    mppProgramArgv(apArgv),
    // Default to printing error if invalid command line, outputting help etc. Turned off if this class is used by a tool
    // the doesn't define a command line
    mCommandLineSupport(true),
    mParseResult(PARSE_SUCCESS),
    mFlags(0)
{
    MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_CMDLINE_LIB);
    FUNCTION_DEBUG_SENTRY;

    MSG_HANDLER.SendAllNonDebugOutputToTheConsole();

    for (int i = 0; i < PRE_PARSE_NUM_FLAGS; i++)
    {
        mPreParseFlags[i] = false;
    }

    // Do a quick pre-parse to extract anything we need asap
    PreParseCmdLine();

    SetExpectedParam(CMDLINE_QUIET_PARAM,  II("Suppress non-essential output"),       NOT_MANDATORY, NOT_HIDDEN);

    SetExpectedParam(CMDLINE_HELP1_PARAM, II("Display this help text"), NOT_MANDATORY, NOT_HIDDEN);

    SetExpectedParamSynonym(CMDLINE_HELP2_PARAM);
    SetExpectedParamSynonym(CMDLINE_HELP3_PARAM);
    SetExpectedParamSynonym(CMDLINE_HELP4_PARAM);
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsParameterAFlag(const istring& aName)
{
    bool flag = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, flag);

    ASSERT(!aName.empty());
    if (!aName.empty())
    {
        flag = (aName[0] == '-');
    }
    return flag;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsCompactHelpFlagOn()
{
    bool ret;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    ret = ((mFlags & CMDLINE_FLAG_COMPACT_HELP) != 0);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsAutoChainFlagOn()
{
    bool ret;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    ret = ((mFlags & CMDLINE_FLAG_AUTO_CHAIN) != 0);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

/// Match function when looking for a list name
bool MatchListName(const ListStruct& aListStruct)
{
    bool ret;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    ret = (istricmp(aListStruct.Name.c_str(), gMatchItem.c_str()) == 0);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

/// Match function when looking for a parameter
bool MatchParamName(const ParamStruct& aParamStruct)
{
    bool found = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, found);

    for (std::list<istring>::const_iterator it = aParamStruct.Names.begin(); (it != aParamStruct.Names.end()) && !found; ++it)
    {
        if (istricmp(it->c_str(), gMatchItem.c_str()) == 0)
        {
            found = true;
        }
    }

    return found;
}

/////////////////////////////////////////////////////////////////////////////

/// Match function when looking for a floating value
bool MatchParamFloating(const ParamStruct& aParamStruct)
{
    bool matched;
    FUNCTION_DEBUG_SENTRY_RET(bool, matched);

    matched = aParamStruct.IsFloating;
    if (matched)
    {
        // Ignore floating parameters unless they match the current index
        matched = (gCurrentFloatingIndex == gRequiredFloatingIndex);
        gCurrentFloatingIndex++;
        if (matched)
        {
            gRequiredFloatingIndex++;   // Full match - don't return this one next time
        }
    }
    return matched;
}

/////////////////////////////////////////////////////////////////////////////

istring GetParamNameOutputText(const ParamStruct& aParam)
{
    istring paramNameText;
    FUNCTION_DEBUG_SENTRY_RET(istring, paramNameText);

    iistringstream tempStream; // Don't use gTempFormatStr, so that this fn can insert into gTempFormatStr

    if (aParam.EncounteredName.empty() || aParam.Names.size() == 1 || (istricmp(aParam.EncounteredName.c_str(), aParam.Names.begin()->c_str()) == 0))
    {
        tempStream << II("\"") << aParam.Names.begin()->c_str() << II("\"");
    }
    else
    {
        tempStream << II("\"") << aParam.EncounteredName.c_str() << II("\"");
    }

    paramNameText = tempStream.str();

    return paramNameText;
}

/////////////////////////////////////////////////////////////////////////////

/// Match function when looking for an encountered param
bool MatchParamEncountered(const ParamStruct& aParamStruct)
{
    bool encountered;
    FUNCTION_DEBUG_SENTRY_RET(bool, encountered);

    encountered = MatchParamName(aParamStruct);
    if (encountered)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "%s was given", GetParamNameOutputText(aParamStruct).c_str());
    }
    return encountered;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::DisplayThisParameter(std::vector<ParamStruct>::const_iterator aParam)
{
    bool ret = true;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::DebugPrintParams()
{
    FUNCTION_DEBUG_SENTRY;

    for (std::vector<ParamStruct>::const_iterator p = mDefinedParamList.begin(); p != mDefinedParamList.end(); ++p)
    {
        iprintf(II("%s\n"), p->Names.begin()->c_str());

        for (std::vector<ValueStruct>::const_iterator v = p->ValueList.begin(); v != p->ValueList.end(); ++v)
        {
            if (v->DataType == DATA_TYPE_INTEGER)
            {
                printf("    integer");
            }
            else if (v->DataType == DATA_TYPE_POSITIVE_INTEGER)
            {
                printf("    +ve integer");
            }
            else if (v->DataType == DATA_TYPE_UNDECORATED_HEX)
            {
                printf("    undecorated hex");
            }
            else if (v->DataType == DATA_TYPE_STRING)
            {
                printf("    string");
            }
            else
            {
                // Unknown data type
                ASSERT(0);
            }
            iprintf(II(" = %s\n"), v->ValueGiven.c_str());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::ErrUnknownParameter(const ichar *apParamStr)
{
    FUNCTION_DEBUG_SENTRY;

    gTempFormatStr.str(II(""));
    gTempFormatStr << II("Unrecognised parameter, got \"") << apParamStr << II("\"");
    mLastError = gTempFormatStr.str();
    return PARSE_ERR_UNRECOGNISED_PARAMETER;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ErrGetParamInvalidIndex
(
    unsigned int aIndex,
    const ichar *apName
)
{
    FUNCTION_DEBUG_SENTRY;

    gTempFormatStr.str(II(""));
    gTempFormatStr << II("Got invalid index of ") << aIndex << II(" for parameter \"") << apName << II("\"");
    mLastError = gTempFormatStr.str();
    MSG_HANDLER.NotifyStatus(STATUS_WARNING, inarrow(mLastError).c_str());

    ASSERT(0); // Application should not request value for an index it has not setup
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::HandleMandatoryParamValueNotGiven(const ichar *apName, const ValueStruct& aValueStruct)
{
    FUNCTION_DEBUG_SENTRY;

    std::vector<ParamStruct>::iterator param;
    bool unused;
    const bool found = FindParam(apName, param, MatchParamName, unused);
    ASSERT(found);

    // If the value was mandatory then it should have been rejected by Parse()
    if (aValueStruct.IsMandatory == MANDATORY)
    {
        gTempFormatStr.str(II(""));
        gTempFormatStr << II("No value given for <") << aValueStruct.ValueHelpType << II("> with parameter ") << GetParamNameOutputText(*param).c_str();
        mLastError = gTempFormatStr.str();
        MSG_HANDLER.NotifyStatus(STATUS_WARNING, inarrow(mLastError).c_str());
        ASSERT(0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputVersioningAndCopyright()
{
    FUNCTION_DEBUG_SENTRY;

    iprintf(II("%s, version %hs\n"), mProgramName.c_str(), VERSION_BLUESUITE_STR);

    // If the special build string is set then show the date of the build
    if (strcmp(VERSION_SPECIAL_BUILD, "") != 0)
    {
        printf("%s\n", VERSION_SPECIAL_BUILD);
    }

    string copyright = VERSION_COPYRIGHT_NO_START_YEAR;
    // If the start year is earlier than the current year, switch to range.
    if (istring(II(VERSION_YEAR)) != mCopyrightStartYear)
    {
        // Find start of year
        string::size_type startPos = copyright.find_first_of("123456789");
        if (startPos != string::npos)
        {
            // Check year is the expected length
            string::size_type endPos = copyright.find_first_not_of("0123456789", startPos);
            const string endYear = VERSION_YEAR;
            if (endPos != string::npos && endPos - startPos == endYear.length())
            {
                stringstream yearRange;
                yearRange << inarrow(mCopyrightStartYear) << "-" << endYear;
                copyright.replace(startPos, endYear.length(), yearRange.str());
            }
        }
    }
    printf("%s\n", copyright.c_str());

    printf("\n\n");
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::HandleFlag(unsigned int aFlag, unsigned int &aFlags)
{
    bool flagIsOn;
    FUNCTION_DEBUG_SENTRY_RET(bool, flagIsOn);
    
    flagIsOn = ((mFlags & aFlag) != 0);

    if (flagIsOn)
    {
        // Remove flags from bits set
        aFlags &= ~aFlag;
    }

    return flagIsOn;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddFlags(unsigned int aFlags)
{
    FUNCTION_DEBUG_SENTRY;
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFlags=0x%x", aFlags);

    mFlags = aFlags;
    if (HandleFlag(CMDLINE_FLAG_NORUN, aFlags))
    {
        SetExpectedParam(CMDLINE_NORUN_PARAM, II("Do not automatically restart the chip afterwards"), NOT_MANDATORY, NOT_HIDDEN);
    }
    if (HandleFlag(CMDLINE_FLAG_TRUNLOCK, aFlags))
    {
        SetExpectedParam(CMDLINE_TRUNLOCK_PARAM, II("Unlocks a transport-lockable chip"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_STRING, II("key or file"), II("Key value or path to file containing the key value"), MANDATORY);
        AddExpectedValue(DATA_TYPE_STRING, II("key type"), 
            II("One of UNENC or ENC. Specifies whether <key or file> represents a key which is unencrypted or encrypted. Default value is UNENC. Applicable to USB Debug transport only. Ignored for all zeros key"), NOT_MANDATORY);
    }
    if (HandleFlag(CMDLINE_FLAG_MACHINE, aFlags))
    {
        SetExpectedParam(CMDLINE_MACHINE_PARAM,
            II("Shows information in an easily parsed form (if appropriate)"), NOT_MANDATORY, NOT_HIDDEN);
    }
    if (HandleFlag(CMDLINE_FLAG_RAW, aFlags))
    {
        SetExpectedParam(CMDLINE_RAW_PARAM,
            II("Operates in 'raw' mode, the details of which vary depending on context"), NOT_MANDATORY, NOT_HIDDEN);
    }

    // The following flags use mFlags directly when tested. Call fn just to remove from set bits
    HandleFlag(CMDLINE_FLAG_COMPACT_HELP, aFlags);
    HandleFlag(CMDLINE_FLAG_AUTO_CHAIN, aFlags);

    // If there are any bits still set after running through the flags that
    // are supported, raise an error.
    if (aFlags)
    {
        ASSERT(false);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddTransportOptions(unsigned int aFlags)
{
    FUNCTION_DEBUG_SENTRY;
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFlags=0x%x", aFlags);


    // Make sure user cannot use more than one transport at the same time
    CreateList(LIST_EXCLUSIVE, CMDLINE_TRANS_LIST);
    if ((CMDLINE_TRANS_FLAG_SPI & aFlags) != 0)
    {
        SetExpectedParam(CMDLINE_USB_PARAM, II("Indicates USB-SPI port"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"), II("USB-SPI port e.g. -usb 0 or -usb nnnnnn (where nnnnnn is the serial number of the USB-SPI converter)"), MANDATORY);
        AddToList(CMDLINE_TRANS_LIST, CMDLINE_USB_PARAM);

#if defined(WIN32) && !defined(WINCE)
        // LPT-SPI is only supported on 32-bit Windows
        uint32 winVerMajor;
        uint32 winVerMinor;
        bool is64Bit;
        if (sysutil::GetWinVersion(winVerMajor, winVerMinor, is64Bit) && !is64Bit)
        {
            SetExpectedParam(CMDLINE_LPT_PARAM, II("Indicates LPT-SPI port"), NOT_MANDATORY, NOT_HIDDEN);
            AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"), II("LPT-SPI port e.g. -lpt 1 to select LPT1"), MANDATORY);
            AddToList(CMDLINE_TRANS_LIST, CMDLINE_LPT_PARAM);
        }
#endif

        SetExpectedParam(CMDLINE_MUL_PARAM, II("Indicates SPI multi-port"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"), II("SPI multi-port (a number from 0 upwards; typically upto 15) e.g. -mul 2"), MANDATORY);
    }


    if ((CMDLINE_TRANS_FLAG_TRB & aFlags) != 0)
    {
        SetExpectedParam(CMDLINE_TRB_PARAM, II("Indicates transaction bus port"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"),
            II("The port number; either specify the sequence number starting from 1 or specify the individual serial number of the dongle"), MANDATORY);
        AddToList(CMDLINE_TRANS_LIST, CMDLINE_TRB_PARAM);
    }

#if !defined(WIN32)
    if ((CMDLINE_TRANS_FLAG_QS & aFlags) != 0)
    {
        SetExpectedParam(CMDLINE_QS_PARAM, II("Indicates QS port"), NOT_MANDATORY, NOT_HIDDEN);
        AddToList(CMDLINE_TRANS_LIST, CMDLINE_QS_PARAM);
    }
#endif

    if ((CMDLINE_TRANS_FLAG_USBDBG & aFlags) != 0)
    {
        SetExpectedParam(CMDLINE_USBDBG_PARAM, II("Indicates USB Debug port"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"),
            II("The port number; specify the enumeration sequence number of the device starting from 1"), MANDATORY);
        AddToList(CMDLINE_TRANS_LIST, CMDLINE_USBDBG_PARAM);
        SetExpectedParamSynonym(CMDLINE_USBDBG2_PARAM);
    }

    if ((CMDLINE_TRANS_FLAG_USBCC & aFlags) != 0)
    {
        SetExpectedParam(CMDLINE_USBCC_PARAM, II("Indicates USB charger case port"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"),
            II("The port number; specify the enumeration sequence number of the device starting from 1"), MANDATORY);
        AddToList(CMDLINE_TRANS_LIST, CMDLINE_USBCC_PARAM);
    }

    if ((CMDLINE_TRANS_FLAG_ADBBT & aFlags) != 0)
    {
        SetExpectedParam(CMDLINE_ADBBT_PARAM, II("Indicates ADB-BT port"), NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("port"),
            II("The port number; specify the enumeration sequence number of the device starting from 1"), MANDATORY);
        AddToList(CMDLINE_TRANS_LIST, CMDLINE_ADBBT_PARAM);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddBroadSupport
(
    const istring& aListName,
    const istring& aAdditionalHelp
)
{
    FUNCTION_DEBUG_SENTRY;

    // Work out the help string
    istring helpText = II("Broadcast the image to all devices in the mask");
    if (!aAdditionalHelp.empty())
    {
        helpText += II(". ");
        helpText += aAdditionalHelp;
    }

    // Add the "-broad" parameter
    SetExpectedParam(CMDLINE_BROAD_PARAM, helpText, NOT_MANDATORY, NOT_HIDDEN);
        AddExpectedValue(DATA_TYPE_STRING, II("file"), II("The file to use"), MANDATORY);
        AddExpectedValue(DATA_TYPE_POSITIVE_INTEGER, II("mask"), II("A binary mask of devices (or 0 for all devices found)"), MANDATORY);

    // If the listname is not empty, add it to the list
    if (!aListName.empty())
    {
        AddToList(aListName, CMDLINE_BROAD_PARAM);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::SetExpectedParam
(
    const istring& aName,
    const istring& aHelpText,
    MandatoryEnum aIsMandatory,
    HiddenEnum aIsHidden
)
{
    FUNCTION_DEBUG_SENTRY;

    // Make sure a duplicate isn't being added
    std::vector<ParamStruct>::iterator notUsed;
    bool unused;
    const bool found = FindParam(aName.c_str(), notUsed, MatchParamName, unused);
    ASSERT(!found);

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Expected param : %s", aName.c_str());

    ParamStruct paramStruct;
    paramStruct.IsMandatory = aIsMandatory;
    paramStruct.IsFloating  = false;
    paramStruct.Names.push_front(aName);
    paramStruct.HelpText    = aHelpText;
    paramStruct.EncounteredName = II("");
    paramStruct.UnterminatedListStartIndex = DEFAULT_UNTERMINATED_LIST_START_INDEX;
    paramStruct.mpData = NULL;

    mDefinedParamList.push_back(paramStruct);
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::ParamExists(const istring& aName)
{
    bool ret;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    std::vector<ParamStruct>::iterator param;
    bool unused;
    ret = FindParam(aName.c_str(), param, MatchParamName, unused);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::SetExpectedParam
(
    const istring& aName,
    const istring& aHelpText,
    MandatoryEnum aIsMandatory,
    HiddenEnum aIsHidden,
    CmdDelegate aHandler,
    void* apData
)
{
    FUNCTION_DEBUG_SENTRY;

    SetExpectedParam(aName, aHelpText, aIsMandatory, aIsHidden);

    mDefinedParamList.back().CmdHandler = aHandler;
    mDefinedParamList.back().mpData = apData;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::SetExpectedParamSynonym
(
    const istring& aSynonym
)
{
    FUNCTION_DEBUG_SENTRY;

    // Make sure this synonym isn't a duplicate of any other parameter name
    // or synonym
    std::vector<ParamStruct>::iterator notUsed;
    bool unused;
    const bool found = FindParam(aSynonym.c_str(), notUsed, MatchParamName, unused);
    ASSERT(!found);

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Expected param synonym: %s", aSynonym.c_str());

    // Cannot add a synonym for a floating parameter
    ASSERT(!mDefinedParamList.back().IsFloating);

    mDefinedParamList.back().Names.push_back(aSynonym);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::SetExpectedFloatingParam
(
    const istring& aName,
    const istring& aValueHelpType,
    const istring& aValueHelpDescription,
    MandatoryEnum aIsMandatory
)
{
    FUNCTION_DEBUG_SENTRY;

    ParamStruct paramStruct;
    paramStruct.IsMandatory = aIsMandatory;
    paramStruct.IsFloating  = true;
    paramStruct.Names.push_front(aName);
    paramStruct.HelpText    = II("");
    paramStruct.EncounteredName = II("");
    paramStruct.UnterminatedListStartIndex = DEFAULT_UNTERMINATED_LIST_START_INDEX;
    paramStruct.mpData = NULL;

    mDefinedParamList.push_back(paramStruct);

    // Add a dummy entry for the floating value (it's type is arbitrary)
    // The dummy entry is used when parsing the command line
    // e.g. to store the value for the floating parameter
    SetupValue(DATA_TYPE_STRING, aValueHelpType, aValueHelpDescription, MANDATORY);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::SetupValue
(
    DataTypeEnum aDataType,
    const istring& aValueHelpType,
    const istring& aValueHelpDescription,
    MandatoryEnum aIsMandatory
)
{
    FUNCTION_DEBUG_SENTRY;

    ValueStruct valueStruct;

    valueStruct.DataType = aDataType;
    valueStruct.ValueHelpType = aValueHelpType;
    valueStruct.ValueHelpDescription = aValueHelpDescription;
    valueStruct.ValueGiven = II("");
    valueStruct.IsMandatory = aIsMandatory;

    // Only allow a mandatory value if all previous values have been mandatory,
    // but since every element (apart from the last one) has already been
    // checked, it is only necessary to check the last one here.
    // Because quiet and hidden are always supported, there will always be
    // a parameter defined.
    if (aIsMandatory == MANDATORY &&
        !mDefinedParamList.back().ValueList.empty() &&
        mDefinedParamList.back().ValueList.back().IsMandatory == NOT_MANDATORY)
    {
        ASSERT(false);
    }

    mDefinedParamList.back().ValueList.push_back(valueStruct);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddExpectedUnterminatedList
(
    DataTypeEnum aDataType,
    const istring& aValueHelpType,
    const istring& aValueHelpDescription
)
{
    FUNCTION_DEBUG_SENTRY;

    // A parameter can only support one unterminated list
    ASSERT(mDefinedParamList.back().UnterminatedListStartIndex == DEFAULT_UNTERMINATED_LIST_START_INDEX);

    mDefinedParamList.back().UnterminatedListStartIndex = static_cast<int>(mDefinedParamList.back().ValueList.size());

    SetupValue(aDataType, aValueHelpType, aValueHelpDescription, MANDATORY); // Must give at least one value
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddExpectedValue
(
    DataTypeEnum aDataType,
    const istring& aValueHelpType,
    const istring& aValueHelpDescription,
    MandatoryEnum aIsMandatory
)
{
    FUNCTION_DEBUG_SENTRY;

    ParamStruct paramStruct = mDefinedParamList.back();

    // Unterminated lists must be the last values so cannot have added one previously
    ASSERT(mDefinedParamList.back().UnterminatedListStartIndex == DEFAULT_UNTERMINATED_LIST_START_INDEX);

    // Cannot add a value to a floating parameter
    ASSERT(!paramStruct.IsFloating);

    SetupValue(aDataType, aValueHelpType, aValueHelpDescription, aIsMandatory);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddExpectedPossibleList(const istring& aValueHelpType, MandatoryEnum aIsMandatory)
{
    FUNCTION_DEBUG_SENTRY;
    // "Type" of value list isn't used
    SetupValue(DATA_TYPE_INTEGER, aValueHelpType, II(""), aIsMandatory);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddExpectedPossibleListValue(const istring& aPossibleValueStr)
{
    FUNCTION_DEBUG_SENTRY;

    // Make sure AddExpectedPossibleList has been called
    ASSERT(!mDefinedParamList.back().ValueList.empty());

    // Add the possible value to the current parameters value list
    mDefinedParamList.back().ValueList.back().PossibleValuesList.push_back(aPossibleValueStr);
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::FindParam
(
    const ichar *apParamStr,
    std::vector<ParamStruct>::iterator& aMatchedParam,
    bool (*aMatchFn) (const ParamStruct& aParamStruct),
    bool& aEncounteredSynonym,
    InternalListToSearchEnum internalListToSearch
)
{
    bool ret = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    std::vector<ParamStruct>::iterator listStart;
    std::vector<ParamStruct>::iterator listEnd;

    switch (internalListToSearch)
    {
    case LIST_DEFINED:
        listStart = mDefinedParamList.begin();
        listEnd = mDefinedParamList.end();
        break;
    case LIST_ENCOUNTERED:
        listStart = mEncounteredParamList.begin();
        listEnd = mEncounteredParamList.end();
        break;
    default:
        ASSERT(0);
    }

    gMatchItem = apParamStr;
    std::vector<ParamStruct>::iterator p = find_if(listStart, listEnd, aMatchFn);
    bool matched = (p != listEnd);
    aEncounteredSynonym = false;
    if (matched && istricmp(p->Names.front().c_str(), apParamStr) != 0)
    {
        aEncounteredSynonym = true;
    }

    if (matched && (DisplayThisParameter(p)))
    {
        ret = true;
    }

    // Always return the iterator, even if not found
    aMatchedParam = p;

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::MatchParameter
(
    const ichar *apParamStr,
    bool aLookForFloatingParams,
    std::vector<ParamStruct>::iterator& aMatchedParam
)
{
    bool matched;
    FUNCTION_DEBUG_SENTRY_RET(bool, matched);

    // firstly look for a match name, otherwise find a floating value
    bool foundSynonym;
    matched = FindParam(apParamStr, aMatchedParam, MatchParamName, foundSynonym);

    if (matched && foundSynonym)
    {
        std::ostringstream msg;
        msg << "\"" << inarrow(apParamStr).c_str() << "\" is deprecated; please use \"" <<
            inarrow(aMatchedParam->Names.front()).c_str() << "\" instead.",
        MSG_HANDLER.NotifyStatus(STATUS_WARNING, msg.str());
    }

    if (aLookForFloatingParams && !matched)
    {
        gCurrentFloatingIndex = 0;
        matched = FindParam(apParamStr, aMatchedParam, MatchParamFloating, foundSynonym);
    }

    return matched;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::ParamWasSpecified(const istring& aParamName)
{
    bool ret;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    std::vector<ParamStruct>::iterator matchedParam;
    bool unused;
    ret = FindParam(aParamName.c_str(), matchedParam, MatchParamEncountered, unused, LIST_ENCOUNTERED);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::CheckList(const ListStruct& aListEntry)
{
    ParseResultEnum result = PARSE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, result);

    bool paramSpecified = false;
    istring firstParamFound;

    for (std::vector<istring>::const_iterator p = aListEntry.ParameterList.begin();
          (p != aListEntry.ParameterList.end()) && (result == PARSE_SUCCESS);
          ++p)
    {
        bool thisParamSpecified = ParamWasSpecified(*p);

        if (thisParamSpecified)
        {
            if (!paramSpecified)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Found first parameter to be specified");
                paramSpecified = true;
                firstParamFound = *p;

                if (aListEntry.ListType == LIST_COMPULSORY)
                {
                    break; //No need to validate for more than one command as these are allowed for compulsory lists
                }
            }
            else
            {
                // Only one entry from a unique or exclusive list can be given
                if ((aListEntry.ListType == LIST_UNIQUE) || (aListEntry.ListType == LIST_EXCLUSIVE))
                {
                    std::vector<ParamStruct>::iterator thisParam;
                    std::vector<ParamStruct>::iterator firstParam;
                    bool found;
                    bool unused;

                    found = FindParam(p->c_str(), thisParam, MatchParamName, unused);
                    ASSERT(found);
                    found = FindParam(firstParamFound.c_str(), firstParam, MatchParamName, unused);
                    ASSERT(found);

                    gTempFormatStr << II("Parameters ") << GetParamNameOutputText(*thisParam).c_str() << II(" and ") 
                        << GetParamNameOutputText(*firstParam).c_str() << II(" cannot be used at the same time");
                    mLastError = gTempFormatStr.str();
                    result = PARSE_ERR_LIST_IS_EXCLUSIVE;
                }
            }
        }
    }

    // If the list is "unique" or "compulsory", or it is the autochain list
    // make sure a parameter from the list was given
    if ((result == PARSE_SUCCESS) &&
         ( (aListEntry.ListType == LIST_UNIQUE) ||
           (aListEntry.ListType == LIST_COMPULSORY) ||
           (IsAutoChainFlagOn() && (istrcmp(aListEntry.Name.c_str(), AUTO_CHAIN_LIST_NAME) == 0))
         )
       )
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "A parameter must be specified for a unique or compulsory list");
        if (!paramSpecified)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "No parameter from list specified");
            gTempFormatStr << II("One of the following parameters must be given: ") << endl;
            size_t currentLineLength = 0;

            for (std::vector<istring>::const_iterator p = aListEntry.ParameterList.begin();
                    (p != aListEntry.ParameterList.end()); ++p)
            {
                // Make sure it's a parameter we can tell the user about
                std::vector<ParamStruct>::iterator matchedParam;
                bool unused;

                bool recognisedParameter = FindParam(p->c_str(), matchedParam, MatchParamName, unused, LIST_DEFINED
                );

                if (recognisedParameter)
                {
                    if ((currentLineLength + p->size() + 3) >= HELP_DISPLAY_WIDTH)
                    {
                        gTempFormatStr << endl;
                        currentLineLength = 0;
                    }
                    gTempFormatStr << II("\"") << p->c_str() << II("\" ");
                    currentLineLength += p->size() + 3;
                }
            }
            mLastError = gTempFormatStr.str();
            result = PARSE_ERR_NONE_FROM_LIST;
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::CheckAssociatedLists()
{
    ParseResultEnum result = PARSE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, result);

    if (!mAssociatedParametersList.empty())
    {
        for (std::vector<ListStruct>::iterator l = mAssociatedParametersList.begin();
                                (l != mAssociatedParametersList.end()) && (result == PARSE_SUCCESS) ; ++l)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Checking list \"%s\"", l->Name.c_str());

            switch (l->ListType)
            {
            case LIST_UNIQUE:
            case LIST_EXCLUSIVE:
            case LIST_COMPULSORY:
            case LIST_CHAINABLE:
                result = CheckList(*l);
                break;

            default:
                ASSERT(0);
            }
        }
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "No associated lists to check");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PreParseCmdLine()
{
    FUNCTION_DEBUG_SENTRY;
    bool setDebugLevel = false;

    for (int i = 1; i < mProgramArgc; i++)
    {

        // "-quiet"?
        if (istricmp(mppProgramArgv[i], CMDLINE_QUIET_PARAM) == 0)
        {
            MSG_HANDLER.SetStatusLevel(STATUS_INFO, false, CMessageHandler::GROUP_ENUM_RSVD_ALL);
            MSG_HANDLER.SetStatusLevel(STATUS_WARNING, false, CMessageHandler::GROUP_ENUM_RSVD_ALL);
            mPreParseFlags[PRE_PARSE_QUIET] = true;
        }
    }

    // If not read debug level from command line, see if set in registry
    if (!setDebugLevel)
    {
#if defined(WIN32) && !defined(WINCE)
        std::string debugLevel;
        const DWORD ret = registry::GetStringFromRegistry(HKEY_CURRENT_USER, REGISTRY_ROOT_KEY, REGISTRY_DEBUG_LEVEL, debugLevel);
        // Do nothing if registry cannot be read
        if (ret == 0)
        {
            MSG_HANDLER.ProcessDebugRequest(inarrow(debugLevel), true);
        }
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////

DataTypeEnum CCmdLine::GetMatchedParamDataType
(
    std::vector<ParamStruct>::const_iterator aMatchedParam,
    int aValueListIndex
)
{
    DataTypeEnum ret;
    FUNCTION_DEBUG_SENTRY_RET(DataTypeEnum, ret);

    ret = aMatchedParam->ValueList.at(aValueListIndex).DataType;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::ValidateAgainstPossibleValues
(
    std::vector<ParamStruct>::const_iterator aMatchedParam,
    unsigned int aNumValues,
    const ichar* apValue
)
{
    ParseResultEnum result = PARSE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, result);

    gTempFormatStr.str(II(""));

    // Build up an error string in case it does not match
    gTempFormatStr << II("Value for \"") << aMatchedParam->Names.begin()->c_str() << II("\" must be one of: ");
    bool found = false;

    const std::vector<istring> *possibleValuesList = &(aMatchedParam->ValueList.at(aNumValues).PossibleValuesList);
    for (std::vector<istring>::const_iterator it = possibleValuesList->begin(); it != possibleValuesList->end(); ++it)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Checking against possible value %s", it->c_str());
        if (it != possibleValuesList->begin())
        {
            gTempFormatStr << II(",");
        }
        gTempFormatStr << II("\"") << it->c_str() << II("\"");
        if (istricmp(it->c_str(), apValue) == 0)
        {
            found = true;
        }
    }

    gTempFormatStr << II(", got \"") << apValue << II("\"");
    if (!found)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Passed value not in possible values");
        mLastError = gTempFormatStr.str();
        result = PARSE_ERR_NOT_A_POSSIBLE_VALUE;
    }
    else
    {
        // Not using error string
        gTempFormatStr.str(II(""));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::ValidateValues
(
    std::vector<ParamStruct>::iterator aMatchedParam,
    unsigned int& aValueIndex,
    const ichar* apValue
)
{
    ParseResultEnum result = PARSE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, result);

    istring strToConvert = apValue;

    // If there is a list of possible values then must match one of them
    if (!aMatchedParam->ValueList.at(aValueIndex).PossibleValuesList.empty())
    {
        result = ValidateAgainstPossibleValues(aMatchedParam, aValueIndex, apValue);
    }
    else
    {
        DataTypeEnum dataType = GetMatchedParamDataType(aMatchedParam, aValueIndex);
        // No list of possible values, match against expected type
        if (dataType == DATA_TYPE_UNDECORATED_HEX)
        {
            strToConvert = PrefixUndecoratedHex(apValue);
        }

        if ((dataType == DATA_TYPE_INTEGER) || (dataType == DATA_TYPE_POSITIVE_INTEGER)
            || (dataType == DATA_TYPE_UNDECORATED_HEX))
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Expecting an integer");
            int64_t val;
            uint64_t uval;
            ichar *pEndptr = NULL;
            int base = 0;
            const ichar *pStr = strToConvert.c_str();
            switch (dataType)
            {
            case DATA_TYPE_INTEGER:
                errno = 0;  /* Clear outstanding errors */
                val = ISTRTOI64(pStr, &pEndptr, 0 /* allow prefixes */);
                if (errno != 0 || (pEndptr && *pEndptr  != '\0') || (!val && !istrchr(pStr, '0')))
                {
                    if (errno == ERANGE)
                    {
                        result = PARSE_ERR_OUT_OF_RANGE;
                    }
                    else
                    {
                        result = PARSE_ERR_NOT_AN_INTEGER;
                    }
                }
                break;
            case DATA_TYPE_UNDECORATED_HEX:
                base = 16;
                /* drop thru */
            case DATA_TYPE_POSITIVE_INTEGER:
                errno = 0;  /* Clear outstanding errors */
                uval = ISTRTOUI64(pStr, &pEndptr, base);
                if (errno != 0 || (pEndptr && *pEndptr  != '\0') || (!uval && !istrchr(pStr, '0')))
                {
                    if (errno == ERANGE)
                    {
                        result = PARSE_ERR_OUT_OF_RANGE;
                    }
                    else
                    {
                        result = PARSE_ERR_NOT_AN_INTEGER;
                    }
                }
                else
                {   /* strtoul does not reject negative values despite being unsigned */
                    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Check for positive integer");
                    if (istrchr(pStr, '-'))
                    {
                        result = PARSE_ERR_EXPECTED_POSITIVE_INTEGER;
                    }
                }
                break;
            default:
                // We already checked that we're dealing with one of the explicitly handled
                // cases before this switch statement, so nothing to do here.
                break;
            }

            if (result != PARSE_SUCCESS)
            {
                if (result == PARSE_ERR_OUT_OF_RANGE)
                {
                    gTempFormatStr << II("Value out of range for ");
                    gTempFormatStr << (dataType == DATA_TYPE_INTEGER ? II(""): II("unsigned "));
                    gTempFormatStr << II("64-bit integer for ");
                }
                else if (dataType == DATA_TYPE_POSITIVE_INTEGER)
                {
                    gTempFormatStr << II("Expecting a positive integer for ");
                }
                else if (dataType == DATA_TYPE_UNDECORATED_HEX)
                {
                    gTempFormatStr << II("Expecting an undecorated hex value for ");
                }
                else
                {
                    gTempFormatStr << II("Expecting an integer for ");
                }
                gTempFormatStr << GetParamNameOutputText(*aMatchedParam).c_str() << II(", got \"") << apValue << II("\"");
            }
        }
    }
    if (result == PARSE_SUCCESS)
    {
        aMatchedParam->ValueList.at(aValueIndex).ValueGiven = apValue;
        aValueIndex++;
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::ProcessHelpRequest()
{
    ParseResultEnum result = PARSE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, result);

    // No further action if the tool doesn't want command line support
    if (mCommandLineSupport)
    {
        // "-Help" cannot be used with application specific parameters
        int maxArgsAllowed = 2;

        for (int i = 0; i < PRE_PARSE_NUM_FLAGS; i++)
        {
            if (mPreParseFlags[i])
            {
                maxArgsAllowed++;
            }
        }

        if (mProgramArgc > maxArgsAllowed)
        {
            gTempFormatStr.str(II("\"Help\" cannot be used with other parameters"));
            result = PARSE_ERR_HELP_NOT_ON_OWN;
        }
        else
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Outputting help");
            result = PARSE_HELP_DISPLAYED;
            PrintHelp();
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::ParseParameter
(
    std::vector<ParamStruct>::iterator& aEncounteredParam,
    const ichar *apParameter,
    ParseExpectingEnum& aExpectingNext,
    unsigned int& aNumValues
)
{
    ParseResultEnum result = PARSE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, result);

    aExpectingNext = PARSE_EXPECTING_UNDEFINED;
    std::vector<ParamStruct>::iterator matchedParam;

    if (!MatchParameter(apParameter, true, matchedParam))
    {
        ErrUnknownParameter(apParameter);
        result = PARSE_ERR_UNRECOGNISED_PARAMETER;
    }
    else
    {
        bool chained = false;
        bool unused;
        bool foundEncounteredParam = FindParam(matchedParam->Names.begin()->c_str(), aEncounteredParam, MatchParamName, unused, LIST_ENCOUNTERED);

        if (foundEncounteredParam)
        {
            if (aEncounteredParam->IsFloating)
            {
                gTempFormatStr << II("Already got value of \"") << aEncounteredParam->ValueList.at(0).ValueGiven.c_str()
                                    << II("\" for \"<") << aEncounteredParam->Names.begin()->c_str() << II(">\" parameter");
                result = PARSE_ERR_FLOATING_PARAM_ALREADY_GIVEN;
            }
            else
            {
                chained = IsParameterChainableOrCompulsory(*aEncounteredParam);

                if (!chained)
                {
                    // Parameter given twice
                    gTempFormatStr << GetParamNameOutputText(*aEncounteredParam).c_str() << II(" given more than once");
                    result = PARSE_ERR_PARAM_ALREADY_GIVEN;
                }
            }
        }

        if (!foundEncounteredParam || chained)
        {
            matchedParam->EncounteredName = matchedParam->IsFloating ? *matchedParam->Names.begin() : apParameter;

            aEncounteredParam = AddToEncounteredList(matchedParam);

            if (aEncounteredParam->IsFloating)
            {
                // Assume value if for floating parameter
                aEncounteredParam->ValueList.at(0).ValueGiven = apParameter;
                // Given floating parameters appear on their own, the next item must be a parameter
                aExpectingNext = PARSE_EXPECTING_PARAMETER;
            }
            else
            {
                // Unless it's a flag we expect some following values
                if (aEncounteredParam->ValueList.size() == 0)
                {
                    aExpectingNext = PARSE_EXPECTING_PARAMETER;
                }
                else
                {
                    aExpectingNext = PARSE_EXPECTING_EITHER;
                    aNumValues = 0;
                }
            }
        }
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////

int CCmdLine::Execute(const istring& aListName)
{
    int rv = 0;
    FUNCTION_DEBUG_SENTRY_RET(int, rv);

    istring cmdName = GetParamForList(aListName);
    vector<ParamStruct>::iterator param;
    bool matched = MatchParameter(cmdName.c_str(), false, param);

    ASSERT(matched);

    if (matched)
    {
        ASSERT(param->CmdHandler);
        if (param->CmdHandler)
        {
            rv = param->CmdHandler(param->mpData);
        }
    }

    return rv;
}

/////////////////////////////////////////////////////////////////////////////

int CCmdLine::Execute(ChainingMode aMode)
{
    int lastFailedErr = 0;
    FUNCTION_DEBUG_SENTRY_RET(int, lastFailedErr);

    istring cmdName;

    while (GetNextChainedCmd(cmdName))
    {
        vector<ParamStruct>::iterator it;
        MatchParameter(cmdName.c_str(), false, it);
        ASSERT(it->CmdHandler);
        if (it->CmdHandler)
        {
            int rv = it->CmdHandler(it->mpData);
            if (rv != 0)
            {
                lastFailedErr = rv;
                // Do not run any more commands if set to abort on error
                if (aMode == CHAINING_STOP_ON_FAILURE)
                {
                    break;
                }
            }
        }
    }

    return lastFailedErr;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::CreateAutoChainList()
{
    FUNCTION_DEBUG_SENTRY;

    CreateList(LIST_CHAINABLE, AUTO_CHAIN_LIST_NAME);

    for (std::vector<ParamStruct>::const_iterator p = mDefinedParamList.begin(); (p != mDefinedParamList.end()); ++p)
    {
        istring paramName = p->Names.begin()->c_str();
        if (IsParameterAFlag(paramName))
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Skipping flag %s", paramName.c_str());
        }
        else
        {
            AddToList(AUTO_CHAIN_LIST_NAME, paramName);
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Adding parameter %s",  paramName.c_str());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

ParseResultEnum CCmdLine::Parse()
{
    FUNCTION_DEBUG_SENTRY_RET(ParseResultEnum, mParseResult);

    mParseResult = PARSE_SUCCESS;
    ParseExpectingEnum expectingItem = PARSE_EXPECTING_PARAMETER;

    std::vector<ParamStruct>::iterator matchedParam;
    unsigned int valueIndex = 0;

    gRequiredFloatingIndex = 0;      // reset floating index

    if (IsAutoChainFlagOn())
    {
        CreateAutoChainList();
    }
    // If the pre-parse didn't find "quiet" then output versioning
    if (!mPreParseFlags[PRE_PARSE_QUIET])
    {
        OutputVersioningAndCopyright();
    }

    gTempFormatStr.str(II(""));

    for (int i = 1; (i < mProgramArgc) && (mParseResult == PARSE_SUCCESS); i++)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Parsing %s", mppProgramArgv[i]);

        gCurrentFloatingIndex = 0;

        if (expectingItem == PARSE_EXPECTING_EITHER)
        {
            // Look to see if it matches a parameter
            std::vector<ParamStruct>::iterator throwAway;
            if (MatchParameter(mppProgramArgv[i], false, throwAway))
            {
                // The next item is a recognised parameter
                expectingItem = PARSE_EXPECTING_PARAMETER;
            }
            else
            {
                // The next item is NOT a recognised parameter
                expectingItem = PARSE_EXPECTING_VALUE;
            }
        }

        if (expectingItem == PARSE_EXPECTING_PARAMETER)
        {
            mParseResult = ParseParameter(matchedParam, mppProgramArgv[i], expectingItem, valueIndex);

            // No assert if tool doesn't use command line support
            if (mCommandLineSupport)
            {
                // Halt if the result was success but the expectingItem was not set
                ASSERT(mParseResult != PARSE_SUCCESS || expectingItem != PARSE_EXPECTING_UNDEFINED);
            }
        }
        else
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Expecting a value");

            // If processing an unterminated list, retrospecively add it to the defined parameters
            if ((matchedParam->UnterminatedListStartIndex != DEFAULT_UNTERMINATED_LIST_START_INDEX)
                && (valueIndex > (unsigned int)matchedParam->UnterminatedListStartIndex))
            {
                istring paramName = matchedParam->Names.begin()->c_str();
                bool unused;

                // Need to add extra entry to both "encountered" and "defined" param lists
                std::vector<ParamStruct>::iterator defined_param;

                (void)FindParam(paramName.c_str(), defined_param, MatchParamName, unused, LIST_DEFINED);

                ValueStruct valueStruct;
                valueStruct.DataType = matchedParam->ValueList.at(matchedParam->UnterminatedListStartIndex).DataType;
                defined_param->ValueList.push_back(valueStruct);
                matchedParam->ValueList.push_back(valueStruct);
            }

            if (valueIndex < matchedParam->ValueList.size())
            {
                mParseResult = ValidateValues(matchedParam, valueIndex, mppProgramArgv[i]);
            }

            // Already got as many values as expected? Don't know if unterminated list is complete
            if ((matchedParam->UnterminatedListStartIndex != DEFAULT_UNTERMINATED_LIST_START_INDEX)
                || (valueIndex < matchedParam->ValueList.size()))
            {
                expectingItem = PARSE_EXPECTING_EITHER;
            }
            else
            {
                expectingItem = PARSE_EXPECTING_PARAMETER;
            }
        }
    }

    if (mParseResult == PARSE_SUCCESS)
    {
        // Check the 'defined param list' for missing compulsory parameters and values not given
        for (std::vector<ParamStruct>::const_iterator p = mDefinedParamList.begin(); (p != mDefinedParamList.end()) && (mParseResult == PARSE_SUCCESS); ++p)
        {
            if (p->IsMandatory == MANDATORY)
            {
                std::vector<ParamStruct>::iterator unused;
                bool unused2;
                bool paramEncountered = FindParam(p->Names.begin()->c_str(), unused, MatchParamName, unused2, LIST_ENCOUNTERED);

                if (!paramEncountered)
                {
                    gTempFormatStr << II("\"") << *(p->Names.begin()) << II("\" must be given");
                    mParseResult = PARSE_ERR_MANDATORY_NOT_GIVEN;
                }
            }
        }
    }

    if (mParseResult == PARSE_SUCCESS)
    {
        // Check the 'encountered param list' for values that have not been given
        for (std::vector<ParamStruct>::const_iterator p = mEncounteredParamList.begin(); (p != mEncounteredParamList.end()) && (mParseResult == PARSE_SUCCESS); ++p)
        {
            for (std::vector<ValueStruct>::const_iterator v = p->ValueList.begin(); (v != p->ValueList.end() && (mParseResult == PARSE_SUCCESS)); ++v)
            {
                if (v->ValueGiven == II("") && v->IsMandatory == MANDATORY)
                {
                    gTempFormatStr << II("<") << v->ValueHelpType << II("> not given for ") << GetParamNameOutputText(*p);
                    mParseResult = PARSE_ERR_VALUE_NOT_GIVEN;
                }
            }
        }
    }

    if (mParseResult == PARSE_SUCCESS)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Check setup of lists");
        mParseResult = CheckAssociatedLists();
    }

    // If help was requested it 'trumps' any other command
    if (GetFlagParameterValue(CMDLINE_HELP1_PARAM))
    {
        mParseResult = ProcessHelpRequest();
    }

    mLastError = gTempFormatStr.str();


    // No further action if the tool doesn't want command line support
    if (mCommandLineSupport)
    {
        // There is a general rule that CCmdLine will not write out messages; it
        // will return the appropriate string to the caller who will then ask for
        // the string and print it out itself. However, we want to handle the
        // situation where the parameters were incorrect in a consistent manner.
        // To avoid duplicating this next block of code verbatim in every caller
        // of this function, an exception is made to the rule to handle this
        // specific situation here and now.
        if (mParseResult != PARSE_HELP_DISPLAYED && mParseResult != PARSE_SUCCESS)
        {
            istring error = GetLastError();
            OutputErrorMessage(error);
            printf("\n");
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Parse failed with status %d", mParseResult);

            // Remember the value of mParseResult (for restoring afterwards) because
            // PrintHelp() will have reset it
            ParseResultEnum savedParseResult = mParseResult;
            PrintHelp();
            mParseResult = savedParseResult;
        }
    }
    return mParseResult;
}

/////////////////////////////////////////////////////////////////////////////

istring CCmdLine::FetchTransportOptions()
{
    FUNCTION_DEBUG_SENTRY;

    int valUsb = 0, valLpt = 0, valTrb = 0, valUsbDbg = 0, valUsbCc = 0, valAdbBt = 0, valMul = 0;
    istring valTrans;
    istring ipAddress = II("127.0.0.1");
    iistringstream combinedTransString;
    vector<ParamStruct>::iterator matchedParam;
    bool unused;

    // GetParameterValue() will assert if -trans is not recognised e.g. if the app
    // has not called AddTransportOptions()

    // Look for each available parameter. USB, LPT, QS, TRB, USBDBG, USBCC and ADBBT are mutually exclusive
    if (FindParam(CMDLINE_USB_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_USB_PARAM, 1, valUsb) == GET_PARAMETER_SUCCESS)
    {
        combinedTransString << II("SPITRANS=USB SPIPORT=") << valUsb;
    }
    else if (FindParam(CMDLINE_LPT_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_LPT_PARAM, 1, valLpt) == GET_PARAMETER_SUCCESS)
    {
        combinedTransString << II("SPITRANS=LPT SPIPORT=") << valLpt;
    }
#if !defined(WIN32)
    else if (FindParam(CMDLINE_QS_PARAM, matchedParam, MatchParamName, unused) && GetFlagParameterValue(CMDLINE_QS_PARAM))
    {
        combinedTransString << II("SPITRANS=TRB SPIPORT=99");
    }
#endif
    else if (FindParam(CMDLINE_TRB_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_TRB_PARAM, 1, valTrb) == GET_PARAMETER_SUCCESS)
    {
        combinedTransString << II("SPITRANS=TRB SPIPORT=") << valTrb;
    }
    else if (FindParam(CMDLINE_USBDBG_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_USBDBG_PARAM, 1, valUsbDbg) == GET_PARAMETER_SUCCESS)
    {
        combinedTransString << II("SPITRANS=USBDBG SPIPORT=") << valUsbDbg;
    }
    else if (FindParam(CMDLINE_USBCC_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_USBCC_PARAM, 1, valUsbCc) == GET_PARAMETER_SUCCESS)
    {
        combinedTransString << II("SPITRANS=USBCC SPIPORT=") << valUsbCc;
    }
    else if (FindParam(CMDLINE_ADBBT_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_ADBBT_PARAM, 1, valAdbBt) == GET_PARAMETER_SUCCESS)
    {
        combinedTransString << II("SPITRANS=ADBBT SPIPORT=") << valAdbBt;
    }

    if (FindParam(CMDLINE_MUL_PARAM, matchedParam, MatchParamName, unused) &&
        GetParameterValueAsInteger(CMDLINE_MUL_PARAM, 1, valMul) == GET_PARAMETER_SUCCESS)
    {
        if (!combinedTransString.str().empty())
        {
            combinedTransString << II(' ');
        }
        combinedTransString << II("SPIMUL=") << valMul;
    }


    if (GetParameterValue(CMDLINE_TRANS_PARAM, 1, valTrans) == GET_PARAMETER_SUCCESS)
    {
        if (!combinedTransString.str().empty())
        {
            combinedTransString << II(' ');
        }
        combinedTransString << valTrans;
    }

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Transport options: %s", combinedTransString.str().c_str());

    return combinedTransString.str();
}

/////////////////////////////////////////////////////////////////////////////

int CCmdLine::GetUnterminatedListLength(const istring& aName)
{
    int listLength = 0;
    FUNCTION_DEBUG_SENTRY_RET(int, listLength);

    std::vector<ParamStruct>::iterator matchedParam;
    bool unused;
    bool recognisedParameter = FindParam(aName.c_str(), matchedParam, MatchParamName, unused, LIST_ENCOUNTERED);

    if (recognisedParameter)
    {
        // See if trying to get length for parameter without list setup
        ASSERT(matchedParam->UnterminatedListStartIndex != DEFAULT_UNTERMINATED_LIST_START_INDEX);

        listLength = static_cast<int>(matchedParam->ValueList.size()) - matchedParam->UnterminatedListStartIndex;
    }
    return listLength;
}

/////////////////////////////////////////////////////////////////////////////

GetParameterResultEnum CCmdLine::GetParameterValue
(
    const istring& aName,
    unsigned int aIndex,
    istring& aArgument
)
{
    GetParameterResultEnum ret = GET_PARAMETER_PARAM_NOT_GIVEN;
    FUNCTION_DEBUG_SENTRY_RET(GetParameterResultEnum, ret);

    std::vector<ParamStruct>::iterator matchedParam;
    bool unused;

    //Check first that the parameter is one that's been defined
    bool recognisedParameter = FindParam(aName.c_str(), matchedParam, MatchParamName, unused);

    istring paramDebugName = recognisedParameter ? GetParamNameOutputText(*matchedParam)
                                                 : aName;

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Get %d value for argument %s", aIndex, paramDebugName.c_str());

    ASSERT(recognisedParameter);                   // check Parameter has been defined
    if (recognisedParameter)
    {
        std::vector<ParamStruct>::iterator encounteredCmdIter;
        ParamStruct encounteredCmd;
        ASSERT(!matchedParam->IsFloating);              // Can't be used for floating parameters

        //Now attempt to fetch the parameter from the list of encountered parameters
        //For a chainable / compulsory parameter, this will fetch the first instance of the parameter
        bool gotEncounteredCmd = FindParam(matchedParam->Names.begin()->c_str(), encounteredCmdIter, MatchParamName, unused, LIST_ENCOUNTERED);

        if (gotEncounteredCmd)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Matched with parameter");
            encounteredCmd = *encounteredCmdIter;

            //A chainable / compulsory command may repeat, so ensure we fetch the appropriate value
            ret = ValidateAndUpdateCmd(encounteredCmd);

            if (ret == GET_PARAMETER_SUCCESS)
            {
                size_t numvalues = encounteredCmd.ValueList.size();
                ASSERT (numvalues != 0); // Must use GetFlagParameterValue for flags

                if ((aIndex > 0) && (aIndex <= numvalues))
                {
                    ValueStruct ValueStruct;
                    ValueStruct.ValueGiven = II("");
                    if (aIndex-1 < encounteredCmd.ValueList.size())
                    {
                        ValueStruct = encounteredCmd.ValueList.at(aIndex-1);
                    }
                    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Found value, was it empty?");
                    if (ValueStruct.ValueGiven != II(""))
                    {
                        aArgument = ValueStruct.ValueGiven;
                        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Matched, returning %s", aArgument.c_str());
                    }
                    else
                    {
                        HandleMandatoryParamValueNotGiven(aName.c_str(), ValueStruct);
                        ret = GET_PARAMETER_VALUE_NOT_GIVEN;
                    }
                }
                else
                {
                    // Attempt to access invalid index
                    ErrGetParamInvalidIndex(aIndex, aName.c_str());
                }
            }
        }
        else
        {
            // Parameter didn't appear on command line
            ret = GET_PARAMETER_PARAM_NOT_GIVEN;
            ASSERT(matchedParam->IsMandatory == NOT_MANDATORY); // If it was mandatory then should have been rejected earlier
        }
    }
    if (ret != GET_PARAMETER_SUCCESS)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "No value given for value %d of %s", aIndex, paramDebugName.c_str());
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

GetParameterResultEnum CCmdLine::ValidateAndUpdateCmd(ParamStruct& aParam)
{
    GetParameterResultEnum ret = GET_PARAMETER_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(GetParameterResultEnum, ret);

    if (IsParameterChainableOrCompulsory(aParam))
    {
        ParamStruct currentChainedCmd;

        const bool gotCurrentChainedCmd = GetCurrentChainedCmdStruct(currentChainedCmd);
        ASSERT(gotCurrentChainedCmd);

        if (istricmp(currentChainedCmd.Names.begin()->c_str(), aParam.Names.begin()->c_str()) != 0)
        {
            gTempFormatStr.str(II(""));
            gTempFormatStr << II("Cannot fetch value for parameter \"") << aParam.EncounteredName.c_str()
                           << II("\" as it is not the current chained command (\"") << currentChainedCmd.EncounteredName.c_str() << II("\")");
            mLastError = gTempFormatStr.str();
            MSG_HANDLER.NotifyStatus(STATUS_WARNING, inarrow(mLastError).c_str());

            ASSERT(0);
        }
        else
        {
            aParam = currentChainedCmd;
        }
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::GetFlagParameterValue(const istring& aParamName)
{
    bool found;
    FUNCTION_DEBUG_SENTRY_RET(bool, found);

    std::vector<ParamStruct>::iterator matchedParam;
    bool unused;

    // Check it is a defined parameter
    found = FindParam(aParamName.c_str(), matchedParam, MatchParamName, unused);
    ASSERT(found);    // Must be found
    ASSERT(!matchedParam->IsFloating); // Can't be used for floating parameters

    // Check whether it was encountered on the command line
    found = FindParam(aParamName.c_str(), matchedParam, MatchParamName, unused, LIST_ENCOUNTERED);

    if (found)
    {
        ASSERT(matchedParam->ValueList.size() == 0);    // If there are any values it isn't a flag
    }

    // If encountered then flag is true
    return (found);
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::GetFloatingParameterValue
(
    const istring& aParamName,
    istring& aArgument
)
{
    bool found;
    FUNCTION_DEBUG_SENTRY_RET(bool, found);

    std::vector<ParamStruct>::iterator matchedParam;
    bool unused;

    found = FindParam(aParamName.c_str(), matchedParam, MatchParamName, unused, LIST_ENCOUNTERED
    );

    if (found)
    {
        ASSERT(matchedParam->IsFloating);    // Can only be used for floating parameters
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Floating value %s was given \n", aParamName.c_str());
        ValueStruct ValueStruct = matchedParam->ValueList.at(0);
        aArgument = ValueStruct.ValueGiven;

        gCurrentFloatingIndex++; // Get next floating parameter if called again
    }
    return (found);
}

/////////////////////////////////////////////////////////////////////////////

DataTypeEnum CCmdLine::GetDataType
(
    const istring& aName,
    unsigned int aIndex
)
{
    DataTypeEnum dataType = DATA_TYPE_INTEGER;  // Value should never be used
    FUNCTION_DEBUG_SENTRY_RET(DataTypeEnum, dataType);

    std::vector<ParamStruct>::iterator matchedParam;
    bool unused;

    //Check first that the parameter is one that's been defined
    bool found = FindParam(aName.c_str(), matchedParam, MatchParamName, unused);
    ASSERT(found);

    if (found)
    {
        dataType = matchedParam->ValueList.at(aIndex-1).DataType;
    }
    return dataType;
}

/////////////////////////////////////////////////////////////////////////////

istring CCmdLine::PrefixUndecoratedHex
(
    const istring& aValueStr
)
{
    FUNCTION_DEBUG_SENTRY;

    iistringstream formatStr;
    const istring HEX_PREFIX(II("0x"));

    // Add hex prefix if not there already
    if (aValueStr.find(HEX_PREFIX) != 0)
    {
        formatStr << HEX_PREFIX;
    }
    formatStr << aValueStr;
    return formatStr.str();
}

/////////////////////////////////////////////////////////////////////////////

GetParameterResultEnum CCmdLine::GetParameterValueAsInteger
(
    const istring& aName,
    unsigned int aIndex,
    int& aValue
)
{
    FUNCTION_DEBUG_SENTRY;
    istring strToConvert;

    GetParameterResultEnum ret = GetParameterValue(aName, aIndex, strToConvert);
    if (ret == GET_PARAMETER_SUCCESS)
    {
        // If it is a hex value expected, prefix with hex indication
        DataTypeEnum dataType = GetDataType(aName, aIndex);
        if (dataType == DATA_TYPE_UNDECORATED_HEX)
        {
            strToConvert = PrefixUndecoratedHex(strToConvert);
        }

        if (!stringutil::StringToInteger(inarrow(strToConvert), aValue))
        {
            MSG_HANDLER.NotifyStatus(STATUS_WARNING, "Attempt to get value as integer when not defined as integer");
            ASSERT(0);
        }
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

GetParameterResultEnum CCmdLine::GetParameterValueAsU64
(
    const istring& aName,
    unsigned int aIndex,
    uint64_t& aValue
)
{
    FUNCTION_DEBUG_SENTRY;

    istring strToConvert;
    GetParameterResultEnum ret = GetParameterValue(aName, aIndex, strToConvert);
    if (ret == GET_PARAMETER_SUCCESS)
    {
        const ichar *pStr = strToConvert.c_str();
        DataTypeEnum dataType = GetDataType(aName, aIndex);
        int base = 0;
        if (dataType == DATA_TYPE_UNDECORATED_HEX)
        {
            base = 16;
        }
        errno = 0;  /* Clear outstanding errors */
        ichar *pEndptr = NULL;
        aValue = ISTRTOUI64(pStr, &pEndptr, base);
        if (errno != 0 || (pEndptr && *pEndptr  != '\0') || (!aValue && !istrchr(pStr, '0')))
        {
            MSG_HANDLER.NotifyStatus(STATUS_WARNING, "Invalid value: " + inarrow(strToConvert));
        }
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PrintIndentString
(
    const istring& aParamStr,
    int aIndent
)
{
    FUNCTION_DEBUG_SENTRY;

    iistringstream tmp;
    tmp << setw(aIndent + static_cast<int>(aParamStr.length())) << aParamStr;

    iprintf(II("%s\n"), tmp.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PrintHelpParameter(const istring& aParamStr)
{
    PrintIndentString(aParamStr, HELP_INDENT);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PrintHelpParameterDescription
(
    const istring& aDescriptionStr,
    int aIndentFirstLine,
    int aIndentSubsequentLines
)
{
    FUNCTION_DEBUG_SENTRY;

    bool firstLine = true;

    istring remainder = aDescriptionStr;
    while (remainder.length() > 0)
    {
        int indent = firstLine ? aIndentFirstLine : aIndentSubsequentLines;

        unsigned int lineLength = (HELP_DISPLAY_WIDTH - indent);
        // If there is enough space, print out the last line
        if (remainder.length() <= lineLength)
        {
            PrintIndentString(remainder, indent);
            remainder = II("");
        }
        else
        {
            // Line is too long - split at last space
            size_t found = remainder.find_last_of(II(" "), lineLength - 1);

            // If there wasn't a space before this point, enforce one
            // otherwise there will be an infinite loop
            if (found == istring::npos)
            {
                found = lineLength;
                ASSERT(false);
            }

            istring displayStr = remainder.substr(0, found);
            PrintIndentString(displayStr, indent);

            remainder = remainder.substr(found + 1, remainder.length());

        }
        firstLine = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PrintFormattedHelpSyntax(const istring& aDescriptionStr)
{
    PrintHelpParameterDescription(aDescriptionStr, HELP_INDENT, HELP_INDENT + static_cast<int>(mProgramName.length()) + 1);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PrintFormattedHelpParameter(const istring& aDescriptionStr)
{
    PrintHelpParameterDescription(aDescriptionStr, HELP_INDENT*2, HELP_INDENT*2);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::WriteParamSyntax(std::vector<ParamStruct>::const_iterator aParamIt)
{
    FUNCTION_DEBUG_SENTRY;

    int valueIndex = 0;
    bool nonMandatoryEncountered = false;

    // Floating parameters do not have a name
    if (!aParamIt->IsFloating)
    {
        gTempFormatStr << *(aParamIt->Names.begin());
    }

    // Show a list of the type of values expected
    for (std::vector<ValueStruct>::const_iterator v = aParamIt->ValueList.begin();
         v != aParamIt->ValueList.end();
         ++v)
    {
        // Print a space *unless* it's the first time round the loop and it's a floating
        // parameter (and thus the name hasn't been printed)
        if (v != aParamIt->ValueList.begin() || !aParamIt->IsFloating)
        {
            gTempFormatStr << II(" ");
        }

        // If it is an unterminated list and we have reached it in the value list, show special syntax
        if ((aParamIt->UnterminatedListStartIndex != DEFAULT_UNTERMINATED_LIST_START_INDEX) && (valueIndex >= aParamIt->UnterminatedListStartIndex))
        {
            istring tempStr = aParamIt->ValueList.at(aParamIt->UnterminatedListStartIndex).ValueHelpType;
            gTempFormatStr << II("<") << tempStr << II("1>") << II(" [<") << tempStr << II("2> ...]");
        }
        else
        {
            // Look for first optional value
            if (v->IsMandatory == NOT_MANDATORY && !nonMandatoryEncountered)
            {
                gTempFormatStr << II("[");
                nonMandatoryEncountered = true;
            }
            gTempFormatStr << II("<") << v->ValueHelpType << II(">");
        }
        valueIndex++;
    }

    // Terminate optional values
    if (nonMandatoryEncountered)
    {
        gTempFormatStr << II("]");
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpParameter(std::vector<ParamStruct>::const_iterator aParamIt)
{
    FUNCTION_DEBUG_SENTRY;

    gTempFormatStr.str(II(""));

    istring helpDescription;

    // If the help for a value is more than one line long, split up the output
    // so that the help for each value is split onto different lines
    // e.g.
    // <parameter> <value1> <value2>
    // <parameter help>
    //
    // <value1> = <value1 help>
    //
    // <value2> = <value1 help>
    // Don't show the name for a floating param - as far as the user is concerned there isn't one
    if (!aParamIt->IsFloating)
    {
        WriteParamSyntax(aParamIt);
        PrintHelpParameter(gTempFormatStr.str());

        // display the help text for this parameter
        helpDescription = aParamIt->HelpText;
    }
    if (aParamIt->IsFloating)
    {
        // for a floating parameter need different format
        ValueStruct value = aParamIt->ValueList[0];

        istring tmp = II("<") + value.ValueHelpType + II(">");
        PrintHelpParameter(tmp);
        helpDescription = value.ValueHelpDescription;
    }
    else
    {
        helpDescription += II(". ");
        bool outputIndividualParameters = false;

        // Now show a list of values with a description of each
        for (std::vector<ValueStruct>::const_iterator v = aParamIt->ValueList.begin(); v != aParamIt->ValueList.end(); ++v)
        {
            gTempFormatStr.str(II(""));

            if (v->ValueHelpDescription.length() > HELP_DISPLAY_WIDTH)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "More than one line of help. Use the extended output format, unless compact help");
                outputIndividualParameters = !IsCompactHelpFlagOn();
            }
            PrintFormattedHelpParameter(helpDescription);
            helpDescription = II("");

            if (!IsCompactHelpFlagOn() || !v->PossibleValuesList.empty())
            {
                gTempFormatStr << II("<") << v->ValueHelpType << II("> = ");
            }
            if (!v->PossibleValuesList.empty())
            {
                gTempFormatStr << II("One of ");
                // show list of possible values
                for (std::vector<istring>::const_iterator it = v->PossibleValuesList.begin(); it != v->PossibleValuesList.end(); ++it)
                {
                    if (it != v->PossibleValuesList.begin())
                    {
                        gTempFormatStr << II(", ");
                    }
                    gTempFormatStr << II("\"") << it->c_str() << II("\"");
                }
            }
            else
            {
                // show text to describe value
                if (!IsCompactHelpFlagOn())
                {
                    gTempFormatStr << v->ValueHelpDescription << II(".");
                }
            }

            if (outputIndividualParameters)
            {
                // If using extended format add a blank line before the value help
                printf("\n");

                PrintFormattedHelpParameter(gTempFormatStr.str());
            }
            else
            {
                helpDescription += gTempFormatStr.str();
            }
        }
    }
    if (helpDescription != II(""))
    {
        PrintFormattedHelpParameter(helpDescription);
    }

    if (!IsCompactHelpFlagOn())
    {
        printf("\n");
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpParameterSynonyms(std::vector<ParamStruct>::const_iterator aParamIt)
{
    FUNCTION_DEBUG_SENTRY;

    //Floating params should have no synonyms
    ASSERT(!aParamIt->IsFloating);

    gTempFormatStr.str(II(""));

    gTempFormatStr << II("Synonyms for '") << aParamIt->Names.begin()->c_str() << II("' are ");

    bool first = true;
    for (std::list<istring>::const_iterator it = ++(aParamIt->Names.begin()); it != aParamIt->Names.end(); ++it)
    {
        if (!first)
        {
            gTempFormatStr << II(", ");
        }

        gTempFormatStr << II("'") << *it << II("'");
        first =false;
    }

    gTempFormatStr << II(".");

    PrintFormattedHelpParameter(gTempFormatStr.str());
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpSyntaxInList(std::vector<ListStruct>::const_iterator aList)
{
    FUNCTION_DEBUG_SENTRY;

    // Flag to indicate if have displayed first parameter. Cannot just check for start of list
    // because some parameters may be hidden
    bool firstEntryShown = false;

    // Flag to indicate that at least one entry has been shown
    bool anyEntryShown = false;

    istring openingParentheses;
    istring closingParentheses;

    // Delimit list
    for (std::vector<istring>::const_iterator p = aList->ParameterList.begin();
                p != aList->ParameterList.end(); ++p)
    {
        std::vector<ParamStruct>::iterator matchedParam;
        bool unused;

        const bool found = FindParam(p->c_str(), matchedParam, MatchParamName, unused);
        ASSERT(found); // Must always find a parameter that is in a list

        if (DisplayThisParameter(matchedParam))
        {
            GetHelpItemParentheses(aList->ListType, openingParentheses, closingParentheses);

            if (firstEntryShown)
            {
                gTempFormatStr << II(" | ");
            }
            else
            {
                // Add a space if required
                const istring str = gTempFormatStr.str();
                if (!str.empty() && !iisspace(str.at(str.length() - 1)))
                {
                    gTempFormatStr << II(' ');
                }

                gTempFormatStr << openingParentheses;
            }
            WriteParamSyntax(matchedParam);
            firstEntryShown = true;
            anyEntryShown = true;
        }
    }

    if (anyEntryShown)
    {
        gTempFormatStr << closingParentheses;
        gTempFormatStr << II(" ");
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::GetHelpItemParentheses(ListTypeEnum aListType, istring& aOpeningParantheses, istring& aClosingParentheses)
{
    FUNCTION_DEBUG_SENTRY;

    switch (aListType)
    {
        case LIST_UNIQUE:
            aOpeningParantheses = II("(");
            aClosingParentheses = II(")");
            break;
        case LIST_EXCLUSIVE:
            aOpeningParantheses = II("[");
            aClosingParentheses = II("]");
            break;
        case LIST_COMPULSORY:
            aOpeningParantheses = II("{");
            aClosingParentheses = II("}");
            break;
        case LIST_CHAINABLE:
            aOpeningParantheses = II("[{");
            aClosingParentheses = II("}]");
            break;
        default:
             ASSERT(0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpSyntaxNotInAList(bool aDoingActions)
{
    FUNCTION_DEBUG_SENTRY;

    // If printed out syntax for a list, already a space added
    bool skipFirstSpace = (!mAssociatedParametersList.empty());

    for (std::vector<ParamStruct>::const_iterator p = mDefinedParamList.begin(); p != mDefinedParamList.end(); ++p)
    {
        // Only shows parameters that are not in a list (because lists have already been displayed)
        if (DisplayThisParameter(p) && (!IsParamInAList(*(p->Names.begin()), LIST_SEARCH_TYPE_ANY)))
        {
            bool isFlag = (IsParameterAFlag(p->Names.begin()->c_str()));
            if (isFlag != aDoingActions)
            {
                if (!skipFirstSpace)
                {
                    gTempFormatStr << II(" ");
                }
                skipFirstSpace = false;

                if (p->IsMandatory == NOT_MANDATORY)
                {
                    gTempFormatStr << II("[");
                }

                WriteParamSyntax(p);

                if (p->IsMandatory == NOT_MANDATORY)
                {
                    gTempFormatStr << II("]");
                }
            }
        }
        else
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, inarrow(II("%s is in a list")).c_str(), inarrow(*p->Names.begin()).c_str());
        }
    }

}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpParamsInList(std::vector<ListStruct>::const_iterator aList)
{
    FUNCTION_DEBUG_SENTRY;

    for (std::vector<istring>::const_iterator p = aList->ParameterList.begin();
                p != aList->ParameterList.end(); ++p)
    {
        std::vector<ParamStruct>::iterator matchedParam;
        bool unused;

        const bool found = FindParam(p->c_str(), matchedParam, MatchParamName, unused);
        ASSERT(found); // Must always find a parameter that is in a list

        if (DisplayThisParameter(matchedParam))
        {
            ShowHelpParameter(matchedParam);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpParametersNotInAList(bool aDoingActions)
{
    FUNCTION_DEBUG_SENTRY;

    for (std::vector<ParamStruct>::const_iterator p = mDefinedParamList.begin(); p != mDefinedParamList.end(); ++p)
    {
        std::vector<ParamStruct>::iterator matchedParam;
        bool unused;

        const bool found = FindParam(p->Names.begin()->c_str(), matchedParam, MatchParamName, unused);
        ASSERT(found); // Must always find a parameter that is in a list

        if (DisplayThisParameter(matchedParam))
        {
            if (!IsParamInAList(*(p->Names.begin()), LIST_SEARCH_TYPE_ANY))
            {
                bool isFlag = IsParameterAFlag(p->Names.begin()->c_str());
                if (isFlag != aDoingActions)
                {
                    ShowHelpParameter(p);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::CommonShowHelpForLists(bool aShowSyntax, bool aDoingActions)
{
    FUNCTION_DEBUG_SENTRY;

    //Display the help for lists of each type. For an application using
    //chaining, the commands themselves will be defined in compulsory / chainable
    //lists, so output these list types first.
    ListTypeEnum listTypes[4] = {LIST_COMPULSORY, LIST_CHAINABLE, LIST_UNIQUE, LIST_EXCLUSIVE};

    for (size_t i = 0; i < (sizeof(listTypes) / sizeof(listTypes[0])); ++i)
    {
        for (std::vector<ListStruct>::iterator l = mAssociatedParametersList.begin();
                               (l != mAssociatedParametersList.end()); ++l)
        {
            if (l->ListType == listTypes[i])
            {
                istring first_name_in_list = l->ParameterList.at(0);
                bool isFlag = IsParameterAFlag(first_name_in_list);
                if (isFlag != aDoingActions)
                {
                    aShowSyntax ? ShowHelpSyntaxInList(l) : ShowHelpParamsInList(l);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpSyntaxForLists(bool aDoingActions)
{
    FUNCTION_DEBUG_SENTRY;

    CommonShowHelpForLists(true, aDoingActions);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowHelpParametersForLists(bool aDoingActions)
{
    FUNCTION_DEBUG_SENTRY;

    CommonShowHelpForLists(false, aDoingActions);
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsParamInAList(const istring& aName, ListSearchTypeEnum aTypeOfList)
{
    bool found = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, found);

    istring listName;
    found = IsParamInAList(aName, aTypeOfList, listName);

    return found;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsParamInAList(const istring& aName, ListSearchTypeEnum aTypeOfList, istring& aListName)
{
    bool found = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, found);

    for (std::vector<ListStruct>::const_iterator l = mAssociatedParametersList.begin();
                           (l != mAssociatedParametersList.end() && !found); ++l)
    {
        for (std::vector<istring>::const_iterator p = l->ParameterList.begin();
                    (p != l->ParameterList.end() && !found); ++p)
        {
            found = (aName == p->c_str());
            if (found)
            {
                found = ((aTypeOfList == LIST_SEARCH_TYPE_ANY)
                        || ((aTypeOfList == LIST_SEARCH_TYPE_EXCLUSIVE)  && (l->ListType == LIST_EXCLUSIVE))
                        || ((aTypeOfList == LIST_SEARCH_TYPE_UNIQUE)     && (l->ListType == LIST_UNIQUE))
                        || ((aTypeOfList == LIST_SEARCH_TYPE_CHAINABLE)  && (l->ListType == LIST_CHAINABLE))
                        || ((aTypeOfList == LIST_SEARCH_TYPE_COMPULSORY) && (l->ListType == LIST_COMPULSORY)));
            }

            if (found)
            {
                aListName = l->Name;
            }
        }
    }

    return found;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowCompactHelp(bool aOnlyFlags)
{
    FUNCTION_DEBUG_SENTRY;

    for (std::vector<ParamStruct>::const_iterator p = mDefinedParamList.begin(); p != mDefinedParamList.end(); ++p)
    {
        std::vector<ParamStruct>::iterator matchedParam;
        istring paramName = p->Names.begin()->c_str();
        bool unused;

        const bool found = FindParam(p->Names.begin()->c_str(), matchedParam, MatchParamName, unused);
        ASSERT(found); // Must always find a parameter that is in a list

        bool isFlag = IsParameterAFlag(p->Names.begin()->c_str());

        if ((DisplayThisParameter(matchedParam)) && (isFlag == aOnlyFlags))
        {
            ShowHelpParameter(p);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowCompactHelpForFlags()
{
    ShowCompactHelp(true);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::ShowCompactHelpForActions()
{
    ShowCompactHelp(false);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::PrintHelp()
{
    FUNCTION_DEBUG_SENTRY;

    // If the application calls this method directly, the final output message
    // must not be displayed, so remember this fact.
    mParseResult = PARSE_HELP_DISPLAYED;

    PrintHelpParameterDescription(mProgramTitle, 0, 0);
    printf("\n");

    if (!mProgramDescription.empty())
    {
        PrintHelpParameterDescription(mProgramDescription, 0, 0);
        printf("\n");
    }

    printf("Usage:\n\n");

    gTempFormatStr.str(II(""));
    gTempFormatStr << mProgramName << II(" ");

    if (IsAutoChainFlagOn())
    {
        // If auto chaining is on, then show a simplified syntax
        gTempFormatStr << II("[<flags>] [<command> [<command> ...]]");
        PrintIndentString(gTempFormatStr.str(), HELP_INDENT);
        printf("\n");
    }
    else
    {
        // Actions
        ShowHelpSyntaxForLists(true);
        ShowHelpSyntaxNotInAList(true);
        // Flags
        ShowHelpSyntaxForLists(false);
        ShowHelpSyntaxNotInAList(false);
        PrintFormattedHelpSyntax(gTempFormatStr.str());
    }

    if (!mPreParseFlags[PRE_PARSE_SYNTAX])
    {
        printf("\n");
        if (IsCompactHelpFlagOn())
        {
            printf("Possible commands are:\n");
            ShowCompactHelpForActions();
            printf("\n\n");
            printf("Possible flags are:\n");
            ShowCompactHelpForFlags();
        }
        else
        {
            // Actions
            ShowHelpParametersForLists(true);
            ShowHelpParametersNotInAList(true);
            // Flags
            ShowHelpParametersForLists(false);
            ShowHelpParametersNotInAList(false);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::NewList
(
    ListTypeEnum aListType,
    const istring& aListName
)
{
    FUNCTION_DEBUG_SENTRY;

    ListStruct listStruct;
    listStruct.ListType = aListType;
    listStruct.Name = aListName;
    mAssociatedParametersList.push_back(listStruct);
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::CreateList
(
    ListTypeEnum aListType,
    const istring& aListName
)
{
    FUNCTION_DEBUG_SENTRY;
    switch (aListType)
    {
        case LIST_UNIQUE:
        case LIST_EXCLUSIVE:
        case LIST_CHAINABLE:
        case LIST_COMPULSORY:
        {
            NewList(aListType, aListName);
            break;
        }

        default:
        {
            ASSERT(0); // Unknown list type
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::FindList
(
    const istring& aListName,
    std::vector<ListStruct>::iterator& aMatchedList
)
{
    bool matched;
    FUNCTION_DEBUG_SENTRY_RET(bool, matched);
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Find the list \"%s\"", aListName.c_str());

    gMatchItem = aListName;
    std::vector<ListStruct>::iterator p = find_if(mAssociatedParametersList.begin(), mAssociatedParametersList.end(), MatchListName);
    matched = (p != mAssociatedParametersList.end());
    if (matched)
    {
        aMatchedList = p;
    }
    return matched;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::AddToList
(
    const istring& aListName,
    const istring& aParamName
)
{
    FUNCTION_DEBUG_SENTRY;
    std::vector<ListStruct>::iterator matchedList;

    // Make sure the param has been defined previously
    std::vector<ParamStruct>::iterator notUsed;
    bool unused;
    const bool found = FindParam(aParamName.c_str(), notUsed, MatchParamName, unused);
    ASSERT(found);

    if (FindList(aListName, matchedList))
    {
        matchedList->ParameterList.push_back(aParamName);
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "List %s does not exist", aListName.c_str());
        ASSERT(0);
    }

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Adding parameter %s to list %s", aParamName.c_str(), aListName.c_str());
}

/////////////////////////////////////////////////////////////////////////////

std::vector<ParamStruct>::iterator CCmdLine::AddToEncounteredList
(
    std::vector<ParamStruct>::iterator aParam
)
{
    FUNCTION_DEBUG_SENTRY;
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Adding parameter \"%s\" to the internal list of encountered parameters", aParam->EncounteredName.c_str());

    mEncounteredParamList.push_back(*aParam);
    std::vector<ParamStruct>::iterator encounteredParam = --(mEncounteredParamList.end());

    if (IsParameterChainableOrCompulsory(*aParam))
    {
        mNumChainedCommands ++;
    }

    return encounteredParam;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::GetNextChainedCmd
(
    istring& aChainedCmd
)
{
    bool found = true;
    FUNCTION_DEBUG_SENTRY_RET(bool, found);

    if (mNumChainedCommands > 0)
    {

        found = MoveToNextChainedCmd();
        if (found)
        {
            ParamStruct chainedCmd = mEncounteredParamList[mCurrentChainedCmdIndex];
            aChainedCmd = chainedCmd.Names.begin()->c_str();
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Current chained command is \"%s\"", aChainedCmd.c_str());
        }
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "No chained commands in internal list of encountered commands.");
        found = false;
    }

    return found;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::GetCurrentChainedCmdStruct
(
    ParamStruct& aChainedCmd
)
{
    bool gotChainedCommand = true;
    FUNCTION_DEBUG_SENTRY_RET(bool, gotChainedCommand);

    if (mNumChainedCommands > 0)
    {
        aChainedCmd = mEncounteredParamList[mCurrentChainedCmdIndex];
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Current chained command is \"%s\"", aChainedCmd.Names.begin()->c_str());
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "No chained commands in internal list of encountered commands.");
        gotChainedCommand = false;
    }
    return gotChainedCommand;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::MoveToNextChainedCmd()
{
    bool foundNextChainableOrCompulsory = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, foundNextChainableOrCompulsory);

    if (mNumChainedCommands > 0)
    {
        for (uint16 i = mCurrentChainedCmdIndex + 1; (i < mEncounteredParamList.size()) && !foundNextChainableOrCompulsory; i++)
        {
            foundNextChainableOrCompulsory = IsParameterChainableOrCompulsory(mEncounteredParamList[i]);

            if (foundNextChainableOrCompulsory)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Updating current chained command to \"%s\"", (mEncounteredParamList[i]).EncounteredName.c_str());
                mCurrentChainedCmdIndex = i;
            }
        }

        if (!foundNextChainableOrCompulsory)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "There are no more chained commands");
        }
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "No chainable / compulsory commands have been encountered on the command line");
    }

    return foundNextChainableOrCompulsory;
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsParameterChainableOrCompulsory
(
    const ParamStruct& aParam
)
{
    bool chainableOrCompulsory;
    FUNCTION_DEBUG_SENTRY_RET(bool, chainableOrCompulsory);

    chainableOrCompulsory = IsParamInAList(*aParam.Names.begin(), LIST_SEARCH_TYPE_CHAINABLE);

    if (chainableOrCompulsory)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Parameter \"%s\" belongs to a Chainable list", aParam.EncounteredName.c_str());
    }

    if (IsParamInAList(*aParam.Names.begin(), LIST_SEARCH_TYPE_COMPULSORY))
    {
        chainableOrCompulsory = true;
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Parameter \"%s\" belongs to a Compulsory list", aParam.EncounteredName.c_str());
    }

    return chainableOrCompulsory;
}

/////////////////////////////////////////////////////////////////////////////

istring CCmdLine::GetParamForList
(
    const istring& aListName
)
{
    istring givenParam;
    FUNCTION_DEBUG_SENTRY_RET(istring, givenParam);

    std::vector<ListStruct>::iterator matchedList;
    if (!FindList(aListName, matchedList))
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "List not matched");
        ASSERT(0);
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "For each entry in the list, find the one that was given on the command line");
        for (std::vector<istring>::iterator listEntryIt = matchedList->ParameterList.begin(); (listEntryIt != matchedList->ParameterList.end()) && (givenParam.empty()); ++listEntryIt)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Checking \"%s\"", listEntryIt->c_str());
            std::vector<ParamStruct>::iterator matchedParam;
            bool unused;

            if (FindParam(listEntryIt->c_str(), matchedParam, MatchParamEncountered, unused, LIST_ENCOUNTERED))
            {
                givenParam = *(matchedParam->Names.begin());
            }
        }
    }

    return givenParam;
}

/////////////////////////////////////////////////////////////////////////////

int CCmdLine::OutputFinalMessage()
{
    int finalExeRetVal;
    FUNCTION_DEBUG_SENTRY_RET(int, finalExeRetVal);

    finalExeRetVal = (MSG_HANDLER.IsErrorSet(CMessageHandler::GROUP_ENUM_RSVD_ALL, false) ? 1 : 0);

    if (mParseResult == PARSE_SUCCESS)
    {
        istring finalMessage;
        istring successString(II("Success"));
        istring failureString(II("Failed"));

#ifndef WIN32
        if (mProgramArgc > 0 && NULL != mppProgramArgv)
        {
            finalMessage = mProgramName;
            for (int j=1; j<mProgramArgc; ++j)
            {
                bool containsSpaces = (istrchr(mppProgramArgv[j], ' ') > 0 ? true : false);
                finalMessage += (containsSpaces ? II(" \"") : II(" "));
                finalMessage += mppProgramArgv[j];
                finalMessage += (containsSpaces ? II("\"")  : II(""));
            }
            finalMessage += II(" : concluded with ");
        }

        successString = II("success");
        failureString = II("failure");
#endif

        finalMessage += (finalExeRetVal == 0 ? successString : failureString);

        // Make sure the level is enabled before writing out the final status
        MSG_HANDLER.SetStatusLevel(STATUS_ESSENTIAL, true);
        MSG_HANDLER.NotifyStatus(STATUS_ESSENTIAL, inarrow(finalMessage));
    }
    else
    {
        finalExeRetVal = 1;
    }

    MSG_HANDLER_NOTIFY_PROFILE_POINT(0);
    return finalExeRetVal;
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputErrorAndFailMessages(const istring& aErrorMessage)
{
    FUNCTION_DEBUG_SENTRY;

    MSG_HANDLER.SetErrorAutomaticDisplay(true);
    MSG_HANDLER.SetErrorMsg(0, inarrow(aErrorMessage));
    CCmdLine::OutputFinalMessage();
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputErrorMessage(const istring& aMessage)
{
    FUNCTION_DEBUG_SENTRY;

    // This method is deprecated and will eventually disappear. In the meantime,
    // set the error string in the cmdline group so that the final output
    // message is correct. This is slightly complicated by the fact that an
    // application might have turned the automatic reporting on or off and that
    // state needs to be preserved. When this method has gone, each application
    // will have their own "error message" to look after, so none of this
    // algorithm currently employed here will be necessary.
    bool priorAutomaticErrorsState = MSG_HANDLER.SetErrorAutomaticDisplay(true);
    MSG_HANDLER.SetErrorMsg(0, inarrow(aMessage));
    if (!priorAutomaticErrorsState)
    {
        MSG_HANDLER.SetErrorAutomaticDisplay(priorAutomaticErrorsState);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputLastErrorMessage()
{
    FUNCTION_DEBUG_SENTRY;

    int16 errorCode;
    string errorString;
    CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

    bool status = MSG_HANDLER.LastError(errorCode, errorString, group, false);

    if (status)
    {
        OutputErrorMessage(icoerce(errorString));
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputWarningMessage(const istring& aMessage)
{
    FUNCTION_DEBUG_SENTRY;

    MSG_HANDLER.NotifyStatus(STATUS_WARNING, inarrow(aMessage));
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputEssentialMessage(const istring& aMessage)
{
    FUNCTION_DEBUG_SENTRY;

    MSG_HANDLER.NotifyStatus(STATUS_ESSENTIAL, inarrow(aMessage));
}

/////////////////////////////////////////////////////////////////////////////

void CCmdLine::OutputInfoMessage(const istring& aMessage)
{
    FUNCTION_DEBUG_SENTRY;

    MSG_HANDLER.NotifyStatus(STATUS_INFO, inarrow(aMessage));
}

/////////////////////////////////////////////////////////////////////////////

bool CCmdLine::IsQuiet()
{
    bool ret;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    ret = mPreParseFlags[PRE_PARSE_QUIET];
    return ret;
}
