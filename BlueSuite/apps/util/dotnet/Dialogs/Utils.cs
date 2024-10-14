//------------------------------------------------------------------------------
//
// <copyright file="Utils.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2021 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System.Windows.Forms;

namespace QTIL.HostTools.Common.Dialogs
{
    /// <summary>
    /// Wrapper class encompassing utility functions
    /// </summary>
    public sealed class Utils
    {
        /// <summary>
        /// Fits the specified path to the specified control.
        /// </summary>
        /// <param name="aControl">The control.</param>
        /// <param name="aPath">The path to fit.</param>
        /// <returns>
        /// A (possibly modified with embedded ellipses) version of path such 
        /// that it will fit into the specified control's width.
        /// </returns>
        public static string FitPathToControl(Control aControl, string aPath)
        {
            return FitPathToSpace(aControl.Width, aControl.Font, aPath);
        }

        /// <summary>
        /// Fits the specified path to the specified available space for a given font.
        /// </summary>
        /// <param name="aWidth">Available width in pixels.</param>
        /// <param name="aFont">The font to be used.</param>
        /// <param name="aPath">The path to fit.</param>
        /// <returns>
        /// A (possibly modified with embedded ellipses) version of path such 
        /// that it will fit into the specified available width.
        /// </returns>
        public static string FitPathToSpace(int aWidth, System.Drawing.Font aFont, string aPath)
        {
            // Compact a copy of the string
            string compactedPath = string.Copy(aPath);
            TextRenderer.MeasureText(compactedPath, aFont, new System.Drawing.Size(aWidth, 0), TextFormatFlags.PathEllipsis | TextFormatFlags.ModifyString);

            // There are issues with MeasureText...it can terminate the modified string with a '\0' but not update the length of the string object, resulting in trailing
            // data being displayed. Cut the string to resolve this.
            int pos = compactedPath.IndexOf('\0');
            if (pos >= 0)
            {
                compactedPath = compactedPath.Substring(0, pos);
            }

            return compactedPath;
        }

        /// <summary>
        /// Prevents a default instance of the <see cref="Utils" /> class from being created.
        /// </summary>
        private Utils()
        {
        }
    }
}
