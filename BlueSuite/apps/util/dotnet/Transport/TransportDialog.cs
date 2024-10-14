//------------------------------------------------------------------------------
//
// <copyright file="TransportDialog.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2022 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Text;
using System.Windows.Forms;

using Microsoft.Win32;

namespace QTIL.HostTools.Common.Transport
{
    public partial class TransportDialog : Form, ITransport
    {

        private const string RegistryPath = @"Software\QTIL\Development Kit\{0}\Connection History";

        #region ITransport

        private uint mBaudRate;

        /// <summary>
        /// Gets the baud rate.
        /// </summary>
        [CLSCompliant(false)]
        public uint BaudRate
        {
            get
            {
                return mBaudRate;
            }
        }

        private readonly IPAddress mIPAddress;

        /// <summary>
        /// Gets the IP address.
        /// </summary>
        public IPAddress IPAddress
        {
            get
            {
                return mIPAddress;
            }
        }

        private Transport mSelectedDevice;

        /// <summary>
        /// Gets the port.
        /// </summary>
        public string Port
        {
            get
            {
                return mSelectedDevice.Port;
            }
        }

        /// <summary>
        /// Gets the trans.
        /// </summary>
        public string Trans
        {
            get
            {
                string transStr = "";
                switch (mTransportType)
                {
                    case TransportTypes.None:
                    case TransportTypes.Serial:
                    case TransportTypes.Any:
                        // Should never get here
                        break;

                    case TransportTypes.BCSP:
                    case TransportTypes.H4:
                    case TransportTypes.H5:
                    case TransportTypes.H4DS:
                        transStr = mSelectedDevice.Trans;
                        break;

                    case TransportTypes.USB:
                        transStr = mSelectedDevice.Trans;
                        break;

                    case TransportTypes.SPI:
                        if (mSelectedDevice.IsRemoteSPI)
                        {
                            transStr = string.Format(CultureInfo.InvariantCulture, "SPITRANS=REMOTE SPIREMOTEIP={0}", mIPAddress);
                        }
                        else
                        {
                            transStr = mSelectedDevice.Trans;
                        }
                        break;

                    case TransportTypes.PTAP:
                    case TransportTypes.TRB:
                    case TransportTypes.USBDBG:
                    case TransportTypes.USBCC:
                    case TransportTypes.ADBBT:
                        transStr = mSelectedDevice.Trans;
                        break;
                }
                return transStr;
            }
        }

        private TransportTypes mTransportType = TransportTypes.SPI;

        /// <summary>
        /// Gets the type.
        /// </summary>
        public TransportTypes Type
        {
            get
            {
                return mTransportType;
            }
        }

        #endregion

        #region DisplaySupport

        // the name to display that means a debug tranport via a PtTransport plugin (more general then SPI)
        const String strSpiName = "DEBUG";

        private TransportTypes TransportTypeFromName(String aName)
        {
            TransportTypes theTransportType = TransportTypes.None;

            switch (aName)
            {
                case "BCSP": theTransportType = TransportTypes.BCSP; break;
                case "USB": theTransportType = TransportTypes.USB; break;
                case "H4": theTransportType = TransportTypes.H4; break;
                case "H5": theTransportType = TransportTypes.H5; break;
                case "H4DS": theTransportType = TransportTypes.H4DS; break;
                case "SPI": theTransportType = TransportTypes.SPI; break;
                case strSpiName: theTransportType = TransportTypes.SPI; break;
                case "PTAP": theTransportType = TransportTypes.PTAP; break;
                case "TRB": theTransportType = TransportTypes.TRB; break;
                case "USBDBG": theTransportType = TransportTypes.USBDBG; break;
                case "USBCC": theTransportType = TransportTypes.USBCC; break;
                case "ADBBT": theTransportType = TransportTypes.ADBBT; break;
            }

            return theTransportType;
        }

        private String TransportTypeToString(TransportTypes aTransportType)
        {
            String theName = "None";

            switch (aTransportType)
            {
                case TransportTypes.BCSP: theName = "BCSP"; break;
                case TransportTypes.USB: theName = "USB"; break;
                case TransportTypes.H4: theName = "H4"; break;
                case TransportTypes.H5: theName = "H5"; break;
                case TransportTypes.H4DS: theName = "H4DS"; break;
                case TransportTypes.SPI: theName = strSpiName; break;
                case TransportTypes.PTAP: theName = "PTAP"; break;
                case TransportTypes.TRB: theName = "TRB"; break;
                case TransportTypes.USBDBG: theName = "USBDBG"; break;
                case TransportTypes.USBCC: theName = "USBCC"; break;
                case TransportTypes.ADBBT: theName = "ADBBT"; break;
            }

            return theName;
        }

