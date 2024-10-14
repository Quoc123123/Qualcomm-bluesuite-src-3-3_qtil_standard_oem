/***********************************************************************************
 *
 *  uestores.c
 *
 *  Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  uenergy memory and storage subsystem details
 *
 ***********************************************************************************/
#include <assert.h>
#include "uestores.h"

/******************************************************************************
@brief Unpack and expand USED SIZE from App Store additional header.

@param[in] aUsedSize   The USED SIZE field extract from the addtional header.

@return     number of octets used or -1 for invalid.

@note USED SIZE field is interpreted as follows:
details  Size is encoded as follows, see CS-302333-DD
    - bit 15:14 = b00 : size = bits 13:0 x 1  octets
    - bit 15:14 = b01 : size = bits 13:0 x 16 octets
    - bit 15:14 = b10 : size = bits 13:0 x 4K octets
    - bit 15:14 = b11 : do not use, spare for future use

@note
Value is number of octets.
-1 is used for invalid value. I expect size_t is 32-bits which gives an address
space of 4 gigabytes. uEnergy is unlikely to have 4GB of memory anytime soon.
*/
size_t NormaliseUsedSize(size_t aUsedSize)
{
    const size_t ENCODING_MASK = 0xC000;
    const size_t SIZE_MASK = 0x3FFF;
    const size_t ENCODING_BITPOS = 14;
    size_t UsedSize = 0;
    switch((aUsedSize & ENCODING_MASK) >> ENCODING_BITPOS)
    {
    case 0:
        UsedSize = (aUsedSize & SIZE_MASK);
        break;
    case 1:
        UsedSize = (aUsedSize & SIZE_MASK) * 16;
        break;
    case 2:
        UsedSize = (aUsedSize & SIZE_MASK) * 4096;
        break;
    default: /* reserved for future */
        UsedSize = -1;
    }
    return UsedSize;
}
