/**********************************************************************
*
*  FILE   :  nocopy.h
*
*            Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  A simple base class which says "this class can't be 
*            copied". Just like boost::nocopyable, but boost has 
*            problems with WinCE.
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/result/include/common/nocopy.h#1 $
*
***********************************************************************/

#ifndef COMMON__NOCOPY_H
#define COMMON__NOCOPY_H

class NoCopy
{
private:
    NoCopy(const NoCopy &);
    const NoCopy &operator=(const NoCopy &);
protected:
    NoCopy() { }
    ~NoCopy() { }
};

#endif
