//------------------------------------------------------------------------------
//
// <copyright file="MainForm.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2018-2022 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary>Main form for HidDfuApp tool</summary>
//
//------------------------------------------------------------------------------

using HidDfuAPI;
using QTIL.HostTools.Common.Dialogs;
using QTIL.HostTools.Common.EngineFrameworkClr;
using QTIL.HostTools.Common.Util;
using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices; // for DllImport etc.
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;
using System.Text;

namespace QTIL.HostTools.HidDfuApp
{
    public partial class MainForm : Form
    {

        #region Types

        // Error codes
        // See - HidDfu.h
        private enum ErrorCodes
        {
            HIDDFU_ERROR_NONE = 0,
            HIDDFU_ERROR_SEQUENCE = -1,
            HIDDFU_ERROR_CONNECTION = -2,
            HIDDFU_ERROR_FILE_OPEN_FAILED = -3,
            HIDDFU_ERROR_FILE_WRITE_FAILED = -4,
            HIDDFU_ERROR_FILE_INVALID_FORMAT = -5,
            HIDDFU_ERROR_FILE_CRC_INCORRECT = -6,
            HIDDFU_ERROR_FILE_READ_FAILED = -7,
            HIDDFU_ERROR_UPGRADE_FAILED = -8,
            HIDDFU_ERROR_RESET_FAILED = -9,
            HIDDFU_ERROR_OUT_OF_MEM = -10,
            HIDDFU_ERROR_INVALID_PARAMETER = -11,
            HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE = -12,
            HIDDFU_ERROR_OPERATION_FAILED_TO_START = -13,
            HIDDFU_ERROR_BUSY = -14,
            HIDDFU_ERROR_CLEAR_STATUS_FAILED = -15,
            HIDDFU_ERROR_DEVICE_FIRMWARE = -16,
            HIDDFU_ERROR_UNSUPPORTED = -17,
            HIDDFU_ERROR_OPERATION_PARTIAL_SUCCESS = -18,
            HIDDFU_ERROR_PARAM_TOO_SMALL = -19,
            HIDDFU_ERROR_UNKNOWN = -20,
            HIDDFU_ERROR_VERSION_MISMATCH = -21,
            HIDDFU_ERROR_NO_OP_IN_PROGRESS = -22,
            HIDDFU_ERROR_NO_RESPONSE = -23,
            HIDDFU_ERROR_OP_PARTIAL_SUCCESS_NO_RESPONSE = -24
        }

        private enum CommandType
        {
            [Description("Backup")]
            BackUp,
            [Description("Upgrade")]
            Upgrade,
            [Description("Upgrade Binary")]
            UpgradeBin
        };

        #endregion  // Types

        #region Constants

        // Default values if registry is not set
        private const UInt16 DEFAULT_LOADER_VID = 0xA12;
        private const UInt16 DEFAULT_LOADER_PID = 0xFFFE;
        private const UInt16 DEFAULT_LOADER_USAGE = 0x0;
        private const UInt16 DEFAULT_LOADER_USAGE_PAGE = 0x0;
        private const UInt16 DEFAULT_LOADER_RESET_AFTER = 0;
        private const UInt16 DEFAULT_APPLICATION_VID = 0xA12;
        private const UInt16 DEFAULT_APPLICATION_PID = 0x4007;
        private const UInt16 DEFAULT_APPLICATION_USAGE = 0x0;
        private const UInt16 DEFAULT_APPLICATION_USAGE_PAGE = 0xFF00; // For CSRA681xx, QCC302x-8x and QCC512x-8x devices usagePage must be 0xFF00

        private const string STR_DFU_FILE_DIALOG_FILTER = "DFU files (*.dfu)|*.dfu;";
        private const string STR_BIN_FILE_DIALOG_FILTER = "DFU files (*.bin)|*.bin";

        // Name for registry entry
        private const string REG_COMMAND = "Command";
        private const string REG_LOADER_VID = "LoaderVid";
        private const string REG_LOADER_PID = "LoaderPid";
        private const string REG_LOADER_USAGE = "LoaderUsage";
        private const string REG_LOADER_USAGEPAGE = "LoaderUsagePage";
        private const string REG_LOADER_RESETAFTER = "ResetAfter";
        private const string REG_LOADER_FILE = "LoaderFile";
        private const string REG_APPLICATION_VID = "ApplicationVid";
        private const string REG_APPLICATION_PID = "ApplicationPid";
        private const string REG_APPLICATION_USAGE = "ApplicationUsage";
        private const string REG_APPLICATION_USAGEPAGE = "ApplicationUsagePage";
        private const string REG_APPLICATION_FILE = "ApplicationFile";
        private const string REG_FILE_INPUT_PATH = "HidDfuFilePath";
        #endregion  // Constants

