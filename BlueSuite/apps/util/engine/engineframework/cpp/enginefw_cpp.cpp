/**********************************************************************
 *
 *  enginefw_cpp.cpp
 *
 *  Copyright (c) 2011-2021 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Handles the engine worker classes.
 *
 ***********************************************************************/

#define NOMINMAX


#ifdef WIN32
// Needed to enable the IsDebuggerPresent() function
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <string>
#else
#include <stdarg.h>
#include <string.h>
#endif

#include "engine/enginefw_interface.h"
#include "thread/critical_section.h"
#include "thread/atomic_counter.h"
#include "thread/thread.h"
#include "time/stop_watch.h"
#include "common/portability.h"
#include "misc/multilistparser.h" // Compile in the source code, don't link with the library

#include "enginefw_cpp.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <deque>
#include <fstream>
#include <assert.h>
#include <stdlib.h>

// Some *compile-time* constants used in this module
// (declared this way to avoid creating global data whose value is set at *run-time*...)
enum { ALL_LEVELS = 32 };
#define CONSOLE_BANNER "=============================================================================="
#define DEFAULT_PARAGRAPH_STRING " "
#define GRP_NONE_STR "none"


using namespace std;

// See B-150107
#define DONT_SILENTLY_IGNORE_GROUP_PROBLEMS
#ifdef  DONT_SILENTLY_IGNORE_GROUP_PROBLEMS
bool gUnGroupedErrorHasOccurred = false;
#endif

// A helper macro for use in various CMessageHandler methods.
#define FOR_LOOP_ROUND_ALL_HANDLERS                                                     \
    CMessageHandler::GroupEnum localGroup = CTheMsgHnd::Instance().GetLocalGroup(this); \
    CTheMsgHnd::MsgHndMap& msgHandlers = CTheMsgHnd::Instance().GetMessageHandlers();   \
    for (CTheMsgHnd::MsgHndIter msgHndIt = msgHandlers.begin();                         \
         msgHndIt != msgHandlers.end() &&                                               \
            localGroup != CMessageHandler::GROUP_ENUM_RSVD_LOCAL;                       \
         ++msgHndIt)

// A helper macro for use in various CMessageHandler methods.
#define MEMBER_VARS (msgHndIt->second)

// A place to remember the state of the engine framework. This is not a member variable
// because it needs to be accesible from a straight-C function.
// The state may only go 'forwards', never 'backwards'.
static enum
{
    NOT_INITIALISED = 0,    //< The framework is not yet initialised (initial state)
    INITIALISED,            //< The framework has been initialised
    SHUTTING_DOWN           //< The framework is being shutdown (and will ignore all calls)
} gEngineFrameworkLifeCycleState = NOT_INITIALISED;


/////////////////////////////////////////////////////////////////////////////
//                             CMsgQueue
/////////////////////////////////////////////////////////////////////////////

CMsgQueue::CMsgQueue(QueueType type, ostream* stream, bool allLevelsEnabled) :
    mQueueType(type),
    mpStream(stream),
    mLevelsEnabled(allLevelsEnabled ? ~0 : 0)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(type, DEBUG_QUEUE);
}

/////////////////////////////////////////////////////////////////////////////

CMsgQueue::~CMsgQueue()
{
}

/////////////////////////////////////////////////////////////////////////////

