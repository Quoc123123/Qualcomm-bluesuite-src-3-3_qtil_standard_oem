/************************************************************************************************************
 *
 *  cmdline.h
 *
 *  Copyright (c) 2010-2022 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Command line parsing class
 *
 *  A general command line parsing class which provides support for:
 *
 *  Contruction
 *  ===========
 *  The constructor takes values that describe the tool for the help output, the date it was developed
 *  (for the copyright). It also takes the argc/argv values so we can pre-parse the command line (e.g. this
 *  is needed so we can get debug output asap, if turned on)
 *
 *  Defining Parameters
 *  ===================
 *  The interface works in terms of "parameters" and "values".
 *
 *    - You firstly add a parameter using "SetExpectedParam"
 *      SetExpectedParam "aHandler" argument allows to add a command handler to the command.
 *      It works together with the Execute() methods. If Execute() is called it will execute the delegate associated to the
 *      command. This mechanism is optional but, if used, it saves the developer from having to write a big if to identify
 *      the issued command / commands.
 *      SetExpectedParam "apData" argument allows to associate an object of any type to the handler. If apData is defined,
 *      It will be passed to the handler function when invoked which will be able to cast it to what it needs.
 *    - If you need synonyms for the parameter you've added then add them using "SetExpectedParamSynonym". (This should only be
 *      used for backwards-compatibility purposes when updating an existing application to use CmdLine. New applications should
 *      not require it.)
 *    - Add zero or more values using "AddExpectedValue"
 *      - If no values are added, the parameter is a boolean flag (e.g. "-debug") and is "off" if not on the command line
 *
 *      Floating parameters
 *      ===================
 *      It is possible to declare a value that has no preceding parameter. This is called a "floating" value. For example,
 *      this is used if you want to give a filename on the command line but you don't want the user to put "-file" before
 *      it (the filename parameter to coredump is therefore a floating parameter)
 *
 *      Built-in parameters
 *      ===================
 *      Some parameters are pre-defined as they are used by existing tools. Search this file for "Built-in parameters"
 *      The "-quiet" flag is always supported
 *
 *      Unterminated lists
 *      ==================
 *      An unterminated list is where the user can pass in any number of values. e.g. "write 1 2" or "write 1 2 3 4 5 6 7 8 9"
 *      - Only one unterminated list can be defined for a parameter
 *      - Other values cannot follow an unterminated list within a parameter
 *           i.e. can have "write <address> <unterminated list>" but not "write <unterminated list> <address>"
 *
 *  Lists
 *  =====
 *  Lists are used to associate parameters in order to:
 *   - insist that precisely one of the parameters is given (unique)
 *   - if any of the parameters are given, only one can appear (exclusive)
 *   - define a set of parameters which may appear more than once, including repeats of the same parameter (chainable)
 *   - define a set of parameters which may appear more than once, including repeats of the same parameter, where
 *     at least one parameter must appear (compulsory)
 *  Note that for a parameter which is not within a chainable or compulsory list, it's implicit that the command may not be
 *  repeated on the command line.
 *
 *  To create a list:
 *     - Call "CreateList" and give the list a name
 *     - Call "AddToList" for each entry (parameters given must already have been declared)
 *
 *  Parsing
 *  =======
 *  Once all the parameters have been declared, call "Parse" to check that the current command line conforms
 *  with the definitions.
 *
 *      The following returns need to be handled
 *      - PARSE_HELP_DISPLAYED. The program should do nothing else because an error
 *        has been detected (or help requested) and the help has been displayed
 *
 *      - PARSE_SUCCESS. The parse was successful. The value of parameters can be queried (see below)
 *
 *      - anything else. An error has occurred. Show the error and display help
 *            cmdline.OutputErrorMessage(aCmdline.GetLastError());
 *              cmdline.PrintHelp();
 *
 *  Querying Values
 *  ===============
 *
 *  To get the value of a parameter, use "GetParameterValue". It takes an index of the value you want to query
 *  NB: This is a "real world" index starting at one i.e. get the first value.
 *  The function "GetParamError" should be used to check if the value was obtained successfully
 *
 *  For floating parameters you must use GetFloatingParameterValue and for flags GetFlagParameterValue
 *  To retrieve values as integers (where appropriate), use GetParameterValueAsInteger.
 *
 *  Retrieving chainable / compulsory parameters and their values in order of appearance on command line
 *  ===================================================================================
 *  (A "chainable" or "compulsory" parameter means one which occurs in a chainable or compulsory list, respectively. Such commands
 *  are repeatable and so may have multiple sets of values specified at the command line. The CmdLine library provides interfaces
 *  for the calling application to step through all chainable / compulsory commands in the order they were supplied on the command
 *  line, thus enabling the calling applicating to process actions in the order they were given.)
 *
 *  Simple interface - process all chainable / compulsory parameters in the order they were given
 *  ---------------------------------------------------------------------------------------------
 *  Use GetCurrentChainedCmd to get the current chainable / compulsory parameter.
 *  Use GetParameterValue ( or GetParameterValueAsInteger / GetFlagParameterValue as appropriate) to get the value(s) of the
 *      current chainable / compulsory parameter. Note that calling GetParameterValue (and variant methods) for
 *      a chainable / compulsory parameter which is not the current chainable / compulsory parameter results in an error.
 *  Use MoveToNextChainedCmd to move to the next chainable / compulsory parameter encountered on the command line.
 *
 *  Interface for processing all chainable / compulsory parameters in order, broken down by list
 *  --------------------------------------------------------------------------------------------
 *  As an alternative to using GetCurrentChainedCmd, the user may use GetParamForList, supplying the name of a given
 *  chainable / compulsory list. Note that MoveToNextChainedCmd moves to the next chainable / compulsory parameter on the
 *  command line, which may be in a different list to the current chainable / compulsory parameter.
 *
 *  User output
 *  ===========
 *  Functions exist to output messages to the user (Info and warning messages do not appear if "quiet" is on)
 *
 *  Final output
 *  ============
 *  The final "success" or "failed" message is given to the user e.g. "OutputFailedMessage". Search this file for
 *  "User output fns"
 *
 * ************************************************************************************************************/

