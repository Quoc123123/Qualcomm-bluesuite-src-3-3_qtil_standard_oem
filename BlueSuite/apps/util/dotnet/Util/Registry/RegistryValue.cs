//------------------------------------------------------------------------------
//
// <copyright file="RegistryValue.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2018 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary>
// HostTools inplementation for wrapper functions to access registry keys/values
// </summary>
//
//------------------------------------------------------------------------------

using System;
using System.Globalization;

using Microsoft.Win32;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace QTIL.HostTools.Common.Util
{
    public class RegistryValue<T>
    {
        #region Local data

        /// <summary>
        /// The parent key name.
        /// </summary>
        private readonly String mParentKeyName;

        #endregion  // Local data

        #region Public methods

        /// <summary>
        /// Gets the registry value, use default if not available.
        /// </summary>
        /// <param name="aName">The name of the registry value. If 'null', gets the (Default) value.</param>
        /// <param name="aDefaultData">The value to return if aName does not exist.</param>
        /// <returns></returns>
        public T GetValue(String aName, T aDefaultData)
        {
            MessageHandler.DebugEntry();
            T value = aDefaultData;

            Type typeParameterType = typeof(T);
            MessageHandler.DebugEnhancedFormat("Getting {0} value '{1}'", typeParameterType, aName);

            RegistryKey subKey = null;
            try
            {
                subKey = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(mParentKeyName);
                if (subKey != null)
                {
                    Object dummy = subKey.GetValue(aName, aDefaultData);
                    value = (T)Convert.ChangeType(dummy, typeParameterType, CultureInfo.InvariantCulture);
                }
            }
            catch (Exception ex)
            {
                MessageHandler.DebugBasic("Exception", ex);
                throw;
            }
            finally
            {
                if (subKey != null)
                {
                    subKey.Close();
                }
            }

            MessageHandler.DebugExit(value);
            return value;
        }

        /// <summary>
        /// Sets the specified registry value.
        /// </summary>
        /// <param name="aName">The name of the registry value. If 'null', sets the (Default) value.</param>
        /// <param name="aData">The data to be stored.</param>
        public void SetValue(String aName, T aData)
        {
            MessageHandler.DebugEntry();

            Type typeParameterType = typeof(T);
            MessageHandler.DebugEnhancedFormat("Setting {0} value '{1}'={2}", typeParameterType, aName, aData);

            RegistryKey subKey = null;
            try
            {
                subKey = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(mParentKeyName);
                if (subKey != null)
                {
                    if (typeParameterType == typeof(String))
                    {
                        // Save it as REG_SZ
                        subKey.SetValue(aName, aData, RegistryValueKind.String);
                    }
                    else if (typeParameterType == typeof(UInt32))
                    {
                        // Save it as REG_DWORD
                        subKey.SetValue(aName, aData, RegistryValueKind.DWord);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageHandler.DebugBasic("Exception", ex);
                throw;
            }
            finally
            {
                if (subKey != null)
                {
                    subKey.Close();
                }
            }

            MessageHandler.DebugExit();
        }

        #endregion  // Public methods

        #region Constructors etc.

        /// <summary>
        /// Initializes a new instance of the <see cref="RegistryStrings" /> class.
        /// </summary>
        /// <param name="aSubKeyName">Name of the sub key.</param>
        internal RegistryValue(String aSubKeyName)
        {
            MessageHandler.DebugEntry();

            mParentKeyName = aSubKeyName;

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Prevents a default instance of the <see cref="Values&lt;T&gt;"/> class from being created.
        /// </summary>
        private RegistryValue()
        {
        }

        #endregion  // Constructors etc.
    }
}
