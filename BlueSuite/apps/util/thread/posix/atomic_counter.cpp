/**********************************************************************
*
* atomic_counter.cpp
*
* Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* Atomic counter class (posix implementation).
*
**********************************************************************/
#include "thread/atomic_counter.h"
#include "thread/critical_section.h"

class AtomicCounter::Impl
{
public:
	Impl(long init_val) : counter(init_val) {}

    CriticalSection cs;
	long counter;
};

AtomicCounter::AtomicCounter(long init_val) : impl_(new Impl(init_val)) {}

AtomicCounter::AtomicCounter(const AtomicCounter &orig) : impl_(new Impl(orig.impl_->counter)) {}

AtomicCounter::~AtomicCounter()
{
	delete impl_;
}

long AtomicCounter::inc()
{
    CriticalSection::Lock lock(impl_->cs);
    ++(impl_->counter);
	return impl_->counter;
}

long AtomicCounter::dec()
{
    CriticalSection::Lock lock(impl_->cs);
    --(impl_->counter);
	return impl_->counter;
}

void AtomicCounter::assign(long v)
{
	impl_->counter = v;
}

long AtomicCounter::read()
{
	return impl_->counter;
}