        private string SuppressFromSpiList
        {
            get
            {
                string suppressed = "";

                if ((TransportTypesAllowed & TransportTypes.SPI) == 0)
                {
                    suppressed += "TRANS=LPT,";
                    suppressed += "TRANS=USB ,"; // just USB
                    suppressed += "TRANS=SDIO,"; // includes SDIO,SDIOCE,SDIOEMB
                    suppressed += "TRANS=KALSIM,";
                    suppressed += "TRANS=XAPSIM,";
                    suppressed += "TRANS=CMSIS,";
                }

                if ((TransportTypesAllowed & TransportTypes.TRB) == 0)
                {
                    suppressed += "TRANS=TRB,"; // includes TRB,TRBTC
                }

                if ((TransportTypesAllowed & TransportTypes.USBDBG) == 0)
                {
                    suppressed += "TRANS=USBDBG,";
                }

                if ((TransportTypesAllowed & TransportTypes.USBCC) == 0)
                {
                    suppressed += "TRANS=USBCC,";
                }

                if ((TransportTypesAllowed & TransportTypes.ADBBT) == 0)
                {
                    suppressed += "TRANS=ADBBT,";
                }

                return suppressed;
            }
        }

        private bool ShouldIncludeSpi(string transString)
        {
            bool keepIt = true;
            string[] ignoreList = SuppressFromSpiList.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
            foreach (string item in ignoreList)
            {
                if (transString.Contains(item))
                {
                    keepIt = false;
                    return keepIt;
                }
            }
            return keepIt;
        }

        private bool TransportIsShown(TransportTypes aTransportType)
        {
            // default case is show if enabled
            bool shouldShow = (aTransportType & mTransportTypesAllowed) == aTransportType;

            // exception: show if SPI and any of SPI/TRB/USBDBG/USBCC/ADBBT was requested
            if (aTransportType == TransportTypes.SPI)
            {
                shouldShow = (mTransportTypesAllowed & (TransportTypes.TRB | TransportTypes.USBDBG | 
                    TransportTypes.USBCC | TransportTypes.ADBBT | TransportTypes.SPI)) != 0;
            }

            // exception: do not show separate TRB/USBDBG/USBCC/ADBBT cases
            if ((aTransportType == TransportTypes.TRB) || (aTransportType == TransportTypes.USBDBG) ||
                 (aTransportType == TransportTypes.USBCC) || (aTransportType == TransportTypes.ADBBT))
            {
                shouldShow = false;
            }

            return shouldShow;
        }

        #endregion

        #region OptionalControls

        /// <summary>
        /// Extends transport dialog by adding a named checkbox
        /// (can be called multiple times if multiple extra options are needed)
        /// </summary>
        /// <param name="aBoolCtrlName">What name to give the control eg ChkStopChip</param>
        /// <param name="aLabel">The user visible text for this control eg "Halt chip on connection"</param>
        /// <param name="aBoolInitialValue">Whether option defaults to True or False</param>
        public void AddBoolOption(string aBoolCtrlName, string aLabel, bool aBoolInitialValue)
        {
            this.SuspendLayout();
            System.Drawing.Size size = this.ClientSize;

            // create a checkbox control at the bottom of the form
            System.Windows.Forms.CheckBox checkBoxBoolCtrl = new System.Windows.Forms.CheckBox();
            checkBoxBoolCtrl.AutoSize = true;
            checkBoxBoolCtrl.Checked = aBoolInitialValue;
            checkBoxBoolCtrl.Location = new System.Drawing.Point(10, size.Height);
            checkBoxBoolCtrl.Name = aBoolCtrlName;
            checkBoxBoolCtrl.Text = aLabel;
            checkBoxBoolCtrl.UseVisualStyleBackColor = true;
            checkBoxBoolCtrl.Visible = true;

            // add the control, and resize the form to make it visible
            this.Controls.Add(checkBoxBoolCtrl);
            size.Height = size.Height + checkBoxBoolCtrl.Height + 1;
            this.ClientSize = size;

            this.ResumeLayout(false);
            this.PerformLayout();
        }

        /// <summary>
        /// Request the state of a named boolean option (a checkbox control)
        /// where the option was previously added with AddBoolOption()
        /// </summary>
        /// <param name="aBoolCtrlName">The name of the option to read</param>
        /// <returns>True if requested checkbox control exists and is checked, False otherwise</returns>
        public bool GetBoolOption(string aBoolCtrlName)
        {
            bool retVal = false;

            // starting from last control and working backwards
            // (because optional controls are most likely to be on the end)
            // Try to locate a checkbox with the requested name
            for (int i = this.Controls.Count - 1; i >= 0; --i)
            {
                var control = this.Controls[i] as System.Windows.Forms.CheckBox;
                if (control?.Name == aBoolCtrlName)
                {
                    retVal = control.Checked;
                    break;
                }
            }
            return retVal;
        }

