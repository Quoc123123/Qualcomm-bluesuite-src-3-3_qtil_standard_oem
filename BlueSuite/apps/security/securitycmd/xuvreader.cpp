/*******************************************************************************
 *
 *  xuvreader.cpp
 *
 *  Copyright (c) 2013-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Define xuv image module to read and write xuv files.
 *
 *  The file is consists of lines which are either a data line or a comment.
 *  Data lines consist of address and data value pair. The address and value
 *  are hex digits. The address must be preceeded by "@". One or more blanks
 *  (space and tabs) must appear between the address and value. Any number of
 *  blanks may appear at the start or end of line.
 *  Lines that have no "@" after any blanks is considered a comment. Comments
 *  will put into the comments map.
 *
 *  The Write function will output the comments at the appropriate position as
 *  read. If data is deleted then the comments at the same address will not be
 *  output.
 *
 ******************************************************************************/

#include <fstream>
#include <sstream>
#include "xuvreader.h"

namespace xuv
{

/*****************************************************************
@brief Extract address and value from a line of xuv

@param[in] aLine        The line to extract from.
@param[out] aAddress    to receive first value in line.
@param[out] aValue      to receive second value in line.

@returns
    PARSE_ERROR     indicates syntax error or a value was too large.
    PARSE_OK        address and value returned ok.
    PARSE_COMMENT   line does not start with @.

@note
There is range checking of values overflowing the address and data type.
*/
ParseXuvLineStatus ParseXuvLine(const std::string &aLine, AddrType &aAddress, DataType &aValue)
{
    std::istringstream iss(aLine);
    char ch;
    /* Let's skip leading whitespace on line */
    if(!(iss >> std::skipws >> ch >> std::noskipws))
    {
        /* empty line after skipping any whitespace */
        return PARSE_COMMENT;
    }

    /* Lines not starting with @ are comments */
    if('@' != ch)
    {
        return PARSE_COMMENT;
    }

    /* Get address */
    if(!(iss >> std::hex >> aAddress))
    {
        return PARSE_ERROR;
    }

    /* Check next character is a blank */
    if(!(iss >> ch && (' ' == ch || '\t' == ch)))
    {
        return PARSE_ERROR;
    }
    if(!(iss >> std::ws >> std::hex >> aValue))
    {
        return PARSE_ERROR;
    }
    /* Check line is empty after any blanks. */
    iss >> std::ws;
    std::string rest;
    getline(iss, rest);
    if(rest.length())
    {
        return PARSE_ERROR;
    }
    return PARSE_OK;
}


/*****************************************************************
@brief Read a xuv file preserving comments.

@param[out] aXuv        image struct containing xuv image
@param[in] aFilename    Filename of xuv file

@returns
    READ_BADFILE    Error opening or reading file.
    READ_SYNTAX     Syntax error or address/data is too large.
    READ_OK         File read successfully.

@note
Comments are considered to be line not starting with [[:blank:]]*@
Data lines must not contain non-blanks after data
*/
ReadStatus Read(image &aXuv, const char *aFilename)
{
    ReadStatus result = READ_OK;
    /* Ensure aXuv is empty*/
    aXuv.data.clear();
    aXuv.comments.clear();
    aXuv.TailComment.clear();

    /*
    Open file. We use ifstream so we don't need to worry about line length
    and buffer size.
    */
    std::ifstream infile(aFilename);
    if(!infile.good())
    {
        return READ_BADFILE;
    }
    /* Iterate each line in file */
    std::string line, comment;
    while(std::getline(infile, line) && !result)
    {
        AddrType addr;
        DataType data;
        switch(ParseXuvLine(line, addr, data))
        {
        case PARSE_OK:
            aXuv.data[addr] = data;
            if(comment.length())
            {
                aXuv.comments[addr] = comment;
                comment = "";
            }
            break;
        case PARSE_COMMENT:
            comment.append(line);
            comment.append("\n");
            break;
        case PARSE_ERROR:
            result = READ_SYNTAX;
            break;
        }
    }
    if(comment.length())
    {
        aXuv.TailComment = comment;
    }
    infile.close();
    return result;
}

/*****************************************************************
@brief Write a xuv file preserving comments.

@param[out] aXuv        image struct containing xuv image
@param[in] aFilename    Filename of xuv file

@return
    WRITE_BADFILE   Error opening or writing file.
    WRITE_OK        File written successfuly.

@note
Comments are considered to be line not starting with [[:blank:]]*@
Data lines must not contain non-blanks after data.

@note
Comments at addresses without corresponding data will not be written.

*/
WriteStatus Write(image &aXuv, const char *aFilename)
{
    /*
    Open file.
    We use c functions for better formatting control.
    */
    FILE *outfile = fopen(aFilename, "w");
    if(!outfile)
    {
        return WRITE_BADFILE;
    }
    for(DataIterator di = aXuv.data.begin(); di != aXuv.data.end(); ++di)
    {
        CommentsIterator ci = aXuv.comments.find(di->first);
        if(ci != aXuv.comments.end())
        {
            fprintf(outfile, "%s", ci->second.c_str());
        }
        fprintf(outfile, "@%06X   %04X\n", di->first, di->second);
    }
    if(aXuv.TailComment.length())
    {
        fprintf(outfile, "%s", aXuv.TailComment.c_str());
    }
    fclose(outfile);
    return WRITE_OK;
}

/*****************************************************************
@brief Extract a contiguous byte block of data from xuv image (big endian).

@param[in] aXuv         image struct containing xuv image
@param[in] aFirst       Address of first data value of block.
@param[in] aLast        Address of last data value of block.
@param[in] aFill        Value to be used where data values are absent in range.

@return     Byte block of requested data.

@note
Where data values are larger than 16-bits the upper bits will be ignored.

*/
ByteBlockType ByteBlockFlattenU16BE(const image &aXuv, AddrType aFirst, AddrType aLast, unsigned char aFill)
{
    ByteBlockType block(2 * (aLast+1-aFirst), aFill);
    for(AddrType ofs = 0, i = aFirst; i <= aLast; ++i, ofs += 2)
    {
        xuv::ConstDataIterator di = aXuv.data.find(i);
        if (di != aXuv.data.end())
        {
            block[ofs] = (di->second >> 8) & 0xFF;
            block[ofs + 1] = di->second & 0xFF;
        }
    }
    return block;
}

/*****************************************************************
@brief Extract a contiguous byte block of data from xuv image (little endian).

@param[in] aXuv         image struct containing xuv image
@param[in] aFirst       Address of first data value of block.
@param[in] aLast        Address of last data value of block.
@param[in] aFill        Value to be used where data values are absent in range.

@return     Byte block of requested data.

@note
Where data values are larger than 16-bits the upper bits will be ignored.
*/
ByteBlockType ByteBlockFlattenU16LE(const image &aXuv, AddrType aFirst, AddrType aLast, unsigned char aFill)
{
    ByteBlockType block(2 * (aLast+1-aFirst), aFill);
    for(AddrType ofs = 0, i = aFirst; i <= aLast; ++i, ofs += 2)
    {
        xuv::ConstDataIterator di = aXuv.data.find(i);
        if (di != aXuv.data.end())
        {
            block[ofs + 1] = (di->second >> 8) & 0xFF;
            block[ofs] = di->second & 0xFF;
        }
    }
    return block;
}

/*****************************************************************
@brief Extract a contiguous byte block of data from xuv image.

@param[in] aXuv         image struct containing xuv image
@param[in] aFirst       Address of first data value of block.
@param[in] aLast        Address of last data value of block.
@param[in] aFill        Value to be used where data values are absent in range.
@param[in] aBe          the data is big endian.

@return     Byte block of requested data.

@note
Where data values are larger than 16-bits the upper bits will be ignored.
*/
ByteBlockType ByteBlockFlattenU16(const image &aXuv, AddrType aFirst, AddrType aLast, unsigned char aFill, bool aBe)
{
    if(aBe)
    {
        return ByteBlockFlattenU16BE(aXuv, aFirst, aLast, aFill);
    }
    else
    {
        return ByteBlockFlattenU16LE(aXuv, aFirst, aLast, aFill);
    }
}

/*****************************************************************
@brief Incorporate a contiguous byte block into a xuv image. (big endian).

Incorporate a contiguous byte block containing uint16 values in big endian byte
order into a xuv image.

@param[in out] aXuv     image struct containing xuv image
@param[in] aDest        address of first destination word.
@param[in] aFirst       start address of byte block to be incorporated.
@param[in] aLast        last address of byte block to be incorporated.
*/
void ByteBlockIncorporateU16BE(image &aXuv, AddrType aDest, unsigned char const *aFirst, unsigned char const *aLast)
{
    AddrType addr = aDest;
    for(unsigned char const *p = aFirst; p <= aLast; ++addr, p += 2)
    {
        aXuv.data[addr] = (p[0] << 8) | p[1];
    }
}

/*****************************************************************
@brief Incorporate a contiguous byte block into a xuv image. (little endian).

Incorporate a contiguous byte block containing uint16 values in little endian byte
order into a xuv image.

@param[in out] aXuv     image struct containing xuv image
@param[in] aDest        address of first destination word.
@param[in] aFirst       start address of byte block to be incorporated.
@param[in] aLast        last address of byte block to be incorporated.
*/
void ByteBlockIncorporateU16LE(image &aXuv, AddrType aDest, unsigned char const *aFirst, unsigned char const *aLast)
{
    AddrType addr = aDest;
    for(unsigned char const *p = aFirst; p <= aLast; ++addr, p += 2)
    {
        aXuv.data[addr] = (p[1] << 8) | p[0];
    }
}

/*****************************************************************
@brief Incorporate a contiguous byte block into a xuv image. (little endian).

Incorporate a contiguous byte block containing uint16 values in little endian byte
order into a xuv image.

@param[in out] aXuv     image struct containing xuv image
@param[in] aDest        address of first destination word.
@param[in] aFirst       start address of byte block to be incorporated.
@param[in] aLast        last address of byte block to be incorporated.
@param[in] aBe          the data is big endian.
*/
void ByteBlockIncorporateU16(image &aXuv, AddrType aDest, unsigned char const *aFirst, unsigned char const *aLast, bool aBe)
{
    if(aBe)
    {
        ByteBlockIncorporateU16BE(aXuv, aDest, aFirst, aLast);
    }
    else
    {
        ByteBlockIncorporateU16LE(aXuv, aDest, aFirst, aLast);
    }
}

} /* namespace xuv */

#if 0
/*
This is an example test program for round tripping a xuv file.
It reads file "app-orig.xuv" and outputs to console "/dev/stdout".
*/
using namespace xuv;

int main(int argc, char *argv[])
{
    image xi;
    ReadStatus r = Read(xi, "app-orig.xuv");
    WriteStatus s = Write(xi, "/dev/stdout");
    return EXIT_SUCCESS;
}
#endif