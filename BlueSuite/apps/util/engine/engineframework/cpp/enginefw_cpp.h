/**********************************************************************
 *
 *  enginefw_cpp.h
 *
 *  Copyright (c) 2011-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Handles the engine worker classes.
 *
 ***********************************************************************/

#ifndef ENGINEFW_CPP_H
#define ENGINEFW_CPP_H

#include "common/types.h"
#include "time/stop_watch.h"

#include <map>
#include <ostream>
#include <string>

/////////////////////////////////////////////////////////////////////////////

/// Handles progress indication (i.e. value from 0% to 100% inclusive)
class CProgressMsg
{
public:

    void Set(uint16 value);

    /// The default implementation of "silent mode" causes notifications to cease,
    /// but specialised implementations have the ability to override this behaviour.
    virtual void SetSilentMode(bool aSilentMode);

    CProgressMsg();

    virtual ~CProgressMsg();

    // increment the progress by 1 (up to 100)
    CProgressMsg& operator++(int);

    // increment the progress by set amount(up to 100)
    CProgressMsg& operator+=(int inc);

protected:
    /// Notify is executed each time the percentage changes
    virtual void Notify(uint16 value);
    bool mSilentMode;

    /// Accessor used to initialise the sync lock CriticalSection object
    /// on first use, avoiding static initialisation order issues.
    /// CriticalSection object used to sync access by the objects.
    /// This is at a very broad level, but it provides consistency between
    /// the base objects used within the framework.
    static CriticalSection& GetSyncLock()
    { 
        static CriticalSection* spSyncLock = new CriticalSection();
        return *spSyncLock;
    }

private:
    uint16 mCurrent;
};

/////////////////////////////////////////////////////////////////////////////

class CConsoleProgressMsg : public CProgressMsg
{
public:

    CConsoleProgressMsg();
    virtual ~CConsoleProgressMsg();

protected:
    virtual void Notify(uint16 value);
};

/////////////////////////////////////////////////////////////////////////////

class CMsgQueue
{
protected:
    enum QueueType
    {
        STATUS_QUEUE,
        DEBUG_QUEUE
    };
    QueueType mQueueType;

public:
    CMsgQueue(QueueType type, std::ostream* stream, bool allLevelsEnabled);

    virtual ~CMsgQueue();

    virtual void OutputMsg(uint32 level, const char* text);

    bool IsLevelEnabled(uint32 level);

    virtual void LevelEnable(uint32 level, bool enable);

    void InheritLevels(CMsgQueue* apOtherQueue);

protected:
    std::ostream* mpStream;
    uint32 mLevelsEnabled;

    /// Accessor used to initialise the sync lock CriticalSection object
    /// on first use, avoiding static initialisation order issues.
    /// CriticalSection object used to sync access by the objects.
    /// This is at a very broad level, but it provides consistency between
    /// the base objects used within the framework.
    static CriticalSection& GetSyncLock()
    {
        static CriticalSection* spSyncLock = new CriticalSection();
        return *spSyncLock;
    }
};

/////////////////////////////////////////////////////////////////////////////

class CErrorMessages
{
public:
    CErrorMessages();

    virtual ~CErrorMessages();

    void Clear();

    void Set(int16 aErrorCode, const std::string& aErrorText, uint32 aErrorSequencePos);

    ///
    /// Retrieve the error for the next thread up (if there is one).
    /// @param[out] aErrorCode The code for the error.
    /// @param[out] aErrorText The text for the error.
    /// @param[out] aErrorSequencePos The sequence position (i.e. the value of mpErrorIndex at the time).
    /// @param[in,out] aThreadId As an input, it determines the start number to use for the lookup
    /// (i.e. look for errors with a thread ID of this number or higher). If/when an error is found,
    /// this parameter is updated with the thread ID where the error occurred.
    /// @return true if an error was found, false otherwise.
    ///
    bool Get(int16& aErrorCode, std::string& aErrorText, uint32& aErrorSequencePos, unsigned long& aThreadId);

    bool IsSet();

