/*
    Copyright © 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: CacheClearE() - Clear the caches with extended control.
    Lang: english
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <aros/libcall.h>

extern void AROS_SLIB_ENTRY(CacheClearE_00,Exec,107)(void);

#include <proto/exec.h>

/* See rom/exec/cachecleare.c for documentation */

AROS_LH3(void, CacheClearE,
    AROS_LHA(APTR, address, A0),
    AROS_LHA(ULONG, length, D0),
    AROS_LHA(ULONG, caches, D1),
    struct ExecBase *, SysBase, 107, Exec)
{
    AROS_LIBFUNC_INIT

    void (*func)();

    if (SysBase->LibNode.lib_OpenCnt == 0)
    	return;

    Disable();
        /* Everybody else (68000, 68010) */
    func = AROS_SLIB_ENTRY(CacheClearE_00, Exec, 107);

    AROS_UFC3NR(void, func,
	AROS_UFCA(APTR, address, A0),
	AROS_UFCA(ULONG, length, D0),
	AROS_UFCA(ULONG, caches, D1));
    SetFunction((struct Library *)SysBase, -LIB_VECTSIZE * 107, func);
    Enable();

    AROS_LIBFUNC_EXIT
} /* CacheClearE */

