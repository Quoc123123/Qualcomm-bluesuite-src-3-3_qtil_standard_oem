/**********************************************************************
 *
 *  EngineFrameworkClr.cpp
 *
 *  Copyright (c) 2013-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 **********************************************************************/

#ifdef _MANAGED
    // Disable warnings about XML comments
#pragma warning(disable : 4635)
    // Disable warnings about __fastcall incompatible with /clr option
#pragma warning(disable : 4561)
#endif

//#include "EngineFrameworkClr.h" - see conditional code below

#include "..\..\..\marshal.h"
#include "cmdline\cmdline.h"
#include <assert.h>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Diagnostics;
using namespace System::Runtime::InteropServices;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

#if _MSC_VER >= 1500
// =======================================================
// ===================== VS2008 code =====================
// ================== (i.e. functional) ==================
// =======================================================
#define IS_DEBUG_ACTIVATED CTheMsgHnd::DebugActivated()
#include "EngineFrameworkClr.h"
#else
// =======================================================
// ===================== VS2005 code =====================
// ================= (i.e. stripped out) =================
// =======================================================
#define ENGINEFRAMEWORKCPP_EXPORTS
#include "EngineFrameworkClr.h"
#define IS_DEBUG_ACTIVATED true
#undef MSG_HANDLER_ADD_TO_GROUP
#define MSG_HANDLER_ADD_TO_GROUP(X, ...)
CMessageHandler::CMessageHandler(GroupEnum aGroupName) { }
CMessageHandler::CMessageHandler() { }
CMessageHandler::~CMessageHandler() { }
string CMessageHandler::FindClassName(const string& aFnName) { return ""; }
string CMessageHandler::DetermineQualifiedFnName(const char* aDecoratedFnName, bool aStripTemplateParameters) { return ""; }
void CMessageHandler::Initialise() { }
void CMessageHandler::Shutdown() { }
void CMessageHandler::NotifyProgress(uint16 aValue) { }
void CMessageHandler::NotifyStatus(StatusLevels aLevel, const string& aText) { }
void CMessageHandler::SetDebugLevel(DebugLevels aLevel, bool aEnable, GroupEnum aGroupName) { }
void CMessageHandler::SetStatusLevel(StatusLevels aLevel, bool aEnable, GroupEnum aGroupName) { }
bool CMessageHandler::SetErrorMsg(int16 aErrorCode, const string& aText) { return false; }
void CMessageHandler::ClearError(GroupEnum aGroupName) { }
bool CMessageHandler::ModifyErrorMsg(int16 aErrorCode, const string& aText, GroupEnum aGroupName) { return false; }
bool CMessageHandler::SetErrorAutomaticDisplay(bool aNewAutomaticDisplay, GroupEnum aGroupName) { return false; }
bool CMessageHandler::LastError(int16& aErrorCode, string& aText, GroupEnum& aGroupName, bool aCurrentThreadOnly) { return false; }
bool CMessageHandler::IsErrorSet(GroupEnum aGroupName, bool aCurrentThreadOnly) { return false; }
bool CMessageHandler::IsDebugLevelEnabled(DebugLevels aLevel) { return false; }
void CMessageHandler::DebugOutputInHandler(uint32 aLevel, bool aEntry, bool aExit, const char* aFunction,
                                           const char* aFilename, uint32 aLineNum, const char* aFormat, ...) { }
void CMessageHandler::DebugOutputBuffer(uint32 aLevel, const char* aFunction, const char* aFilename,
    uint32 aLineNum, const char* aDescription, uint8* apBuffer,
    uint32 aLength) { }
