/**********************************************************************
*
*  MultiListParser.cpp
*
*  Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
*  All Rights Reserved.
*  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  A class to capture and parse a string in to a group of lists.
*
***********************************************************************/

#include "multilistparser.h"
#include "assert.h"
#include "stringutil.h" // Only for the WHITESPACE definition - do not use anything else from this library
#include "common/portability.h"
#include "engine/enginefw_interface.h"

#include <algorithm>

using namespace std;

// String constant declarations
const char* const CMULTILISTPARSER::ALL_TEXT              = "all";
const char* const CMULTILISTPARSER::ADDITION_DELIMITER    = "+";
const char* const CMULTILISTPARSER::SUBTRACTION_DELIMITER = "-";
const char* const CMULTILISTPARSER::GROUP_NAME_DELIMITER  = ":";
const char* const CMULTILISTPARSER::GROUPS_DELIMITER      = ",";

/////////////////////////////////////////////////////////////////////////////

// This local definition of ToLower is needed because when this file is being 
// compiled for use WITHIN the EngineFramework library, it does not link with
// the Misc library (it has a local copy of the CMULTILISTPARSER object).

string LocalToLower(const string& aString)
{
    string ret = aString;

    // explicit cast needed to resolve ambiguity
    transform(aString.begin(), aString.end(), ret.begin(), (int(*)(int))tolower);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

CMULTILISTPARSER::CMULTILISTPARSER()
{
    MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_UTILITY);
    FUNCTION_DEBUG_SENTRY;

    assert(strlen(ADDITION_DELIMITER)    == 1);
    assert(strlen(SUBTRACTION_DELIMITER) == 1);
    assert(strlen(GROUP_NAME_DELIMITER)  == 1);
    assert(strlen(GROUPS_DELIMITER)      == 1);
}

/////////////////////////////////////////////////////////////////////////////

CMULTILISTPARSER::~CMULTILISTPARSER()
{
    FUNCTION_DEBUG_SENTRY;
}

/////////////////////////////////////////////////////////////////////////////

string CMULTILISTPARSER::GetLastError()
{
    return mErrorString;
}

/////////////////////////////////////////////////////////////////////////////

