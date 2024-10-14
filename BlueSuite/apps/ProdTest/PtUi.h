//**************************************************************************************************
//
//  PtUi.h
//
//  Copyright (c) 2020 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  User interface class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_UI_H
#define PT_UI_H

#include <common/types.h>
#include <string>

///
/// Production test user interface class (singleton).
///
class CPtUi
{
public:
    ///
    /// Gets the CPtUi object.
    /// @return Reference to the CPtUi object.
    ///
    static CPtUi& Ref();

    ///
    /// Asks the user interface operator to perform a check, and indicate pass or fail.
    /// @param[in] aMessage Message indicating the check to perform.
    /// @return true if user interface operator indicates check passed, false otherwise.
    ///
    bool AskForCheck(const std::string& aMessage) const;

    ///
    /// Asks the user interface operator to do something, then waits for a key press.
    /// @param[in] aMessage Message indicating what is to be done.
    ///
    void AskHitKey(const std::string& aMessage) const;

    ///
    /// Sets quite mode on/off.
    /// @param[in] aQuiet true to enable quiet mode, false to disable.
    ///
    void SetQuiet(bool aQuiet) { mQuiet = aQuiet; };

    ///
    /// Write a message to the user interface.
    /// @param[in] aMessage Message to write.
    /// @param[in] aEssential true to indicate essential message, false means display
    ///   only when in not in quiet mode.
    ///
    void Write(const std::string& aMessage, bool aEssential = true) const;

    ///
    /// Write a message to the user interface.
    /// @param[in] aMessage Message to write.
    /// @param[in] aEssential true to indicate essential message, false means display
    ///   only when in not in quiet mode.
    ///
    void Write(const std::wstring& aMessage, bool aEssential = true) const;

    ///
    /// Opens a log file for logging writes to the UI
    /// @param[in] aFilePath File path to open
    /// @throws CPtException
    ///
    void SetLogFile(const std::string& aFilePath);

    ///
    /// Write a test status message to the user interface.
    /// @param[in] aMessage Message to write.
    /// @param[in] aPass true if the test(s) passed, false otherwise.
    ///
    void WriteStatus(const std::string& aMessage, bool aPass) const;

    ///
    /// Write an error message to the user interface.
    /// @param[in] aMessage Message to write.
    ///
    void WriteError(const std::string& aMessage) const;

    ///
    /// Updates a line on the user interface (does not terminate with a newline).
    /// Useful for progress reporting where the value changes within the same console line.
    /// @param[in] aMessage Message to update the line with.
    ///
    void UpdateLine(const std::string& aMessage) const;

private:
    ///
    /// Constructor.
    ///
    CPtUi();

    ///
    /// Destructor.
    ///
    ~CPtUi();

    ///
    /// Copy constructor.
    ///
    CPtUi(const CPtUi&);

    ///
    /// Assignment operator.
    ///
    CPtUi& operator=(const CPtUi&);

    ///
    /// Sets the console text attributes.
    /// @param[in] aNewAttributes The attributes to set.
    /// @param[out] aOldAttributes The attributes before setting the new attributes.
    /// Attribute values are as for WinAPI function SetConsoleTextAttr.
    /// return true if successful, false otherwise.
    ///
    bool SetConsoleTextAttr(uint16 aNewAttributes, uint16& aOldAttributes) const;

    ///
    /// Indicates whether quiet mode should be used.
    ///
    bool mQuiet;

    ///
    /// Log file
    ///
    std::ofstream* mpLogFile;
};

#endif // PT_UI_H