void CMessageHandler::DebugOutputProfile(uint32 aProfileNumber) { }
void CMessageHandler::SendDebugOutputToTheConsole() { }
void CMessageHandler::SendAllNonDebugOutputToTheConsole() { }
void CMessageHandler::AddObserver(CMessageHandlerObserver& aObserver, GroupEnum aGroupName) { }
void CMessageHandler::NewDebugObject(ostream* aStream, GroupEnum aGroupName) { }
void CMessageHandler::NewStatusObject(ostream* aStream, GroupEnum aGroupName) { }
void CMessageHandler::NewErrorObject(bool automaticDisplay, GroupEnum aGroupName) { }
void CMessageHandler::NewProgressObject(GroupEnum aGroupName) { }
const string CMessageHandler::GetParagraphString() const { return ""; }
void CMessageHandler::SetParagraphString(const string& aNewParagraphString) { }
string CMessageHandler::GetDecoratedGroupName() { return ""; }
CSubject::CSubject(void) { }
void CSubject::Notify(unsigned long) { }
void CSubject::Detach(class CMessageHandlerObserver *) { }
void CSubject::Attach(class CMessageHandlerObserver *) { }
CSubject::~CSubject(void) { }
CMessageHandlerObserver::CMessageHandlerObserver(void) { }
CMessageHandlerObserver::~CMessageHandlerObserver(void) { }
CEngineInitialise::~CEngineInitialise(void) { }
void CDebugSentry::PrintExit(void) { }
CDebugSentry::~CDebugSentry(void) { }
class CMessageHandler & CTheMsgHnd::GetAppropriateHandler(enum CMessageHandler::GroupEnum,char const *,bool) { static CMessageHandler x; return x; }
CTheMsgHnd::CTheMsgHnd() { }
CTheMsgHnd::~CTheMsgHnd() { }
class CTheMsgHnd & CTheMsgHnd::Instance(void) { static CTheMsgHnd x; return x; }
CDebugSentry::CDebugSentry(class CMessageHandler *,unsigned long,char const *,char const *) { }
void CDebugSentry::CheckPrintExit(void) { }
bool CTheMsgHnd::mDebugActivated;
#endif

using namespace QTIL::HostTools::Common::EngineFrameworkClr;

///////////////////////////////////////////////////////////////////////////

static MessageHandler::MessageHandler()
{
    // Since VS IDE fails to copy *all* referenced dlls i.e. in this case
    //  EngineFrameworkCpp et al to a location where it can find them in design mode
    //  e.g. C:\Users\<user>\AppData\Local\Microsoft\VisualStudio\9.0\ProjectAssemblies
    //  *this* dll has been copied but its dependencies haven't. Therefore don't even
    //  attempt to access EngineFrameworkCpp because otherwise the designed component
    //  will fail to load - typically occurs if using a Form with a UserControl which itself
    //  uses EngineFrameworkClr.
    //  See: http://social.msdn.microsoft.com/Forums/en-US/vclanguage/thread/692e5049-c86d-4a5b-95e8-747db1956dba, answer 1


    String^ EngineFrameworkCpp_Dll = "EngineFrameworkCpp.dll";

    // Need to extract EngineFrameworkCpp.dll if it doesn't exist
    String^ path = Assembly::GetExecutingAssembly()->Location;
    String^ destinationFileName = Path::Combine(Path::GetDirectoryName(path), EngineFrameworkCpp_Dll);

    if (!File::Exists(destinationFileName))
    {
        try
        {
           Stream^ readStream = Assembly::GetExecutingAssembly()->GetManifestResourceStream(EngineFrameworkCpp_Dll);
           if (readStream != nullptr)
           {
                FileStream^ writeStream = gcnew FileStream(destinationFileName, FileMode::Create);

                array<Byte>^ data = gcnew array<Byte>(4096);
                int count;
                while ((count = readStream->Read(data, 0, data->Length)) != 0)
                {
                    writeStream->Write(data, 0, count);
                }

                readStream->Close();
                writeStream->Close(); // Required to flush the buffer & have non-zero filesize
           }
        }
        catch (...)
        {
        }
    }


    mInDesignMode = ((LicenseManager::UsageMode == LicenseUsageMode::Designtime) || Process::GetCurrentProcess()->ProcessName->ToLowerInvariant()->Contains("devenv"));
    if (!mInDesignMode)
    {
        MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_APPLICATION);

        // Munge cli::array<String ^, 1>^ to char**
        // Get the command line arguments
        cli::array<String ^, 1>^ args = Environment::GetCommandLineArgs();
        int argc = args->Length;

        char** argv = new char* [argc];
        for (int i = 0; i < argc; i++)
        {
            argv[i] = marshal::to<char *>(args[i]);
        }

        istring progName = "Unknown";
        istring progTitle = "Unknown";
        istring progDescription = "Probably running in design mode";

        Assembly^ entryAssembly = Assembly::GetEntryAssembly();
        // In design mode entryAssembly is <nullptr>
        if (entryAssembly != nullptr)
        {
            progName = marshal::to<istring>(Path::GetFileName(entryAssembly->Location));

            cli::array<Object^, 1>^ attributes;

            String^ assemblyTitle = Path::GetFileNameWithoutExtension(entryAssembly->CodeBase);
            attributes = entryAssembly->GetCustomAttributes(AssemblyTitleAttribute::typeid, false);
            if (attributes->Length > 0)
            {
                AssemblyTitleAttribute^ titleAttribute = (AssemblyTitleAttribute^)attributes[0];
                if (!String::IsNullOrEmpty(titleAttribute->Title))
                {
                    assemblyTitle = titleAttribute->Title;
                }
            }
            progTitle = marshal::to<istring>(assemblyTitle);

            String^ assemblyDescription = String::Empty;
            attributes = entryAssembly->GetCustomAttributes(AssemblyDescriptionAttribute::typeid, false);
            if (attributes->Length > 0)
            {
                assemblyDescription = ((AssemblyDescriptionAttribute^)attributes[0])->Description;
            }
            progDescription = marshal::to<istring>(assemblyDescription);
        }

        // Just create (and promptly forget about) a CCmdLine so that it can parse for debug stuff
        //  and set up CMessageHandler
        CCmdLine* cmdLine = new CCmdLine(progName,
        progTitle,
        progDescription,
        "",
        argc,
        argv);

        mPreParseError = gcnew String(cmdLine->GetLastError().c_str());

        // CLR apps don't yet have access to CCmdLine so they can't set up expected params etc.
        //  so, ...
        cmdLine->TurnOffCommandLineSupport();

        // use this to emit sign-on message
        cmdLine->Parse();

        if(!String::IsNullOrEmpty(mPreParseError))
        {
            Console::WriteLine(mPreParseError);
        }
    }
}

