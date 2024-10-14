//**************************************************************************************************
//
//  PtUi.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  User interface class definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "PtUi.h"
#include "PtException.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <conio.h>
#include <codecvt>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

CPtUi& CPtUi::Ref()
{
    static CPtUi instance;
    return instance;
}

////////////////////////////////////////////////////////////////////////////////

CPtUi::CPtUi()
: mQuiet(false), mpLogFile(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

CPtUi::~CPtUi()
{ 
    delete mpLogFile;
}

////////////////////////////////////////////////////////////////////////////////

bool CPtUi::AskForCheck(const std::string& aMessage) const
{
    Write(aMessage);
    
    bool pass = false;
    char ch;
    do
    {
        Write("Hit 'P' for pass or 'F' for fail");

        ch = static_cast<char>(::toupper(_getch()));
        
        // Show the input
        Write(string({ ch }), true);
    }
    while (ch != 'P' && ch != 'F');

    if (ch == 'P')
    {
        pass = true;
    }

    return pass;
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::AskHitKey(const std::string& aMessage) const
{
    // Clear any buffered characters first
    while (_kbhit())
    {
        (void)_getch();
    }
    
    cout << aMessage << endl;

    (void)_getch();
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::Write(const std::string& aMessage, bool aEssential) const
{
    if (aEssential || !mQuiet)
    {
        cout << aMessage.c_str() << endl;
    }

    // Everything goes into the log file
    if (mpLogFile != NULL)
    {
        *mpLogFile << aMessage.c_str() << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::Write(const std::wstring& aMessage, bool aEssential) const
{
    if (aEssential || !mQuiet)
    {
        wcout << aMessage.c_str() << endl;
    }

    // Everything goes into the log file
    if (mpLogFile != NULL)
    {
        // Narrow the wide string
        wstring wMsg(aMessage);
        wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
        string msg = converter.to_bytes(wMsg);

        *mpLogFile << msg << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::WriteStatus(const std::string& aMessage, bool aPass) const
{
    // Change to text colour to red/green depending on pass/fail
    const WORD newColourAttributes = (aPass ? FOREGROUND_GREEN : FOREGROUND_RED) | FOREGROUND_INTENSITY;
    WORD savedColourAttributes;
    bool colourChanged = SetConsoleTextAttr(newColourAttributes, savedColourAttributes);

    Write(aMessage, true);

    // Revert to previous colours
    if (colourChanged)
    {
        (void)SetConsoleTextAttr(savedColourAttributes, savedColourAttributes);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::WriteError(const std::string& aMessage) const
{
    // It's an error, use red text
    WORD savedColourAttributes;
    bool colourChanged = SetConsoleTextAttr(FOREGROUND_RED | FOREGROUND_INTENSITY,
                                            savedColourAttributes);

    Write("ERROR: " + aMessage, true);

    // Revert to previous colours
    if (colourChanged)
    {
        (void)SetConsoleTextAttr(savedColourAttributes, savedColourAttributes);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::UpdateLine(const std::string& aMessage) const
{
    // Updating from start of line
    cout << "\r" << aMessage.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void CPtUi::SetLogFile(const std::string& aFilePath)
{
    delete mpLogFile;
    mpLogFile = new ofstream(aFilePath, ios_base::out);
    if (mpLogFile == NULL || !mpLogFile->good())
    {
        ostringstream msg;
        msg << "Could not open log file \"" << aFilePath << "\"";
        throw CPtException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CPtUi::SetConsoleTextAttr(uint16 aNewAttributes, uint16& aOldAttributes) const
{
    bool success = false;
    CONSOLE_SCREEN_BUFFER_INFO csbInfo;

    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdOutHandle == INVALID_HANDLE_VALUE)
    {
        Write("ERROR: GetStdHandle failed (can't change text colour)", true);
    }
    else
    {
        // Get the current attributes
        if (GetConsoleScreenBufferInfo(stdOutHandle, &csbInfo) == FALSE)
        {
            Write("ERROR: GetConsoleScreenBufferInfo failed (can't change text colour)", true);
        }
        else
        {
            aOldAttributes = csbInfo.wAttributes;

            // Set the new attributes
            if (SetConsoleTextAttribute(stdOutHandle, aNewAttributes) == FALSE)
            {
                Write("ERROR: SetConsoleTextAttribute failed (can't change text colour)", true);
            }
            else
            {
                success = true;
            }
        }
    }

    return success;
}
