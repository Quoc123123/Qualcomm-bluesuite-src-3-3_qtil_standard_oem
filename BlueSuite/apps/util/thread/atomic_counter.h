/**********************************************************************
*
* atomic_counter.h
*
* Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* Atomic counter class.
*
**********************************************************************/
#ifndef ATOMIC_COUNTER_H_
#define ATOMIC_COUNTER_H_

class AtomicCounter
{
public:
	explicit AtomicCounter(long init_val = 0);
	AtomicCounter(const AtomicCounter &);
	~AtomicCounter();

	long inc();
	long dec();

	long operator++() { return inc(); }
	long operator--() { return dec(); }

	void assign(long);
	long read();

private:
	class Impl;
	Impl *impl_;

	// protect assignment to stop it happening accidentally.
	AtomicCounter& operator=(const AtomicCounter&);
};

#endif
