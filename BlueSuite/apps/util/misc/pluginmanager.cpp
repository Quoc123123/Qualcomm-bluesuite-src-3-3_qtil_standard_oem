/** @file pluginmanager.cpp */ 

/**********************************************************************
 *
 *  pluginmanager.cpp
 *
 *  Copyright (c) 2012-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Defines a class to dynamically load plugins
 *
 ***********************************************************************/

#include "engine/enginefw_interface.h"
#include "stringutil.h"
#include "fileutil.h"
#include "common/portability.h"
#include "pluginmanager.h"

namespace plugin
{
    using namespace std;

    //////////////////////////////////////////////////////////////////////////////
    // CPluginManager
    //////////////////////////////////////////////////////////////////////////////

    CPluginManager::CPluginManager(const istring& aGetName, const istring& aGetSignature, const istring& aSignature) : 
        mGetName(aGetName),
        mGetSignature(aGetSignature),
        mSignature(aSignature)
    {
        MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_CONFIGTOOLSCOREFRAMEWORK_LIB);
        FUNCTION_DEBUG_SENTRY;
    }

    //////////////////////////////////////////////////////////////////////////////

    CPluginManager::~CPluginManager()
    {
        FUNCTION_DEBUG_SENTRY;

        UnloadAll();
    }

    //////////////////////////////////////////////////////////////////////////////

    void CPluginManager::Load(const istring& aDir, const istring& aExt)
    {
        FUNCTION_DEBUG_SENTRY;

        UnloadAll();

        GetModules(aDir, aExt);
        
        for(list<istring>::iterator it = mModuleNames.begin(); it != mModuleNames.end(); ++it)
        {
            LoadModule(*it);
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    void CPluginManager::Load(const istring& aDir)
    {
        FUNCTION_DEBUG_SENTRY;

        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Loading plug-ins from %s", aDir.c_str());

    #ifdef WIN32
        static const istring EXT = II("dll");
    #else
        static const istring EXT = II("so");
    #endif
        
        UnloadAll();

        GetModules(aDir, EXT);
        
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Found %u plug-ins", mModuleNames.size());

        for(list<istring>::iterator it = mModuleNames.begin(); it != mModuleNames.end(); ++it)
        {   
            LoadModule(*it);
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    void CPluginManager::Load(const istring& aDir, const list<istring>& aWildCards)
    {
        FUNCTION_DEBUG_SENTRY;

        UnloadAll();

        for(list<istring>::const_iterator it = aWildCards.begin(); it != aWildCards.end(); ++it)
        {
            GetModules(aDir, *it);
        }
        
        for(list<istring>::iterator it = mModuleNames.begin(); it != mModuleNames.end(); ++it)
        {
            LoadModule(*it);
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    void CPluginManager::LoadModule(const istring &aName)
    {
        FUNCTION_DEBUG_SENTRY;

        typedef const ichar* (* PGETNAME)();    // function pointer to a GetName function
        typedef const ichar* (* PGETTYPE)();    // function pointer to a GetType function

        // it doesn't try to load the module if module is itself 
        // (otherwise unpredictable behavior might happen).
        string currmod = inarrow(aName);
        stringutil::ToLower(currmod);
        
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Try loading %s", aName.c_str());

    #ifdef WIN32
        MPMODULE pModule = LoadLibrary(aName.c_str());
        if (!pModule)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Error loading module %s", aName.c_str());
        }
    #else
        MPMODULE pModule = dlopen(aName.c_str(), RTLD_GLOBAL|RTLD_NOW);
        if (!pModule)
        {
            char* error = dlerror();
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Module %s error returned by dlopen: %s", aName.c_str(), error);
        }
    #endif
            
        if (pModule)
        {
    #ifdef WIN32
            PGETNAME fGetName = (PGETNAME) GetProcAddress(pModule, mGetName.c_str());
    #else
            PGETNAME fGetName = (PGETNAME) dlsym(pModule, inarrow(mGetName).c_str());
    #endif
            PGETTYPE fGetSignature = NULL;
            istring signature;

            if (fGetName && !mGetSignature.empty())
            {
    #ifdef WIN32
                fGetSignature = (PGETTYPE) GetProcAddress(pModule, mGetSignature.c_str());
    #else
                fGetSignature = (PGETTYPE) dlsym(pModule, inarrow(mGetSignature).c_str());
    #endif
                signature = fGetSignature != NULL ? fGetSignature() : II("");
            }

            if (fGetName)
            {
                if ((fGetSignature && signature == mSignature) || (!fGetSignature))
                {
                    const istring name = fGetName();
                    mModules[name] = new Module(pModule, name, aName);
                }
            }
            else
            {
    #ifdef WIN32
                FreeLibrary(pModule);    // it is not a plug-in
    #else
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Error returned by dlsym: %s", dlerror());
                dlclose(pModule);
    #endif
            }
        }
        
    }

    //////////////////////////////////////////////////////////////////////////////

    void CPluginManager::GetModules(const istring& aDir, const istring& aWildCard)
    {
        FUNCTION_DEBUG_SENTRY;
        
        // fileutil uses std::string, so need to convert list to istrings.
        std::list<std::string> moduleNames;
        fileutil::GetFiles(inarrow(aDir), inarrow(aWildCard), moduleNames);
        
        for(std::list<std::string>::const_iterator it = moduleNames.begin();
            it != moduleNames.end();
            ++it)
        {
            mModuleNames.push_back(icoerce(*it));
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    void CPluginManager::UnloadAll()
    {
        FUNCTION_DEBUG_SENTRY;

        for(map<istring, Module*>::iterator it = mModules.begin(); it != mModules.end(); ++it)
        {
    #ifdef WIN32
            FreeLibrary(it->second->handle);
    #else
            dlclose(it->second->handle);
    #endif
            delete it->second;
        }

        mModules.clear();
        mModuleNames.clear();
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t CPluginManager::Count()
    {
        size_t retVal;
        FUNCTION_DEBUG_SENTRY_RET(size_t, retVal);

        retVal = mModules.size();

        return retVal;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t CPluginManager::ModulesName(list<istring>& aModules)
    {
        FUNCTION_DEBUG_SENTRY;

        if (!mModules.empty())
        {
            for(map<istring, Module*>::iterator it = mModules.begin(); it != mModules.end(); ++it)
            {
                aModules.push_back((*it).first);
            }
        }

        return Count();
    }

    //////////////////////////////////////////////////////////////////////////////

    const CPluginManager::Module* CPluginManager::GetModule(const istring& aModuleName)
    {
        FUNCTION_DEBUG_SENTRY;

        const Module* pModule = NULL;

        if (mModules.find(aModuleName) != mModules.end())
        {
            pModule = mModules[aModuleName];
        }

        return pModule;
    }

    //////////////////////////////////////////////////////////////////////////////

    bool CPluginManager::ModuleExists(const istring& aModuleName, bool aCaseSensitive)
    {
        bool found = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, found);

        for(map<istring, Module*>::iterator it = mModules.begin(); it != mModules.end() && !found; ++it)
        {
            found = aCaseSensitive ? (aModuleName == it->first) : (stringutil::ToLower(inarrow(aModuleName)) == stringutil::ToLower(inarrow(it->first)));
        }

        return found;
    }
}