#ifndef CMDLINE_H
#define CMDLINE_H

#ifdef WIN32
//  Switch off the warning which says name mangling has produced something
//  too long (ie you are using the STL...)
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include <string>
#include <vector>
#include <list>
#include "common/globalversioninfo.h"
#include "common/portability.h"
#include "common/types.h"
#include "common/types_t.h"
#include "unicode/ichar.h"
#include "misc/multilistparser.h"
#include "misc/delegate.hpp"

// "Built-in parameters"
// The following parameters are pre-declared as they are already used in the tools
#define CMDLINE_QUIET_PARAM      II("-quiet")     //!< Suppresses non-essential output, such as version/copyright and warnings
#define CMDLINE_DEBUG_PARAM      II("-debug")     //!< Turn on debug output
#define CMDLINE_SYNTAX_PARAM     II("-syntax")    //!< Only shows the syntax for help, not the full descriptions
#define CMDLINE_HELP1_PARAM      II("-help")      //!< The main parameter string for help
#define CMDLINE_HELP2_PARAM      II("--help")     //!< Synonym for help
#define CMDLINE_HELP3_PARAM      II("-?")         //!< Synonym for help
#define CMDLINE_HELP4_PARAM      II("/?")         //!< Synonym for help
#define CMDLINE_MACHINE_PARAM    II("-machine")   //!< String for the 'machine readable' flag
#define CMDLINE_RAW_PARAM        II("-raw")       //!< String for the 'raw' flag
#define CMDLINE_NORUN_PARAM      II("-norun")     //!< String for the 'no run' flag
#define CMDLINE_TRANS_PARAM      II("-trans")     //!< General transport string e.g. -trans "SPITRANS=USB"
#define CMDLINE_BROAD_PARAM      II("broad")      //!< String for selecting the broadcast option
#define CMDLINE_USB_PARAM        II("-usb")       //!< Shortcut for: -trans "SPITRANS=USB SPIPORT=<number>"
#define CMDLINE_LPT_PARAM        II("-lpt")       //!< Shortcut for: -trans "SPITRANS=LPT SPIPORT=<number>"
#define CMDLINE_QS_PARAM         II("-qs")        //!< Shortcut for: -trans "SPITRANS=TRB SPIPORT=100"
#define CMDLINE_TRB_PARAM        II("-trb")       //!< Shortcut for: -trans "SPITRANS=TRB SPIPORT=<number>"
#define CMDLINE_USBDBG_PARAM     II("-usbdbg")    //!< Shortcut for: -trans "SPITRANS=USBDBG SPIPORT=<number>"
#define CMDLINE_USBDBG2_PARAM    II("-usbdbgtc")  //!< Synonum for USBDBG
#define CMDLINE_USBCC_PARAM      II("-usbcc")     //!< Shortcut for: -trans "SPITRANS=USBCC SPIPORT=<number>"
#define CMDLINE_ADBBT_PARAM      II("-adbbt")     //!< Shortcut for: -trans "SPITRANS=ADBBT SPIPORT=<number>"
#define CMDLINE_MUL_PARAM        II("-mul")       //!< Shortcut for: -trans "SPIMUL=<number>"
#define CMDLINE_TRUNLOCK_PARAM   II("-unlock")    //!< Attempts to unlock a lockable chip

#define CMDLINE_TRANS_LIST       II("transport")   //!< List to make "lpt" and "usb" exclusive ("-trans" is no longer in the list)


// macro to make easier creating a delegate to pass to SetExpectedParam
// It creates a delegate which will invoke the method "mtd" of the class "cls" for the object "obj"
// cls: the class name
// mtd: the method name
// obj: the object name (or usually "this")
#define CMDLINE_DELEGATE(cls, mtd, obj)  (CmdDelegate::from_method<cls, &cls::mtd>(obj))

// macro to make easier create a "function" delegate to pass to SetExpectedParam
// It creates a delegate which will invoke the function "fn"
// fn: the function name
#define CMDLINE_PFUNCTION(fn)           (CmdDelegate::from_function<&fn>())

// Flags to control cmdline behaviour
const unsigned int CMDLINE_FLAG_NORUN            = (1 << 0);  //!< don't run chip after running command
const unsigned int CMDLINE_FLAG_COMPACT_HELP     = (1 << 1);  //!< Use less lines for help output
const unsigned int CMDLINE_FLAG_AUTO_CHAIN       = (1 << 2);  //!< Enable auto chaining of commands
const unsigned int CMDLINE_FLAG_TRUNLOCK         = (1 << 3);  //!< Enable transport-unlocking support
const unsigned int CMDLINE_FLAG_EXPERT           = (1 << 4);  //!< Enable 'expert mode' (to allow developers to do non-standard things)
const unsigned int CMDLINE_FLAG_MACHINE          = (1 << 5);  //!< Enable machine readable output (where appropriate)
const unsigned int CMDLINE_FLAG_RAW              = (1 << 6);  //!< Enable raw mode (to modify behaviour of certain operations)