        #endregion

        #region Properties

        /// <summary>
        /// true when program is setting values on the dialog, false during a user edit
        /// </summary>
        private bool mIsProgramStuffingValues = false;

        private TransportTypes mTransportTypesAllowed = TransportTypes.SPI;

        /// <summary>
        /// Gets or sets the transport types allowed to be selected from the dialog.
        /// </summary>
        /// <value>
        /// The transport types allowed.
        /// </value>
        public TransportTypes TransportTypesAllowed
        {
            get
            {
                return mTransportTypesAllowed;
            }
            set
            {
                mTransportTypesAllowed = value;
            }
        }

        private string mApplicationName;

        /// <summary>
        /// Gets or sets the name of the application.
        /// </summary>
        /// <value>
        /// The name of the application.
        /// </value>
        /// <remarks>
        /// Used to set the subkey in the registry for connection history.
        /// Connection history disabled if this not set.
        /// </remarks>
        public string ApplicationName
        {
            get
            {
                return mApplicationName;
            }
            set
            {
                mApplicationName = value;
                HistoryMenuItem.Enabled = !string.IsNullOrEmpty(mApplicationName);
            }
        }

        #endregion

        #region PTAP host interfaces

        /// <summary>
        /// Gets the available PTAP host interfaces.
        /// </summary>
        private void getAvailablePtapHifs()
        {
            int status;
            ushort maxLength = 1024;
            ushort count = 0;
            StringBuilder sbHifIds = new StringBuilder(maxLength);
            StringBuilder sbTypes = new StringBuilder(maxLength);
            StringBuilder sbNames = new StringBuilder(maxLength);

            status = PTapLibrary.EnumerateHifs(ref maxLength, sbHifIds, sbTypes, sbNames, ref count);

            if (status == PTapLibrary.SHORT_STRINGS)
            {
                sbHifIds.Capacity = maxLength;
                sbTypes.Capacity = maxLength;
                sbNames.Capacity = maxLength;

                status = PTapLibrary.EnumerateHifs(ref maxLength, sbHifIds, sbTypes, sbNames, ref count);
            }

            if (status == PTapLibrary.OK)
            {
                char[] delimiters = {','};
                string[] hifIds = sbHifIds.ToString().Split(delimiters, StringSplitOptions.None);
                string[] types = sbTypes.ToString().Split(delimiters, StringSplitOptions.None);
                string[] names = sbNames.ToString().Split(delimiters, StringSplitOptions.None);

                for (uint index = 0; index < count; ++index)
                {
                    ComboBoxPorts.Items.Add(new Transport(hifIds[index], hifIds[index]));
                }
            }
        }

        #endregion

        #region SPI ports

        /// <summary>
        /// Gets the available SPI ports.
        /// </summary>
        private void getAvailableSPIPorts()
        {
            ushort arraySize = 1024; // Must NOT be const, passed ByRef into pttrans_enumerate_strings

            // Initialise the string buffers
            StringBuilder csvPorts = new StringBuilder(arraySize);
            StringBuilder csvTrans = new StringBuilder(arraySize);
            ushort count = 0;

            // Enumerate
            int status = NativeMethods.pttrans_enumerate_strings(ref arraySize, csvPorts, csvTrans, ref count);
            if ((status != 0) && (arraySize != 0))
            {
                // Failed due to buffer(s) being too small
                //  Resize buffers and re-run API
                csvPorts.Capacity = arraySize;
                csvTrans.Capacity = arraySize;
                status = NativeMethods.pttrans_enumerate_strings(ref arraySize, csvPorts, csvTrans, ref count);
            }

            if ((status == 0) && (count > 0))
            {
                string[] portsArray = csvPorts.ToString().Split(new char[] { ',' }, count, StringSplitOptions.RemoveEmptyEntries);
                string[] transArray = csvTrans.ToString().Split(new char[] { ',' }, count, StringSplitOptions.RemoveEmptyEntries);
                for (int i = 0; i <= (count - 1); i++)
                {
                    if (ShouldIncludeSpi(transArray[i]))
                    {
                        ComboBoxPorts.Items.Add(new Transport(portsArray[i], transArray[i]));
                    }
                }

                // If none found then this is handled in ComboBoxTransports.SelectedIndexChanged
            }
        }

        #endregion

