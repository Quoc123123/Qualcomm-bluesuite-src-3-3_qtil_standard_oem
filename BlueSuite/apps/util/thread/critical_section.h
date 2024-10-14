///////////////////////////////////////////////////////////////////////
//
//  FILE   :  critical_section.h
//
//            Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
//            All Rights Reserved.
//            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PURPOSE:  Header file for all the machine dependent
//            definitions of Critical section control functions.
//            This bit should be the same for all of them.
//
//  $Id: //depot/hosttools/source_releases/BlueSuite/BlueSuite.SRC.3.3/SRC/BlueSuite/apps/util/thread/critical_section.h#1 $
//
///////////////////////////////////////////////////////////////////////

#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct critical_section critical_section;

struct critical_section *create_non_recursive_critical_section();
struct critical_section *create_recursive_critical_section();
void destroy_critical_section(struct critical_section *);
void lock_critical_section(struct critical_section *);
void unlock_critical_section(struct critical_section *);

#ifdef __cplusplus
}  // extern "C"

#include "common/nocopy.h"

class CriticalSection : private NoCopy
{
public:
    CriticalSection()
        : cs(InitialiseCS(true))
    { }
    explicit CriticalSection(bool recursive)
        : cs(InitialiseCS(recursive))
    { }

    ~CriticalSection()
    { DestroyCS(cs); }

    class Lock : private NoCopy
    {
    public:
        Lock(CriticalSection &x)
            : locked(x)
        { locked.Enter(); }

        Lock(CriticalSection *x)
            : locked(*x)
        { locked.Enter(); }

        ~Lock()
        { locked.Leave(); }

    private:
        CriticalSection &locked;
    };

    class AntiLock : private NoCopy
    {
    public:
        AntiLock(CriticalSection &x)
            : locked(x)
        { locked.Leave(); }

        AntiLock(CriticalSection *x)
            : locked(*x)
        { locked.Leave(); }

        ~AntiLock()
        { locked.Enter(); }

    private:
        CriticalSection &locked;
    };

    void Enter() { EnterCS(cs); }
    void Leave() { LeaveCS(cs); }
private:
    void *cs;

    // Platform dependent code
    static void *InitialiseCS(bool recursive);
    static void EnterCS(void *section);
    static void LeaveCS(void *section);
    static void DestroyCS(void *section);
};

#endif // __cplusplus

#endif /* ndef CRITICAL_SECTION_H */
