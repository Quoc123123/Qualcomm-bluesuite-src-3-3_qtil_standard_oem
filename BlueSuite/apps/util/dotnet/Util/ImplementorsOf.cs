//------------------------------------------------------------------------------
//
// <copyright file="ImplementorsOf.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Reflection;
using System.Security;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// Class to find classes which implement type T
    /// </summary>
    /// <typeparam name="T">The type that must be implemented.</typeparam>
    public sealed class ImplementorsOf<T>
    {
        #region Support methods

        /// <summary>
        /// Finds the implementors of the specified type contained in the specified assembly.
        /// </summary>
        /// <param name="assembly">The assembly to scan.</param>
        /// <param name="attribute">A custom assembly attribute used as a top-level filter on the assemblies.
        /// Only assemblies that have this attribute (if not null) will be scanned.</param>
        /// <returns>List of all types in the specified assembly that implement the specified type.</returns>
        private ReadOnlyCollection<Type> Find(Assembly assembly, Attribute attribute)
        {
            MessageHandler.DebugEntry();

            List<Type> implementors = new List<Type>();

            if (assembly != null)
            {
                // Find the executing assembly (so can 'allow' internal classes from own assembly)
                Assembly executingAssembly = Assembly.GetExecutingAssembly();
                MessageHandler.DebugEnhancedFormat("Executing assembly '{0}'", executingAssembly);

                // If we're filtering on a custom assembly attribute, see if assembly exposes it
                Boolean includeInSearch = false;
                if (attribute != null)
                {
                    Object[] assemblyAttributes = assembly.GetCustomAttributes(false);
                    foreach (Object assemblyAttribute in assemblyAttributes)
                    {
                        if (assemblyAttribute.GetType() == attribute.GetType())
                        {
                            includeInSearch = true;
                            break;
                        }
                    }
                }
                else
                {
                    // Not filtering, scan anyway
                    includeInSearch = true;
                }

                if (includeInSearch)
                {
                    // Scan through all types exposed by the assembly
                    MessageHandler.DebugEnhancedFormat("Scanning assembly '{0}'", assembly);

                    foreach (Type assemblyType in assembly.GetTypes())
                    {
                        MessageHandler.DebugEnhancedFormat("Checking type '{0}'", assemblyType);

                        // Find concrete *IMPLEMENTORS* of type
                        if (typeof(T).IsAssignableFrom(assemblyType))
                        {
                            // Check that the 'class' ...
                            if (assemblyType.IsClass)
                            {
                                Boolean isValid = false;

                                // ... is public
                                if (assemblyType.IsPublic)
                                {
                                    MessageHandler.DebugEnhanced("is Public");
                                    isValid = true;
                                }

                                // ... or is internal from *this* assembly
                                if (assemblyType.IsNotPublic && assembly.Equals(executingAssembly))
                                {
                                    MessageHandler.DebugEnhanced("is Internal to executing assembly");
                                    isValid = true;
                                }

                                // ... and is not an abstract class
                                if (assemblyType.IsAbstract)
                                {
                                    MessageHandler.DebugEnhanced("is Abstract");
                                    // so no longer valid
                                    isValid = false;
                                }

                                if (isValid)
                                {
                                    MessageHandler.DebugEnhanced("is valid, adding");
                                    implementors.Add(assemblyType);
                                }
                            }
                        }
                    }
                }
                else
                {
                    MessageHandler.DebugEnhancedFormat("Skipping assembly '{0}'", assembly);
                }
            }
            else
            {
                MessageHandler.StatusErrorFormat("null assembly");
                throw new ArgumentNullException("assembly");
            }

            ReadOnlyCollection<Type> retVal = new ReadOnlyCollection<Type>(implementors);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        #endregion

        #region Public interface

        /// <summary>
        /// Finds the implementors of the specified type contained in the specified file.
        /// </summary>
        /// <param name="fileInfo">The file to scan.</param>
        /// <param name="attribute">A custom assembly attribute used as a top-level filter on the assemblies.
        /// Only assemblies that have this attribute (if not null) will be scanned.</param>
        /// <returns>List of all types in the specified file that implement the specified type.</returns>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2001:AvoidCallingProblematicMethods", MessageId = "System.Reflection.Assembly.LoadFrom")]
        public ReadOnlyCollection<Type> Find(FileSystemInfo fileInfo, Attribute attribute)
        {
            const string STR_Exception = "Allowable exception";

            MessageHandler.DebugEntry();

            List<Type> implementors = new List<Type>();

            // C# doesn't allow the (potentially useful) feature of catching multiple exception types.
            // Just consume the exceptions as there's no need to do much more if they occur.
            // So log these 'errors' as DEBUG level. If running in the IDE will still get the IDE's exception notification.
            try
            {
                Assembly assembly = Assembly.LoadFrom(fileInfo.Name);
                implementors.AddRange(Find(assembly, attribute));
            }
            catch (ArgumentNullException ex)
            {
                MessageHandler.DebugEnhanced(STR_Exception, ex);
            }
            catch (FileNotFoundException ex)
            {
                MessageHandler.DebugEnhanced(STR_Exception, ex);
            }
            catch (FileLoadException ex)
            {
                MessageHandler.DebugEnhanced(STR_Exception, ex);
            }
            catch (BadImageFormatException ex)
            {
                MessageHandler.DebugEnhanced(STR_Exception, ex);
            }
            catch (ArgumentException ex)
            {
                MessageHandler.DebugEnhanced(STR_Exception, ex);
            }
            catch (SecurityException ex)
            {
                MessageHandler.DebugEnhanced(STR_Exception, ex);
            }

            ReadOnlyCollection<Type> retVal = new ReadOnlyCollection<Type>(implementors);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Finds the implementors of the specified type contained in the specified file.
        /// </summary>
        /// <param name="fileInfo">The file to scan.</param>
        /// <returns>List of all types in the specified file that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find(FileSystemInfo fileInfo)
        {
            MessageHandler.DebugEntry();

            ReadOnlyCollection<Type> retVal = Find(fileInfo, null);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Finds the implementors of the specified type contained in the specified files.
        /// </summary>
        /// <param name="fileSystemInfoCollection">The files to scan.</param>
        /// <param name="attribute">A custom assembly attribute used as a top-level filter on the assemblies.
        /// Only assemblies that have this attribute (if not null) will be scanned.</param>
        /// <returns>List of all types in the specified fileInfo that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find(Collection<FileSystemInfo> fileSystemInfoCollection, Attribute attribute)
        {
            MessageHandler.DebugEntry();

            List<Type> implementors = new List<Type>();

            // Search the specified fileInfos for implementors
            foreach (FileSystemInfo fileSystemInfo in fileSystemInfoCollection)
            {
                implementors.AddRange(Find(fileSystemInfo, attribute));
            }

            ReadOnlyCollection<Type> retVal = new ReadOnlyCollection<Type>(implementors);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Finds the implementors of the specified type contained in the specified files.
        /// </summary>
        /// <param name="fileSystemInfoCollection">The files to scan.</param>
        /// <returns>List of all types in the specified fileInfo that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find(Collection<FileSystemInfo> fileSystemInfoCollection)
        {
            MessageHandler.DebugEntry();

            ReadOnlyCollection<Type> retVal = Find(fileSystemInfoCollection, null);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Finds the implementors of the specified type contained in any file which matches the specified filter(s) in 
        /// the executing assembly's location.
        /// </summary>
        /// <param name="filters">The filter to apply to the executing assembly's folder for files to scan in format
        /// "*.xxx;*.yyy".</param>
        /// <param name="attribute">A custom assembly attribute used as a top-level filter on the assemblies.
        /// Only assemblies that have this attribute (if not null) will be scanned.</param>
        /// <returns>List of all types in the specified assembly that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find(String filters, Attribute attribute)
        {
            MessageHandler.DebugEntry();

            String appFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            DirectoryInfo directoryInfo = new DirectoryInfo(appFolder);

            // Scan through all files that match the specified filters in the executing assembly's folder to determine all classes that implement type.
            List<FileSystemInfo> fileInfos = new List<FileSystemInfo>();
            foreach (String filter in filters.Split(new char[] { ';' }))
            {
                fileInfos.AddRange(directoryInfo.GetFiles(filter));
            }

            ReadOnlyCollection<Type> retVal = Find(new Collection<FileSystemInfo>(fileInfos), attribute);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Finds the implementors of the specified type contained in any file which matches the specified filter(s) in 
        /// the executing assembly's location.
        /// </summary>
        /// <param name="filters">The filter to apply to the executing assembly's folder for files to scan in format
        /// "*.xxx;*.yyy".</param>
        /// <returns>List of all types in the specified assembly that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find(String filters)
        {
            MessageHandler.DebugEntry();

            ReadOnlyCollection<Type> retVal = Find(filters, null);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Finds the implementors of the specified type from any file in the executing assembly's location.
        /// </summary>
        /// <param name="attribute">A custom assembly attribute used as a top-level filter on the assemblies.
        /// Only assemblies that have this attribute (if not null) will be scanned.</param>
        /// <returns>List of all types in the specified assembly that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find(Attribute attribute)
        {
            MessageHandler.DebugEntry();

            // Scan through all files in the executing assembly's folder to determine all classes that implement type.
            String appFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            DirectoryInfo directoryInfo = new DirectoryInfo(appFolder);
            List<FileSystemInfo> fileInfos = new List<FileSystemInfo>();
            fileInfos.AddRange(directoryInfo.GetFiles());

            ReadOnlyCollection<Type> implementors = Find(new Collection<FileSystemInfo>(fileInfos), attribute);

            MessageHandler.DebugExit(implementors);

            return implementors;
        }

        /// <summary>
        /// Finds the implementors of the specified type from any file in the executing assembly's location.
        /// </summary>
        /// <returns>List of all types in the specified assembly that implement the specified type.</returns>
        public ReadOnlyCollection<Type> Find()
        {
            MessageHandler.DebugEntry();

            ReadOnlyCollection<Type> implementors = Find((Attribute)null);

            MessageHandler.DebugExit(implementors);

            return implementors;
        }

        #endregion

    }
}