        #region COM ports

        /// <summary>
        /// The supported baud rates
        /// </summary>
        private uint[] BaudRates = new uint[] { 
            1382400,
            921600,
            460800,
            230400,
            115200,
            57600,
            38400,
            19200,
            9600,
            4800,
            2400,
            1200,
            600,
            300,
            120
        };

        #endregion

        #region USB ports

        /// <summary>
        /// Determines whether the specified USB device is present.
        /// </summary>
        /// <param name="device">The device.</param>
        /// <returns>
        ///   <c>true</c> if the specified USB device is present; otherwise, <c>false</c>.
        /// </returns>
        private static bool isUSBDevicePresent(string device)
        {
            bool isPresent = false;

            IntPtr portHandle = NativeMethods.CreateFile(device, NativeMethods.GENERIC_READ | NativeMethods.GENERIC_WRITE, 0, IntPtr.Zero, NativeMethods.OPEN_EXISTING, NativeMethods.FILE_ATTRIBUTE_NORMAL, IntPtr.Zero);
            if (portHandle.ToInt32() != -1)
            {
                NativeMethods.CloseHandle(portHandle);
                isPresent = true;
            }

            return isPresent;
        }

        /// <summary>
        /// Gets the available USB devices.
        /// </summary>
        private void getAvailableUSBDevices()
        {
            const UInt16 MAX_USB_PORT = 127;
            int portIndex = 0;
            do
            {
                string device = string.Format(CultureInfo.CurrentCulture, @"\\.\csr{0}", portIndex);
                if (isUSBDevicePresent(device))
                {
                    ComboBoxPorts.Items.Add(new Transport(device, device));
                }
                portIndex++;
            }
            while (portIndex <= MAX_USB_PORT);
        }

        #endregion

        #region TRB ports

        /// <summary>
        /// Gets the available TRB adapters.
        /// </summary>
        private void getAvailableTrbAdapters()
        {
            Int32 status;
            UInt16 maxLength = 1024;
            UInt16 count = 0;
            StringBuilder sbNames = new StringBuilder(maxLength);
            StringBuilder sbTranss = new StringBuilder(maxLength);

            status = HydProtocols.EnumerateTrbs(ref maxLength, sbNames, sbTranss, ref count);

            if (status == HydProtocols.RESULT_SHORT_STRINGS)
            {
                sbNames.Capacity = maxLength;
                sbTranss.Capacity = maxLength;

                status = HydProtocols.EnumerateTrbs(ref maxLength, sbNames, sbTranss, ref count);
            }

            if (status == HydProtocols.RESULT_OK)
            {
                Char[] delimiters = { ',' };
                String[] names = sbNames.ToString().Split(delimiters, StringSplitOptions.None);
                String[] transs = sbTranss.ToString().Split(delimiters, StringSplitOptions.None);

                for (Int32 index = 0; index < count; ++index)
                {
                    ComboBoxPorts.Items.Add(new Transport("Adapter " + names[index], "SPIPORT=" + transs[index]));
                }
            }
        }

        #endregion

        #region USBDBG ports

        /// <summary>
        /// Gets the available USBDBG devices.
        /// </summary>
        private void getAvailableUsbDbgDevices()
        {
            Int32 status;
            UInt16 maxLength = 1024;
            UInt16 count = 0;
            StringBuilder sbNames = new StringBuilder(maxLength);
            StringBuilder sbTranss = new StringBuilder(maxLength);

            status = HydProtocols.EnumerateUsbDbgs(ref maxLength, sbNames, sbTranss, ref count);

            if (status == HydProtocols.RESULT_SHORT_STRINGS)
            {
                sbNames.Capacity = maxLength;
                sbTranss.Capacity = maxLength;

                status = HydProtocols.EnumerateUsbDbgs(ref maxLength, sbNames, sbTranss, ref count);
            }

            if (status == HydProtocols.RESULT_OK)
            {
                Char[] delimiters = {','};
                String[] names = sbNames.ToString().Split(delimiters, StringSplitOptions.None);
                String[] transs = sbTranss.ToString().Split(delimiters, StringSplitOptions.None);

                for (Int32 index = 0; index < count; ++index)
                {
                    ComboBoxPorts.Items.Add(new Transport("Device " + names[index], "SPIPORT=" + transs[index]));
                }
            }
        }

        #endregion

        #region USBCC ports