        #region Member Variables

        private readonly Registry mRegistry = new Registry();   // Access to registry settings for the application

        private ToolTip mToolTip;                               // ToolTip for some of the controls on Main Form

        private static System.Timers.Timer mTimer;

        private CommandType mCommand;
        private UInt16 mVid;
        private UInt16 mPid;
        private UInt16 mUsage;
        private UInt16 mUsagePage;
        private UInt16 mCount;
        private string mFileName;
        private bool mOptionControlModified;    // Fetch values from registry only if none of the controls (except command) are modified
        private bool mUpdatingCommand;          // Do not update from registry while update from registry is in progress
        private bool mStopped;
        private bool mResetAfter;
        private bool mWaitMsgPrinted;           // Display rebooting message only once for 'Upgrade Binary' operation

        #endregion  // Member Variables

        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------

        public MainForm()
        {
            MessageHandler.DebugEntry();

            // Create a timer, to update the progress bar while an operation is running
            mTimer = new System.Timers.Timer(1000); // 1 second interval
            mTimer.Elapsed += OnTimedEvent;
            mTimer.AutoReset = true;

            // Create the controls for this form
            InitializeComponent();

            // Add the "About..." entry to the system menu
            ExtendSystemMenu();

            // Set defaults for controls
            SetDefaults();
            MessageHandler.DebugExit();
        }

        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------

        #region AboutDialog // Support for showing AboutDialog

        // define interface to a native Windows API routine that has no direct Winforms version
        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr GetSystemMenu(IntPtr hWnd, bool bRevert);

        // define interface to a native Windows API routine that has no direct Winforms version
        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool AppendMenu(IntPtr hMenu, int uFlags, int uIDNewItem, string lpNewItem);

        // define some constants that match the Windows.h API values
        private const int WM_SYSCOMMAND = 0x112; // message is a sys command
        private const int MF_STRING = 0x0;
        private const int MF_SEPARATOR = 0x800;
        private const int SYSMENU_ABOUT_ID = 0x1; // define a command id value for the "About..." menu item

        private void ExtendSystemMenu()
        {
            // Get a handle to a copy of this form's system (window) menu
            // and use it to add a separator and About <appname>... menu item.
            string aboutText = @"&About " + this.Text + @"...";
            IntPtr hSysMenu = GetSystemMenu(this.Handle, false);
            AppendMenu(hSysMenu, MF_SEPARATOR, 0, string.Empty);
            AppendMenu(hSysMenu, MF_STRING, SYSMENU_ABOUT_ID, aboutText);
        }

        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);