// Transport flags to control transport options
const unsigned int CMDLINE_TRANS_FLAG_SPI        = (1 << 0);  //!< All SPI transports
const unsigned int CMDLINE_TRANS_FLAG_QS         = (1 << 1);  //!< Quicksilver
const unsigned int CMDLINE_TRANS_FLAG_TRB        = (1 << 2);  //!< TRB (Scarlet)
const unsigned int CMDLINE_TRANS_FLAG_USBDBG     = (1 << 3);  //!< USB Debug
const unsigned int CMDLINE_TRANS_FLAG_USBCC      = (1 << 4);  //!< USB charger case comms (not including in ALL_CDA as its functionality is limited to debug currently)
const unsigned int CMDLINE_TRANS_FLAG_ADBBT      = (1 << 5);  //!< Android Debug Bridge <-> Bluetooth (not including in ALL_CDA as its functionality is limited to debug currently)
const unsigned int CMDLINE_TRANS_FLAG_ALL_CDA    = (CMDLINE_TRANS_FLAG_QS | CMDLINE_TRANS_FLAG_TRB | CMDLINE_TRANS_FLAG_USBDBG);  //!< All fully functional transports for CDA ICs

/// Used to denote the type of a parameter
typedef enum
{
    DATA_TYPE_INTEGER,              //!< By default, a decimal but can also have a standard base prefix e.g. "0x" for hex
    DATA_TYPE_POSITIVE_INTEGER,     //!< Same as an integer but must be positive
    DATA_TYPE_UNDECORATED_HEX,      //!< A hex number without any "0x" prefix
    DATA_TYPE_STRING                //!< A string. Can be anything separated by spaces
}
DataTypeEnum;

/// Used to show whether a parameter is mandatory
typedef enum
{
    MANDATORY,
    NOT_MANDATORY
}
MandatoryEnum;

/// Used to show whether a parameter is hidden
typedef enum
{
    NOT_HIDDEN
}
HiddenEnum;

/// The result of calling the "Parse" function
typedef enum
{
    PARSE_HELP_DISPLAYED,                   //!< Help was displayed because a "help" parameter was encountered. The calling program should do nothing else
    PARSE_SUCCESS,                          //!< Parsed successfully
    PARSE_ERR_LIST_IS_EXCLUSIVE,            //!< Cannot give more than one parameter from a list
    PARSE_ERR_NONE_FROM_LIST,               //!< Must specify one entry from a list
    PARSE_ERR_UNRECOGNISED_PARAMETER,       //!< Unrecognised parameter
    PARSE_ERR_PARAM_ALREADY_GIVEN,          //!< Param already given
    PARSE_ERR_FLOATING_PARAM_ALREADY_GIVEN, //!< Floating param already given
    PARSE_ERR_NOT_AN_INTEGER,               //!< Expected an integer
    PARSE_ERR_EXPECTED_POSITIVE_INTEGER,    //!< Expected positive integer
    PARSE_ERR_MANDATORY_NOT_GIVEN,          //!< Mandatory parameter not given
    PARSE_ERR_VALUE_NOT_GIVEN,              //!< Value not given
    PARSE_ERR_HELP_NOT_ON_OWN,              //!< Gave "-help" with other parameters
    PARSE_ERR_BAD_UNLOCK_KEY,               //!< The transport-unlocking key is invalid
    PARSE_ERR_OUT_OF_RANGE,                 //!< The value given exceeds the types range
    PARSE_ERR_NOT_A_POSSIBLE_VALUE          //!< The value given was not one of the possible values
}
ParseResultEnum;

/// Possible returns from the "GetParameter*" fns
typedef enum
{
    GET_PARAMETER_SUCCESS,
    GET_PARAMETER_PARAM_NOT_GIVEN,
    GET_PARAMETER_VALUE_NOT_GIVEN
}
GetParameterResultEnum;

typedef struct
{
    DataTypeEnum   DataType;
    istring        ValueGiven;
    istring        ValueHelpType;
    istring        ValueHelpDescription;
    MandatoryEnum  IsMandatory;    //!< If true, the user must always give a value for this value
    std::vector<istring> PossibleValuesList; //!< List of possible values, if applicable
}
ValueStruct;

// defines a delegate type for command events
typedef srutil::delegate1<int, void*> CmdDelegate;

typedef struct
{
    std::list<istring>          Names;          //!< List storing the name of the parameter, plus any synonyms
    MandatoryEnum               IsMandatory;    //!< If true, the user must always give a value for this parameter
    bool                        IsFloating;     //!< If floating, it is value without a preceeding parameter e.g. a filename
    istring                     HelpText;       //!< The help text to display
    std::vector<ValueStruct>    ValueList;      //!< The list of values associated with this parameter
    istring                     EncounteredName;//!< If the user passes this parameter on the comnmand line, set to the name or synonym passed
    int                         UnterminatedListStartIndex; //!< If -1 then no unterminated list, otherwise index of start of list in values
    CmdDelegate                 CmdHandler;     //!< Delegate to the method to be called when the current command is issued
    void*                       mpData;         //!< Data to be passed to the handler
}
ParamStruct;

typedef enum
{
    LIST_UNIQUE,                  //!< Indicates that one and only one of the parameters in the list must be given
    LIST_EXCLUSIVE,               //!< Indicates that only one can be given but none are compulsory
    LIST_COMPULSORY,              //!< As per LIST_CHAINABLE, except that at least one parameter from the list must be given
    LIST_CHAINABLE                //!< Indicates multiple parameters in the list may be given, including repeats of the same parameter
}
ListTypeEnum;

/// The type of list to consider
typedef enum
{
    LIST_SEARCH_TYPE_UNIQUE,            //!< Only unique lists
    LIST_SEARCH_TYPE_EXCLUSIVE,         //!< Only exclusive lists
    LIST_SEARCH_TYPE_CHAINABLE,         //!< Only chainable lists
    LIST_SEARCH_TYPE_COMPULSORY,        //!< Only compulsory lists
    LIST_SEARCH_TYPE_ANY                //!< Any type of list

}
ListSearchTypeEnum;

typedef enum
{
    LIST_DEFINED,    //!< Search the defined list mDefinedParamList
    LIST_ENCOUNTERED //!< Search the encountered list mEncounteredParamList
}
InternalListToSearchEnum;