MessageHandler::MessageHandler()
{
    if (!mInDesignMode)
    {
        mEngineObserver = new CUnmanagedEngineObserver();
        MSG_HANDLER.AddObserver(*mEngineObserver, CMessageHandler::GROUP_ENUM_RSVD_ALL);

        mUnmanaged = new CUnmanagedMessageHandler(this);
        mEngineObserver->SetObserver(mUnmanaged);
    }
}

MessageHandler::~MessageHandler()
{
    delete mUnmanaged;
}

/////////////////////////////////////////////////////////////////////////

void MessageHandler::EmitDebugMessage(DebugLevel debugLevel, bool isEntry, bool isExit, String^ text, Object^ retVal)
{
    // Really need to check for level enabled here i.e. before anything 'expensive' is done
    if (IsDebugLevelEnabled(debugLevel))
    {
        // skip 2 frames because:
        //  0: this function
        //  1: caller in this module e.g. NotifyDebug
        //  2: 'real' caller of interest
        // Capture source information (slow but useful)
        StackTrace^ stackTrace = gcnew StackTrace(2, true);
        char* fileName = "-- Unknown file --";
        char* methodName = "-- Unknown method --";
        int lineNumber = 0;

        if (stackTrace != nullptr)
        {
            StackFrame^ callingFrame = stackTrace->GetFrame(0);
            if (callingFrame != nullptr)
            {
                MethodBase^ callingMethod = callingFrame->GetMethod();
                if (!String::IsNullOrEmpty(callingMethod->Name))
                {
                    methodName = marshal::to<char*>(callingMethod->Name);

                    if (isEntry || isExit)
                    {
                        String^ className = callingMethod->DeclaringType->Name;
                        text = String::Format("{0} {1}::{2}", text, className, callingMethod->Name);

                        // Process exit values
                        if (isExit && (retVal != nullptr))
                        {
                            text = String::Format("{0} [returning {1}]", text, retVal->ToString());
                        }
                    }
                }

                String^ managedFileName = callingFrame->GetFileName();
                if (!String::IsNullOrEmpty(managedFileName))
                {
                    fileName = marshal::to<char*>(managedFileName);
                }

                lineNumber = callingFrame->GetFileLineNumber();
            }
            else
            {
                // Oops, no stack frame
                methodName = "-- No stack frame --";
                fileName = "-- No stack frame --";
            }
        }
        else
        {
            // Oops, no stack trace
            methodName = "-- No stack trace --";
            fileName = "-- No stack trace --";
        }

        MSG_HANDLER.DebugOutputInHandler((uint32)debugLevel, isEntry, isExit, methodName, fileName, lineNumber, marshal::to<char*>(text));
    }
}

