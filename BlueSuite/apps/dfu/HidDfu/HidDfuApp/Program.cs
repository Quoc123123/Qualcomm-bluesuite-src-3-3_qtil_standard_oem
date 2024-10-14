//------------------------------------------------------------------------------
//
// <copyright file="Program.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2018 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary>Entry Point for HidDfuApp</summary>
//
//------------------------------------------------------------------------------

using System;
using System.Threading;
using System.Windows.Forms;

namespace QTIL.HostTools.HidDfuApp
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // Run only single instance of the application
            bool singleInstance = true;
            using (Mutex mutex = new Mutex(true, "HidDfuApp", out singleInstance))
            {
                if (singleInstance)
                {
                    Application.EnableVisualStyles();
                    Application.SetCompatibleTextRenderingDefault(false);
                    Application.Run(new MainForm());
                }
                else
                {
                    MessageBox.Show("Another instance of HidDfuApp is already running.",
                            "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }
    }
}