typedef struct
{
    ListTypeEnum           ListType;
    istring                Name;
    std::vector<istring>   ParameterList;
}
ListStruct;

typedef enum
{
    PRE_PARSE_QUIET,        //!< Set true if preparse finds "-quiet"
    PRE_PARSE_SYNTAX,       //!< Set true if preparse finds "-syntax"
    PRE_PARSE_DEBUG,        //!< Set true if preparse finds "-debug"
    PRE_PARSE_NUM_FLAGS
}
PreParseFlags;

/// Whether to ignore errors on chaining
typedef enum
{
    CHAINING_STOP_ON_FAILURE,  //!< Exit if any command fails
    CHAINING_IGNORE_FAILURES   //!< Run all commands regardless of errors 
}
ChainingMode;

class CCmdLine
{
    public:
        /**
         *  Constructor.
         *  @param[in] aProgramName The name of the program executable (minus the extension if any)
         *  @param[in] aProgramTitle The title for the program (a handful of words to describe the tool)
         *  @param[in] aProgramDescription Briefly describes the functionality of the program for help
         *  @param[in] aCopyrightStartYear The 4-digit year when the code was first written/released
         *  @param[in] aArgc The argc passed in to main
         *  @param[in] apArgv The argv passed in to main
         */
        CCmdLine
        (
            const istring& aProgramName,
            const istring& aProgramTitle,
            const istring& aProgramDescription,
            const istring& aCopyrightStartYear,
            int     aArgc,
            ichar*  apArgv[]
        );

        // ======================================================================================
        // Defining the command line
        // ======================================================================================

        /**
         *  Returns whether a parameter has been added or not
         *  @param[in] aName The name of the parameter
         *  @return true if param exists, false otherwise
         */
        bool ParamExists(const istring& aName);

        /**
         *  Add an expected parameter.
         *  @param[in] aName The name of the parameter
         *  @param[in] aHelpText The help text for the parameter
         *  @param[in] aIsMandatory Whether the parameter is mandatory or not
         *  @param[in] aIsHidden Whether the parameter is hidden or not
         */
        void SetExpectedParam(const istring& aName, const istring& aHelpText, MandatoryEnum aIsMandatory, HiddenEnum aIsHidden);

        /**
         *  Add an expected parameter.
         *  @param[in] aName The name of the parameter
         *  @param[in] aHelpText The help text for the parameter
         *  @param[in] aIsMandatory Whether the parameter is mandatory or not
         *  @param[in] aIsHidden Whether the parameter is hidden or not
         *  @param[in] aHandler A delegate which is the command handler
         *  @param[in] apData Data to be passed to the command handler
         */
        void SetExpectedParam(const istring& aName, const istring& aHelpText, MandatoryEnum aIsMandatory, HiddenEnum aIsHidden, CmdDelegate aHandler, void* apData = NULL);

        /**
         *  Add a synonym for the name of the most recently added parameter.
         *  @param[in] aSynonym The synonym.
         */
        void SetExpectedParamSynonym(const istring& aSynonym);

        /**
         *  Add a parameter that has no preceeding delimiter (e.g. a filename where there is no
         *  need to put "-file" in front of it).
         *  @param[in] aName The name of the parameter
         *  @param[in] aValueHelpType A string that describes the type of the "value"
         *  @param[in] aValueHelpDescription A string that describes the value
         *  @param[in] aIsMandatory Whether the parameter is mandatory or not
         */
        void SetExpectedFloatingParam(const istring& aName, const istring& aValueHelpType, const istring& aValueHelpDescription, MandatoryEnum aIsMandatory);

        /**
         *  Add an expected value to the most recently declared parameter.
         *  Can be called multiple times to add more than one value to a parameter.
         *  @param[in] aValueStruct The type of the value
         *  @param[in] aValueHelpType A string that describes the type of the "value"
         *  @param[in] aValueHelpDescription A string that describes the value
         *  @param[in] aIsMandatory Whether the value is mandatory or not
         *  @note Any non-mandatory values MUST appear AFTER all mandatory values
         */
        void AddExpectedValue(DataTypeEnum aValueStruct, const istring& aValueHelpType,
            const istring& aValueHelpDescription, MandatoryEnum aIsMandatory);

        /**
         *  Indicate values can be one of a number of possibilities
         *  @param[in] aValueHelpType A string that describes the type of the "value"
         *  @param[in] aIsMandatory Whether the value is mandatory or not
         */
        void AddExpectedPossibleList(const istring& aValueHelpType, MandatoryEnum aIsMandatory);

        /**
         *  Indicate a possible value
         *  @param[in] aPossibleValueStr The possible value
         */
        void AddExpectedPossibleListValue(const istring& aPossibleValueStr);

        /**
         *  Add an unterminated list to the most recently declared parameter.
         *  @param[in] aValueStruct The type of the value
         *  @param[in] aValueHelpType A string that describes the type of the "value".
         *             The application's help will use aValueHelpType in the output for the
         *             relevant parameter. E.g. if aValueHelpType="val", the help will
         *             report "<val1> [<val2> ...]" as the list of arguments for the parameter.
         *  @param[in] aValueHelpDescription A string that describes the value
         */
        void AddExpectedUnterminatedList(DataTypeEnum aValueStruct, const istring& aValueHelpType,
            const istring& aValueHelpDescription);

        // ======================================================================================
        // Lists
        // ======================================================================================

        /**
         *  Create a new parameter inter-dependency list.
         *  @param[in] aListType The type of list
         *  @param[in] aListName The name of the list
         */
        void CreateList(ListTypeEnum aListType, const istring& aListName);