/////////////////////////////////////////////////////////////////////////

void MessageHandler::NotifyProgress(uint16 value)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.NotifyProgress(value);
    }
}

/////////////////////////////////////////////////////////////////////////

void MessageHandler::DebugEntry()
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        EmitDebugMessage(DebugLevel::EntryExit, true, false, "->", nullptr);
    }
}

void MessageHandler::DebugExit()
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        EmitDebugMessage(DebugLevel::EntryExit, false, true, "<-", nullptr);
    }
}

void MessageHandler::DebugExit(Object^ aRetVal)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        if (aRetVal == nullptr)
        {
            DebugExit();
        }
        else
        {
            EmitDebugMessage(DebugLevel::EntryExit, false, true, "<-", aRetVal);
        }
    }
}

void MessageHandler::DebugExitFormat(String^ aFormat, ... cli::array<Object ^, 1>^ aArgs)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        if (aFormat!= nullptr)
        {
            String^ formatted = String::Format(aFormat, aArgs);
            EmitDebugMessage(DebugLevel::EntryExit, false, true, "<-", formatted);
        }
        else
        {
            // Edge case; an object of type String^ that happens to be null!
            EmitDebugMessage(DebugLevel::EntryExit, false, true, "<- <null>", nullptr);
        }
    }
}

/////////////////////////////////////////////////////////////////////////

void MessageHandler::NotifyDebug(DebugLevel debugLevel, String^ text)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        EmitDebugMessage(debugLevel, false, false, text, nullptr);
    }
}

void MessageHandler::NotifyDebug(DebugLevel debugLevel, String^ aFormat, ... cli::array<Object ^, 1>^ args)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(aFormat, args);

        EmitDebugMessage(debugLevel, false, false, formatted, nullptr);
    }
}

/////////////////////////////////////////////////////////////////////////

// log4net-like interface

void MessageHandler::DebugBasic(Object^ message)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = message->ToString();

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugBasic(Object^ message, Exception^ exception)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format("{0} {1}", message->ToString(), exception->ToString());

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugBasicFormat(String^ format, Object^ arg0)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, arg0);

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugBasicFormat(String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, args);

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugBasicFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(provider, format, args);

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugBasicFormat(String^ format, Object^ arg0, Object^ arg1)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, arg0, arg1);

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugBasicFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, arg0, arg1, arg2);

        EmitDebugMessage(DebugLevel::Basic, false, false, formatted, nullptr);
    }
}


void MessageHandler::DebugEnhanced(Object^ message)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = message->ToString();

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugEnhanced(Object^ message, Exception^ exception)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format("{0} {1}", message->ToString(), exception->ToString());

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugEnhancedFormat(String^ format, Object^ arg0)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, arg0);

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugEnhancedFormat(String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, args);

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugEnhancedFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(provider, format, args);

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugEnhancedFormat(String^ format, Object^ arg0, Object^ arg1)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, arg0, arg1);

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}

void MessageHandler::DebugEnhancedFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2)
{
    if (!mInDesignMode && IS_DEBUG_ACTIVATED)
    {
        String^ formatted = String::Format(format, arg0, arg1, arg2);

        EmitDebugMessage(DebugLevel::Enhanced, false, false, formatted, nullptr);
    }
}


/////////////////////////////////////////////////////////////////////////

void MessageHandler::NotifyStatus(StatusLevel level, String^ text)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.NotifyStatus((StatusLevels)level, marshal::to<istring>(text));
    }
}

void MessageHandler::StatusInfo(Object^ message)
{
    if (!mInDesignMode)
    {
        String^ formatted = message->ToString();

        NotifyStatus(StatusLevel::Info, formatted);
    }
}

void MessageHandler::StatusInfoFormat(String^ format, Object^ arg0)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0);

        NotifyStatus(StatusLevel::Info, formatted);
    }
}

void MessageHandler::StatusInfoFormat(String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, args);

        NotifyStatus(StatusLevel::Info, formatted);
    }
}

