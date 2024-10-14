/******************************************************************************
 The UPDCRC macro is derived from an article Copyright 1986 by Stephen Satchell.

 Programmers may incorporate any or all code into their programs, giving
 proper credit within the source. Publication of the source routines is
 permitted so long as proper credit is given to Steven Satchell, Satchell
 Evaluations, and Chuck Forsberg, Omen technology."
*****************************************************************************/
#define UPDCRC(accum,delta) (accum)=crctbl[((accum)^(delta))&0xff]^((accum)>>8)

/* Defined in crctbl.c */
#ifdef __XAP__
extern unsigned long const crctbl[];
#else
#include <common/types_t.h>
extern uint32_t const crctbl[];
#endif