        /**
         *  Add a new parameter name to an existing inter-dependency list.
         *  @param[in] aListName The name of the list
         *  @param[in] aParamName Name of parameter to add
         */
        void AddToList(const istring& aListName, const istring& aParamName);

        /**
         *  Returns the parameter that was given from the possible list items.
         *  @param[in] aListName The name of the list
         *  @return Parameter from list that was given on command line
         */
        istring GetParamForList(const istring& aListName);

        /**
         *  Get the next chainable / compulsory command in the encountered param list.
         *  @param[out] aChainedCmd Current chainable / compulsory command
         *  @return true, if encountered chainable / compulsory command found, false otherwise.
         */
        bool GetNextChainedCmd(istring& aChainedCmd);

        /**
         *  Get the number of chained commands in the encountered param list.
         *  @return number of chained commands given on command line
         */
        unsigned int GetNumberOfChainedCommands() {return mNumChainedCommands;}

        // ======================================================================================
        // Support for built-in parameters
        // ======================================================================================

        /**
         *  Add flags that configure how CmdLine behaves
         *  @param[in] aFlags The flags to add, made up from CMDLINE_FLAG_*, OR'd together.
         */
        void AddFlags(unsigned int aFlags);

        /**
         *  Add support for transport options.
         *  -trans and -remote (hidden) are always added, regardless of the aFlags value.
         *  @param[in] aFlags The flags to add, made up from CMDLINE_TRANS_FLAG_*, OR'd together.
         */
        void AddTransportOptions(unsigned int aFlags);

        /**
         *  Add support for "broad".
         *  @param[in] aListName The name of a [previously created] list to which this option
         *  will be added. If the empty string is supplied, it will not be added to a list.
         *  @param[in] aAdditionalHelp An additional string appended to the general help text
         */
        void AddBroadSupport(const istring& aListName, const istring& aAdditionalHelp);

        // ======================================================================================
        // Parsing
        // ======================================================================================

        /**
         *  Parses the command line, looking for values for parameters.
         *  @return The result of the operation
         */
        ParseResultEnum Parse();

        // ======================================================================================
        // Executing
        // ======================================================================================

        /**
         *  Executes the issued command invoking its handler.
         *  The handler must be valid
         *  The command must exist in the passed list
         *  @param[in] aListName The commands list
         *  @return the return value from the
         */
        int Execute(const istring& aListName);

        /**
         *  Executes commands in the auto chain list invoking their handlers.
         *  Command handlers must be defined.
         *  If any of the handler returns an error and aStopOnFailure is true the execution aborts.
         *  Otherwise it keeps executing the other commands. it returns success or the last which failed
         *  @param[in] aMode Whether to abort on a chaining error or carry on with other commands
         *  @return success or the first failed command if aMode = CHAINING_STOP_ON_FAILURE, the last failure if false.
         */
        int Execute(ChainingMode aMode);

        // ======================================================================================
        // Errors
        // ======================================================================================

        /**
         *  Get the last error.
         *  @return A string containing the last error
         */
        istring GetLastError() { return mLastError; }

        /**
         *  Do not print out an error if a command line syntax error is found. This allows tool that
         *  have not been converted to define command line parameters to use the other facilities e.g.
         *  version output
         */
        void TurnOffCommandLineSupport() { mCommandLineSupport = false; }

        /**
         *  Creates the transport string for pttrans_open.
         *  @return The string containing the transport options
         */
        istring FetchTransportOptions();

        /**
         *  Returns a value given for a parameter based on its index
         *  @param[in] aName Name of the value
         *  @param[in] aIndex Its index for its parameter (1-based)
         *  @param[out] aArgument The value given
         *  @return The result of the operation
         */
        GetParameterResultEnum GetParameterValue(const istring& aName, unsigned int aIndex, istring& aArgument);

        /**
         *  Returns a value given for a floating parameter
         *  @param[in] aParamName Name of the parameter
         *  @param[out] aArgument The value given
         *  @return true, if floating value was given
         */
        bool GetFloatingParameterValue(const istring& aParamName, istring& aArgument);

        /**
         *  Returns a value given for a flag parameter i.e. can only be on or off
         *  @param[in] aParamName Name of the parameter
         *  @return whether flag set or not
         */
        bool GetFlagParameterValue(const istring& aParamName);

        /**
         *  Returns an integer value given for a parameter based on its index
         *  @param[in] aName Name of the value
         *  @param[in] aIndex Its index for its parameter (1-based)
         *  @param[out] aValue The value given
         *  @return The result of the operation
         */
        GetParameterResultEnum GetParameterValueAsInteger(const istring& aName, unsigned int aIndex, int& aValue);

        /**
         *  Returns an uint64_t value given for a parameter based on its index
         *  @param[in] aName Name of the value
         *  @param[in] aIndex Its index for its parameter (1-based)
         *  @param[out] aValue The value given
         *  @return The result of the operation
         */
        GetParameterResultEnum GetParameterValueAsU64(const istring& aName, unsigned int aIndex, uint64_t& aValue);

        /**
         *  Returns the number of values passed for an unterminated list
         *  @return number of values given on command line
         */
        int GetUnterminatedListLength(const istring& aName);

        // ======================================================================================
        // User output fns
        // Support for generic handling of exit status messages, errors and warnings
        // ======================================================================================

        /**
         *  Prints out the final "Success" or "Failed" message for a cmdline tool when exiting.
         *  @return The value to be returned from main.
         */
        int OutputFinalMessage();

        /**
         *  Prints out supplied error message, followed by a 'failed' message.
         *  @param[in] aErrorMessage Error message.
         */
        void OutputErrorAndFailMessages(const istring& aErrorMessage);

        /**
         *  Prints out an error message.
         *  The message is always displayed (even in 'quiet' mode).
         *  @param[in] aMessage The message.
         */
        void OutputErrorMessage(const istring& aMessage);

