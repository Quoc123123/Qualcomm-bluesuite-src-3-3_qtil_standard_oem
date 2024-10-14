/**********************************************************************
*
*  MultiListParser.h
*
*  Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
*  All Rights Reserved.
*  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  A class to capture and parse a string in to a group of lists.
*
***********************************************************************/

#ifndef MULTILISTPARSER_H
#define MULTILISTPARSER_H

#include <deque>
#include <map>
#include <string>
#include <vector>

// Give the class a different name when it is included separately to the library
// to avoid linker problems under Linux
#ifdef EF_DISABLE_ALL_DEBUG
#define CMULTILISTPARSER CMultiListParserInsideEF
#else
#define CMULTILISTPARSER CMultiListParser
#endif	

class CMULTILISTPARSER
{
public:
    CMULTILISTPARSER();
    ~CMULTILISTPARSER();

    /// The type for a collection of strings.
    typedef std::deque< std::string > StringCollectionType;

    /// The type describing a group of data.
    /// The 'first' part is the group name and the 'second' part is the
    /// collection of strings in the group.
    typedef std::pair< std::string, StringCollectionType > StringPairType;

    ///
    /// Add a synonym for the given text.
    /// @param[in] aSynonym The search text.
    /// @param[in] aExpandedVersion The replacement text.
    /// @return true if both texts are valid and the operation was successful, false otherwise.
    ///
    bool AddSynonym(const std::string& aSynonym, const std::string& aExpandedVersion);

    ///
    /// Add a valid group name.
    /// Group names that do not match the list given to this method will be rejected as invalid.
    /// @param[in] aName The name of the group.
    ///
    void AddValidGroup(const std::string& aName);

    ///
    /// Add a valid element name.
    /// Element names that do not match the list given to this method will be rejected as invalid.
    /// @param[in] aName The name of the element.
    ///
    void AddValidElement(const std::string& aName);

    ///
    /// Parse the string supplied and populate the output parameter with the information found.
    /// If any group names have been defined (by calling AddValidGroupName), the optional
    /// \<group_name\> items are part of the syntax but if there have been no calls to the method,
    /// then the \<group_name\> is not part of the syntax. This is the definition of the the syntax
    /// expected:
    /// <p> <code>
    /// element_list := [ all- \<element\> [{-\<element\>}] | \<element\> [{+\<element\>}] ]
    ///  <br>
    /// [ \<group_name\>: ] \<element_list\> [{, [ \<group_name\>: ] \<element_list\>}]
    /// </code>
    /// @param[in] aStatement The string to parse (see above).
    /// @param[out] aGroupsOfElements The list of groups and elements found.
    /// @return true if the output parameter was populated with data, false if there was a
    /// syntax or semantic error found. If false, use GetLastError() to retrieve an error message.
    ///
    bool ParseStatement(const std::string& aStatement, std::deque<StringPairType>& aGroupsOfElements);

    ///
    /// Obtain an error string of the latest problem (if any).
    /// @return The latest error text, or an empty string if there was no error in the last parse operation.
    ///
    std::string GetLastError();

    // String constants used within the class.
    // See the definitions within the source file for the (constant) literal values.
    // The intention was to document the constants in the header file, but that was not practical in C++
    // (without using the pre-processor).
    static const char* const ALL_TEXT;
    static const char* const ADDITION_DELIMITER;
    static const char* const SUBTRACTION_DELIMITER;
    static const char* const GROUP_NAME_DELIMITER;
    static const char* const GROUPS_DELIMITER;

private:

    ///
    /// Search the collection for the item specified.
    /// @param[in] aSearchItem The string to look for.
    /// @param[in] aCollection The collection in which to look.
    /// @return true if the search item was found, false otherwise.
    ///
    bool IsPresentInCollection(const std::string& aSearchItem, const StringCollectionType& aCollection);

    ///
    /// Parse the string looking for elements (i.e. the groups have already been processed).
    /// @param[in] aStatement The string to parse.
    /// @param[out] aElements The list of elements found.
    /// @return true if the output parameter was populated with data, false if there was a
    /// syntax or semantic error found. If false, use GetLastError() to retrieve an error message.
    ///
    bool ParseElements(const std::string& aStatement, StringCollectionType& aElements);

    ///
    /// Determine if the specified text has been redefined as a synonym.
    /// @param[in] aSearchItem The string to search for.
    /// @return The real string to be parsed - if there was no synonym, just return the same as the input.
    ///
    std::string GetSynonym(const std::string& aSearchItem);

    /// The type used to store the synonyms.
    typedef std::map<std::string, std::string> StringMapType;

    /// The synonyms of alternative texts.
    StringMapType mSynonyms;

    /// The collection of valid element names.
    StringCollectionType mValidElements;

    /// The collection of valid group names.
    StringCollectionType mValidGroups;

    /// The latest error string.
    std::string mErrorString;
};

#endif