        /// <summary>
        /// Gets the available USBCC devices.
        /// </summary>
        private void getAvailableUsbCcDevices()
        {
            Int32 status;
            UInt16 maxLength = 1024;
            UInt16 count = 0;
            StringBuilder sbNames = new StringBuilder(maxLength);
            StringBuilder sbTranss = new StringBuilder(maxLength);

            status = HydProtocols.EnumerateUsbCcs(ref maxLength, sbNames, sbTranss, ref count);

            if (status == HydProtocols.RESULT_SHORT_STRINGS)
            {
                sbNames.Capacity = maxLength;
                sbTranss.Capacity = maxLength;

                status = HydProtocols.EnumerateUsbCcs(ref maxLength, sbNames, sbTranss, ref count);
            }

            if (status == HydProtocols.RESULT_OK)
            {
                Char[] delimiters = { ',' };
                String[] names = sbNames.ToString().Split(delimiters, StringSplitOptions.None);
                String[] transs = sbTranss.ToString().Split(delimiters, StringSplitOptions.None);

                for (Int32 index = 0; index < count; ++index)
                {
                    ComboBoxPorts.Items.Add(new Transport("Device " + names[index], "SPIPORT=" + transs[index]));
                }
            }
        }

        #endregion

        #region ADBBT ports

        /// <summary>
        /// Gets the available ADBBT devices.
        /// </summary>
        private void getAvailableAdbBtDevices()
        {
            Int32 status;
            UInt16 maxLength = 1024;
            UInt16 count = 0;
            StringBuilder sbNames = new StringBuilder(maxLength);
            StringBuilder sbTranss = new StringBuilder(maxLength);

            status = HydProtocols.EnumerateAdbBts(ref maxLength, sbNames, sbTranss, ref count);

            if (status == HydProtocols.RESULT_SHORT_STRINGS)
            {
                sbNames.Capacity = maxLength;
                sbTranss.Capacity = maxLength;

                status = HydProtocols.EnumerateAdbBts(ref maxLength, sbNames, sbTranss, ref count);
            }

            if (status == HydProtocols.RESULT_OK)
            {
                Char[] delimiters = { ',' };
                String[] names = sbNames.ToString().Split(delimiters, StringSplitOptions.None);
                String[] transs = sbTranss.ToString().Split(delimiters, StringSplitOptions.None);

                for (Int32 index = 0; index < count; ++index)
                {
                    ComboBoxPorts.Items.Add(new Transport("Device " + names[index], "SPIPORT=" + transs[index]));
                }
            }
        }

        #endregion

        #region Event handlers

        /// <summary>
        /// Handles the SelectedIndexChanged event of the ComboBoxTransports control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        /// <remarks>
        /// Update transport selection.
        /// </remarks>
        private void ComboBoxTransports_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (ComboBoxTransports.SelectedIndex >= 0)
            {
                // Set the selected transport to that selected in the combo box
                mTransportType = TransportTypeFromName(ComboBoxTransports.SelectedItem.ToString());

                // Clear other controls until we decide otherwise
                LabelPort.Text = "";
                ComboBoxPorts.Items.Clear();
                LabelOnlyDevice.Hide();
                LabelOption.Text = "";
                ComboBoxOption.Hide();
                ButtonOK.Enabled = false;
                Refresh();

                // The use the selected transport to fill in the next level
                switch (mTransportType)
                {
                    case TransportTypes.None:
                    case TransportTypes.Serial:
                    case TransportTypes.Any:
                        // Should never get here
                        break;

                    case TransportTypes.BCSP:
                    case TransportTypes.H4:
                    case TransportTypes.H5:
                    case TransportTypes.H4DS:
                        LabelPort.Text = "Serial Port";
                        SerialPortUtil.AddAvailablePorts(ComboBoxPorts);
                        break;

                    case TransportTypes.USB:
                        LabelPort.Text = "USB Device";
                        getAvailableUSBDevices();
                        break;

                    case TransportTypes.SPI:
                        LabelPort.Text = "Device";
                        getAvailableSPIPorts();
                        break;

                    case TransportTypes.PTAP:
                        LabelPort.Text = "HifId";
                        getAvailablePtapHifs();
                        break;

                    case TransportTypes.TRB:
                        LabelPort.Text = "Adapter";
                        getAvailableTrbAdapters();
                        break;

                    case TransportTypes.USBDBG:
                        LabelPort.Text = "Device";
                        getAvailableUsbDbgDevices();
                        break;

                    case TransportTypes.USBCC:
                        LabelPort.Text = "Device";
                        getAvailableUsbCcDevices();
                        break;

                    case TransportTypes.ADBBT:
                        LabelPort.Text = "Device";
                        getAvailableAdbBtDevices();
                        break;
                }

                // Show the available ports, and...
                // If there are no ports available, disable the control
                if (ComboBoxPorts.Items.Count == 0)
                {
                    // Float the dummy label over the front to keep colour the same
                    LabelOnlyDevice.Text = "None found";
                    LabelOnlyDevice.Show();
                    ComboBoxPorts.Enabled = false;
                }
                else if ((ComboBoxPorts.Items.Count == 1) && !mIsProgramStuffingValues)
                {   // There is only one port option, and it is a user edit (rather than the program filling all values)
                    // so 'disable' the control and autoselect the only available value

                    // Float a dummy label over the front of the disabled control (to make text appear enabled)
                    LabelOnlyDevice.Text = ComboBoxPorts.Items[0].ToString();
                    LabelOnlyDevice.Show();
                    ComboBoxPorts.SelectedIndex = 0;
                    ComboBoxPorts.Enabled = false;
                }
                else // enable the control so the user can select the appropriate port later
                {
                    ComboBoxPorts.Enabled = true;
                }
                ComboBoxPorts.Show();
            }
        }