        /**
         *  Prints out the last error message which was set (if there is one).
         *  The message is always displayed (even in 'quiet' mode).
         */
        void OutputLastErrorMessage();

        /**
         *  Prints out a warning message.
         *  The message is not displayed in 'quiet' mode.
         *  @param[in] aMessage The message.
         */
        void OutputWarningMessage(const istring& aMessage);

        /**
         *  Prints out an essential message.
         *  The message is always displayed (even in 'quiet' mode).
         *  @note THIS METHOD IS NOT INTENDED FOR OUTPUT OF FINAL 'SUCCESS' OR 'FAILURE' MESSAGES,
         *  ERROR MESSAGES OR WARNING MESSAGES.
         *  @param[in] aMessage The message.
         */
        void OutputEssentialMessage(const istring& aMessage);

        /**
         *  Prints out an info message.
         *  The message is not displayed in 'quiet' mode.
         *  @note THIS METHOD IS NOT INTENDED FOR OUTPUT OF FINAL 'SUCCESS' OR 'FAILURE' MESSAGES,
         *  ERROR MESSAGES OR WARNING MESSAGES.
         *  @param[in] aMessage The message.
         */
        void OutputInfoMessage(const istring& aMessage);

        /**
         *  Determines whether or not in 'quiet' mode.
         *  @return true if quiet mode is on, false otherwise
         */
        bool IsQuiet();

        /**
         *  Prints out help information
         */
        void PrintHelp();

    private:

        // ======================================================================================
        // Help output
        // ======================================================================================

        /**
         *  Shows help for an individual parameter.
         *  @param[in] aParamIt An iterator to the help string
         */
        void ShowHelpParameter(std::vector<ParamStruct>::const_iterator aParamIt);

        /**
         *  Shows help for any synonyms of a parameter.
         *  @param[in] aParamIt Iterator to the parameter to print 'synonym' help for
         */
        void ShowHelpParameterSynonyms(std::vector<ParamStruct>::const_iterator aParamIt);

        /**
         *  Writes the command syntax to gTempFormatStr.
         *  @param[in] aParamIt An iterator to the help string
         */
        void WriteParamSyntax(std::vector<ParamStruct>::const_iterator aParamIt);

        /**
         *  Print a string with a indent
         *  @param[in] aParamStr String to print
         *  @param[in] aIdent Amount of indent
         */
        void PrintIndentString(const istring& aParamStr, int aIndent);

        /**
         *  Print the string that describes a help parameter e.g. -trans <string>
         *  @param[in] aParamStr String to print
         */
        void PrintHelpParameter(const istring& aParamStr);

        /**
         *  Print out the description of a help parameter over multiple lines
         *  @param[in] aDescriptionStr String to print
         *  @param[in] aIndentFirstLine Ident for first line
         *  @param[in] aIndentSubsequentLines Indent for subsequent lines
         */
        void PrintHelpParameterDescription(const istring& aDescriptionStr, int aIndentFirstLine, int aIndentSubsequentLines);

        /**
         *  Print the syntax for the parameters
         *  @param[in] aDescriptionStr String to print
         */
        void PrintFormattedHelpSyntax(const istring& aDescriptionStr);

        /**
         *  Print a parameter for help
         *  @param[in] aDescriptionStr String to print
         */
        void PrintFormattedHelpParameter(const istring& aDescriptionStr);

        /** 
         *  Set the last error string to indicate an unknown parameter.
         *  @param[in] apParamStr The string to set
         */
        ParseResultEnum ErrUnknownParameter(const ichar* apParamStr);

        /**
         *  Attempt to get parameter at invalid index.
         *  @param[in] aIndex The index of the parameter (1-based)
         *  @param[in] apName The name of the parameter
         */
        void ErrGetParamInvalidIndex(unsigned int aIndex, const ichar* apName);

        /**
         *  A value for a parameter was not given
         *  @param[in] apName The name of the parameter
         *  @param[in] aValueStruct Infomation about the value at the current index
         */
        void HandleMandatoryParamValueNotGiven(const ichar* apName, const ValueStruct& aValueStruct);

        // ======================================================================================
        // Lists
        // ======================================================================================

        /**
         *  Make a new list.
         *  @param[in] aListType The type of list
         *  @param[in] aName The name of list
         */
        void NewList(ListTypeEnum aListType, const istring& aName);

        /**
         *  Add a new parameter name to cmdline's internal 'encountered list'.
         *  @param[in]  aParam Parameter to add
         *  @return Iterator to the element added to the 'encountered list'.
         */
        std::vector<ParamStruct>::iterator AddToEncounteredList(std::vector<ParamStruct>::iterator aParam);

        /**
         *  Checks the lists specified by the user for required associations between a list of
         *  parameters.
         *  @return The result of the operation
         */
        ParseResultEnum CheckAssociatedLists();

        /**
         *  Checks that the parameters on the command line match the requirements of a list.
         *  Note: there is no validation to be performed for a chainable list as it can have zero,
         *        one or many items specified (including repeats). So this method should not
         *        be called for chainable lists.
         *  @param[in] aListStruct The details of the list being checked
         *  @return The result of the operation
         */
        ParseResultEnum CheckList(const ListStruct& aListStruct);

        /**
         *  Gets a pointer to a list from its name.
         *  @param[in] aListName The name of the list
         *  @param[out] aMatchedList A reference to the list
         *  @return true if the list exists, false otherwise
         */
        bool FindList(const istring& aListName, std::vector<ListStruct>::iterator& aMatchedList);

        // ======================================================================================
        // Misc
        // ======================================================================================

        /**
         *  Prints debug information about the parameters.
         */
        void DebugPrintParams();

        /**
         *  Pre-parses the command line to extract information need before doing "Parse".
         */
        void PreParseCmdLine();

