/** @file pluginmanager.h */ 

/**********************************************************************
 *
 *  pluginmanager.h
 *
 *  Copyright (c) 2012-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Defines a general class to dynamically load plugins
 *
 ***********************************************************************/

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H


#ifdef WIN32
#pragma warning( disable: 4251 )
#endif

#include "unicode/ichar.h"

#include <list>
#include <map>
#include <string>
#include <sstream>

#ifdef WIN32
 #include <windows.h>
#include <tchar.h>
#else
 #include <sys/types.h>
 #include <dirent.h>
 #include <dlfcn.h>
#endif

#ifdef WIN32
    typedef HMODULE MPMODULE;
#else
    typedef void* MPMODULE;
#endif

namespace plugin
{
    /**
     * Defines a class to load / unload plugins 
     * and get functions 
     */
    class CPluginManager
    {
    public:
        /**
         * Struct to save module information
         */
        struct Module
        {
            MPMODULE handle;   ///< handle to the module (plugin)
            istring name;      ///< module friendly name
            istring fileName;  ///< file name
            
            /**
             * Default constructor
             */
            Module() : handle(NULL) {}
            
            /**
             * Constructor
             * @param aHandle the module's handle
             */
            Module(MPMODULE apHandle, const istring& aName, const istring& aFileName) : handle(apHandle), name(aName), fileName(aFileName) {}
        };
    private:
        istring                     mGetName;       ///< name of the function that returns the friendly name
        istring                     mGetSignature;  ///< name of the function that returns the module signature
        istring                     mSignature;     ///< expected signature for the module
        std::map<istring, Module*>  mModules;       ///< list of the loaded modules
        std::list<istring>          mModuleNames;   ///< list of the modules found in the specified directory
    public:
        /**
         * Constructor
         * Information passed to the constructor is used to validate a plugin.
         * A plugin must have a function to return its friendly name (named as mGetName).
         * A plugin can optionally have a GetSignature function (named as mGetSignature).
         * If this is the case then that function must returns the same value as mSignature
         * for the plugin to be considered valid.
         * @param aGetName the name of the function which returns the plugin name
         * @param aGetSignature the name of the function which returns the signature. If passed empty it will be ignored
         * @param aSignature the plugin signature (identifies the type of plugin). Make sense only if mGetSignature not empty
         */
        CPluginManager(const istring& aGetName, const istring& aGetSignature, const istring& aSignature);
        
        /**
         * Destructor. 
         * Unloads modules if necessary.
         */
        ~CPluginManager();

        /**
         * Loads the modules matching aWildCard searching in aDir.
         * When a module is found it looks for mGetName and mGetSignature
         * functions. If it finds them it invokes them to get the plugin
         * name and then compare the value returned by mGetSignature to
         * mSignature to make sure the plugin is the one the user is looking
         * for. If the plugin matches the requirements its handler is saved
         * in the modules collection.
         * Note: Every call to any of the Load() methods will cause Unload()
         * to be called. That means if there are pending handles to the modules
         * they will become invalid!
         * @param aDir the directory to search in for modules.
         * @param aWildCard the wildcard to match
         */
        void Load(const istring& aDir, const istring& aWildCard);

        /**
         * Loads the modules searching in aDir.
         * Extension is by default "dll" for windows and "so" for linux
         * @param aDir the directory to search in for modules.
         */
        void Load(const istring& aDir);

        /**
         * Loads the modules matching wildcards in aWildCards
         * @param aDir the directory to search in for modules.
         * @param aWildCards a list of wildcards
         */
        void Load(const istring& aDir, const std::list<istring>& aWildCards);

        /**
         * Unloads loaded modules
         */
        void UnloadAll();

        /**
         * Returns the number of modules loaded
         * @return the number of modules loader
         */
        size_t Count();

        /**
         * Returns the list of modules name
         * @param aModules the destination list
         */
        size_t ModulesName(std::list<istring>& aModules);

        /**
         * Returns the handler to the module named aModuleName
         * @return the module's handler or NULL if not present
         */
        const Module* GetModule(const istring& aModuleName);

        /**
         * @param aModuleName the plug-in's name
         * @return true if the module exists, false otherwise
         */
        bool ModuleExists(const istring& aModuleName, bool aCaseSensitive = true);
    private:
        /**
         * Retrieve all the modules in aDir which match aWildCard
         * @param aDir the folder to search for modules
         * @param aWildCard the wildcard to match
         */
        void GetModules(const istring& aDir, const istring& aWildCard);
        
        /**
         * Loads and inspect the module and save it in its collection
         * if it has the required features (right functions / signature)
         * @param aName the module's name
         */
        void LoadModule(const istring &aName);
    };

    //////////////////////////////////////////////////////////////////////////////

    /**
     * Returns the pointer to the factory function (its type depends on
     * the template) or NULL if the function doesn't exist.
     * @param aModuleName the friendly name of the plugin
     * @param aFunctionName the name of the factory function
     * @return a pointer to the factory function or NULL if not present.
     */
    template<class T>
    T PluginFactory(CPluginManager& aPluginManager, const istring& aModuleName, const istring& aFunctionName)
    {
        T func = NULL;

        const CPluginManager::Module* pModule = aPluginManager.GetModule(aModuleName);
        if (pModule)
        {
#ifdef WIN32
            func = (T) GetProcAddress(pModule->handle, aFunctionName.c_str());
#else
            func = (T) dlsym(pModule->handle, inarrow(aFunctionName).c_str());
#endif
        }

        return func;
    }
}

#endif
