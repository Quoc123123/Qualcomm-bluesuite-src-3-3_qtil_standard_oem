/**********************************************************************
*
*  FILE   :  countedpointer.h
*
*            Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
*            All Rights Reserved.
*            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
*  PURPOSE:  Simple reference counted pointer
*
*  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/result/include/common/countedpointer.h#1 $
*
***********************************************************************/

#ifndef COUNTEDPOINTER_H
#define COUNTEDPOINTER_H

//  Counter is a base class with a virtual destructor.
//  Any class which inherits from it will work with a CountedPointer.
class Counter
{
  mutable int count;

public:
  Counter()
  { count = 0; }
  Counter(const Counter &)
  { }
  virtual ~Counter()
  { }
  Counter &operator=(const Counter &)
  { return *this; }

  void inc() const
  { ++count; }
  void dec() const
  { if(--count == 0) delete this; }
};

//  Counted is a template for a counter base class which requires
//  no virtual destructor.
//  By constructing a class as
//    class T : public Counted<T> {...};
//  the class T contains a count, and the cast in Counted<T>::dec()
//  is compiled with the knowledge of what T is, so Counted<T>
//  doesn't need a virtual destructor.
//  Any class contructed in this way will work with a CountedPointer.
template<class T> class Counted
{
  mutable int count;

public:
  Counted()
  { count = 0; }
  Counted(const Counted<T> &)
  { }
  Counted &operator=(const Counted<T> &)
  { return *this; }

  void inc() const
  { ++count; }
  void dec() const
  { if(--count == 0) delete (T *) this; }
};

//  Template for a reference counted pointer.
//  The template class T should inherit from one of the two
//  classes above, or at least provide inc() and dec() methods.
template<class T> class CountedPointer
{
  T *ptr;
  void set(T *p)
  { ptr = p; if(ptr) ptr->inc(); }
  void clr()
  { if(ptr) ptr->dec(); ptr = 0; }

public:
  CountedPointer(T *p = 0)
  { set(p); }
  
  // We need an un-templated copy constructor as well as the templated variety
  // to stop the compiler generating a default copy constructor. Similarly with
  // operator=() below.
  CountedPointer(const CountedPointer<T> &x)
  { set(x.ptr); }
#if !defined(_MSC_VER) || _MSC_VER >= 1400
  // This code doesn't compile with MSVC 6.0
  template<class U>
  CountedPointer(const CountedPointer<U> &x)
  { set(x.ptr); }
#endif /* !defined(_MSC_VER) || _MSC_VER >= 1400 */
  
  ~CountedPointer()
  { clr(); }
  
  CountedPointer<T> &operator=(const CountedPointer<T> &x)
  { if(ptr != x.ptr) { clr(); set(x.ptr); } return *this; }
#if !defined(_MSC_VER) || _MSC_VER >= 1400
  // This code doesn't compile with MSVC 6.0
  template<class U>
  CountedPointer<T> &operator=(const CountedPointer<U> &x)
  { if(ptr != x.ptr) { clr(); set(x.ptr); } return *this; }

  template<class U> friend class CountedPointer;
#endif /* !defined(_MSC_VER) || _MSC_VER >= 1400 */

  T &operator*() const
  { return *ptr; }
  T *operator->() const
  { return ptr; }
  operator bool() const
  { return ptr != 0; }
};
  
#endif
