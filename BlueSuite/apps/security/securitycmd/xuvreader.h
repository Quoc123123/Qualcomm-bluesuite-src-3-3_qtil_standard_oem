/*******************************************************************************
 *
 *  xuvreader.h
 *
 *  Copyright (c) 2013-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Define xuv image module to read and write xuv files.
 *
 ******************************************************************************/

#ifndef __XUV_READER_H__
#define __XUV_READER_H__

#include <vector>
#include <string>
#include <map>

namespace xuv
{

    /* Define base data types. */
    typedef unsigned int AddrType; /* type to store xuv address */
    typedef unsigned int DataType; /* type to store xuv data values */
    typedef std::vector<unsigned char> ByteBlockType;

    /**************************************************************************
    @brief xuv image struct

    This struct contains the xuv data along with comments.

    Data is contained in a map since xuv file are not always contiguous data.
    Comments are also contained in a map with the exception of comment following
    the last data line. Comments take the address of the following data line.
    The comment following the last data line is contained in the string TailComment.
    */
    struct image
    {
        /** The xuv image data map containing (address, value) pairs. */
        std::map<AddrType, DataType> data;
        /** The comments map containing (address, comment) pairs. */
        std::map<AddrType, std::string> comments;
        /** Comment lines that appear after last data line. */
        std::string TailComment;
    };

    /** Define iterator types for xuv data and comment maps. */
    typedef std::map<AddrType, DataType>::const_iterator ConstDataIterator;
    typedef std::map<AddrType, DataType>::iterator DataIterator;
    typedef std::map<AddrType, std::string>::const_iterator ConstCommentsIterator;
    typedef std::map<AddrType, std::string>::iterator CommentsIterator;

    /**************************************************************************
    @brief Define function to parse a xuv line.
    */
    enum ParseXuvLineStatus{PARSE_ERROR= -1, PARSE_OK, PARSE_COMMENT};
    ParseXuvLineStatus ParseXuvLine(const std::string &aLine, AddrType &aAddress, DataType &aValue);

    /**************************************************************************
    @brief Define function to read xuv file.
    */
    enum ReadStatus{READ_BADFILE= -2, READ_SYNTAX, READ_OK = 0};
    ReadStatus Read(image &aXuv, const char *aFilename);

    /**************************************************************************
    @brief Define function to write xuv file.
    */
    enum WriteStatus{WRITE_BADFILE= -1, WRITE_OK = 0};
    WriteStatus Write(image &aXuv, const char *aFilename);

    /**************************************************************************
    @brief Define functions to extract a contiguous byte block.

    These functions specifically extract uint16 values into a byte block.
    ByteBlockFlattenBE extracts a block converting uint16 values into big endian byte order.
    ByteBlockFlattenLE extracts a block converting uint16 values into little endian byte order.
    */
    ByteBlockType ByteBlockFlattenU16BE(const image &aXuv, AddrType aFirst, AddrType aLast, unsigned char aFill = 0xFF);
    ByteBlockType ByteBlockFlattenU16LE(const image &aXuv, AddrType aFirst, AddrType aLast, unsigned char aFill = 0xFF);
    ByteBlockType ByteBlockFlattenU16(const image &aXuv, AddrType aFirst, AddrType aLast, unsigned char aFill, bool aBe);

    /**************************************************************************
    @brief Define functions to incorporate a contiguous byte block into a xuv image.

    This function specifically incorporate 2-byte words into xuv image as uint16 values.
    ByteBlockIncorporateU16BE incorporates a contiguous byte block from big endian byte order
    ByteBlockIncorporateU16LE incorporates a contiguous byte block from big endian byte order
    */
    void ByteBlockIncorporateU16BE(image &aXuv, AddrType aDest, unsigned char const *aFirst, unsigned char const *aLast);
    void ByteBlockIncorporateU16LE(image &aXuv, AddrType aDest, unsigned char const *aFirst, unsigned char const *aLast);
    void ByteBlockIncorporateU16(image &aXuv, AddrType aDest, unsigned char const *aFirst, unsigned char const *aLast, bool aBe);

} /* namespace xuv */


#endif /* __XUV_READER_H__ */


