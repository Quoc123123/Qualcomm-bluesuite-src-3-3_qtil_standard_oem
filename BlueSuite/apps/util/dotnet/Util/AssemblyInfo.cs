//------------------------------------------------------------------------------
//
// <copyright file="AssemblyInfo.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.IO;
using System.Reflection;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// Simple class wrapping access to assembly information
    /// </summary>
    public sealed class AssemblyInfo
    {
        /// <summary>
        /// Returns the entry assembly's title.
        /// </summary>
        /// <returns></returns>
        public static String Title()
        {
            MessageHandler.DebugEntry();

            String retVal = Title(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's title.
        /// </summary>
        /// <returns></returns>
        public static String Title(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Set the default value to the .exe name
                retVal = Path.GetFileNameWithoutExtension(assembly.CodeBase);

                // Get all Title attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyTitleAttribute), false);

                // If there is at least one Title attribute
                if (attributes.Length > 0)
                {
                    // Select the first one
                    AssemblyTitleAttribute titleAttribute = (AssemblyTitleAttribute)attributes[0];

                    // If it is not an empty string, return it
                    if (!String.IsNullOrEmpty(titleAttribute.Title))
                    {
                        retVal = titleAttribute.Title;
                    }
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's version.
        /// </summary>
        /// <returns></returns>
        public static String Version()
        {
            MessageHandler.DebugEntry();

            String retVal = Version(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's version.
        /// </summary>
        /// <returns></returns>
        public static String Version(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                retVal = assembly.GetName().Version.ToString();
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's file version.
        /// </summary>
        /// <returns></returns>
        public static String FileVersion()
        {
            MessageHandler.DebugEntry();

            String retVal = FileVersion(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's file version.
        /// </summary>
        /// <returns></returns>
        public static String FileVersion(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Get all Version attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyFileVersionAttribute), false);

                // If there is a Version attributes, return its value
                if (attributes.Length > 0)
                {
                    retVal = ((AssemblyFileVersionAttribute)attributes[0]).Version;
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's informational version.
        /// </summary>
        /// <returns></returns>
        public static String InformationalVersion()
        {
            MessageHandler.DebugEntry();

            String retVal = InformationalVersion(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's informational version.
        /// </summary>
        /// <returns></returns>
        public static String InformationalVersion(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Get all Version attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyInformationalVersionAttribute), false);

                // If there is a Version attributes, return its value
                if (attributes.Length > 0)
                {
                    retVal = ((AssemblyInformationalVersionAttribute)attributes[0]).InformationalVersion;
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's description.
        /// </summary>
        /// <returns></returns>
        public static String Description()
        {
            MessageHandler.DebugEntry();

            String retVal = Description(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's description.
        /// </summary>
        /// <returns></returns>
        public static String Description(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Get all Description attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyDescriptionAttribute), false);

                // If there is a Description attributes, return its value
                if (attributes.Length > 0)
                {
                    retVal = ((AssemblyDescriptionAttribute)attributes[0]).Description;
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's name.
        /// </summary>
        /// <returns></returns>
        public static String Name()
        {
            MessageHandler.DebugEntry();

            String retVal = Name(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's name.
        /// </summary>
        /// <returns></returns>
        public static String Name(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                retVal = assembly.GetName().Name;
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's product.
        /// </summary>
        /// <returns></returns>
        public static String Product()
        {
            MessageHandler.DebugEntry();

            String retVal = Product(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's product.
        /// </summary>
        /// <returns></returns>
        public static String Product(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Get all Product attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyProductAttribute), false);

                // If there is a Product attributes, return its value
                if (attributes.Length > 0)
                {
                    retVal = ((AssemblyProductAttribute)attributes[0]).Product;
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's copyright.
        /// </summary>
        /// <returns></returns>
        public static String Copyright()
        {
            MessageHandler.DebugEntry();

            String retVal = Copyright(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's copyright.
        /// </summary>
        /// <returns></returns>
        public static String Copyright(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Get all Copyright attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyCopyrightAttribute), false);

                // If there is a Copyright attribute, return its value
                if (attributes.Length > 0)
                {
                    retVal = ((AssemblyCopyrightAttribute)attributes[0]).Copyright;
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }


        /// <summary>
        /// Returns the entry assembly's company.
        /// </summary>
        /// <returns></returns>
        public static String Company()
        {
            MessageHandler.DebugEntry();

            String retVal = Company(Assembly.GetEntryAssembly());

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns the assembly's company.
        /// </summary>
        /// <returns></returns>
        public static String Company(Assembly assembly)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            if (assembly != null)
            {
                // Get all Company attributes on this assembly
                object[] attributes = assembly.GetCustomAttributes(typeof(AssemblyCompanyAttribute), false);

                // If there is a Company attributes, return its value
                if (attributes.Length > 0)
                {
                    retVal = ((AssemblyCompanyAttribute)attributes[0]).Company;
                }
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Prevents a default instance of the <see cref="AssemblyInfo" /> class from being created.
        /// </summary>
        private AssemblyInfo()
        {
        }
    }
}
