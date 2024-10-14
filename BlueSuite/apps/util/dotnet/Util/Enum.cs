//------------------------------------------------------------------------------
//
// <copyright file="Enum.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2018 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary>Provide utilities for using enum type with descrition</summary>
//
//------------------------------------------------------------------------------

using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace System
{
    /// <summary>
    /// Simple class to aid parsing/listing of enums (of type T)
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <remarks>
    /// To use enum descriptions i.e. values to be returned by EnumToList that are
    /// (possibly) different to the enum name, simply add a [Description] (or 
    /// [DescriptionAttribute]) to the enum.
    /// e.g.
    /// using System.ComponentModel;
    /// enum Encoding
    /// {
    ///     [System.ComponentModel.Description("UPC-A")]
    ///     UPCA,
    /// ...
    /// };
    /// </remarks>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1716:IdentifiersShouldNotMatchKeywords", MessageId = "Enum"), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix")]
    public static class Enum<T>
    {
        /// <summary>
        /// Get Description from Enum Value.
        /// </summary>
        /// <param name="aEnumValue">Enum Value</param>
        /// <returns>
        /// The enum description (if a [DescriptionAttribute] is applied) or the enum name
        /// </returns>
        public static String GetDescriptionFromEnum(Enum aEnumValue)
        {
            MessageHandler.DebugEntry();

            string retVal = "";

            var t = typeof(T);
            if (!t.IsEnum)
            {
                string logMsg = String.Format("Error: Unhandled enum type");
                MessageHandler.DebugBasicFormat(logMsg);
                throw new ArgumentException(logMsg);
            }
            else if (t != aEnumValue.GetType())
            {
                string logMsg = String.Format("Error: Enum type mismatch, {0} is not same as {1}", t.ToString(), aEnumValue.GetType().ToString());
                MessageHandler.DebugBasicFormat(logMsg);
                throw new ArgumentException(logMsg);
            }
            else
            {
                retVal = aEnumValue.ToString();

                DescriptionAttribute[] enumAttributes = (DescriptionAttribute[])aEnumValue.GetType()
                                                        .GetField(aEnumValue.ToString())
                                                        .GetCustomAttributes(typeof(DescriptionAttribute), false);

                if ((enumAttributes != null) && (enumAttributes.Length > 0))
                {
                    retVal = enumAttributes[0].Description;
                }
            }
            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Get Enum Value from Description.
        /// </summary>
        /// <param name="aDescription">Description of the enum value</param>
        /// <returns>Enum Value</returns>
        public static T GetEnumFromDescription(string aDescription)
        {
            MessageHandler.DebugEntry();
            T retEnumValue;

            var t = typeof(T);
            if (!t.IsEnum)
            {
                string logMsg = String.Format("Error: Unhandled enum type");
                MessageHandler.DebugBasicFormat(logMsg);
                throw new ArgumentException(logMsg);
            }

            var getEnumVal = t.GetFields().SelectMany(f => f.GetCustomAttributes(
                          typeof(DescriptionAttribute), false),
                          (f, a) => new { Field = f, Att = a })
                          .Where(a => ((DescriptionAttribute)a.Att)
                          .Description == aDescription).SingleOrDefault();

            retEnumValue = (getEnumVal == null) ? default(T) : (T)getEnumVal.Field.GetRawConstantValue();

            MessageHandler.DebugExit(retEnumValue);
            return retEnumValue;
        }

        /// <summary>
        /// Get Enum description and value as a list
        /// </summary>
        /// <param name="aEnumType">Enum</param>
        /// <returns>Enum and description as a list</returns>
        public static IList GetListFromEnum()
        {
            MessageHandler.DebugEntry();

            if (typeof(T) == null)
            {
                string logMsg = String.Format("Error: Unhandled enum type");
                MessageHandler.DebugBasicFormat(logMsg);
                throw new ArgumentException(logMsg);
            }

            ArrayList enumList = new ArrayList();
            Array enumValues = Enum.GetValues(typeof(T));

            foreach (Enum val in enumValues)
            {
                enumList.Add(new KeyValuePair<Enum, string>(val, GetDescriptionFromEnum(val)));
            }

            MessageHandler.DebugExit(enumList);
            return enumList;
        }

        /// <summary>
        /// Enumerates enums to a list.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns>Enum (and not description) as a list</returns>
        /// <example>
        /// DropDownList stateDropDown = new DropDownList();
        /// foreach (States state in EnumToList<States>())
        /// {
        ///        stateDropDown.Items.Add(GetEnumDescription(state));
        /// }
        /// </example>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1000:DoNotDeclareStaticMembersOnGenericTypes")]
        public static IEnumerable<T> EnumToList()
        {
            MessageHandler.DebugEntry();

            Type enumType = typeof(T);

            // Can't use generic type constraints on value types, so have to do check like this
            if (enumType.BaseType != typeof(Enum))
            {
                throw new ArgumentException("T must be of type System.Enum");
            }

            Array enumValueArray = Enum.GetValues(enumType);
            List<T> enumValueList = new List<T>(enumValueArray.Length);
            foreach (Int32 enumValue in enumValueArray)
            {
                enumValueList.Add((T)Enum.Parse(enumType, enumValue.ToString(CultureInfo.InvariantCulture)));
            }

            MessageHandler.DebugExit(enumValueList);
            return enumValueList;
        }

        #region Support Functions

        /// <summary>
        /// Converts the value to a UInt64 (irrespective of underlying type).
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns>The value as a UInt64</returns>
        /// <exception cref="System.InvalidOperationException">InvalidOperation_UnknownEnumType</exception>
        private static UInt64 ToUInt64(Object value)
        {
            switch (Convert.GetTypeCode(value))
            {
                case TypeCode.SByte:
                case TypeCode.Int16:
                case TypeCode.Int32:
                case TypeCode.Int64:
                    return (UInt64)Convert.ToInt64(value, CultureInfo.InvariantCulture);

                case TypeCode.Byte:
                case TypeCode.UInt16:
                case TypeCode.UInt32:
                case TypeCode.UInt64:
                    return Convert.ToUInt64(value, CultureInfo.InvariantCulture);
            }
            throw new InvalidOperationException("InvalidOperation_UnknownEnumType");
        }

        #endregion

        #region Parsing

        /// <summary>
        /// The enum separator char array
        /// </summary>
        private static Char[] sEnumSeparatorCharArray = new Char[] { ',' };

        /// <summary>
        /// Internal parser for an enum value.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        /// <exception cref="System.ArgumentException">Not found.;value</exception>
        private static T InternalParse(String value)
        {
            MessageHandler.DebugEntry();

            value = value.Trim();

            T retVal = default(T);
            Boolean foundIt = false;
            foreach (FieldInfo field in typeof(T).GetFields())
            {
                DescriptionAttribute attribute = Attribute.GetCustomAttribute(field, typeof(DescriptionAttribute)) as DescriptionAttribute;
                if (attribute != null)
                {
                    // Try to match [DescriptionAttribute]
                    foundIt = (0 == String.Compare(attribute.Description, value, StringComparison.OrdinalIgnoreCase));
                }
                else
                {
                    // Try to match name
                    foundIt = (0 == String.Compare(field.Name, value, StringComparison.OrdinalIgnoreCase));
                }

                if (foundIt)
                {
                    retVal = (T)field.GetValue(null);
                    break;
                }
            }

            if (!foundIt)
            {
                throw new ArgumentException("Not found.", "value");
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Internal parser for a [Flags] attributed enum.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        private static T InternalParseFlags(String value)
        {
            MessageHandler.DebugEntry();

            UInt64 enumValue = 0UL;

            String[] strArray = value.Split(sEnumSeparatorCharArray);
            for (Int32 i = 0; i < strArray.Length; i++)
            {
                enumValue |= ToUInt64(InternalParse(strArray[i]));
            }

            T retVal = (T)Enum.ToObject(typeof(T), enumValue); ;
            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Performs a case-insensitive parse of the specified value.
        /// </summary>
        /// <param name="value">The value, can be either [DescriptionAttribute] or name.</param>
        /// <returns>
        /// The parsed value, or exception.
        /// </returns>
        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentException"></exception>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1000:DoNotDeclareStaticMembersOnGenericTypes")]
        public static T Parse(String value)
        {
            MessageHandler.DebugEntry();

            T retVal = default(T);

            // Try to parse, throws an exception if it fails
            value = value.Trim();
            if (String.IsNullOrEmpty(value))
            {
                throw new ArgumentNullException("value", "Cannot be null.");
            }

            Type enumType = typeof(T);

            // Can't use generic type constraints on value types, so have to do check like this
            if (enumType.BaseType != typeof(Enum))
            {
                throw new ArgumentException(String.Format(CultureInfo.CurrentCulture, "T must be of type System.Enum (not {0})", enumType));
            }

            if (enumType.IsDefined(typeof(FlagsAttribute), false))
            {
                // Flags attributed, can have multiple values
                retVal = InternalParseFlags(value);
            }
            else
            {
                // Not flags attributed, can only have a single value
                retVal = InternalParse(value);
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        #endregion

        #region Formatting

        /// <summary>
        /// Internal formatter for an enum value.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns>The enum name (or descriptions)</returns>
        private static String InternalFormat(Enum value)
        {
            MessageHandler.DebugEntry();

            FieldInfo fieldInfo = typeof(T).GetField(value.ToString());

            DescriptionAttribute[] attributes = fieldInfo.GetCustomAttributes(typeof(DescriptionAttribute), false) as DescriptionAttribute[];

            // Preferentially use the description, otherwise use 'standard' Enum.ToString implementation
            String retVal = ((attributes != null) && (attributes.Length > 0)) ? attributes[0].Description : value.ToString();

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Internal formatter for a [Flags] attributed enum.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns>A comma separated list of enum names (or descriptions)</returns>
        private static string InternalFormatFlags(Enum value)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            UInt64 uint64Value = ToUInt64(value);
            if (uint64Value != 0)
            {
                StringBuilder sb = new StringBuilder();
                Boolean firstValue = true;

                // Pick up the values in the enum
                foreach (Enum enumValue in Enum.GetValues(typeof(T)))
                {
                    UInt64 uint64EnumValue = ToUInt64(enumValue);

                    // Ignore 'None' (prevents "NameX, NameY, None" i.e. gives "NameX, NameY")
                    if (uint64EnumValue == 0)
                    {
                        continue;
                    }

                    // Is this value set?
                    if ((uint64EnumValue & uint64Value) == uint64EnumValue)
                    {
                        if (!firstValue)
                        {
                            // Prepend separator
                            sb.Insert(0, ", ");
                        }
                        firstValue = false;

                        // Prepend enum's name
                        sb.Insert(0, InternalFormat(enumValue));
                    }
                }

                retVal = sb.ToString();
            }
            else
            {
                // Returns "None" (well 'None's' name!)
                retVal = InternalFormat(value);
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        /// <summary>
        /// Returns a <see cref="System.String" /> that represents this instance.
        /// </summary>
        /// <param name="enumValue">The enum value.</param>
        /// <returns>
        /// A <see cref="System.String" /> that represents this instance.
        /// </returns>
        /// <exception cref="ArgumentException"></exception>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1000:DoNotDeclareStaticMembersOnGenericTypes")]
        public static string ToString(Enum enumValue)
        {
            MessageHandler.DebugEntry();

            String retVal = String.Empty;

            Type enumType = typeof(T);

            // Can't use generic type constraints on value types, so have to do check like this
            if (enumType.BaseType != typeof(Enum))
            {
                throw new ArgumentException(String.Format(CultureInfo.CurrentCulture, "T must be of type System.Enum (not {0})", enumType));
            }

            if (enumType.IsDefined(typeof(FlagsAttribute), false))
            {
                // Flags attributed, can have multiple values
                retVal = InternalFormatFlags(enumValue);
            }
            else
            {
                // Not flags attributed, can only have a single value
                retVal = InternalFormat(enumValue);
            }

            MessageHandler.DebugExit(retVal);
            return retVal;
        }

        #endregion
    }
}
