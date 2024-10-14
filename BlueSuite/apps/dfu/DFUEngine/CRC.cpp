//*******************************************************************************
//
//  CRC.cpp
//
//  Copyright (c) 2000-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Calculate ANSI X3.66 32-bit cyclic redundancy check (CRC) values, as used 
//  within DFU files.
//
//*******************************************************************************

#include "CRC.h"
extern "C"
{
#include "crc/crctbl.h"
}

// Constructor
CRC::CRC(uint32 aInitCRC)
{
    mCrc = aInitCRC;
}

// Update the CRC and return the current value
uint32 CRC::operator()(const void *apBuffer, size_t aBufferLength)
{
    // Include the new data within the CRC
    const uint8 * const pBuffer = (uint8 *)apBuffer;
    for (size_t index = 0; index < aBufferLength; ++index)
    {
        mCrc = crctbl[(mCrc ^ pBuffer[index]) & 0xff] ^ (mCrc >> 8);
    }

    // Return the current value
    return mCrc;
}