    class CErrorMsgData
    {
    public:
        CErrorMsgData() : mSequencePos(0), mCode(0) { }
        ~CErrorMsgData() { }

        std::string mText;
        uint32      mSequencePos;
        int16       mCode;
    };

private:
    /// Accessor used to initialise the sync lock CriticalSection object
    /// on first use, avoiding static initialisation order issues.
    /// CriticalSection object used to sync access by the objects.
    /// This is at a very broad level, but it provides consistency between
    /// the base objects used within the framework.
    static CriticalSection& GetSyncLock()
    {
        static CriticalSection* spSyncLock = new CriticalSection();
        return *spSyncLock;
    }

    /// A collection of data objects (one for each thread)
    std::map<unsigned long, CErrorMsgData> mData;
};

/////////////////////////////////////////////////////////////////////////////

class CStatusMsg : public CMsgQueue
{
public:
    CStatusMsg(std::ostream* stream);

    virtual ~CStatusMsg();
};

/////////////////////////////////////////////////////////////////////////////

class CDebugMsg : public CMsgQueue
{
public:
    CDebugMsg(std::ostream* stream);

    virtual ~CDebugMsg();

    void SetTimeStampState(bool enabled);

    void DebugOutputWithList(uint32 level, bool entry, bool exit, const char* function,
        const char* filename, uint32 linenum, const char* format, va_list argptr);

    void LevelEnable(uint32 level, bool enable);

    void SendDebugToOutputPane();

    void WriteDebugLineToOutputPane(const char* aLine);

    void OutputMsg(uint32 level, const char* text);

private:
    uint16 GetIndent();
    uint16 SetIndent(uint16 aNewIndent);

    uint16 DecrementIndent();
    uint16 IncrementIndent();

    typedef ThreadSpecificPtr<uint16> DebugIndent;
    /// Accessor used to initialise the debug indent ThreadSpecificPtr object
    /// on first use, avoiding static initialisation order issues.
    static DebugIndent& GetDebugIndent()
    {
        static DebugIndent* spTsp = new DebugIndent();
        return *spTsp;
    }

    StopWatch* mpTimeSinceStart;
    bool mTimeStampsEnabled;
    bool mWritingToOutputPane;
};

/////////////////////////////////////////////////////////////////////////////

class CConsoleStatusMsg : public CStatusMsg
{
public:
    CConsoleStatusMsg();
    virtual ~CConsoleStatusMsg();

    void OutputMsg(uint32 level, const char* text);
};

/////////////////////////////////////////////////////////////////////////////

class CGuiStatusMsg : public CStatusMsg
{
public:
    CGuiStatusMsg(std::ostream* stream, const CMessageHandlerObserver* obs);
    virtual ~CGuiStatusMsg();
    const CMessageHandlerObserver* mObs;
    void OutputMsg(uint32 level, const char* text);
};

/////////////////////////////////////////////////////////////////////////////

template <class T>
class CMessageObserver : public T
{
public:
    CMessageObserver(CMessageHandlerObserver& observer) : T(NULL), mObserver(observer) { }
    ~CMessageObserver() { }

    void OutputMsg(uint32 level, const char* text)
    {
        if (T::IsLevelEnabled(level))
        {
            switch (T::mQueueType)
            {
            case T::STATUS_QUEUE:
                mObserver.NotifyText(0, CMessageHandler::MESSAGE_TYPE_STATUS, level, text);
                break;

            case T::DEBUG_QUEUE:
                mObserver.NotifyText(0, CMessageHandler::MESSAGE_TYPE_DEBUG, DEBUG_ALL, text);
                break;
            }
        }
    }
private:
    CMessageHandlerObserver& mObserver;
};

/////////////////////////////////////////////////////////////////////////////

class CProgressObserver : public CProgressMsg
{
public:
    CProgressObserver(CMessageHandlerObserver& observer);
    ~CProgressObserver();

protected:
    virtual void Notify(uint16 value);

private:
    CMessageHandlerObserver& mObserver;
};

#endif