            if ((m.Msg == WM_SYSCOMMAND) && ((int)m.WParam == SYSMENU_ABOUT_ID))
            {
                ShowAboutBox();
            }
        }

        private void ShowAboutBox()
        {
            using (AboutDialog aboutDialog = new AboutDialog())
            {
                aboutDialog.ShowDialog();
            }
        }
        #endregion  // AboutDialog

        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------

        #region Support methods

        /// <summary>
        /// Check if backup file(s) exist
        /// </summary>
        /// <returns>true if any of the backup file(s) exists, false otherwise</returns>
        private bool BackupFilesExist()
        {
            MessageHandler.DebugEntry();
            bool filesExist = false;

            for (UInt16 i = 0; i < mCount; i++)
            {
                string backupFileName = GetBackupFileName(i);
                if (File.Exists(backupFileName))
                {
                    HidDfuAppLog("Warning: " + backupFileName + " already exists.");
                    filesExist = true;
                }
            }

            MessageHandler.DebugExit(filesExist);
            return filesExist;
        }

        /// <summary>
        /// Connect to device over HID
        /// </summary>
        /// <returns>HID DLL's API error code</returns>
        private Int32 ConnectDevices()
        {
            MessageHandler.DebugEntry();
            Int32 hidReturnVal = (Int32)ErrorCodes.HIDDFU_ERROR_CONNECTION;

            string logMsg = String.Empty;
            bool proceedWithMultipleDevs = false;

            if ((mCommand == CommandType.UpgradeBin) && (mUsagePage != DEFAULT_APPLICATION_USAGE_PAGE))
            {
                logMsg = "Error: Usage Page should be 0x" + (DEFAULT_APPLICATION_USAGE_PAGE).ToString("X")
                        + " for Upgrade Binary command (CSRA681xx, QCC302x-8x and QCC512x-8x devices) ";
                hidReturnVal = (Int32)ErrorCodes.HIDDFU_ERROR_CONNECTION;
            }
            else
            {   // Connect
                HidDfuAppLog(Environment.NewLine + "Attempting to connect...");

                hidReturnVal = HidDfu.hidDfuConnect(mVid, mPid, mUsage, mUsagePage, out mCount);

                if (hidReturnVal != (Int32)ErrorCodes.HIDDFU_ERROR_NONE)
                {
                    logMsg = "Error connecting: " + Marshal.PtrToStringAnsi(HidDfu.hidDfuGetLastError());
                }
                else if (mCount == 1)
                {
                    logMsg = "Found 1 device and connected.";
                }
                else
                {
                    string warningMsg = String.Format("Found {0} devices and connected. Do you want to run the '{1}' operation on all devices?",
                            mCount, Enum<CommandType>.GetDescriptionFromEnum(mCommand));

                    DialogResult dialogResult = MessageBox.Show(warningMsg, AssemblyInfo.Title(), MessageBoxButtons.YesNo);
                    if (dialogResult == DialogResult.No)
                    {
                        warningMsg += " No";
                        hidReturnVal = (Int32)ErrorCodes.HIDDFU_ERROR_CONNECTION;
                        proceedWithMultipleDevs = true;
                    }
                    else
                    {
                        warningMsg += " Yes";
                    }
                    logMsg = warningMsg;
                }
            }

            HidDfuAppLog(logMsg);
            if ((hidReturnVal != (Int32)ErrorCodes.HIDDFU_ERROR_NONE) && !proceedWithMultipleDevs)
            {
                HidDfuAppError(logMsg);
            }

            MessageHandler.DebugExit(hidReturnVal);
            return hidReturnVal;
        }

        /// <summary>
        /// Send Disconnect to device(s) over HID
        /// </summary>
        private void DisconnectDevices()
        {
            MessageHandler.DebugEntry();

            if (HidDfuAPI.HidDfu.hidDfuDisconnect() == (Int32)ErrorCodes.HIDDFU_ERROR_NONE)
            {
                mCount = 0;
            }
            else
            {
                string logMsg = String.Format("ERROR (when disconnecting): ") + Marshal.PtrToStringAnsi(HidDfu.hidDfuGetLastError());
                HidDfuAppLog(logMsg);
            }
            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Returns the backup file name
        /// If more than 1 devices are connected, for backup operation the device name is appended with a "-"
        /// followed by device number (0, 1, 2...) e.g. backup-0.dfu, backup-1.dfu.
        /// </summary>
        /// <param name="aDevIndex">Number to append in backup file name</param>
        /// <returns>Backup file name</returns>
        private String GetBackupFileName(int aDevIndex)
        {
            MessageHandler.DebugEntry();
            string backupFileName = mFileName;

            // Append the suffix if there are more than 1 devices
            if (mCount > 1)
            {
                backupFileName = backupFileName.Insert(backupFileName.LastIndexOf("."), "-" + aDevIndex.ToString());
            }

            MessageHandler.DebugExit(backupFileName);
            return backupFileName;
        }

        /// <summary>
        /// Display an error, and exit if requested.
        /// </summary>
        /// <param name="aErrorMsg">Error Message for display using a dialog box</param>
        /// <param name="aAbort">Exit if true</param>
        private void HidDfuAppError(string aErrorMsg, bool aAbort = false)
        {
            MessageBox.Show(aErrorMsg, "Error", MessageBoxButtons.OK,
                MessageBoxIcon.Error);

            if (aAbort)
            {
                Close();
                System.Environment.Exit(1);
            }
        }

        /// <summary>
        /// Display log message
        /// </summary>
        /// <param name="aLogMsg">Message for display in text box</param>
        private void HidDfuAppLog(string aLogMsg)
        {
            MessageHandler.DebugBasic(aLogMsg);
            // Are we running as the non-GUI thread?
            if (txtBoxLog.InvokeRequired)
            {   // Ask the GUI thread to do this for us
                this.BeginInvoke((MethodInvoker)delegate
                {
                    txtBoxLog.AppendText(aLogMsg + Environment.NewLine);
                });
            }
            else
            {   // We are the GUI thread, so just do the command
                txtBoxLog.AppendText(aLogMsg + Environment.NewLine);
            }
        }

        /// <summary>
        /// Get the registry variables for the application
        /// </summary>
        private void GetRegistry()
        {
            MessageHandler.DebugEntry();

            if (!mUpdatingCommand)
            {
                cmbBoxCommand.SelectedValue = Enum<CommandType>.GetEnumFromDescription(
                        mRegistry.GetRegistry(REG_COMMAND,
                        Enum<CommandType>.GetDescriptionFromEnum(CommandType.UpgradeBin)));
            }

            if (mCommand == CommandType.UpgradeBin)
            {
                txtBoxFileName.Text = mRegistry.GetRegistry(REG_APPLICATION_FILE, null);
                numUpDnVid.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_APPLICATION_VID, DEFAULT_APPLICATION_VID));
                numUpDnPid.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_APPLICATION_PID, DEFAULT_APPLICATION_PID));
                numUpDnUsage.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_APPLICATION_USAGE, DEFAULT_APPLICATION_USAGE));
                numUpDnUsagePage.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_APPLICATION_USAGEPAGE, DEFAULT_APPLICATION_USAGE_PAGE));
            }
            else
            {
                txtBoxFileName.Text = mRegistry.GetRegistry(REG_LOADER_FILE, null);
                numUpDnVid.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_LOADER_VID, DEFAULT_LOADER_VID));
                numUpDnPid.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_LOADER_PID, DEFAULT_LOADER_PID));
                numUpDnUsage.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_LOADER_USAGE, DEFAULT_LOADER_USAGE));
                numUpDnUsagePage.Value = Convert.ToDecimal(
                        mRegistry.GetRegistry(REG_LOADER_USAGEPAGE, DEFAULT_LOADER_USAGE_PAGE));
                chkBoxReset.Checked = Convert.ToBoolean(
                    mRegistry.GetRegistry(REG_LOADER_RESETAFTER, DEFAULT_LOADER_RESET_AFTER));
            }

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Check the progress on timer expiry
        /// </summary>
        private void OnTimedEvent(object aSender, ElapsedEventArgs aArgs)
        {
            UInt16 progress = HidDfu.hidDfuGetProgress();
            Int32 hidReturnVal = HidDfu.hidDfuGetResult();

            this.BeginInvoke((MethodInvoker)delegate
            {
                btnStop.Enabled = (progress > 0 && progress < 100);

                if (mStopped)
                {
                    mTimer.Stop();
                    prgBar.Value = 0;
                    RunPostOperation();
                }
                else if (progress == 100)
                {
                    mTimer.Stop();
                    prgBar.Value = progress;

                    if (hidReturnVal == (Int32)ErrorCodes.HIDDFU_ERROR_NONE)
                    {
                        if (mCommand == CommandType.BackUp)
                        {   // For backup operation, display file name(s)
                            HidDfuAppLog("File(s) backed up as:");
                            for (UInt16 i = 0; i < mCount; i++)
                            {
                                HidDfuAppLog(GetBackupFileName(i));
                            }
                        }

                        HidDfuAppLog("Successfully completed \'" + Enum<CommandType>.GetDescriptionFromEnum(mCommand) + "\' operation.");
                    }
                    else
                    {
                        prgBar.Value = 0;
                        string logMsg = String.Format("ERROR: ") + Marshal.PtrToStringAnsi(HidDfu.hidDfuGetLastError());
                        HidDfuAppLog(logMsg);
                        HidDfuAppError(logMsg);
                    }

                    RunPostOperation();
                }
                else
                {
                    prgBar.Value = progress;

                    // The sequence of Upgrading CSRA681xx, QCC302x-8x and QCC512x-8x devices with binary image
                    // necessitates restart after the image has been fully copied, the DLL returns 95 as progress
                    // when the image has been copied and transfer complete message is sent to device to initiate reboot.
                    // The device is reconnected to after a short delay, and more messages exchanged with device to commit the image.
                    // So display a relevant message for the restart duration.
                    if (!mWaitMsgPrinted && (progress == 95) && (mCommand == CommandType.UpgradeBin))
                    {
                        mWaitMsgPrinted = true;
                        HidDfuAppLog("Device(s) rebooting, waiting for 60 seconds...");
                    }
                }
            });
        }

        /// <summary>
        /// Display the version(if applicable), and
        /// call HID DLL's API for upgrade / backup
        /// </summary>
        /// <returns>HID DLL's API error code</returns>
        private Int32 RunCommand()
        {
            MessageHandler.DebugEntry();

            Int32 hidReturnVal = ConnectDevices();

            if ((mCount > 0) && (hidReturnVal == (Int32)ErrorCodes.HIDDFU_ERROR_NONE))
            {
                // Check Version
                if (mCommand == CommandType.UpgradeBin)
                {
                    ShowVersionMsg(false, "before upgrade");
                }

                bool proceedWithOperation = true;
                // Run Command
                if (!String.IsNullOrEmpty(mFileName))
                {
                    HidDfuAppLog("Starting the \'"
                            + Enum<CommandType>.GetDescriptionFromEnum(mCommand) + "\' operation...");

                    if (mCommand == CommandType.Upgrade)
                    {
                        hidReturnVal = HidDfu.hidDfuUpgrade(mFileName, Convert.ToByte(mResetAfter));
                    }
                    else if (mCommand == CommandType.BackUp)
                    {
                        // Raise warning if file(s) already exist
                        if (BackupFilesExist())
                        {
                            string warningMsg = (mCount == 1)
                                    ? String.Format("{0} already exists. Do you want to replace?",
                                        Path.GetFileName(mFileName))
                                    : String.Format("One or more files in the range {0}-<0..{1}>.dfu already exists. Do you want to replace?",
                                       Path.GetFileNameWithoutExtension(mFileName), mCount - 1);

                            DialogResult dialogResult = MessageBox.Show(warningMsg, AssemblyInfo.Title(), MessageBoxButtons.YesNo);

                            if (dialogResult == DialogResult.No)
                            {
                                warningMsg += " No";
                                hidReturnVal = (Int32)ErrorCodes.HIDDFU_ERROR_UNKNOWN;
                                proceedWithOperation = false;
                            }
                            else
                            {
                                warningMsg += " Yes";
                            }
                            HidDfuAppLog(warningMsg);
                        }

                        if (proceedWithOperation)
                        {
                            hidReturnVal = HidDfu.hidDfuBackup(mFileName, Convert.ToByte(mResetAfter));
                        }
                    }
                    else if (mCommand == CommandType.UpgradeBin)
                    {
                        hidReturnVal = HidDfu.hidDfuUpgradeBin(mFileName);
                    }
                }

                if ((hidReturnVal != (Int32)ErrorCodes.HIDDFU_ERROR_NONE) && proceedWithOperation)
                {
                    string logMsg = String.Format("ERROR: ") + Marshal.PtrToStringAnsi(HidDfu.hidDfuGetLastError());
                    HidDfuAppLog(logMsg);
                    HidDfuAppError(logMsg);
                }
            }

            MessageHandler.DebugExit(hidReturnVal);
            return hidReturnVal;
        }

        /// <summary>
        /// Display the version(if applicable), disconnect and reset the states
        /// </summary>
        private void RunPostOperation()
        {
            MessageHandler.DebugEntry();

            Int32 hidReturnVal = HidDfu.hidDfuGetResult();

            if (hidReturnVal == (Int32)ErrorCodes.HIDDFU_ERROR_OPERATION_PARTIAL_SUCCESS
                || hidReturnVal == (Int32)ErrorCodes.HIDDFU_ERROR_OP_PARTIAL_SUCCESS_NO_RESPONSE)
            {
                string logMsg = String.Format("Error on {0} device(s).", HidDfu.hidDfuGetFailedDevicesCount());
                HidDfuAppLog(logMsg);
            }

            if ((hidReturnVal != (Int32)ErrorCodes.HIDDFU_ERROR_NO_RESPONSE)
                && (hidReturnVal != (Int32)ErrorCodes.HIDDFU_ERROR_OP_PARTIAL_SUCCESS_NO_RESPONSE))
            {
                if ((mCount > 0)
                        && !mStopped
                        && (mCommand == CommandType.UpgradeBin))
                {
                    // Check Version
                    ShowVersionMsg(true, "after upgrade");
                }
            }

            DisconnectDevices();

            mStopped = false;
            mWaitMsgPrinted = false;
            SetControlsState(true);

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Select file for upgrade or backup
        /// </summary>
        private void SelectFile()
        {
            MessageHandler.DebugEntry();
            using (OpenFileDialog ofd = new OpenFileDialog())
            {
                try
                {   // Try to get from registry
                    ofd.InitialDirectory = System.IO.Path.GetDirectoryName(mRegistry.GetRegistry(REG_FILE_INPUT_PATH, Application.ExecutablePath));
                }
                catch
                {   // Any problems, use exe folder
                    ofd.InitialDirectory = System.IO.Path.GetDirectoryName(Application.ExecutablePath);
                }

                ofd.Filter = (mCommand == CommandType.UpgradeBin) ? STR_BIN_FILE_DIALOG_FILTER : STR_DFU_FILE_DIALOG_FILTER;
                ofd.CheckFileExists = (mCommand != CommandType.BackUp);

                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtBoxFileName.Text = ofd.FileName;
                    mRegistry.SetRegistry(REG_FILE_INPUT_PATH, ofd.FileName);

                    mFileName = txtBoxFileName.Text;
                    btnRun.Enabled = true;
                }
            }
            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Switch state of control when the operation is running/stopped
        /// </summary>
        /// <param name="aIdle">true(disabled) if operation is stopped, false otherwise</param>
        private void SetControlsState(bool aIdle)
        {
            MessageHandler.DebugEntry();
            btnStop.Enabled = !aIdle;
            btnRun.Enabled = aIdle;
            cmbBoxCommand.Enabled = aIdle;
            txtBoxFileName.Enabled = aIdle;
            btnFileName.Enabled = aIdle;
            numUpDnVid.Enabled = aIdle;
            numUpDnPid.Enabled = aIdle;
            numUpDnUsage.Enabled = aIdle;
            numUpDnUsagePage.Enabled = aIdle;
            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Set defaults for the application
        /// </summary>
        private void SetDefaults()
        {
            MessageHandler.DebugEntry();

            cmbBoxCommand.DisplayMember = "Value";
            cmbBoxCommand.ValueMember = "Key";
            cmbBoxCommand.DataSource = Enum<CommandType>.GetListFromEnum();

            btnRun.Enabled = !String.IsNullOrEmpty(mFileName);
            btnStop.Enabled = false;

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Set registry settings for the application
        /// </summary>
        private void SetRegistry()
        {
            MessageHandler.DebugEntry();
            mRegistry.SetRegistry(REG_COMMAND, cmbBoxCommand.Text);

            if (mCommand == CommandType.UpgradeBin)
            {
                mRegistry.SetRegistry(REG_APPLICATION_FILE, txtBoxFileName.Text);
                mRegistry.SetRegistry(REG_APPLICATION_VID, Convert.ToUInt16(numUpDnVid.Value));
                mRegistry.SetRegistry(REG_APPLICATION_PID, Convert.ToUInt16(numUpDnPid.Value));
                mRegistry.SetRegistry(REG_APPLICATION_USAGE, Convert.ToUInt16(numUpDnUsage.Value));
                mRegistry.SetRegistry(REG_APPLICATION_USAGEPAGE, Convert.ToUInt16(numUpDnUsagePage.Value));
            }
            else
            {
                mRegistry.SetRegistry(REG_LOADER_FILE, txtBoxFileName.Text);
                mRegistry.SetRegistry(REG_LOADER_VID, Convert.ToUInt16(numUpDnVid.Value));
                mRegistry.SetRegistry(REG_LOADER_PID, Convert.ToUInt16(numUpDnPid.Value));
                mRegistry.SetRegistry(REG_LOADER_USAGE, Convert.ToUInt16(numUpDnUsage.Value));
                mRegistry.SetRegistry(REG_LOADER_USAGEPAGE, Convert.ToUInt16(numUpDnUsagePage.Value));
                mRegistry.SetRegistry(REG_LOADER_RESETAFTER, Convert.ToUInt16(chkBoxReset.Checked));
            }
            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Display device firmware version information
        /// </summary>
        /// <param name="aCheckMatch">Check if all the devices have the same version</param>
        /// <param name="aMessage">Text message for display with version information</param>
        private void ShowVersionMsg(bool aCheckMatch, string aMessage)
        {
            MessageHandler.DebugEntry();

            // Calculate length to allocate memory for -
            // Major, Minor and Config Version,
            // The 3 above are UInt16 which will take maximum 5 characters, so for 3 UInt16's allocate 15 character
            // and each version number is followed by a comma/semicolon so allocate another 3 for each version numbers
            // which makes it a total of 18 character for 1 device.
            const UInt16 MULTIPLIER = 18;
            UInt16 length = (UInt16)((mCount * MULTIPLIER) + 1);
            StringBuilder versionStrBuf = new StringBuilder(length);

            Int32 hidReturnVal = HidDfuAPI.HidDfu.hidDfuGetFirmwareVersions(versionStrBuf, out length, Convert.ToByte(aCheckMatch));

            if (hidReturnVal == (Int32)ErrorCodes.HIDDFU_ERROR_NONE)
            {
                const Int16 COL_SIZE = 20;

                string logMsg = Environment.NewLine;
                logMsg += "-----------------------------------------------------------------------------";
                logMsg += Environment.NewLine;
                logMsg += "DEVICE INFORMATION (" + aMessage + ")";
                logMsg += Environment.NewLine;
                logMsg += "-----------------------------------------------------------------------------";

                string[] deviceInfo = { "Device # ", "Version Major = ", "Version Minor = ", "Config Version = " };
                string version = versionStrBuf.ToString();

                // Print version
                Int16 counter = 1;
                string[] devicesVersion = version.Remove(version.LastIndexOf(';')).Split(';');

                foreach (string deviceVersion in devicesVersion)
                {
                    logMsg += Environment.NewLine;
                    // Insert device number initially, and
                    // after printing set of 3 numbers reperesenting version information
                    logMsg += deviceInfo[0] + counter.ToString().PadRight(COL_SIZE);
                    logMsg += Environment.NewLine;
                    string[] ver = deviceVersion.Split(',');
                    int i = 1;
                    foreach (string v in ver)
                    {
                        logMsg += deviceInfo[i++] + v.PadRight(COL_SIZE);
                        logMsg += Environment.NewLine;
                    }
                    // After printing set of 3 numbers reperesenting version information,
                    // insert a new line
                    logMsg += "-----------------------------------------------------------------------------";
                    counter++;
                }

                HidDfuAppLog(logMsg);
            }
            MessageHandler.DebugExit();
        }

#endregion  // Support methods

        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------

#region Events

        private void btnFileName_Click(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            SelectFile();
            mOptionControlModified = true;
            MessageHandler.DebugExit();
        }

        private void btnHelp_Click(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            // Invoke the help system with a path that is relative to the installed location
            string path = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(Application.ExecutablePath), @"help");
            string helpFile = System.IO.Path.Combine(path, @"HidDfuTools.chm");

            if (File.Exists(helpFile))
            {
                Help.ShowHelp(this, helpFile, "hiddfuapp.htm");
            }
            else
            {
                // No installed help, may indicate we are running from a build folder rather
                // than an installation folder. See if we can reach the help build folder...
                string altHelpFile = System.IO.Path.Combine(path, @"..\..\..\..\help\HidDfuTools.chm");
                if (File.Exists(altHelpFile))
                {
                    helpFile = altHelpFile;
                    Help.ShowHelp(this, helpFile, "hiddfuapp.htm");
                }
                else
                {
                    string msg = "Could not find the help file :\n\n" + helpFile;
                    MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            MessageHandler.DebugExit();
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();

            if (!String.IsNullOrEmpty(mFileName)
                && !String.IsNullOrEmpty(numUpDnVid.Text)
                && !String.IsNullOrEmpty(numUpDnPid.Text)
                && !String.IsNullOrEmpty(numUpDnUsage.Text)
                && !String.IsNullOrEmpty(numUpDnUsagePage.Text))
            {
                prgBar.Value = 0;

                if (RunCommand() == (Int32)ErrorCodes.HIDDFU_ERROR_NONE)
                {
                    mTimer.Start();
                    SetControlsState(false);
                }
                else
                {
                    DisconnectDevices();
                }
            }
            else
            {
                string logMsg = "Error: Value(s) not set for running the operation.";
                HidDfuAppLog(logMsg);
                HidDfuAppError(logMsg);
            }

            MessageHandler.DebugExit();
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();

            string logMsg = Environment.NewLine + String.Format("Stopping '{0}' operation...", Enum<CommandType>.GetDescriptionFromEnum(mCommand));
            HidDfuAppLog(logMsg);
            logMsg = String.Empty;

            // Wait time (in milliseconds) for operation to stop
            // The start of the operation usually takes 40 seconds, if the stop is sent during
            // this time, device is not able to handle it, so wait for 40 seconds to let it complete
            const UInt16 WAIT_TIMER_MS = 40000;

            Int32 hidReturnVal = HidDfu.hidDfuStop(WAIT_TIMER_MS);
            if (hidReturnVal == (Int32)ErrorCodes.HIDDFU_ERROR_NONE)
            {
                logMsg += String.Format("Successfully stopped '{0}' operation.", Enum<CommandType>.GetDescriptionFromEnum(mCommand))
                        + Environment.NewLine;
            }
            else
            {
                logMsg += Marshal.PtrToStringAnsi(HidDfu.hidDfuGetLastError());
            }

            mStopped = true;

            HidDfuAppLog(logMsg);
            MessageHandler.DebugExit();
        }

        private void chkBoxReset_CheckedChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mResetAfter = chkBoxReset.Checked;
            MessageHandler.DebugExit();
        }

        private void cmbBoxCommand_SelectedIndexChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mCommand = Enum<CommandType>.GetEnumFromDescription(cmbBoxCommand.Text);

            if (mCommand == CommandType.UpgradeBin)
            {
                chkBoxReset.Enabled = false;
            }
            else
            {
                chkBoxReset.Enabled = true;
            }

            if (!mOptionControlModified)
            {
                mUpdatingCommand = true;
                GetRegistry();
                mUpdatingCommand = false;
                mOptionControlModified = false;
            }

            MessageHandler.DebugExit();
        }

        private void MainPage_FormClosing(object sender, FormClosingEventArgs e)
        {
            MessageHandler.DebugEntry();
            SetRegistry();
            MessageHandler.DebugExit();
        }

        private void MainPage_MouseClick(object sender, MouseEventArgs e)
        {
            MessageHandler.DebugEntry();
            if (e.Button == MouseButtons.Right)
            {
                ShowAboutBox();
            }
            MessageHandler.DebugExit();
        }

        private void MainPage_Shown(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            // ToolTip
            mToolTip = new ToolTip();
            mToolTip.AutoPopDelay = 10000;
            mToolTip.InitialDelay = 500;
            mToolTip.ReshowDelay = 500;
            mToolTip.ShowAlways = true;

            mToolTip.SetToolTip(this.chkBoxReset, "Perform a device reset, after Upgrade/Backup, only applicable to BlueCore ICs");
            mToolTip.SetToolTip(this.cmbBoxCommand,
                    "Upgrade/Backup options are only applicable to BlueCore ICs, Upgrade Binary is only applicable to Combo Digital Architecture ICs");
            mToolTip.SetToolTip(this.txtBoxFileName, "DFU file name");
            mToolTip.SetToolTip(this.numUpDnVid, "USB Vendor ID");
            mToolTip.SetToolTip(this.numUpDnPid, "USB Product ID");
            mToolTip.SetToolTip(this.numUpDnUsage, "USB Usage value (can be set to zero to ignore)");
            mToolTip.SetToolTip(this.numUpDnUsagePage, String.Format(
                    "USB Usage Page value (can be set to zero to ignore for BlueCore ICs, should be set to 0x{0} for Combo Digital Architecture ICs)",
                    DEFAULT_APPLICATION_USAGE_PAGE));

            string logMsg = "Note:";
            logMsg += Environment.NewLine;
            logMsg += "The commands 'Backup' and 'Upgrade', and the option 'Reset After', are only applicable to BlueCore ICs.";
            logMsg += Environment.NewLine;
            logMsg += "The command 'Upgrade Binary' is only applicable to CSRA681xx, QCC302x-8x and QCC512x-8x devices.";
            logMsg += Environment.NewLine;
            logMsg += "For 'Upgrade Binary' the following apply:";
            logMsg += Environment.NewLine;
            logMsg += "    'Usage' should be 0x1 for QCC304x-8x and QCC514x-8x devices.";
            logMsg += Environment.NewLine;
            logMsg += "    'Usage Page' should be 0xFF00.";
            HidDfuAppLog(logMsg);

            GetRegistry();
            mOptionControlModified = false;
            MessageHandler.DebugExit();
        }

        private void numUpDnPid_ValueChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mPid = Convert.ToUInt16(numUpDnPid.Value);
            mOptionControlModified = true;
            MessageHandler.DebugExit();
        }

        private void numUpDnUsage_ValueChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mUsage = Convert.ToUInt16(numUpDnUsage.Value);
            mOptionControlModified = true;
            MessageHandler.DebugExit();
        }

        private void numUpDnUsagePage_ValueChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mUsagePage = Convert.ToUInt16(numUpDnUsagePage.Value);
            mOptionControlModified = true;
            MessageHandler.DebugExit();
        }

        private void numUpDnVid_ValueChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mVid = Convert.ToUInt16(numUpDnVid.Value);
            mOptionControlModified = true;
            MessageHandler.DebugExit();
        }

        private void txtBoxFileName_TextChanged(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();
            mFileName = txtBoxFileName.Text;
            btnRun.Enabled = !String.IsNullOrEmpty(mFileName);
            mOptionControlModified = true;
            MessageHandler.DebugExit();
        }

#endregion  // Events
    } //end partial class MainForm
} // end namespace QTIL.HostTools.HidDfuApp