bool CMULTILISTPARSER::AddSynonym(const string& aSynonym, const string& aExpandedVersion)
{
    bool retVal = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, retVal);

    if (aSynonym.empty() == false
    &&  aSynonym.find_first_of(stringutil::WHITESPACE_DELIMS) == string::npos
    &&  aExpandedVersion.find_first_of(stringutil::WHITESPACE_DELIMS) == string::npos)
    {
        mSynonyms.insert(make_pair(LocalToLower(aSynonym), LocalToLower(aExpandedVersion)));
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

string CMULTILISTPARSER::GetSynonym(const string& aSearchItem)
{
    FUNCTION_DEBUG_SENTRY;
    string retVal = aSearchItem;
    
    StringMapType::iterator it = mSynonyms.find(aSearchItem);
    if (it != mSynonyms.end())
    {
        retVal = it->second;
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////

void CMULTILISTPARSER::AddValidElement(const string& aName)
{
    mValidElements.push_back(aName);
}

/////////////////////////////////////////////////////////////////////////////

void CMULTILISTPARSER::AddValidGroup(const string& aName)
{
    mValidGroups.push_back(aName);
}

/////////////////////////////////////////////////////////////////////////////

bool CMULTILISTPARSER::IsPresentInCollection(const std::string& aSearchItem, const StringCollectionType& aCollection)
{
    bool ret = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    StringCollectionType::const_iterator it = aCollection.begin();
    while (ret == false && it != aCollection.end())
    {
        if (STRICMP(aSearchItem.c_str(), (*it).c_str()) == 0)
        {
            ret = true;
        }
        ++it;
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool CMULTILISTPARSER::ParseElements(const string& aStatement, StringCollectionType& aElements)
{
    bool ret = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    string stmt = aStatement;
    string validDelimiter = string(ADDITION_DELIMITER);
    string invalidDelimiter = SUBTRACTION_DELIMITER;
    bool justAll = false;
    StringCollectionType encountered;

    // Empty the output list
    aElements.clear();

    // Does it start with "all"?
    if (STRNICMP(stmt.c_str(), ALL_TEXT, strlen(ALL_TEXT)) == 0)
    {
        // Yes
        validDelimiter = SUBTRACTION_DELIMITER;
        invalidDelimiter = string(ADDITION_DELIMITER);
        stmt.replace(0, strlen(ALL_TEXT), "");

        // Just "all" or are some modules listed as well?
        if (stmt.length() == 0)
        {
            justAll = true;
        }

        // If it was "all-", remove the first subtraction delimiter
        if (STRNICMP(stmt.c_str(), validDelimiter.c_str(), validDelimiter.length()) == 0)
        {
            stmt.replace(0, validDelimiter.length(), "");
        }
    }

    // Make sure the invalid delimiter is not present
    if (mErrorString.length() == 0 && stmt.find_first_of(invalidDelimiter) != string::npos)
    {
        mErrorString = "Invalid delimeter \'" + invalidDelimiter + "\' encountered.";
    }

    // Loop through pulling out the text string encountered
    while (mErrorString.length() == 0 && stmt.length() > 0)
    {
        string::size_type firstDelimiterLocn = stmt.find_first_of(validDelimiter);
        const string element = stmt.substr(0, firstDelimiterLocn);
        encountered.push_back(element);
        if (firstDelimiterLocn == string::npos)
        {
            stmt.clear();
        }
        else
        {
            stmt = stmt.substr(firstDelimiterLocn + 1);
        }

        // If it's invalid, reject it
        if (IsPresentInCollection(element, mValidElements) == false)
        {
            mErrorString = "The text \'" + element + "\' is invalid.";
        }
    }

    // Go through adding elements to output list
    ret = mErrorString.empty();
    if (ret)
    {
        bool addIfEnountered = false;
        bool addIfNotEnountered = false;

        if (justAll || STRICMP(validDelimiter.c_str(), SUBTRACTION_DELIMITER) == 0)
        {
            addIfNotEnountered = true;
        }
        else
        {
            addIfEnountered = true;
        }

        StringCollectionType::iterator itValid = mValidElements.begin();
        while (itValid != mValidElements.end())
        {
            bool wasEncountered = IsPresentInCollection(*itValid, encountered);

            if ((wasEncountered && addIfEnountered) || (wasEncountered == false && addIfNotEnountered))
            {
                // Add it
                aElements.push_back(*itValid);
            }

            ++itValid;
        }
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool CMULTILISTPARSER::ParseStatement(const string& aStatement, deque<StringPairType>& aGroupsOfElements)
{
    bool ret = false;
    FUNCTION_DEBUG_SENTRY_RET(bool, ret);

    mErrorString.clear();
    aGroupsOfElements.clear();

    // Perform a synonym substitution if applicable
    string stmt = GetSynonym(LocalToLower(aStatement));

    // Make sure there is no whitespace
    if (stmt.find_first_of(stringutil::WHITESPACE_DELIMS) != string::npos)
    {
        mErrorString = "Whitespace found.";
    }
    else
    {
        if (mValidGroups.empty())
        {
            if (stmt.find_first_of(string(GROUP_NAME_DELIMITER)+string(GROUPS_DELIMITER)) != string::npos)
            {
                // No groups allowed
                mErrorString = "Invalid character(s) '" + string(GROUP_NAME_DELIMITER) +
                    "' or '" + string(GROUPS_DELIMITER) + "' found.";
            }
            else if (stmt.empty() == false)
            {
                StringCollectionType emptyCollection;
                StringPairType emptyPair = make_pair("", emptyCollection);
                aGroupsOfElements.push_back(emptyPair);

                ParseElements(LocalToLower(aStatement), aGroupsOfElements[0].second);
            }
        }
        else
        {
            int groupNum = 0;

            while (stmt.empty() == false)
            {
                // Work out the group name specified...
                string thisStatement;
                string::size_type firstGroupsDelimiter = stmt.find_first_of(GROUPS_DELIMITER);
                if (firstGroupsDelimiter != string::npos)
                {
                    // ...multiple groups found; get the first one
                    thisStatement = stmt.substr(0, firstGroupsDelimiter);
                    stmt = stmt.substr(firstGroupsDelimiter + 1);
                }
                else
                {
                    // ...only one group found
                    thisStatement = stmt;
                    stmt.clear();
                }

                string groupName;
                string modListStmt = thisStatement;
                string::size_type groupDelimiter = thisStatement.find_first_of(GROUP_NAME_DELIMITER);
                if (groupDelimiter != string::npos)
                {
                    // ...group name was specified
                    groupName = thisStatement.substr(0, groupDelimiter);
                    modListStmt = thisStatement.substr(groupDelimiter + 1);

                    if (groupName.empty())
                    {
                        // ...group empty
                        mErrorString = "Group identifier before '" + string(GROUP_NAME_DELIMITER) + "' empty.";
                    }
                    else if (IsPresentInCollection(groupName, mValidGroups) == false)
                    {
                        // ...group non-empty and not valid
                        mErrorString = "Group identifier '" + groupName + "' found but invalid.";
                    }
                }

                if (mErrorString.empty())
                {
                    // Work out the modules list
                    StringCollectionType emptyCollection;
                    StringPairType emptyPair = make_pair(groupName, emptyCollection);
                    aGroupsOfElements.push_back(emptyPair);
                    ParseElements(modListStmt, aGroupsOfElements[groupNum].second);
                    groupNum++;
                }
            }
        }
    }

    ret = mErrorString.empty();
    if (ret == false)
    {
        aGroupsOfElements.clear();
    }

    return ret;
}