        /**
         *  Handles the enabling of the debug log file.
         *  @param[in] aFileName The string to parse as a log file.
         */
        void ProcessDebugLogFileRequest(const istring& aFileName);

        /**
         *  Print out versioning and copyright.
         */
        void OutputVersioningAndCopyright();

        /**
         *  Indicates if a parameter was given on the command line.
         *  @param[in] aParamName The name of the parameter
         *  @return true if the parameter was specified, false otherwise
         */
        bool ParamWasSpecified(const istring& aParamName);

        /**
         *  Search the parameter list to see if the param name matches one of those expected.
         *  @param[in] apParamStr Name of parameter
         *  @param[in] aLookForFloatingParams Look for floating paramaters (as well as non-floating ones)
         *  @param[out] aMatchedParam Pointer to matched parameter
         *  @return true if the parameter exists, false otherwise
         */
        bool MatchParameter(const ichar* apParamStr, bool aLookForFloatingParams,
            std::vector<ParamStruct>::iterator& aMatchedParam);

        /**
         *  Check whether parameter is chainable / compulsory.
         *  @param[in] aParam The parameter
         *  @return true if the parameter belongs to a chainable or compulsory list, false otherwise
         */
        bool IsParameterChainableOrCompulsory(const ParamStruct& aParam);

        /**
         *  Does nothing for commands which are not chainable or compulsory.
         *  For a chainable / compulsory command, it ensures that the parameter reflects the current value(s) for that parameter.
         *  @param[in] aParam The parameter
         *  @return GET_PARAMETER_NOT_CURR_CHAINED, if param is chainable / compulsory but not the current encountered param
         *          GET_PARAMETER_SUCCESS, otherwise
         */
        GetParameterResultEnum ValidateAndUpdateCmd(ParamStruct& aParam);

        // Used to show whether a parameter is hidden
        typedef enum
        {
            PARSE_EXPECTING_UNDEFINED, //!< The undefined state
            PARSE_EXPECTING_PARAMETER, //!< The next argument should be a parameter
            PARSE_EXPECTING_VALUE,     //!< The next argument should be a value
            PARSE_EXPECTING_EITHER     //!< The next argument could be either a parameter or a value, it's not possible to predict
        }
        ParseExpectingEnum;

        /**
         *  Look for the currently encountered parameter in the collection of expected commands.
         *  @param[out] aEncounteredParam The parameter found
         *  @param[in]  apParameter The name of the parameter
         *  @param[out] aExpectingNext Used to determine the type of the next item; see the enum definition for details
         *  @param[out] aNumValues Current entry in value list being processed
         */
        ParseResultEnum ParseParameter(std::vector<ParamStruct>::iterator& aEncounteredParam, const ichar* apParameter,
            ParseExpectingEnum& aExpectingNext, unsigned int& aNumValues);

        /**
         *  Validate that the values conform to the rules (Eg. that they're integers if they're meant to be).
         *  @param[in] aMatchedParam Current parameter
         *  @param[in, out] aValueIndex Current entry in value list being processed. On success, increments to next in list
         *  @param[in] apValue The value given on the command line
         */
        ParseResultEnum ValidateValues(std::vector<ParamStruct>::iterator aMatchedParam, unsigned int& aValueIndex, const ichar* apValue);

        /**
         *  Validate that the value is one of the possibilities allowed
         *  @param[in] aMatchedParam Current parameter
         *  @param[in] aNumValues Current entry in value list being processed
         *  @param[in] apValue The value given on the command line
         *  @return An enumeration indicating if the value is validated
         */
        ParseResultEnum ValidateAgainstPossibleValues(std::vector<ParamStruct>::const_iterator aMatchedParam, unsigned int aNumValues, const ichar* apValue);

        /**
         *  Process a request for help
         */
        ParseResultEnum ProcessHelpRequest();

        /**
         *  Search the parameter list to see if the param name matches one of those expected.
         *  @param[in] apParamStr Name of parameter
         *  @param[out] aMatchedParam Pointer to matched parameter
         *  @param[in] aMatchFn A function to call which will determine if it is found or not
         *  @param[out] aEncounteredSynonym Set to true if a synonym was used
         *  @param[in] InternalListToSearch Determines whether to search the defined parameter list or encountered parameter list
         *  @return true if the parameter is a match, false otherwise
         */
        bool FindParam(const ichar* apParamStr, std::vector<ParamStruct>::iterator& aMatchedParam,
            bool (*aMatchFn) (const ParamStruct& aParamStruct), bool& aEncounteredSynonym, InternalListToSearchEnum InternalListToSearch=LIST_DEFINED
        );

        /**
         *  Defines the entries for a value.
         *  @param[in] aValueStruct Structure to initialise
         *  @param[in] aValueHelpType A string that describes the value e.g. "<filename>"
         *  @param[in] aValueHelpDescription A description of the value
         *  @param[in] aIsMandatory Whether the value is mandatory or not
         */
        void SetupValue(DataTypeEnum aValueStruct, const istring& aValueHelpType, const istring& aValueHelpDescription, MandatoryEnum aIsMandatory);

        /**
         *  Get the current chainable / compulsory command in the encountered param list - full parameter structure.
         *  @param[out] aChainedCmd Current chainable / compulsory command
         *  @return true, if encountered chainable / compulsory command found, false otherwise.
         */
        bool GetCurrentChainedCmdStruct(ParamStruct& aChainedCmd);

        /**
         *  Determines if parameter is displayed to user. 
         *  @param[in] aParam Pointer to parameter
         *  @return true if parameter is to be displayed
         */
        bool DisplayThisParameter(std::vector<ParamStruct>::const_iterator aParam);