        /// <summary>
        /// Handles the DropDown event of the ComboBoxPorts control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        /// <remarks>
        /// Update port selection
        /// </remarks>
        private void ComboBoxPorts_DropDown(object sender, EventArgs e)
        {
            if (ComboBoxTransports.SelectedIndex >= 0)
            {
                // Set the selected transport to that selected in the combo box
                mTransportType = TransportTypeFromName(ComboBoxTransports.SelectedItem.ToString());

                // Use the selected transport to fill in the next level
                switch (mTransportType)
                {
                    // Refresh the SPI list, incase if SPI's have changed (connected/disconnected)
                    case TransportTypes.SPI:
                        LabelPort.Text = "Device";
                        ComboBoxPorts.Items.Clear();
                        getAvailableSPIPorts();
                        break;
                }
            }
        }

        /// <summary>
        /// Handles the SelectedIndexChanged event of the ComboBoxPorts control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        /// <remarks>
        /// Update port selection
        /// </remarks>
        private void ComboBoxPorts_SelectedIndexChanged(object sender, EventArgs e)
        {
            mSelectedDevice = (Transport)ComboBoxPorts.SelectedItem;

            // Clear other controls until we decide otherwise
            LabelOption.Text = "";
            ButtonOK.Enabled = false;
            Refresh();

            // then switch on the selected transport to set the relevant variables
            //  and fill the next level if necessary
            switch (mTransportType)
            {
                case TransportTypes.None:
                case TransportTypes.Serial:
                case TransportTypes.Any:
                    // Should never get here
                    break;

                case TransportTypes.BCSP:
                case TransportTypes.H4:
                case TransportTypes.H5:
                case TransportTypes.H4DS:
                    // Initialise with available baudrates
                    ComboBoxOption.Items.Clear();
                    for (int i = 0; i <= BaudRates.GetUpperBound(0); i++)
                    {
                        ComboBoxOption.Items.Add(BaudRates[i].ToString(CultureInfo.CurrentCulture));
                    }

                    // Show the baud rates combo
                    LabelOption.Text = "Baud Rate";
                    ComboBoxOption.Show();
                    ComboBoxOption.SelectedItem = "38400";
                    break;

                case TransportTypes.USB:
                    // we have now completed selection so we can show the ok button
                    ComboBoxOption.Hide();
                    ButtonOK.Enabled = true;
                    break;

                case TransportTypes.SPI:
                    if (mSelectedDevice != null)
                    {
                        if (mSelectedDevice.IsRemoteSPI)
                        {
                            LabelOption.Text = "IP address";
                        }
                        else
                        {
                            // we have now completed selection so we can show the ok button
                            ComboBoxOption.Hide();
                            ButtonOK.Enabled = true;
                        }
                    }
                    break;

                case TransportTypes.PTAP:
                    ButtonOK.Enabled = true;
                    LabelOption.Text = "Connection";
                    // default to SDIO
                    ComboBoxOption.Items.Clear();
                    ComboBoxOption.Items.Add("SDIO");

                    // if this is just a PTAP editor, only allow SDIO, otherwise, allow the PTAP UART choices as well
                    if ((mTransportTypesAllowed & ~TransportTypes.PTAP) != 0)
                    {
                        // add available com ports
                        SerialPortUtil.AddAvailablePorts(ComboBoxOption);
                    }
                    ComboBoxOption.SelectedIndex = 0;
                    ComboBoxOption.Show();
                    break;

                case TransportTypes.TRB:
                case TransportTypes.USBDBG:
                case TransportTypes.USBCC:
                case TransportTypes.ADBBT:
                    // we have now completed selection so we can show the ok button
                    ComboBoxOption.Hide();
                    ButtonOK.Enabled = true;
                    break;
            }
        }

