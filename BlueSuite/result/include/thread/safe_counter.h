///////////////////////////////////////////////////////////////////////
//
//  FILE   :  safe_counter.h
//
//            Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  To declare a thread safe "counted" base class for
//            pointers from common/counter_pointer.h
//
//  WARNING:  This class hass a high overhead.
//
///////////////////////////////////////////////////////////////////////

#include "thread/critical_section.h"
#include "common/countedpointer.h"

#ifndef SAFE_COUNTER_H
#define SAFE_COUNTER_H

class SafeCounter
{
  mutable int count;
  mutable CriticalSection p;

public:
  SafeCounter()
  { count = 0; }
  SafeCounter(const SafeCounter &)
  { }
  virtual ~SafeCounter()
  { }
  SafeCounter &operator=(const SafeCounter &)
  { return *this; }

  void inc() const
  { CriticalSection::Lock here(p); ++count; }
  void dec() const
  {
    {
      CriticalSection::Lock here(p);
      if(--count)
          return;
    }
    delete this;
  }
};

#endif