void MessageHandler::StatusInfoFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(provider, format, args);

        NotifyStatus(StatusLevel::Info, formatted);
    }
}

void MessageHandler::StatusInfoFormat(String^ format, Object^ arg0, Object^ arg1)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1);

        NotifyStatus(StatusLevel::Info, formatted);
    }
}

void MessageHandler::StatusInfoFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1, arg2);

        NotifyStatus(StatusLevel::Info, formatted);
    }
}


void MessageHandler::StatusEssential(Object^ message)
{
    if (!mInDesignMode)
    {
        String^ formatted = message->ToString();

        NotifyStatus(StatusLevel::Essential, formatted);
    }
}

void MessageHandler::StatusEssentialFormat(String^ format, Object^ arg0)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0);

        NotifyStatus(StatusLevel::Essential, formatted);
    }
}

void MessageHandler::StatusEssentialFormat(String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, args);

        NotifyStatus(StatusLevel::Essential, formatted);
    }
}

void MessageHandler::StatusEssentialFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(provider, format, args);

        NotifyStatus(StatusLevel::Essential, formatted);
    }
}

void MessageHandler::StatusEssentialFormat(String^ format, Object^ arg0, Object^ arg1)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1);

        NotifyStatus(StatusLevel::Essential, formatted);
    }
}

void MessageHandler::StatusEssentialFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1, arg2);

        NotifyStatus(StatusLevel::Essential, formatted);
    }
}


void MessageHandler::StatusWarning(Object^ message)
{
    if (!mInDesignMode)
    {
        String^ formatted = message->ToString();

        NotifyStatus(StatusLevel::Warning, formatted);
    }
}

void MessageHandler::StatusWarningFormat(String^ format, Object^ arg0)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0);

        NotifyStatus(StatusLevel::Warning, formatted);
    }
}

void MessageHandler::StatusWarningFormat(String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, args);

        NotifyStatus(StatusLevel::Warning, formatted);
    }
}

void MessageHandler::StatusWarningFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(provider, format, args);

        NotifyStatus(StatusLevel::Warning, formatted);
    }
}

void MessageHandler::StatusWarningFormat(String^ format, Object^ arg0, Object^ arg1)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1);

        NotifyStatus(StatusLevel::Warning, formatted);
    }
}

void MessageHandler::StatusWarningFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1, arg2);

        NotifyStatus(StatusLevel::Warning, formatted);
    }
}


void MessageHandler::StatusError(Object^ message)
{
    if (!mInDesignMode)
    {
        String^ formatted = message->ToString();

        NotifyStatus(StatusLevel::Error, formatted);
    }
}

void MessageHandler::StatusErrorFormat(String^ format, Object^ arg0)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0);

        NotifyStatus(StatusLevel::Error, formatted);
    }
}

void MessageHandler::StatusErrorFormat(String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, args);

        NotifyStatus(StatusLevel::Error, formatted);
    }
}

void MessageHandler::StatusErrorFormat(IFormatProvider^ provider, String^ format, ... cli::array<Object^, 1>^ args)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(provider, format, args);

        NotifyStatus(StatusLevel::Error, formatted);
    }
}

void MessageHandler::StatusErrorFormat(String^ format, Object^ arg0, Object^ arg1)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1);

        NotifyStatus(StatusLevel::Error, formatted);
    }
}

void MessageHandler::StatusErrorFormat(String^ format, Object^ arg0, Object^ arg1, Object^ arg2)
{
    if (!mInDesignMode)
    {
        String^ formatted = String::Format(format, arg0, arg1, arg2);

        NotifyStatus(StatusLevel::Error, formatted);
    }
}


/////////////////////////////////////////////////////////////////////////

bool MessageHandler::SetErrorMsg(int16 errorCode, String^ text)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.SetErrorMsg(errorCode, marshal::to<istring>(text));
    }

    return retVal;
}

bool MessageHandler::ModifyErrorMsg(int16 errorCode, String^ text)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.ModifyErrorMsg(errorCode, marshal::to<istring>(text));
    }

    return retVal;
}

bool MessageHandler::ModifyErrorMsg(int16 errorCode, String^ text, GroupEnum groupEnum)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.ModifyErrorMsg(errorCode, marshal::to<istring>(text), (CMessageHandler::GroupEnum)groupEnum);
    }

    return retVal;
}

