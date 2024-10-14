//------------------------------------------------------------------------------
//
// <copyright file="AboutDialog.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2020 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

#region Imports

using System;
using System.Globalization;
using System.Reflection;
using System.Windows.Forms;

using QTIL.HostTools.Common.Util;

#endregion

namespace QTIL.HostTools.Common.Dialogs
{

    public partial class AboutDialog : Form
    {
        public AboutDialog()
        {
            // This call is required by the Windows Form Designer.
            InitializeComponent();

            // Add any initialization after the InitializeComponent() call.

            //  Initialize the AboutBox to display the product information from the assembly information.
            //  Change assembly information settings for your application through either:
            //  - Project->Properties->Application->Assembly Information
            //  - AssemblyInfo.cs

            // Get the title bar text - the application descriptive name (title)
            Text = String.Format(CultureInfo.CurrentCulture, "About {0}", AssemblyInfo.Title());

            LabelAssemblyProduct.Text = AssemblyInfo.Product();

            // Get the version information - use the informational version, which should be either the special
            // build info, or the real version (for releases)
            LabelVersionNumber.Text = String.Format(CultureInfo.CurrentCulture, "Version: {0}", AssemblyInfo.FileVersion());

            LabelInformationalVersion.Text = AssemblyInfo.InformationalVersion();
            LabelCopyright.Text = AssemblyInfo.Copyright();
        }

        public AboutDialog(Assembly aAssembly)
        {
            // This call is required by the Windows Form Designer.
            InitializeComponent();

            // Add any initialization after the InitializeComponent() call.

            //  Initialize the AboutBox to display the product information from the assembly information.
            //  Change assembly information settings for your application through either:
            //  - Project->Properties->Application->Assembly Information
            //  - AssemblyInfo.cs

            // Get the title bar text - the application descriptive name (title)
            Text = String.Format(CultureInfo.CurrentCulture, "About {0}", AssemblyInfo.Title(aAssembly));

            LabelAssemblyProduct.Text = AssemblyInfo.Product(aAssembly);

            // Get the version information - use the informational version, which should be either the special
            // build info, or the real version (for releases)
            LabelVersionNumber.Text = String.Format(CultureInfo.CurrentCulture, "Version: {0}", AssemblyInfo.FileVersion(aAssembly));

            LabelInformationalVersion.Text = AssemblyInfo.InformationalVersion(aAssembly);
            LabelCopyright.Text = AssemblyInfo.Copyright(aAssembly);
        }
    }
}
