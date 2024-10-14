//*******************************************************************************
//
//  CRC.h
//
//  Copyright (c) 2000-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Calculate ANSI X3.66 32-bit cyclic redundancy check (CRC) values, as used 
//  within DFU files.
//
//  The constructor can optionally be passed an initialisation value, otherwise 
//  the default of 0xFFFFFFFF is used (correct for DFU files).
//
//  Operator () should be called for each block of data to be included in the 
//  CRC. It returns the result up to that point. To just read the current value 
//  without affecting the CRC use operator () without any parameters.
//
//*******************************************************************************

#ifndef CRC_H
#define CRC_H

#include "common/types.h"

// CRC class
class CRC  
{
public:

    // Constructor
    CRC(uint32 aInitCRC = 0xFFFFFFFF);

    // Update the CRC and return the current value
    uint32 operator()(const void *apBuffer = 0, 
                      size_t aBufferLength = 0);

private:
    // The accumulator
    uint32 mCrc;
};

#endif