bool MessageHandler::SetErrorAutomaticDisplay(bool newAutomaticDisplay)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.SetErrorAutomaticDisplay(newAutomaticDisplay);
    }

    return retVal;
}

bool MessageHandler::SetErrorAutomaticDisplay(bool newAutomaticDisplay, GroupEnum groupEnum)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.SetErrorAutomaticDisplay(newAutomaticDisplay, (CMessageHandler::GroupEnum)groupEnum);
    }

    return retVal;
}

bool MessageHandler::LastError(int16% errorCode, String^% errorText)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        std::string errorMessage;
        int16 err = 0;
        CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

        retVal = MSG_HANDLER.LastError(err, errorMessage, group);

        // Set the [Out] parameters
        errorCode = err;
        errorText = gcnew String(errorMessage.c_str());
    }

    return retVal;
}

int16 MessageHandler::LastError(String^% errorText)
{
    int16 retVal = 0;

    if (!mInDesignMode)
    {
        std::string errorMessage;
        int16 err = 0;
        CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

        bool hasError = MSG_HANDLER.LastError(err, errorMessage, group);

        // Set the [Out] parameters
        errorText = gcnew String(errorMessage.c_str());

        retVal =  hasError ? err : 0;
    }

    return retVal;
}

bool MessageHandler::LastError(int16% errorCode, String^% errorText, GroupEnum% groupEnum)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        std::string errorMessage;
        int16 err = 0;
        CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

        bool hasError = MSG_HANDLER.LastError(err, errorMessage, group);

        // Set the [Out] parameters
        errorCode = err;
        errorText = gcnew String(errorMessage.c_str());
        groupEnum = (EngineFrameworkClr::GroupEnum)group;

        retVal = hasError;
    }

    return retVal;
}

int16 MessageHandler::LastError(String^% errorText, GroupEnum% groupEnum)
{
    int16 retVal = 0;

    if (!mInDesignMode)
    {
        std::string errorMessage;
        int16 err = 0;
        CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

        bool hasError = MSG_HANDLER.LastError(err, errorMessage, group);

        // Set the [Out] parameters
        errorText = gcnew String(errorMessage.c_str());
        groupEnum = (EngineFrameworkClr::GroupEnum)group;

        retVal = hasError ? err : 0;
    }

    return retVal;
}

bool MessageHandler::LastError(int16% errorCode, String^% errorText, GroupEnum% groupEnum, bool currentThreadOnly)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        std::string errorMessage;
        int16 err = 0;
        CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

        bool hasError = MSG_HANDLER.LastError(err, errorMessage, group, currentThreadOnly);

        // Set the [Out] parameters
        errorCode = err;
        errorText = gcnew String(errorMessage.c_str());
        groupEnum = (EngineFrameworkClr::GroupEnum)group;

        retVal = hasError;
    }

    return retVal;
}

int16 MessageHandler::LastError(String^% errorText, GroupEnum% groupEnum, bool currentThreadOnly)
{
    int16 retVal = 0;

    if (!mInDesignMode)
    {
        std::string errorMessage;
        int16 err = 0;
        CMessageHandler::GroupEnum group = CMessageHandler::GROUP_ENUM_RSVD_ALL;

        bool hasError = MSG_HANDLER.LastError(err, errorMessage, group, currentThreadOnly);

        // Set the [Out] parameters
        errorText = gcnew String(errorMessage.c_str());
        groupEnum = (EngineFrameworkClr::GroupEnum)group;

        retVal = hasError ? err : 0;
    }

    return retVal;
}

bool MessageHandler::IsErrorSet()
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.IsErrorSet();
    }

    return retVal;
}

bool MessageHandler::IsErrorSet(GroupEnum groupEnum)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.IsErrorSet((CMessageHandler::GroupEnum)groupEnum);
    }

    return retVal;
}

bool MessageHandler::IsErrorSet(GroupEnum groupEnum, bool currentThreadOnly)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.IsErrorSet((CMessageHandler::GroupEnum)groupEnum, currentThreadOnly);
    }

    return retVal;
}

bool MessageHandler::IsDebugLevelEnabled(DebugLevel debugLevel)
{
    bool retVal = false;

    if (!mInDesignMode)
    {
        retVal = MSG_HANDLER.IsDebugLevelEnabled((DebugLevels)debugLevel);
    }

    return retVal;
}