void CMsgQueue::OutputMsg(uint32 level, const char* text)
{
    CriticalSection::Lock lock(GetSyncLock());

    if (mpStream && IsLevelEnabled(level))
    {
        *mpStream << text << endl;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool CMsgQueue::IsLevelEnabled(uint32 level)
{
    bool retVal = false;
    bool levelIsValid = true;

    // The XXXXX_ALL enumeration is not valid at this point
    switch (mQueueType)
    {
        case STATUS_QUEUE:
            ASSERT_IF_VALUE_OUT_OF_RANGE(level, STATUS_COUNT_DO_NOT_USE_THIS - 1);
            if (level == STATUS_ALL)
            {
                levelIsValid = false;
            }
            break;
        case DEBUG_QUEUE:
            ASSERT_IF_VALUE_OUT_OF_RANGE(level, DEBUG_COUNT_DO_NOT_USE_THIS - 1);
            if (level == DEBUG_ALL)
            {
                levelIsValid = false;
            }
            break;
        default:
            levelIsValid = false;
            assert(false);
            break;
    }

    if (level < ALL_LEVELS && levelIsValid && (mLevelsEnabled & (1 << level)))
    {
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

void CMsgQueue::InheritLevels(CMsgQueue* apOtherQueue)
{
    if (apOtherQueue)
    {
        mLevelsEnabled = apOtherQueue->mLevelsEnabled;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMsgQueue::LevelEnable(uint32 level, bool enable)
{
    if (level < ALL_LEVELS)
    {
        if (enable)
        {
            mLevelsEnabled |= (1 << level);
        }
        else
        {
            mLevelsEnabled &= ~(1 << level);
        }
    }
    else
    {
        if (enable)
        {
            mLevelsEnabled = ~0;
        }
        else
        {
            mLevelsEnabled = 0;
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
//                             CStatusMsg
/////////////////////////////////////////////////////////////////////////////

CStatusMsg::CStatusMsg(ostream* stream) :
    CMsgQueue(STATUS_QUEUE, stream, true)
{
}

/////////////////////////////////////////////////////////////////////////////

CStatusMsg::~CStatusMsg()
{
}

/////////////////////////////////////////////////////////////////////////////
//                             CDebugMsg
/////////////////////////////////////////////////////////////////////////////

CDebugMsg::CDebugMsg(ostream* stream) :
    CMsgQueue(DEBUG_QUEUE, stream, false),
    mpTimeSinceStart(NULL),
    mTimeStampsEnabled(true),
    mWritingToOutputPane(false)
{
    mpTimeSinceStart = new StopWatch;

    SetIndent(0);
}

/////////////////////////////////////////////////////////////////////////////

CDebugMsg::~CDebugMsg()
{
    delete mpTimeSinceStart;
}

/////////////////////////////////////////////////////////////////////////////

void CDebugMsg::SetTimeStampState(bool enabled)
{
    mTimeStampsEnabled = enabled;
}

/////////////////////////////////////////////////////////////////////////////

void CDebugMsg::DebugOutputWithList(uint32 level, bool entry, bool exit, const char* function,
    const char* filename, uint32 linenum, const char* format, va_list argptr)
{
    if (IsLevelEnabled(level))
    {
        /*
        NOTE on va_list usage
        According to the standard, va_list is not reusable. See ISO C99, 7.15(3).

        CMessageHandler::DebugOutputInHandler() is already using a va_list and VSNPRINTF
        is going to use another. This causes a segfault on linux. argptr needs to be
        cloned with va_copy and VSNPRINTF needs to  use the clone. VSNPRINTF calls vsnprintf
        in the standard C library on linux. vsnprintf will corrupt the va_list every
        call. Successive calls on a corrupted va_list will likely cause an exception.

        MSVS2005 does not have va_copy but doesn't exhibit any problems.
        */
        va_list argPtrCopy;
#ifdef WIN32
        argPtrCopy = argptr;         // Straight copy for WIN32
#endif
        unsigned long threadId = ThreadID::Id();
        int16 indentForThisLine = GetIndent();
        int16 extraSpacesToAddToIndent = 0;
        static uint16 maximumPrefixTextSizeSoFar = 0;

        static const int STANDARD_BUFFER_SIZE = 500;
        char prefixText[STANDARD_BUFFER_SIZE];

        if (mWritingToOutputPane)
        {
            SNPRINTF(prefixText, sizeof(prefixText), "%s(%lu): ", filename, linenum);
            // Cast to uint16 is ok as long as STANDARD_BUFFER_SIZE (used to size prefixText), is <= uint16 max
            uint16 prefixTextLength = static_cast<uint16>(strlen(prefixText));
            maximumPrefixTextSizeSoFar = std::max<uint16>(maximumPrefixTextSizeSoFar, prefixTextLength);
            maximumPrefixTextSizeSoFar = std::min<uint16>(maximumPrefixTextSizeSoFar, 120);
            extraSpacesToAddToIndent = std::max<int16>(maximumPrefixTextSizeSoFar - prefixTextLength, 0);
        }
        else
        {
            // Strip off the file path to leave raw file name (including extension)
            string strFilename(filename);
            size_t found = strFilename.find_last_of("/\\");
            if (found != string::npos)
            {
                strFilename.replace(0, found + 1, "");
            }

            // Format and determine the prefix text
            if (mTimeStampsEnabled && mpTimeSinceStart)
            {
                unsigned long milliSec = mpTimeSinceStart->duration();
                SNPRINTF(prefixText, sizeof(prefixText), "[%03lu.%03lu %04lX %20s(%4lu)] ",
                    (milliSec / 1000) % 1000,
                    milliSec % 1000,
                    threadId,
                    strFilename.c_str(),
                    linenum);
            }
            else
            {
                SNPRINTF(prefixText, sizeof(prefixText), "[%04lX %20s(%4lu)] ", threadId, strFilename.c_str(), linenum);
            }
        }

        // Add the spacing to the prefix
        if (level == DEBUG_ENTRY_EXIT)
        {
            if (entry)
            {
                indentForThisLine = IncrementIndent();
            }
            if (exit)
            {
                indentForThisLine = DecrementIndent();
            }
        }
        indentForThisLine += extraSpacesToAddToIndent;
        while (indentForThisLine-- > 0)
        {
            const char SINGLE_SPACE[] = " ";
            strncat(prefixText, SINGLE_SPACE, sizeof(SINGLE_SPACE));
        }

        // Format and determine the user specified text
        bool textGenerated = false;
        char userSpecifiedTextStatic[STANDARD_BUFFER_SIZE];
        char* pUserSpecifiedText = userSpecifiedTextStatic;
        int currentBufferSize = sizeof(userSpecifiedTextStatic);
        do
        {
            // In Linux, if the buffer is not big enough, it can still cause a segmentation fault by
            // (internally) running strlen() on a non-null-terminated string, so make sure there is
            // always a null terminator.
            pUserSpecifiedText[currentBufferSize - 1] = '\0';
            pUserSpecifiedText[currentBufferSize - 2] = '\0';
#ifndef WIN32
            // vsnprintf will render the va_list unusable so clone before each call
            va_copy(argPtrCopy, argptr); // Clone arguments for reuse by different call to vsnprintf.
#endif
            VSNPRINTF(pUserSpecifiedText, currentBufferSize, format, argPtrCopy);
#ifndef WIN32
            va_end(argPtrCopy); // Cleanup clone of argptr.
#endif

            if (pUserSpecifiedText[currentBufferSize - 2] != '\0')
            {
                // The text has overflowed, so make the buffer bigger and try again
                currentBufferSize *= 2;
                if (pUserSpecifiedText == userSpecifiedTextStatic)
                {
                    pUserSpecifiedText = static_cast<char*>(malloc(currentBufferSize));
                }
                else
                {
                    pUserSpecifiedText = static_cast<char*>(realloc(pUserSpecifiedText, currentBufferSize));
                }
            }
            else
            {
                textGenerated = true;
            }
        } while (textGenerated == false);

        // Merge the text together and output it
        string finalText = prefixText;
        finalText += pUserSpecifiedText;

        if (pUserSpecifiedText != userSpecifiedTextStatic)
        {
            free(pUserSpecifiedText);
        }

        // Legacy code sometimes writes an empty string or single newline, so throw these away.
        // It's safe to use the static block because the dynamic one would only have been
        // created if the text was very long.
        if (((userSpecifiedTextStatic[1] == '\0' && (userSpecifiedTextStatic[0] == '\n' || userSpecifiedTextStatic[0] == '\r'))
            || userSpecifiedTextStatic[0] == '\0') == false)
        {
            if (mWritingToOutputPane)
            {
                // When running in the VisualStudio IDE, write to the output pane instead
                // of the requested stream.
                WriteDebugLineToOutputPane(finalText.c_str());
            }
            else
            {
                OutputMsg(level, finalText.c_str());
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

uint16 CDebugMsg::GetIndent()
{
    CriticalSection::Lock lock(GetSyncLock());
    DebugIndent& pDebugIndent = GetDebugIndent();
    if (pDebugIndent == NULL)
    {
        pDebugIndent = new uint16(0);
    }
    return *pDebugIndent;
}

/////////////////////////////////////////////////////////////////////////////

uint16 CDebugMsg::SetIndent(uint16 aNewIndent)
{
    CriticalSection::Lock lock(GetSyncLock());
    DebugIndent& pDebugIndent = GetDebugIndent();
    if (pDebugIndent == NULL)
    {
        pDebugIndent = new uint16(0);
    }
    *pDebugIndent = aNewIndent;
    return *pDebugIndent;
}

/////////////////////////////////////////////////////////////////////////////

uint16 CDebugMsg::DecrementIndent()
{
    uint16 indent = GetIndent();
    if (indent > 1)
    {
        indent = SetIndent(indent - 2);
    }
    return indent;
}

/////////////////////////////////////////////////////////////////////////////

uint16 CDebugMsg::IncrementIndent()
{
    uint16 previousValue = GetIndent();
    if (previousValue < 49)
    {
        SetIndent(previousValue + 2);
    }
    return previousValue;
}

/////////////////////////////////////////////////////////////////////////////

void CDebugMsg::LevelEnable(uint32 level, bool enable)
{
    CMsgQueue::LevelEnable(level, enable);

    if ((level == DEBUG_ENTRY_EXIT || level == ALL_LEVELS) && enable == false)
    {
        SetIndent(0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CDebugMsg::SendDebugToOutputPane()
{
    mWritingToOutputPane = true;
}

/////////////////////////////////////////////////////////////////////////////

void CDebugMsg::WriteDebugLineToOutputPane(const char* aLine)
{
    assert(aLine);
    CriticalSection::Lock lock(GetSyncLock());

#ifdef WIN32
#ifdef UNICODE
    // Convert single-byte chars to wide (which can be tidied if and when the
    // EngineFramework is converted to use istring).
    string nstr(aLine);
    wstring wstr(nstr.length(), L' ');
    copy(nstr.begin(), nstr.end(), wstr.begin());
    OutputDebugString((LPCWSTR)wstr.c_str());
    OutputDebugString(L"\n");
#else
    OutputDebugString(aLine);
    OutputDebugString("\n");
#endif
#else
    // Only supported on Windows
    assert(false);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CDebugMsg::OutputMsg(uint32 level, const char* text)
{
    if (mpStream && IsLevelEnabled(level))
    {
        CriticalSection::Lock lock(GetSyncLock());

        *mpStream << text << endl;
    }
}

/////////////////////////////////////////////////////////////////////////////
//                             CErrorMessages
/////////////////////////////////////////////////////////////////////////////

CErrorMessages::CErrorMessages()
{
    Clear();
}

/////////////////////////////////////////////////////////////////////////////

CErrorMessages::~CErrorMessages()
{
}

/////////////////////////////////////////////////////////////////////////////

bool CErrorMessages::IsSet()
{
    CriticalSection::Lock lock(GetSyncLock());

    return (mData.find(ThreadID::Id()) != mData.end());
}

/////////////////////////////////////////////////////////////////////////////

void CErrorMessages::Clear()
{
    CriticalSection::Lock lock(GetSyncLock());

    mData.erase(ThreadID::Id());
}

/////////////////////////////////////////////////////////////////////////////

void CErrorMessages::Set(int16 aErrorCode, const std::string& aErrorText, uint32 aErrorSequencePos)
{
    CriticalSection::Lock lock(GetSyncLock());

    const unsigned long currentThreadId = ThreadID::Id();

    mData[currentThreadId].mCode = aErrorCode;
    mData[currentThreadId].mText = aErrorText;
    mData[currentThreadId].mSequencePos = aErrorSequencePos;
}

/////////////////////////////////////////////////////////////////////////////

bool CErrorMessages::Get(int16& aErrorCode, std::string& aErrorText, uint32& aErrorSequencePos, unsigned long& aThreadId)
{
    bool retVal = false;
    CriticalSection::Lock lock(GetSyncLock());

    map<unsigned long, CErrorMsgData>::const_iterator threadDataIt = mData.lower_bound(aThreadId);
    if (threadDataIt != mData.end())
    {
        aErrorCode = threadDataIt->second.mCode;
        aErrorText = threadDataIt->second.mText;
        aErrorSequencePos = threadDataIt->second.mSequencePos;
        aThreadId = threadDataIt->first;
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//                             CProgressMsg
/////////////////////////////////////////////////////////////////////////////

CProgressMsg::CProgressMsg() :
    mSilentMode(false),
    mCurrent(0)
{
}

/////////////////////////////////////////////////////////////////////////////

CProgressMsg::~CProgressMsg()
{
}

/////////////////////////////////////////////////////////////////////////////

void CProgressMsg::Set(uint16 value)
{
    if (value <= 100)
    {
        mCurrent = value;

        if (mSilentMode == false)
        {
            Notify(mCurrent);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CProgressMsg::SetSilentMode(bool aSilentMode)
{
    mSilentMode = aSilentMode;
}

/////////////////////////////////////////////////////////////////////////////

// increment the progress by 1 (up to 100)
CProgressMsg& CProgressMsg::operator++(int)
{
    return (*this += 1);
}

/////////////////////////////////////////////////////////////////////////////

// increment the progress by set amount(up to 100)
CProgressMsg& CProgressMsg::operator+=(int inc)
{
    // increment up to (but not over) 100
    if (mCurrent < 100 - inc)
    {
        mCurrent += inc;
    }
    else
    {
        mCurrent = 100;
    }
    Notify(mCurrent);

    return *this;
};

/////////////////////////////////////////////////////////////////////////////

void CProgressMsg::Notify(uint16 value)
{
}

/////////////////////////////////////////////////////////////////////////////
//                             CConsoleProgressMsg
/////////////////////////////////////////////////////////////////////////////

CConsoleProgressMsg::CConsoleProgressMsg()
{
}

/////////////////////////////////////////////////////////////////////////////

CConsoleProgressMsg::~CConsoleProgressMsg()
{
}

/////////////////////////////////////////////////////////////////////////////

void CConsoleProgressMsg::Notify(uint16 value)
{
    if (ISATTY(FILENO(stdout)) == 0)
    {
        static StopWatch timer;
        static uint32 timeLastDotPrinted = 0; // A non-zero value means a dot was printed last
        static const uint16 DOT_PRINT_INTERVAL_MS = 2000;
        uint32 timeNow = timer.duration();

        if (value >= 100)
        {
            if (timeLastDotPrinted != 0)
            {
                printf("\n");
                timeLastDotPrinted = 0;
            }
        }
        else if ((timeNow - timeLastDotPrinted) > DOT_PRINT_INTERVAL_MS)
        {
            printf(".");
            fflush(stdout);
            timeLastDotPrinted = timeNow;
        }
    }
    else
    {
        if (value < 100)
        {
            printf("\r%3u%%\r", value);
            fflush(stdout);
        }
        else
        {
            printf("100%%\n");
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//                             CConsoleStatusMsg
/////////////////////////////////////////////////////////////////////////////

CConsoleStatusMsg::CConsoleStatusMsg() :
    CStatusMsg(&cout)
{
}

/////////////////////////////////////////////////////////////////////////////

CConsoleStatusMsg::~CConsoleStatusMsg()
{
}

/////////////////////////////////////////////////////////////////////////////

void CConsoleStatusMsg::OutputMsg(uint32 level, const char* text)
{
    CriticalSection::Lock lock(GetSyncLock());

    if (mpStream && IsLevelEnabled(level))
    {
        if (level == STATUS_WARNING)
        {
            *mpStream << CONSOLE_BANNER << endl << "WARNING: " << text << endl << CONSOLE_BANNER << endl;
        }
        else if (level == STATUS_ERROR)
        {
            *mpStream << CONSOLE_BANNER << endl << "ERROR: " << text << endl << CONSOLE_BANNER << endl;
        }
        else
        {
            *mpStream << text << endl;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//                             CMessageHandler
/////////////////////////////////////////////////////////////////////////////

CMultiListParserInsideEF& CMessageHandler::GetDebugParser()
{
    static CMultiListParserInsideEF* spMlParser = new CMultiListParserInsideEF();
    return *spMlParser;
}

/////////////////////////////////////////////////////////////////////////////

CMessageHandler::CMessageHandler(GroupEnum aGroupName) :
    mpProgressMsg(NULL),
    mpDebugMsg(NULL),
    mpStatusMsg(NULL),
    mpErrorMsgs(NULL),
    mAutomaticallyDisplayErrorMessages(false),
    mGroupName(aGroupName),
    mParagraphString(DEFAULT_PARAGRAPH_STRING)
{
}

/////////////////////////////////////////////////////////////////////////////

// Needed only to make the DLL backwards compatible...
CMessageHandler::CMessageHandler() :
    mpProgressMsg(NULL),
    mpDebugMsg(NULL),
    mpStatusMsg(NULL),
    mpErrorMsgs(NULL),
    mAutomaticallyDisplayErrorMessages(false),
    mGroupName(GROUP_ENUM_RSVD_LOCAL),
    mParagraphString(DEFAULT_PARAGRAPH_STRING)
{
}

/////////////////////////////////////////////////////////////////////////////

CMessageHandler::~CMessageHandler()
{
    Shutdown();
}

/////////////////////////////////////////////////////////////////////////////

string CMessageHandler::FindClassName(const string& aFnName)
{
    string retVal = aFnName;

    string::size_type locationOfLastColon = retVal.find_last_of(':');
    if (locationOfLastColon == string::npos)
    {
        // If the string passed in was not in the C++ "Class::Method" or
        // "Namespace::Class::Method" format, (i.e. it was a straight
        // function name), clear the whole text.
        locationOfLastColon = 0;
    }
    else
    {
        // If there were any colon characters in the string, there must have
        // been pairs of them, so consume the other one of the pair.
        // (Just check here to make sure there were two of them...)
        assert(locationOfLastColon > 0);
        --locationOfLastColon;
        assert(retVal.at(locationOfLastColon) == ':');
    }
    retVal = retVal.substr(0, locationOfLastColon);

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

string CMessageHandler::DetermineQualifiedFnName(
    const char* aDecoratedFnName, bool aStripTemplateParameters)
{
    string retString(aDecoratedFnName);

#ifndef WIN32
    // Strip off the parameter list and return type from the function name...

    // ...find the first '(' and remove everything from that point onwards
    size_t firstOpenBracket = retString.find_first_of("(");
    if (firstOpenBracket != string::npos)
    {
        retString = retString.substr(0, firstOpenBracket);
    }

    // ...find the last space and remove everything up to and including that point
    size_t lastSpace = retString.find_last_of(" ");
    if (lastSpace != string::npos)
    {
        retString = retString.substr(lastSpace + 1);
    }
#endif

    if (aStripTemplateParameters)
    {
        // To cope with template definitions, remove any text within '<' and '>' (inclusive)
        size_t firstLessThanBracket = retString.find_first_of("<");
        size_t lastGreaterThanBracket = retString.find_last_of(">");
        if (firstLessThanBracket != string::npos)
        {
            assert(firstLessThanBracket < lastGreaterThanBracket);
            retString.erase(firstLessThanBracket, lastGreaterThanBracket - firstLessThanBracket + 1);
        }
    }

    return retString;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::Initialise()
{
    // Nothing to be done...
    // The CTheMsgHnd class looks after whether the Engine Framework has been initialised.
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::Shutdown()
{
    delete mpProgressMsg;
    mpProgressMsg = NULL;
    delete mpDebugMsg;
    mpDebugMsg = NULL;
    delete mpStatusMsg;
    mpStatusMsg = NULL;
    delete mpErrorMsgs;
    mpErrorMsgs = NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::NotifyProgress(uint16 aValue)
{
    if (mpProgressMsg)
    {
        mpProgressMsg->Set(aValue);
        Notify(MESSAGE_TYPE_PROGRESS);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::NotifyStatus(StatusLevels aLevel, const string& aText)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aLevel, STATUS_ALL);

    if (mpStatusMsg)
    {
        mpStatusMsg->OutputMsg(aLevel, aText.c_str());

        if (CTheMsgHnd::DebugActivated())
        {
            // Write this information (that a status message has been written)
            // to the debug log...
            ASSERT_IF_VALUE_OUT_OF_RANGE(aLevel, STATUS_ALL - 1);
            const char* statusLevelTexts[STATUS_ALL];
            statusLevelTexts[STATUS_INFO]      = "INFO";
            statusLevelTexts[STATUS_ESSENTIAL] = "ESSENTIAL";
            statusLevelTexts[STATUS_WARNING]   = "WARNING";
            statusLevelTexts[STATUS_ERROR]     = "ERROR";
            string groupName = GetDecoratedGroupName();
            DebugOutputInHandler(DEBUG_ENHANCED, false, false, "<n/a>", groupName.c_str(), 0,
                "%s STATUS: %s", (aLevel < STATUS_ALL ? statusLevelTexts[aLevel] : "INVALID"), aText.c_str());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::SetDebugLevel(DebugLevels aLevel, bool aEnable, GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aLevel, DEBUG_COUNT_DO_NOT_USE_THIS - 1);

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
        {
            // Perform the operation
            if (MEMBER_VARS->mpDebugMsg)
            {
                if (aLevel == DEBUG_ALL)
                {
                    MEMBER_VARS->mpDebugMsg->LevelEnable(ALL_LEVELS, aEnable);
                }
                else
                {
                    MEMBER_VARS->mpDebugMsg->LevelEnable(aLevel, aEnable);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::SetStatusLevel(StatusLevels aLevel, bool aEnable, GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aLevel, STATUS_COUNT_DO_NOT_USE_THIS - 1);

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
        {
            // Perform the operation
            if (MEMBER_VARS->mpStatusMsg)
            {
                if (aLevel == STATUS_ALL)
                {
                    MEMBER_VARS->mpStatusMsg->LevelEnable(ALL_LEVELS, aEnable);
                }
                else
                {
                    MEMBER_VARS->mpStatusMsg->LevelEnable(aLevel, aEnable);
                }
            }

            // Also turn off the notification of progress if information level messages are not required
            if (MEMBER_VARS->mpProgressMsg && (aLevel == STATUS_INFO || aLevel == STATUS_ALL))
            {
                MEMBER_VARS->mpProgressMsg->SetSilentMode(aEnable == false);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool CMessageHandler::SetErrorMsg(int16 aErrorCode, const string& aText)
{
    bool retVal = false;

    if (mpErrorMsgs)
    {
        const uint32 index = CTheMsgHnd::Instance().mpErrorIndex->inc();
        mpErrorMsgs->Set(aErrorCode, aText, index);
        retVal = true;

        // Work out if any of the error logging situations apply...
        static const int LOG_ERROR_AS_STATUS_MSG = 1;
        static const int LOG_ERROR_AS_DEBUG_MSG  = 2;
        int logError = 0;

        logError += (mpStatusMsg && mAutomaticallyDisplayErrorMessages) ? LOG_ERROR_AS_STATUS_MSG : 0;
        logError += CTheMsgHnd::DebugActivated() ? LOG_ERROR_AS_DEBUG_MSG  : 0;

        if (logError > 0)
        {
            ostringstream stream;
            stream << aText;

            if (aErrorCode != 0)
            {
                stream << " (Error Code: " << aErrorCode << ")";
            }

            if ((logError & LOG_ERROR_AS_STATUS_MSG) > 0)
            {
                mpStatusMsg->OutputMsg(STATUS_ERROR, stream.str().c_str());
            }

            if ((logError & LOG_ERROR_AS_DEBUG_MSG) > 0)
            {
                // Write this information (that an error message has been set)
                // to the debug log...
                string groupName = GetDecoratedGroupName();
                DebugOutputInHandler(DEBUG_ENHANCED, false, false, "<n/a>", groupName.c_str(), 0,
                    "ERROR SET: Code=%d (0x%X); Text=%s", aErrorCode, aErrorCode, aText.c_str());
            }
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::ClearError(GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
        {
            if (MEMBER_VARS->mpErrorMsgs)
            {
                MEMBER_VARS->mpErrorMsgs->Clear();
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool CMessageHandler::ModifyErrorMsg(int16 aErrorCode, const string& aText,
    GroupEnum aGroupName, bool aCurrentThreadOnly)
{
    bool retVal = false;

    if (mpErrorMsgs)
    {
        string newMessage = aText;
        int16  lastErrorCode = aErrorCode;
        string lastErrorText;

        if (LastError(lastErrorCode, lastErrorText, aGroupName, aCurrentThreadOnly))
        {
            if (aErrorCode != 0)
            {
                lastErrorCode = aErrorCode;
            }

            if (newMessage.empty() == false)
            {
                newMessage.append(" - ");
            }
            newMessage.append(lastErrorText);
        }

        retVal = SetErrorMsg(lastErrorCode, newMessage);
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

bool CMessageHandler::SetErrorAutomaticDisplay(bool aNewAutomaticDisplay, GroupEnum aGroupName)
{
    bool retVal = true;

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
        {
            retVal = MEMBER_VARS->mAutomaticallyDisplayErrorMessages;
            MEMBER_VARS->mAutomaticallyDisplayErrorMessages = aNewAutomaticDisplay;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

bool CMessageHandler::LastError(int16& aErrorCode, string& aText, GroupEnum& aGroupName, bool aCurrentThreadOnly)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    bool errorFound = false;
    GroupEnum copyOfGroupName = aGroupName;
    uint32 latestSequencePos = 0;
    int16 tempCode;
    string tempText;
    uint32 tempSequencePos;
    const unsigned long currentThreadId = ThreadID::Id();

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        unsigned long foundThreadId = (aCurrentThreadOnly ? currentThreadId : 0);

        if (IsHandlerToBeActioned(copyOfGroupName, localGroup, msgHndIt->first))
        {
            CMessageHandler* theMessageHandlerForThisGroup = MEMBER_VARS;

            if (theMessageHandlerForThisGroup->mpErrorMsgs)
            {
                while (theMessageHandlerForThisGroup->mpErrorMsgs->Get(tempCode, tempText, tempSequencePos, foundThreadId))
                {
                    if (  (tempSequencePos > latestSequencePos)
                        && (aCurrentThreadOnly ? foundThreadId == currentThreadId : true)
                        )
                    {
                        // Copy the details of this error retrieved over to the output
                        // variables because this is is most appropriate so far.
                        aErrorCode = tempCode;
                        aText      = tempText;
                        aGroupName = msgHndIt->first;

                        latestSequencePos = tempSequencePos;
                        errorFound = true;
                    }

                    // Increment the thread ID to look for potential errors from other threads
                    ++foundThreadId;
                }
            }
        }
    }

#ifdef DONT_SILENTLY_IGNORE_GROUP_PROBLEMS
    // Never reset this flag; always report an error and expect the developer to try
    // the application in the debugger and resolve the resultant assert failure...
    if (gUnGroupedErrorHasOccurred)
    {
        errorFound = true;
        aErrorCode = 0;
        aText = "Internal Error: An \'ungrouped\' error has occurred prior to this point...";
        aGroupName = GROUP_ENUM_RSVD_ALL;

        DebugOutputInHandler(DEBUG_ENHANCED, false, false, "<n/a>", "<ungrouped>", 0, aText.c_str());
        printf("%s\n", aText.c_str());
    }
#endif

    return errorFound;
}

/////////////////////////////////////////////////////////////////////////////

bool CMessageHandler::IsErrorSet(GroupEnum aGroupName, bool aCurrentThreadOnly)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    // Share the code to get the last error and throw away the details.
    int16 tempCode;
    string tempText;
    return LastError(tempCode, tempText, aGroupName, aCurrentThreadOnly);
}

/////////////////////////////////////////////////////////////////////////////

bool CMessageHandler::IsDebugLevelEnabled(DebugLevels aLevel)
{
    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (MEMBER_VARS->mpDebugMsg)
        {
            if (MEMBER_VARS->mpDebugMsg->IsLevelEnabled(aLevel))
            {
                // This level is enabled 'somewhere'
                return true;
            }
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::DebugOutputInHandler(uint32 aLevel, bool aEntry, bool aExit, const char* aFunction,
    const char* aFilename, uint32 aLineNum, const char* aFormat, ...)
{
    if (mpDebugMsg)
    {
        va_list argptr;
        va_start(argptr, aFormat);

        mpDebugMsg->DebugOutputWithList(aLevel, aEntry, aExit, aFunction, aFilename, aLineNum, aFormat, argptr);

        va_end(argptr);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::DebugOutputBuffer(
    uint32 aLevel, const char* aFunction, const char* aFilename,
    uint32 aLineNum, const char* aDescription, uint8* apBuffer,
    uint32 aLength)
{
    if (mpDebugMsg)
    {
        // Build up the buffer in one long string
        char tempBuffer[20]; // Sufficient to store MAX_INT plus a sign plus parenthesis
        string tempString;
        for (uint32 i=0; i<aLength; i++)
        {
            if ((i % 4) == 0)
            {
                SNPRINTF(tempBuffer, sizeof(tempBuffer), "(%lx) ", i);
                tempString += tempBuffer;
            }
            SNPRINTF(tempBuffer, sizeof(tempBuffer), "%02x ", apBuffer[i]);
            tempString += tempBuffer;
        }
        DebugOutputInHandler(aLevel, false, false, aFunction, aFilename,
            aLineNum, "%s [length=0x%x] %s", aDescription, aLength, tempString.c_str());
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::DebugOutputProfile(uint32 aProfileNumber)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aProfileNumber, MAX_VALUE_FOR_PROFILE_NUMBER);

    if (mpDebugMsg)
    {
        DebugOutputInHandler(DEBUG_PROFILE, false, false, "", "(PROFILING)",
            mGroupName, "(%lx)", aProfileNumber);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::SendDebugOutputToTheConsole()
{
    // Debug
    NewDebugObject(&cerr, GROUP_ENUM_RSVD_ALL);
#ifdef WIN32
    if (IsDebuggerPresent() != FALSE)
    {
        // Running in the VisualStudio IDE
        FOR_LOOP_ROUND_ALL_HANDLERS
        {
            if (MEMBER_VARS->mpDebugMsg)
            {
                MEMBER_VARS->mpDebugMsg->SendDebugToOutputPane();
            }
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::SendAllNonDebugOutputToTheConsole()
{
    // Error
    NewErrorObject(true, GROUP_ENUM_RSVD_ALL);

    // Status - (Allow the macro to define a local variable multiple times)
    {
        FOR_LOOP_ROUND_ALL_HANDLERS
        {
            delete MEMBER_VARS->mpStatusMsg;
            MEMBER_VARS->mpStatusMsg = new CConsoleStatusMsg();
        }
    }

    // Progress - (Allow the macro to define a local variable multiple times)
    {
        FOR_LOOP_ROUND_ALL_HANDLERS
        {
            delete MEMBER_VARS->mpProgressMsg;
            MEMBER_VARS->mpProgressMsg = new CConsoleProgressMsg();
        }
    }

    // Set a suitable paragraph mark
    SetParagraphString("\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::AddObserver(CMessageHandlerObserver& aObserver, GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    // Create all the handlers
    CTheMsgHnd::Instance().CreateGroup(GROUP_ENUM_RSVD_ALL);

    // Debug - (Allow the macro to define a local variable multiple times)
    {
        FOR_LOOP_ROUND_ALL_HANDLERS
        {
            if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
            {
                CDebugMsg* pOldDebugMsg = MEMBER_VARS->mpDebugMsg;
                MEMBER_VARS->mpDebugMsg = new CMessageObserver<CDebugMsg>(aObserver);
                MEMBER_VARS->mpDebugMsg->InheritLevels(pOldDebugMsg);
                delete pOldDebugMsg;
            }
        }

        CTheMsgHnd::mDebugActivated = true;
    }

    // Status - (Allow the macro to define a local variable multiple times)
    {
        FOR_LOOP_ROUND_ALL_HANDLERS
        {
            if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
            {
                CStatusMsg* pOldStatusMsg = MEMBER_VARS->mpStatusMsg;
                MEMBER_VARS->mpStatusMsg = new CMessageObserver<CStatusMsg>(aObserver);
                MEMBER_VARS->mpStatusMsg->InheritLevels(pOldStatusMsg);
                delete pOldStatusMsg;
            }
        }
    }

    // Progress - (Allow the macro to define a local variable multiple times)
    {
        FOR_LOOP_ROUND_ALL_HANDLERS
        {
            if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
            {
                delete MEMBER_VARS->mpProgressMsg;
                MEMBER_VARS->mpProgressMsg = new CProgressObserver(aObserver);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::NewDebugObject(ostream* apStream, GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (IsHandlerToBeActioned(aGroupName, localGroup, msgHndIt->first))
        {
            // Perform the operation
            CDebugMsg* pOldDebugMsg = MEMBER_VARS->mpDebugMsg;
            MEMBER_VARS->mpDebugMsg = new CDebugMsg(apStream);
            MEMBER_VARS->mpDebugMsg->InheritLevels(pOldDebugMsg);
            delete pOldDebugMsg;
        }
    }
    
    CTheMsgHnd::mDebugActivated = true;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::NewStatusObject(ostream* aStream, GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        CStatusMsg* pOldStatusMsg = mpStatusMsg;
        mpStatusMsg = new CStatusMsg(aStream);
        mpStatusMsg->InheritLevels(pOldStatusMsg);
        delete pOldStatusMsg;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::NewErrorObject(bool automaticDisplay, GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        delete MEMBER_VARS->mpErrorMsgs;
        MEMBER_VARS->mpErrorMsgs = new CErrorMessages;
        MEMBER_VARS->mAutomaticallyDisplayErrorMessages = automaticDisplay;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::NewProgressObject(GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, GROUP_ENUM_RSVD_LOCAL);

    CTheMsgHnd::Instance().CreateGroup(aGroupName);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        delete MEMBER_VARS->mpProgressMsg;
        MEMBER_VARS->mpProgressMsg = new CProgressMsg;
    }
}

/////////////////////////////////////////////////////////////////////////////

const string CMessageHandler::GetParagraphString() const
{
    return mParagraphString;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::SetParagraphString(const string& aNewParagraphString)
{
    // The paragraph mark applies to all handlers in the system
    CTheMsgHnd::Instance().CreateGroup(GROUP_ENUM_RSVD_ALL);

    FOR_LOOP_ROUND_ALL_HANDLERS
    {
        if (aNewParagraphString.empty())
        {
            MEMBER_VARS->mParagraphString = DEFAULT_PARAGRAPH_STRING;
        }
        else
        {
            MEMBER_VARS->mParagraphString = aNewParagraphString;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

string CMessageHandler::GetDecoratedGroupName()
{
    string retVal = "<unknown>";
    GroupEnum localGroup = CTheMsgHnd::Instance().GetLocalGroup(this);
    bool enumFound = false;
    int groupNumber = sizeof(EF_GROUP_INFO) / sizeof(EF_GROUP_INFO[0]);

    while (--groupNumber >= 0 && enumFound == false)
    {
        if (localGroup == EF_GROUP_INFO[groupNumber].group)
        {
            retVal  = "<";
            retVal += EF_GROUP_INFO[groupNumber].name;
            retVal += ">";
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//                          CEfAutoErrorDisable
/////////////////////////////////////////////////////////////////////////////
CEfAutoErrorDisable::CEfAutoErrorDisable(CMessageHandler& aMsgHandler,
    bool aWarn, CMessageHandler::GroupEnum aGroupName)
  : mMsgHandler(aMsgHandler),
    mGroupName(aGroupName),
    mWarn(aWarn),
    mErrCodeStart(0),
    mErrAlreadyPresent(false),
    mErrWasOn(false)
{
    // Save any existing error details
    mMsgHandler.LastError(mErrCodeStart, mErrTextStart, mGroupName);
    mErrAlreadyPresent = !mErrTextStart.empty();
    
    // Disable auto-reporting
    mErrWasOn = mMsgHandler.SetErrorAutomaticDisplay(false, mGroupName);
}

CEfAutoErrorDisable::~CEfAutoErrorDisable()
{
    // Reinstate auto-reporting
    mMsgHandler.SetErrorAutomaticDisplay(mErrWasOn, mGroupName);
    
    // Get error details
    int16 errCodeEnd = 0;
    string errTextEnd;
    mMsgHandler.LastError(errCodeEnd, errTextEnd, mGroupName);
    
    if ((mErrCodeStart != errCodeEnd) || (mErrTextStart != errTextEnd))
    {   
        // New error was raised, did we have an older one?
        if (mErrAlreadyPresent)
        {
            // Only need to show the error if logging was on
            if (mErrWasOn && !errTextEnd.empty())
            {
                // Ok to let the new error be seen as an error (because there is already an error report)
                mMsgHandler.ModifyErrorMsg(errCodeEnd, "", mGroupName);
            }
        }
        else
        {
            // Remove the new error, and report as a non-error diagnostic
            mMsgHandler.ClearError(mGroupName);
            if (!errTextEnd.empty())
            {
                if (mWarn)
                {
                    errTextEnd.insert(0, "Ignored: ");
                    mMsgHandler.NotifyStatus(STATUS_WARNING, errTextEnd);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//                        CDebugSentry
/////////////////////////////////////////////////////////////////////////////

CDebugSentry::CDebugSentry(CMessageHandler* msgHandlerPtr, uint32 aLineNum,
    const char* aFileName, const char* aFunctionName) :
    mpMsgHandlerPtr(msgHandlerPtr),
    mLineNum(aLineNum),
    mExitPrinted(false),
    mFileName(aFileName)
{
    assert(msgHandlerPtr);

    mFunctionName = mpMsgHandlerPtr->DetermineQualifiedFnName(aFunctionName, false);

    if (CTheMsgHnd::DebugActivated())
    {
        mpMsgHandlerPtr->DebugOutputInHandler(DEBUG_ENTRY_EXIT, true, false,
            aFunctionName, aFileName, aLineNum, "-> %s", mFunctionName.c_str());
        mpMsgHandlerPtr->DebugOutputProfile(0);
    }
}

/////////////////////////////////////////////////////////////////////////////

CDebugSentry::~CDebugSentry()
{
    CheckPrintExit();
}

/////////////////////////////////////////////////////////////////////////////

void CDebugSentry::CheckPrintExit()
{
    if (CTheMsgHnd::DebugActivated() && mExitPrinted == false)
    {
        if (false
#ifndef _WIN32_WCE
            || std::uncaught_exception()
#endif
            )
        {
            DO_PRINT_EXIT(" !!! EXCEPTION !!!", 0);
        }
        else
        {
            PrintExit();
        }
        mExitPrinted = true; // make sure we don't print again.
    }
}

/////////////////////////////////////////////////////////////////////////////

void CDebugSentry::PrintExit()
{
    DO_PRINT_EXIT("", 0);
}

/////////////////////////////////////////////////////////////////////////////
//                             CTheMsgHnd
/////////////////////////////////////////////////////////////////////////////

bool CTheMsgHnd::mDebugActivated = false;
CriticalSection* CTheMsgHnd::mpSynchroniseLock = NULL;
CTheMsgHnd *gpTheMsgHnd = NULL;
CMessageHandler *gpDummyHandler = NULL;

/////////////////////////////////////////////////////////////////////////////

CTheMsgHnd::CTheMsgHnd()
{
    // This is the index to be used for the *NEXT* error to occur, so initialise to 1
    // (because 0 is used elsewhere to mean "an error for the combination of group and
    // thread requested has not occurred).
    mpErrorIndex = new AtomicCounter(1);
    mpSynchroniseLock = new CriticalSection();
}

/////////////////////////////////////////////////////////////////////////////

CTheMsgHnd::~CTheMsgHnd()
{
    delete mpErrorIndex, mpErrorIndex = NULL;
    delete mpSynchroniseLock, mpSynchroniseLock = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CTheMsgHnd& CTheMsgHnd::Instance()
{
    if (!gpTheMsgHnd)
    {
        gpTheMsgHnd = new CTheMsgHnd();
    }
    return *gpTheMsgHnd;
}

/////////////////////////////////////////////////////////////////////////////

CMessageHandler::GroupEnum CTheMsgHnd::GetLocalGroup(CMessageHandler* apThisMsgHandler)
{
    CMessageHandler::GroupEnum retVal = CMessageHandler::GROUP_ENUM_RSVD_LOCAL;
    CriticalSection::Lock lock(*mpSynchroniseLock);

    CTheMsgHnd::MsgHndMap& msgHandlers = GetMessageHandlers();
    for (MsgHndIter msgHndIt = msgHandlers.begin();
         msgHndIt != msgHandlers.end();
         ++msgHndIt)
    {
        if (apThisMsgHandler == msgHndIt->second)
        {
            retVal = msgHndIt->first;
            break;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

void CTheMsgHnd::CreateGroups(int aLowerIndex, int aUpperIndex)
{
    if (aLowerIndex == aUpperIndex)
    {
        CreateGroup(EF_GROUP_INFO[aLowerIndex].group);
    }
    else if (aLowerIndex < aUpperIndex)
    {
        int midIndex = ((aUpperIndex - aLowerIndex) / 2) + aLowerIndex;
        CreateGroups(midIndex, midIndex);
        CreateGroups(aLowerIndex, midIndex - 1);
        CreateGroups(midIndex + 1, aUpperIndex);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTheMsgHnd::CreateGroup(CMessageHandler::GroupEnum aGroupName)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, CMessageHandler::GROUP_ENUM_RSVD_LOCAL);

    // If the group has not been created yet (because there has been no
    // source code to assign itself to a group) when a request to set the
    // group data arrives, use this method to create a handler for the group
    // if one does not already exist.
    if (aGroupName == CMessageHandler::GROUP_ENUM_RSVD_ALL)
    {
        // In order to balance the map, use this method when creating all groups
        CreateGroups(0, (sizeof(EF_GROUP_INFO) / sizeof(EF_GROUP_INFO[0])) - 1);
    }
    else if (aGroupName != CMessageHandler::GROUP_ENUM_RSVD_LOCAL)
    {
        GetAppropriateHandler(aGroupName, "", true);
    }
}

/////////////////////////////////////////////////////////////////////////////

CMessageHandler& CTheMsgHnd::GetAppropriateHandler(CMessageHandler::GroupEnum aGroupName, const char* apFnName, bool aCreateLink)
{
    ASSERT_IF_VALUE_OUT_OF_RANGE(aGroupName, CMessageHandler::GROUP_ENUM_RSVD_LOCAL);

    // Because this method returns a reference, it must return something.
    // If it gets to the end of this function and has not reassigned the pRetVal variable,
    // one of the following will be true:
    // 1. The engine framework has not been initialised.
    // 2. The engine framework is shutting down because the initialise object has
    //    gone out of scope.
    // 3. A call other than MSG_HANDLER_ADD_TO_GROUP() has been placed in code
    //    outside the lifetime of main() - in other words, a static object constructor
    //    or destructor.
    // Because the process of "changing one instance of something to a static" should
    // *NOT* result in having to go round removing all references to the engine framework
    // (especially when other instances remain non-static), there should not be an assert
    // in this situation. However, it must still cope gracefully.
    // There was an edge case where the mpSynchroniseLock object had been destroyed before
    // a later call to the framework, so it must also cope with this situation.
    if (!gpDummyHandler)
    {
        gpDummyHandler = new CMessageHandler();
    }
    CMessageHandler* pRetVal = gpDummyHandler;

    CTheMsgHnd::MsgHndMap& msgHandlers = GetMessageHandlers();

    if (mpSynchroniseLock && gEngineFrameworkLifeCycleState != SHUTTING_DOWN)
    {
        CriticalSection::Lock lock(*mpSynchroniseLock);

        // The engine framework must have been initialised prior to use (by
        // creating an instance of the CEngineInitialise class that remains
        // in scope for the lifetime of the application).
        // However, the exception to this is from within inside static
        // constructors, where the class name is being added to a group.
        // Therefore, only assert if not creating a group.
        if (aCreateLink == false)
        {
            // If this assert is being hit, the initialisation has not taken place.
            // Refer to the documentation for details.
            assert(gEngineFrameworkLifeCycleState == INITIALISED);
        }

        // Look for the group name first
        MsgHndIter msgHndIt = msgHandlers.find(aGroupName);
        if (aCreateLink == false && msgHndIt != msgHandlers.end()
            && aGroupName != CMessageHandler::GROUP_ENUM_RSVD_ALL)
        {
            pRetVal = msgHndIt->second;
        }
        else
        {
            // Look for the class name instead
            string className = CMessageHandler::DetermineQualifiedFnName(apFnName, true);
            className = CMessageHandler::FindClassName(className);
            CTheMsgHnd::GroupsMap& groups = GetGroups();
            GroupsIter groupsIt = groups.find(className);
            if (groupsIt != groups.end())
            {
                // Make sure the association (of this class) has not already been made
                // with a different group name. If this assert fires, then this class
                // has already been associated with a different group.
                const bool CLASS_BEING_ASSOCIATED_WITH_UNIQUE_GROUP =
                    (aCreateLink == false || groupsIt->second == aGroupName);
                assert(CLASS_BEING_ASSOCIATED_WITH_UNIQUE_GROUP);

                msgHndIt = msgHandlers.find(groupsIt->second);
                if (msgHndIt != msgHandlers.end())
                {
                    pRetVal = msgHndIt->second;
                }
            }
            else
            {
                // There are some reserved group names that are not allowed to be used
                // at this point because they are reserved for passing to methods as the
                // optional group name.
                // It is not valid to assign a class name, or source code to these group
                // names and these asserts will trap accidental and erronous use of them.
                // If these asserts are failing, it MORE THAN LIKELY indicates a problem
                // with the application code not assigning code to groups correctly.
                assert(aGroupName != CMessageHandler::GROUP_ENUM_RSVD_ALL);
                assert(aGroupName != CMessageHandler::GROUP_ENUM_RSVD_LOCAL);

                if (aCreateLink && className.length() > 0)
                {
                    groups.insert(make_pair(className, aGroupName));
                }
                if (msgHndIt == msgHandlers.end())
                {
                    msgHandlers.insert(make_pair(aGroupName, new CMessageHandler(aGroupName)));

                    // Progress, Status and Debug all require the application to state how
                    // it wants the information to be displayed. Error is different - lower
                    // layers in the stack set and get the error expecting there to be an error
                    // object. Therefore, create an error object by default just in case
                    // the DLL is being used by a third-party application and the application
                    // has (therefore) not called the engine framework to set things up.
                    // This has to be done here (rather than in the CMessageHandler constructor)
                    // because the NewErrorObject method looks at mTheHandlers to work out which
                    // handler it is and during the construction, it hasn't been added yet.
                    if (gEngineFrameworkLifeCycleState != SHUTTING_DOWN)
                    {
                        msgHandlers[aGroupName]->NewErrorObject(false, CMessageHandler::GROUP_ENUM_RSVD_LOCAL);
                    }
                }
                msgHndIt = msgHandlers.find(aGroupName);

                // The entry has just been added, so it must be there...
                assert(msgHndIt != msgHandlers.end());
                pRetVal = msgHndIt->second;
            }
        }
    }

#ifdef DONT_SILENTLY_IGNORE_GROUP_PROBLEMS
    if (msgHandlers.find(CMessageHandler::GROUP_ENUM_RSVD_ALL)   != msgHandlers.end() ||
        msgHandlers.find(CMessageHandler::GROUP_ENUM_RSVD_LOCAL) != msgHandlers.end())
    {
        gUnGroupedErrorHasOccurred = true;
    }
#endif

    return *pRetVal;
}

/////////////////////////////////////////////////////////////////////////////
//                             CSubject
/////////////////////////////////////////////////////////////////////////////

void CSubject::Attach(CMessageHandlerObserver* o)
{
    observers.push_back(o);
}

/////////////////////////////////////////////////////////////////////////////

void CSubject::Detach(CMessageHandlerObserver* o)
{
    observers.remove(o);
}

/////////////////////////////////////////////////////////////////////////////

void CSubject::Notify(uint32 supplementaryNumber)
{
    list<CMessageHandlerObserver*>::iterator i;

    for (i = observers.begin(); i != observers.end(); ++i)
    {
        (*i)->NotifyNumber(0, CMessageHandler::MESSAGE_TYPE_PROGRESS, 0, supplementaryNumber);
    }
}

/////////////////////////////////////////////////////////////////////////////
//                             CProgressObserver
/////////////////////////////////////////////////////////////////////////////

CProgressObserver::CProgressObserver(CMessageHandlerObserver& observer) :
    mObserver(observer)
{
}

/////////////////////////////////////////////////////////////////////////////

CProgressObserver::~CProgressObserver()
{
}

/////////////////////////////////////////////////////////////////////////////

void CProgressObserver::Notify(uint16 value)
{
    mObserver.NotifyNumber(0, CMessageHandler::MESSAGE_TYPE_PROGRESS, 0, value);
}

/////////////////////////////////////////////////////////////////////////////

const string& CMessageHandler::GetDebugParserGuidance()
{
    CMultiListParserInsideEF& mlParser = GetDebugParser();
    string& parserGuidanceStr = GetDebugParserGuidanceStr();
    if (mlParser.GetLastError().empty() == false)
    {
        parserGuidanceStr = mlParser.GetLastError();
    }
    return parserGuidanceStr;
}

/////////////////////////////////////////////////////////////////////////////

void CMessageHandler::ProcessDebugRequest(const string& aReqLevels, bool aSendToConsole)
{
    CMultiListParserInsideEF& mlParser = GetDebugParser();

    mlParser.AddValidGroup(GRP_NONE_STR);
    mlParser.AddValidGroup(inarrow(DEBUG_VALUE_BASIC_STR));
    mlParser.AddValidGroup(inarrow(DEBUG_VALUE_ENHANCED_STR));
    mlParser.AddValidGroup(inarrow(DEBUG_VALUE_PROFILE_STR));
    mlParser.AddValidGroup(inarrow(DEBUG_VALUE_FULL_STR));
    mlParser.AddValidGroup("!" + inarrow(DEBUG_VALUE_PROFILE_STR));

    // Get the list of groups from the engine framework...
    int groupNumber = sizeof(EF_GROUP_INFO) / sizeof(EF_GROUP_INFO[0]);
    while (--groupNumber >= 0)
    {
        mlParser.AddValidElement(EF_GROUP_INFO[groupNumber].name);
    }

    // Add 'shortcuts' for common/popular combinations
    mlParser.AddSynonym(inarrow(DEBUG_VALUE_BASIC_STR),
        inarrow(DEBUG_VALUE_BASIC_STR) + ":all");
    mlParser.AddSynonym(inarrow(DEBUG_VALUE_ENHANCED_STR),
        inarrow(DEBUG_VALUE_ENHANCED_STR) + ":all," + inarrow(DEBUG_VALUE_BASIC_STR) + ":all");
    mlParser.AddSynonym(inarrow(DEBUG_VALUE_FULL_STR),
        inarrow(DEBUG_VALUE_FULL_STR) + ":all");
    mlParser.AddSynonym(inarrow(DEBUG_VALUE_STANDARD_STR),
        inarrow(DEBUG_VALUE_BASIC_STR) + ":all," + GRP_NONE_STR + ":cmdline");
    mlParser.AddSynonym(inarrow(DEBUG_VALUE_ALMOSTALL_STR),
        inarrow(DEBUG_VALUE_FULL_STR) + ":all-cmdline," + "!" + inarrow(DEBUG_VALUE_PROFILE_STR) + ":all");

    // These are only to support "legacy" parameters for the -debug option
    mlParser.AddSynonym("limited", inarrow(DEBUG_VALUE_BASIC_STR) + ":all");
    mlParser.AddSynonym("detailed",
        inarrow(DEBUG_VALUE_ENHANCED_STR) + ":all," + inarrow(DEBUG_VALUE_BASIC_STR) + ":all");

    std::deque<CMULTILISTPARSER::StringPairType> parserResults;

    bool ret = mlParser.ParseStatement(inarrow(aReqLevels).c_str(), parserResults);
    if (ret)
    {
        if (aSendToConsole)
        {
            SendDebugOutputToTheConsole();
        }

        while (parserResults.empty() == false)
        {
            CMULTILISTPARSER::StringPairType stringPair = parserResults.front();
            DebugLevels level;
            bool enable = true;

            // Take account of the "!" 'not' identifier for the group
            if (stringPair.first.empty() == false && stringPair.first.at(0) == '!')
            {
                enable = false;
                stringPair.first = stringPair.first.erase(0, 1);
            }

            if (STRICMP(stringPair.first.c_str(), GRP_NONE_STR) == 0)
            {
                level = DEBUG_ALL;
                enable = false;
            }
            else if (STRICMP(stringPair.first.c_str(), inarrow(DEBUG_VALUE_BASIC_STR).c_str())     == 0)
            {
                level = DEBUG_BASIC;
            }
            else if (STRICMP(stringPair.first.c_str(), inarrow(DEBUG_VALUE_ENHANCED_STR).c_str())  == 0)
            {
                level = DEBUG_ENHANCED;
            }
            else if (STRICMP(stringPair.first.c_str(), inarrow(DEBUG_VALUE_FULL_STR).c_str())      == 0)
            {
                level = DEBUG_ALL;
            }
            else if (STRICMP(stringPair.first.c_str(), inarrow(DEBUG_VALUE_PROFILE_STR).c_str())   == 0)
            {
                level = DEBUG_PROFILE;
            }
            else if (STRICMP(stringPair.first.c_str(), "")                                         == 0)
            {
                // In this case, the user has not specifed a group name (i.e. just elements)
                // and as such it is not possible to tell what the user wants to do with these
                // elements. The only course of action available (because this is during the
                // pre-parse of the command line arguments) is to ignore them and warn the user
                // accordingly.
                GetDebugParserGuidanceStr() = "Some of the debug commands are invalid and are being ignored; refer to the help for the correct syntax.";
                stringPair.second.clear();
            }
            else
            {
                // If this point is reached, a group name has been specified and has managed to
                // get through the (multilist) parse routine without it being detected as invalid.
                // Alternatively, a group has been added but there is no corresponding entry in this
                // "if" statement.
                assert(false);
            }

            while (stringPair.second.empty() == false)
            {
                const char* element = stringPair.second.back().c_str();

                // Find the enum equivalent of the string
                bool enumFound = false;
                int groupNumber = sizeof(EF_GROUP_INFO) / sizeof(EF_GROUP_INFO[0]);
                while (--groupNumber >= 0)
                {
                    if (STRICMP(element, EF_GROUP_INFO[groupNumber].name) == 0)
                    {
                        enumFound = true;
                        SetDebugLevel(level, enable, EF_GROUP_INFO[groupNumber].group);
                    }
                }

                stringPair.second.pop_back();

                if (enumFound == false)
                {
                    // This should not be possible because the list of possible values
                    // is populated from the same source...
                    assert(false);
                }
            }
            parserResults.pop_front();
        }
    }
}



//////////////////////////////////////////////////////////////////////////////
//              * * * * * * IMPORTANT NOTE * * * * * *                      //
//////////////////////////////////////////////////////////////////////////////
// This Startup/shutdown section must be the LAST thing in this file to     //
// ensure that it is run while the global data in this file is still valid. //
//                                                                          //
// (If any more global data is added, it must be added ealier in the file)  //
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//                 Engine Framework Startup and Shutdown
/////////////////////////////////////////////////////////////////////////////
//
// The startup and shutdown sequences are handled slightly differently on
// each OS. Originally, it was the responsibility of the application to
// perform this task, but for various reasons this has proved impractical.
//
// The operation of the engine framework is not guaranteed outside the
// lifetime of main(), but it is provided on a "best-can-do basis" outside
// this scope (i.e. during the construction and destruction of static objects).
//
// A static object is not used to control the startup and shutdown sequence
// because the order of execution for static objects is non-deterministic,
// so here are the rules for which calls can be made in STATIC object
// CONSTRUCTORS and DESTRUCTORS:
//
//    "MSG_HANDLER."
//    - Do not use this in these locations; they are liable to be ignored.
//
//    "MSG_HANDLER_ADD_TO_GROUP"
//    - These are fine (and often necessary), so do use them.
//
//    "MSG_HANDLER_NOTIFY_DEBUG" and "FUNCTION_DEBUG_SENTRY[_RET]"
//    - These are allowed, but don't be surprised if they are ignored.
//
/////////////////////////////////////////////////////////////////////////////

CEngineInitialise* gpAutomaticEngineInitialisation = NULL;

/// The startup function cannot be (within) a C++ class.
void EngineStartupFunction()
{
    gpAutomaticEngineInitialisation = new CEngineInitialise;
}

/// The shutdown function cannot be (within) a C++ class.
void EngineShutdownFunction()
{
    delete gpAutomaticEngineInitialisation;
    gpAutomaticEngineInitialisation = NULL;
}

#ifdef WIN32

#ifdef _MANAGED
#pragma managed(push, off)
#endif
BOOL WINAPI DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            // Do nothing
            break;

        case DLL_PROCESS_ATTACH:
            EngineStartupFunction();
            break;

        case DLL_PROCESS_DETACH:
            EngineShutdownFunction();
            break;
    }

    return TRUE;
}
#ifdef _MANAGED
#pragma managed(pop)
#endif

#else

// On Windows, the DLL_PROCESS_DETACH message is received BEFORE static
// objects are destructed. However, on Linux, the my_fini() function is called
// AFTER static objects are destructed. Therefore, if a static object is
// destroyed under Linux, then also call the shutdown procedure.
// Both methods are utilised under Linux, so that the shutdown sequence
// looks the same. Otherwise, when comparing logs of full debug, there is a
// discrepency at the end (when the 'global' destructors are called and it
// unecessarily distracts attention away from what the developer is looking for).

class CStaticClassObject
{
public:
    CStaticClassObject() { }
    ~CStaticClassObject() { EngineShutdownFunction(); }
};

static CStaticClassObject staticObjectForEngineInit;

void __attribute__((constructor)) my_init(void)
{
    EngineStartupFunction();
}

void __attribute__((destructor)) my_fini(void)
{
    EngineShutdownFunction();
}

#endif

/////////////////////////////////////////////////////////////////////////////
//                        CEngineInitialise
/////////////////////////////////////////////////////////////////////////////
ofstream * gpDebugStream = NULL;

CEngineInitialise::CEngineInitialise()
{
    if (gEngineFrameworkLifeCycleState != SHUTTING_DOWN)
    {
        // If the initialisation has been called more than once, this assert will hit.
        assert(gEngineFrameworkLifeCycleState == NOT_INITIALISED);

#if !defined(_WINCE) & !defined(_WIN32_WCE)
        // Look for the environment variables to determine how to process the HostToolsDebug tracing.
        char* envHtdGroups = getenv("HTDEBUG_GROUPS");
        if (envHtdGroups)
        {
            // Make sure all the groups have been created
            CTheMsgHnd::Instance().CreateGroup(CMessageHandler::GROUP_ENUM_RSVD_ALL);
            // It doesn't matter which group is chosen here, because it will make the changes for all groups.
            CTheMsgHnd::MsgHndMap& msgHandlers = CTheMsgHnd::Instance().GetMessageHandlers();
            msgHandlers[CMessageHandler::GROUP_ENUM_APPLICATION]->ProcessDebugRequest(envHtdGroups, true);

            // There is no point looking for the filename if the user has not specified what the levels should be
            char* envHtdFile = getenv("HTDEBUG_FILE");
            if (envHtdFile)
            {
                char* envHtdAppend = getenv("HTDEBUG_APPEND");
                gpDebugStream = new ofstream(envHtdFile, (envHtdAppend ? ios_base::app : ios_base::trunc));
                if (gpDebugStream->good())
                {
                    // It doesn't matter which group is chosen here, because it will make the changes for all groups.
                    msgHandlers[CMessageHandler::GROUP_ENUM_APPLICATION]->
                        NewDebugObject(gpDebugStream, CMessageHandler::GROUP_ENUM_RSVD_ALL);
                }
            }
        }
#endif

        gEngineFrameworkLifeCycleState = INITIALISED;
    }
}

/////////////////////////////////////////////////////////////////////////////

CEngineInitialise::~CEngineInitialise()
{
    gEngineFrameworkLifeCycleState = SHUTTING_DOWN;

    CTheMsgHnd::MsgHndMap& msgHandlers = CTheMsgHnd::Instance().GetMessageHandlers();
    for (CTheMsgHnd::MsgHndIter msgHndIt = msgHandlers.begin();
         msgHndIt != msgHandlers.end();
         ++msgHndIt)
    {
        delete msgHndIt->second;
        msgHndIt->second = NULL;
    }
    msgHandlers.clear();
    delete gpDebugStream;
}

//////////////////////////////////////////////////////////////////////////////
//              End of Engine Framework Startup and Shutdown                //
// NOTE: Do not add any more data/code below this point in this file        //
//////////////////////////////////////////////////////////////////////////////

