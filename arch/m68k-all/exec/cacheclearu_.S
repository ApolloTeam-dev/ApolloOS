/*
    Copyright © 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

/*****************************************************************************

    NAME
 
 	AROS_LH0(void, CacheClearU,
 
    LOCATION
 	struct ExecBase *, SysBase, 106, Exec)
 
    FUNCTION
 	Flushes the contents of all CPU caches in a simple way.
 
    INPUTS
 
    RESULT
 
    NOTES
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
 
    INTERNALS
	68000/10: do nothing
	68020/30: clear instruction cache and (030) data cache
	68040/60: push dirty lines to memory and invalidate both caches
 
    HISTORY
 
******************************************************************************/


/*
   XDEF AROS_SLIB_ENTRY(CacheClearU_00,Exec,106)   	; for 68000/68010
   XDEF AROS_SLIB_ENTRY(CacheClearU_20,Exec,106)	; for 68020/68030
   XDEF AROS_SLIB_ENTRY(CacheClearU_40,Exec,106)	; for 68040/68060
   XDEF AROS_SLIB_ENTRY(CacheClearU_60,Exec,106)	; for 68060 (clears not only
						; the i and d caches, but also
						; the branch cache)
*/

	#include "aros/m68k/asm.h"
	#include "cache.h"

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CacheClearU_00,Exec,106)
	.type	AROS_SLIB_ENTRY(CacheClearU_00,Exec,106),@function
AROS_SLIB_ENTRY(CacheClearU_00,Exec,106):
	/* Simple 68000s have no chaches */


	rts