void MessageHandler::ClearError()
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.ClearError();
    }
}

void MessageHandler::ClearError(GroupEnum groupEnum)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.ClearError((CMessageHandler::GroupEnum)groupEnum);
    }
}

/////////////////////////////////////////////////////////////////////////

void MessageHandler::SetDebugLevel(DebugLevel debugLevel, bool enable)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.SetDebugLevel((DebugLevels)debugLevel, enable, CMessageHandler::GroupEnum::GROUP_ENUM_RSVD_LOCAL);
    }
}

void MessageHandler::SetDebugLevel(DebugLevel debugLevel, bool enable, GroupEnum groupEnum)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.SetDebugLevel((DebugLevels)debugLevel, enable, (CMessageHandler::GroupEnum)groupEnum);
    }
}

void MessageHandler::SetStatusLevel(StatusLevel statusLevel, bool enable)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.SetStatusLevel((StatusLevels)statusLevel, enable, CMessageHandler::GroupEnum::GROUP_ENUM_RSVD_LOCAL);
    }
}

void MessageHandler::SetStatusLevel(StatusLevel statusLevel, bool enable, GroupEnum groupEnum)
{
    if (!mInDesignMode)
    {
        MSG_HANDLER.SetStatusLevel((StatusLevels)statusLevel, enable, (CMessageHandler::GroupEnum)groupEnum);
    }
}

/////////////////////////////////////////////////////////////////////////

void MessageHandler::OnProgress(int16 progress)
{
    if (!mInDesignMode)
    {
        Progress(this, gcnew ProgressEventArgs(progress));
    }
}

void MessageHandler::OnStatus(StatusLevels level, const std::string& message)
{
    if (!mInDesignMode)
    {
        String^ managedMessage = gcnew String(message.c_str());
        Status(this, gcnew StatusEventArgs((StatusLevel)level, managedMessage));
    }
}

void MessageHandler::OnDebug(DebugLevels level, const std::string& message)
{
    if (!mInDesignMode)
    {
        String^ managedMessage = gcnew String(message.c_str());
        Debug(this, gcnew DebugEventArgs((DebugLevel)level, managedMessage));
    }
}

/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////

CUnmanagedMessageHandler::CUnmanagedMessageHandler(MessageHandler^ parent)
{
    mParent = parent;
}

void CUnmanagedMessageHandler::OnProgress(int16 progress)
{
    mParent->OnProgress(progress);
}

void CUnmanagedMessageHandler::OnStatus(StatusLevels level, const std::string& response)
{
    mParent->OnStatus(level, response);
}

void CUnmanagedMessageHandler::OnDebug(DebugLevels level, const std::string& response)
{
    mParent->OnDebug(level, response);
}

/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////

CUnmanagedEngineObserver::CUnmanagedEngineObserver()
    : mObserver(NULL)
{
}

CUnmanagedEngineObserver::~CUnmanagedEngineObserver()
{
}

void CUnmanagedEngineObserver::NotifyText(uint32 aRsvdForFuture, CMessageHandler::MessageType major, uint32 minor, const char* str)
{
    if (mObserver)
    {
        switch (major)
        {
        case CMessageHandler::MESSAGE_TYPE_PROGRESS:
            break;

        case CMessageHandler::MESSAGE_TYPE_DEBUG:
            mObserver->OnDebug((DebugLevels)minor, str);
            break;

        case CMessageHandler::MESSAGE_TYPE_STATUS:
            mObserver->OnStatus((StatusLevels)minor, str);
            break;

        case CMessageHandler::MESSAGE_TYPE_COUNT:
            break;
        }
    }
}

void CUnmanagedEngineObserver::NotifyNumber(uint32 aRsvdForFuture, CMessageHandler::MessageType major, uint32 minor, int32 number)
{
    assert(major == CMessageHandler::MESSAGE_TYPE_PROGRESS);
    if (mObserver)
    {
        mObserver->OnProgress((int16)number);
    }
}

void CUnmanagedEngineObserver::SetObserver(CUnmanagedMessageHandler* ob)
{
    mObserver = ob;
}

/////////////////////////////////////////////////////////////////////////
