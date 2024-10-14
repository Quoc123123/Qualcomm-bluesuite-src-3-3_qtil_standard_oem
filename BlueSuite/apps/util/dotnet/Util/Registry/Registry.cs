//------------------------------------------------------------------------------
//
// <copyright file="Registry.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2018 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary>HostTools wrapper functions for accessing registry keys/values</summary>
//
//------------------------------------------------------------------------------

using System;
using System.IO;
using System.Security.Permissions;

using Microsoft.Win32;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// Simple class for accessing registry keys/values
    /// </summary>
    public class Registry
    {
        #region Internal classes

        #endregion

        #region Local data

        /// <summary>
        /// The sub key name.
        /// </summary>
        private readonly String mParentKeyName;

        #endregion

        #region Properties

        /// <summary>
        /// Gets the sub key count.
        /// </summary>
        /// <value>
        /// The sub key count.
        /// </value>
        public Int32 SubkeyCount
        {
            get
            {
                MessageHandler.DebugEntry();

                Int32 count = 0;

                RegistryKey subKey = null;
                try
                {
                    subKey = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(mParentKeyName);
                    if (subKey != null)
                    {
                        count = subKey.SubKeyCount;
                        subKey.Close();
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

                MessageHandler.DebugExit(count);

                return count;
            }
        }

        /// <summary>
        /// Gets the value count.
        /// </summary>
        /// <value>
        /// The value count.
        /// </value>
        public Int32 ValueCount
        {
            get
            {
                MessageHandler.DebugEntry();

                Int32 count = 0;

                RegistryKey subKey = null;
                try
                {
                    subKey = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(mParentKeyName);
                    if (subKey != null)
                    {
                        count = subKey.ValueCount;
                        subKey.Close();
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

                MessageHandler.DebugExit(count);

                return count;
            }
        }

        /// <summary>
        /// The registry String value.
        /// </summary>
        private readonly RegistryValue<String> mStrings;

        /// <summary>
        /// Gets the registry String value, use default if not available.
        /// </summary>
        /// <param name="aName">The name of the registry value. If 'null', gets the (Default) value.</param>
        /// <param name="aDefaultData">The value to return if aName does not exist.</param>
        /// <returns>The value associated with aName, or aDefaultData if aName is not found.</returns>
        public String GetRegistry(String aName, String aDefaultData)
        {
            return mStrings.GetValue(aName, aDefaultData);
        }

        /// <summary>
        /// Sets the specified registry value.
        /// If the specified value does not exist, it is created.
        /// </summary>
        /// <param name="aName">The name of the registry value. If 'null', sets the (Default) value.</param>
        /// <param name="aData">The String data to be stored.</param>
        public void SetRegistry(String aName, String aData)
        {
            mStrings.SetValue(aName, aData);
        }

        /// <summary>
        /// The registry DWORD values.
        /// </summary>
        private readonly RegistryValue<UInt32> mDwords;

        /// <summary>
        ///  Gets the registry DWORD value, use default if not available.
        /// </summary>
        /// <param name="aName">The name of the registry value. If 'null', gets the (Default) value.</param>
        /// <param name="aDefaultData">The value to return if aName does not exist.</param>
        /// <returns>The value associated with aName, or aDefaultData if aName is not found.</returns>
        [CLSCompliant(false)]
        public UInt32 GetRegistry(String aName, UInt32 aDefaultData)
        {
            return mDwords.GetValue(aName, aDefaultData);
        }

        /// <summary>
        /// Sets the specified registry value.
        /// If the specified value does not exist, it is created.
        /// </summary>
        /// <param name="aName">The name of the registry value. If 'null', sets the (Default) value.</param>
        /// <param name="aData">The DWORD data to be stored.</param>
        [CLSCompliant(false)]
        public void SetRegistry(String aName, UInt32 aData)
        {
            mDwords.SetValue(aName, aData);
        }

        #endregion

        #region Constructors etc.

        /// <summary>
        /// Initializes a new instance of the <see cref="Registry"/> class.
        /// </summary>
        public Registry()
            : this(@"SOFTWARE\QTIL", AssemblyInfo.Title())
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Registry"/> class.
        /// </summary>
        /// <param name="aRootKey">The root key.</param>
        /// <param name="aParentKey">The parent key.</param>
        public Registry(String aRootKey, String aParentKey)
        {
            MessageHandler.DebugEntry();

            mParentKeyName = Path.Combine(aRootKey, aParentKey);
            MessageHandler.DebugEnhancedFormat("Using key '{0}'", mParentKeyName);
            MessageHandler.DebugEnhancedFormat("Contains {0} sub keys", SubkeyCount);
            MessageHandler.DebugEnhancedFormat("Contains {0} values", ValueCount);

            mStrings = new RegistryValue<String>(mParentKeyName);
            mDwords = new RegistryValue<UInt32>(mParentKeyName);

            MessageHandler.DebugExit();
        }

        #endregion
    }
}