        /**
         *  Loops through lists, extracting parameters to display help.
         *  @param[in] aShowSyntax Either showing syntax (true) or detailed help for parameters (false)
         *  @param[in] aDoingActions True if only showing actions, not flags
         */
        void CommonShowHelpForLists(bool aShowSyntax, bool aDoingActions);

        /**
         *  Shows the help syntax for parameters in this list.
         *  @param[in] aList Pointer to list
         */
        void ShowHelpSyntaxInList(std::vector<ListStruct>::const_iterator aList);

        /**
         *  Chooses brackets to use when displaying help items from a particular list type.
         *  @param[in] aListType The list type
         *  @param[out] aOpeningParantheses Opening Parentheses to use
         *  @param[out] aClosingParentheses Closing Parentheses to use
         */
        void GetHelpItemParentheses(ListTypeEnum aListType, istring& aOpeningParantheses, istring& aClosingParentheses);

        /**
         *  Shows the detailed help for parameters in this list.
         *  @param[in] aList Pointer to list
         */
        void ShowHelpParamsInList(std::vector<ListStruct>::const_iterator aList);

        /**
         *  Shows help syntax for parameters that are in associated lists.
         *  @param[in] aDoingActions True if only showing actions, not flags
         */
        void ShowHelpSyntaxForLists(bool aDoingActions);

        /**
         *  Shows help syntax for parameters not in associated lists.
         *  @param[in] aDoingActions True if only showing actions, not flags
         */
        void ShowHelpSyntaxNotInAList(bool aDoingActions);

        /**
         *  Shows detailed help for parameters that are in associated lists.
         *  @param[in] aDoingActions True if only showing actions, not flags
         */
        void ShowHelpParametersForLists(bool aDoingActions);

        /**
         *  Shows detailed help for parameters not in associated lists.
         *  @param[in] aDoingActions True if only showing actions, not flags
         */
        void ShowHelpParametersNotInAList(bool aDoingActions);

        /**
         *  Find out if a parameter is in a list.
         *  See other overload of this method.
         *  @param[in] aName Name of parameter
         *  @param[in] aTypeOfList The types of list to search
         *  @return true if parameter is in a list
         */
        bool IsParamInAList(const istring& aName, ListSearchTypeEnum aTypeOfList);

        /**
         *  Find out if a parameter is in a list.
         *  See other overload of this method.
         *  @param[in] aName Name of parameter
         *  @param[in] aTypeOfList The types of list to search
         *  @param[out] aListName Name of list found
         *  @return true if parameter is in a list
         */
        bool IsParamInAList(const istring& aName, ListSearchTypeEnum aTypeOfList, istring& aListName);

        /**
         *  Gets the data type of a parameter given the name
         *  @param[in] aName Name of parameter
         *  @param[in] aIndex Index of the entry in the value list
         */
        DataTypeEnum GetDataType(const istring& aName, unsigned int aIndex);

        /**
         *  Gets the data type of a parameter given an interator in the list
         *  @param[in] aMatchedParam The iterator in the list
         *  @param[in] aValueListIndex Index of the entry in the value list
         */
        DataTypeEnum GetMatchedParamDataType(std::vector<ParamStruct>::const_iterator aMatchedParam, int aValueListIndex);

        /**
         *  Append "0x" to an undecorated hex string
         *  @param[in] aValueStr String without prefix
         *  @return The prefixed string
         */
        istring PrefixUndecoratedHex(const istring& aValueStr);

        /**
         *  Move to next chained command given on command line
         *  @return true, if more chained commands exist
         */
        bool MoveToNextChainedCmd();

        /**
         *  Automatically create a list of commands that can be chained
         */
        void CreateAutoChainList();

        /**
         *  Show compact help
         *  @param[in] aOnlyFlags If true, show flags which start with '-', otherwise other parameters (actions)
         */
        void ShowCompactHelp(bool aOnlyFlags);

        /**
         *  Show compact help for flag parameters
         */
        void ShowCompactHelpForFlags();

        /**
         *  Show compact help for action parameters
         */
        void ShowCompactHelpForActions();

        /**
         *  @return true, if compact help is on
         */
        bool IsCompactHelpFlagOn();

        /**
         *  @return true, if autochain is on
         */
        bool IsAutoChainFlagOn();

        /**
         *  Process the flag specified and clear from the bit-field.
         *  @param[in] aFlag The flag being processed
         *  @param[in,out] aFlags Flag bits left to process
         *  @return true, if flag is on
         */
        bool HandleFlag(unsigned int aFlag, unsigned int& aFlags);

        /**
         *  Indicates if the parameter is a flag
         *  @param[in] aName The name of the parameter
         *  @return true, if parameter is a flag
         */
        bool IsParameterAFlag(const istring& aName);

        // Member variables
        std::vector<ParamStruct>    mDefinedParamList;           // The allowed parameters defined by the application - no arg values stored
        std::vector<ParamStruct>    mEncounteredParamList;       // The actual parameters encountered on the command line in the order they
                                                                 // were encountered - arg values stored
        int16                       mCurrentChainedCmdIndex;     // Index of most recently retrieved chainable / compulsory command within mEncounteredParamList
        unsigned int                mNumChainedCommands;         // 0 if no chained commands, otherwise number encountered
        std::vector<ListStruct>     mAssociatedParametersList;   // List of all parameter lists defined by the application
        istring                     mLastError;
        istring                     mProgramName;
        istring                     mProgramTitle;
        istring                     mProgramDescription;
        istring                     mCopyrightStartYear;
        int                         mProgramArgc;
        ichar**                     mppProgramArgv;

        bool                        mPreParseFlags[PRE_PARSE_NUM_FLAGS]; //!< Flags set during pre-parse
        bool                        mCommandLineSupport;           //!< If false, don't do anything with the command line e.g. only output version info
        ParseResultEnum             mParseResult; //!< Remember the parse result

        unsigned int                mFlags;
};

#endif
