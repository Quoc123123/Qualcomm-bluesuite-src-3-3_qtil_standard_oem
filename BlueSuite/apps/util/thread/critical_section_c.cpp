/**********************************************************************
*
* critical_section_c.cpp
*
* Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
* All Rights Reserved.
* Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*
* Critical section wrappers.
*
**********************************************************************/
#include "critical_section.h"

struct critical_section
{
    CriticalSection cs;
    explicit critical_section(bool recursive) : cs(recursive) {}
};

struct critical_section *create_non_recursive_critical_section()
{
    return new critical_section(false);
}

struct critical_section *create_recursive_critical_section()
{
    return new critical_section(true);
}

void destroy_critical_section(struct critical_section *cs)
{
    delete cs;
}

void lock_critical_section(struct critical_section *cs)
{
    cs->cs.Enter();
}

void unlock_critical_section(struct critical_section *cs)
{
    cs->cs.Leave();
}