        /// <summary>
        /// Handles the SelectedIndexChanged event of the ComboBoxOption control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void ComboBoxOption_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (mTransportType != TransportTypes.PTAP)
            {
                mBaudRate = Convert.ToUInt32(ComboBoxOption.SelectedItem, CultureInfo.CurrentCulture);

                // we have now completed selection so we can show the ok button
                ButtonOK.Enabled = true;
            }
        }

        /// <summary>
        /// Handles the Click event of the ButtonOK control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void ButtonOK_Click(object sender, EventArgs e)
        {
            if (mTransportType == TransportTypes.PTAP)
            {
                // PTAP transport can connect either over SDIO only or using SDIO and an optional com port
                // if a com port has been selected append to hifid
                if (ComboBoxOption.SelectedIndex != 0)
                {
                    ((Transport)ComboBoxPorts.SelectedItem).Port += ":" + ComboBoxOption.SelectedItem;
                    // Append to the transport string too as it can be used instead of the port
                    ((Transport)ComboBoxPorts.SelectedItem).Trans += ":" + ComboBoxOption.SelectedItem;
                }
            }
          
            // Maintain the history menu
            if (HistoryMenuItem.Enabled)
            {
                UpdateTransportHistory();
            }

            DialogResult = DialogResult.OK;
            Close();
        }

        /// <summary>
        /// Handles the Click event of the ButtonCancel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void ButtonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }

        #endregion

        #region Transport history

        private readonly List<string> mHistory = new List<string>();

        private const ushort MAX_TRANS_HISTORY_ITEMS = 5;

        /// <summary>
        /// Gets the name of the registry subkey for use for connection history.
        /// </summary>
        /// <value>
        /// The name of the registry subkey.
        /// </value>
        private string SubkeyName
        {
            get
            {
                Debug.Assert(!string.IsNullOrEmpty(mApplicationName));
                return string.Format(CultureInfo.InvariantCulture, RegistryPath, mApplicationName);
            }
        }


        /// <summary>
        /// Add the transport selection to the history in the registry
        /// </summary>
        private void UpdateTransportHistory()
        {
            // Construct the string to save
            string transStr = string.Format(CultureInfo.CurrentCulture, "{0}|{1}", ComboBoxTransports.Text, ComboBoxPorts.Text);

            switch (mTransportType)
            {
                case TransportTypes.None:
                case TransportTypes.Serial:
                case TransportTypes.Any:
                    // Shouldn't get here
                    break;

                case TransportTypes.BCSP:
                case TransportTypes.H4:
                case TransportTypes.H5:
                case TransportTypes.H4DS:
                    transStr = transStr + string.Format(CultureInfo.CurrentCulture, "|{0}", ComboBoxOption.Text);
                    break;

                case TransportTypes.USB:
                case TransportTypes.SPI:
                    break;

            }

            // Remove any/all duplicates of the current transport option
            while (mHistory.Remove(transStr))
            {
            }

            // Insert the latest transport at the head of the list
            mHistory.Insert(0, transStr);

            // Lose any entries that make the list too big once we've inserted the latest
            if (mHistory.Count > MAX_TRANS_HISTORY_ITEMS)
            {
                mHistory.RemoveRange(MAX_TRANS_HISTORY_ITEMS, mHistory.Count - MAX_TRANS_HISTORY_ITEMS);
            }

            // Before re-writing transport list to registry, clean it up completely
            RegistryKey regKey = Registry.CurrentUser.OpenSubKey(SubkeyName);
            if (regKey != null)
            {
                regKey.Close();
                Registry.CurrentUser.DeleteSubKeyTree(SubkeyName);
            }

            // Create the key
            regKey = Registry.CurrentUser.CreateSubKey(SubkeyName);
            if (regKey != null)
            {
                // Write the updated history to the registry
                for (int i = 0; i <= (mHistory.Count - 1); i++)
                {
                    regKey.SetValue(i.ToString(CultureInfo.InvariantCulture), mHistory[i]);
                }
                regKey.Close();
            }
        }

        /// <summary>
        /// Gets the transport history from the registry.
        /// </summary>
        private void GetTransportHistory()
        {
            mHistory.Clear();

            RegistryKey regKey = Registry.CurrentUser.OpenSubKey(SubkeyName);
            if (regKey != null)
            {
                // Get the existing value names
                foreach (string valueName in regKey.GetValueNames())
                {
                    mHistory.Add(regKey.GetValue(valueName).ToString());
                }

                // Lose any entries that make the list too big
                if (mHistory.Count > MAX_TRANS_HISTORY_ITEMS)
                {
                    mHistory.RemoveRange(MAX_TRANS_HISTORY_ITEMS, mHistory.Count - MAX_TRANS_HISTORY_ITEMS);
                }

                regKey.Close();
            }
        }

        /// <summary>
        /// Adds to the transport history.
        /// </summary>
        private void AddTransportHistory()
        {
            // Get the history from the registry
            GetTransportHistory();

            HistoryMenuItem.DropDownItems.Clear();

            foreach (string transString in mHistory)
            {
                ToolStripMenuItem newMenuItem = new ToolStripMenuItem(transString.Replace("|", " - "));
                HistoryMenuItem.DropDownItems.Add(newMenuItem);
            }

            // Add an "empty", disabled item to the list if it is empty
            if (HistoryMenuItem.DropDownItems.Count == 0)
            {
                ToolStripMenuItem dummyMenuItem = new ToolStripMenuItem("empty");
                dummyMenuItem.Enabled = false;
                HistoryMenuItem.DropDownItems.Add(dummyMenuItem);
            }
        }

        /// <summary>
        /// Handles the DropDownItemClicked event of the HistoryMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.ToolStripItemClickedEventArgs"/> instance containing the event data.</param>
        private void HistoryMenuItem_DropDownItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            // Open a connection from the option string
            string[] options = e.ClickedItem.Text.Replace(" - ", "|").Split(new Char[] { '|' }, StringSplitOptions.RemoveEmptyEntries);
            FillTransportParamsFromOptions(options);

            // We would like the item just selected to become top of the history list,
            // so rewrite it - but only if it is still considered a valid selection
            // (the historic hardware may no longer be present for example)
            if (ButtonOK.Enabled)
            {
                UpdateTransportHistory();
            }
        }

        /// <summary>
        /// Sets the controls for the transport connection to match the options given
        /// </summary>
        /// <param name="options">a collection of options, such as "SPI","USB SPI(263456)" or "H4","COM8","2400"</param>
        private void FillTransportParamsFromOptions(string[] options)
        {
            if (options.Length >= 2)
            {
                mIsProgramStuffingValues = true;

                int index = ComboBoxTransports.FindStringExact(TransportTypeToString(TransportTypeFromName(options[0])));

                if (index >= 0)
                {
                    ComboBoxTransports.SelectedIndex = index;

                    index = ComboBoxPorts.FindStringExact(options[1]);
                    if (index >= 0)
                    {
                        ComboBoxPorts.SelectedIndex = index;

                        if (options.Length == 3)
                        {
                            index = ComboBoxOption.FindStringExact(options[2]);
                            if (index >= 0)
                            {
                                ComboBoxOption.SelectedIndex = index;
                            }
                        }
                    }
                }

                mIsProgramStuffingValues = false;

                ButtonOK.Focus();
            }
        }

        /// <summary>
        /// Handles the DropDownOpening event of the HistoryMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void HistoryMenuItem_DropDownOpening(object sender, EventArgs e)
        {
            AddTransportHistory();
        }

        #endregion

        #region Constructors etc.

        /// <summary>
        /// Handles the Load event of the Transport control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void Transport_Load(object sender, EventArgs e)
        {
            // Clear the other controls until we decide otherwise
            LabelPort.Text = "";
            LabelOption.Text = "";

            // Load the allowable transports
            for (int transportTypeMask = 1; transportTypeMask != 0; transportTypeMask = transportTypeMask << 1)
            {
                if (Enum.IsDefined(typeof(TransportTypes), transportTypeMask))
                {
                    TransportTypes transportType = (TransportTypes)transportTypeMask;
                    if (TransportIsShown(transportType))
                    {
                        ComboBoxTransports.Items.Add(TransportTypeToString(transportType));
                    }
                }
            }

            // If there's only one transport option, 'disable' the control and select it
            if (ComboBoxTransports.Items.Count == 1)
            {
                // Float the dummy label over the front to keep colour the same
                LabelOnlyTransport.Text = ComboBoxTransports.Items[0].ToString();
                LabelOnlyTransport.Show();

                ComboBoxTransports.Enabled = false;
                ComboBoxTransports.SelectedIndex = 0;
            }

            // Initialise the transport history
            AddTransportHistory();

            // autopopulate with the top history item (if there is one...)
            if (mHistory.Count > 0)
            {
                string[] options = mHistory[0].Split(new Char[] { '|' }, StringSplitOptions.RemoveEmptyEntries);
                FillTransportParamsFromOptions(options);
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="TransportDialog"/> class.
        /// </summary>
        public TransportDialog()
        {
            base.Load += Transport_Load;

            InitializeComponent();
        }

        #endregion

    }
}
