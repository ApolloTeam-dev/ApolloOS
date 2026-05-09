/*
    Copyright © 1995-2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Disable() - Stop interrupts from occurring.
    Lang: english
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <aros/libcall.h>

#include "exec_intern.h"

/*****************************************************************************/
#undef  Exec
#ifdef UseExecstubs
#    define Exec _Exec
#endif

/*  NAME */
#include <proto/exec.h>

    AROS_LH0(void, Disable,

/*  LOCATION */
    struct ExecBase *, SysBase, 20, Exec)

/*  FUNCTION
    This function will prevent interrupts from occurring. You can
    start the interrupts again with a call to Enable().

    Note that calls to Disable() nest, and for every call to
    Disable() you need a matching call to Enable().

    ***** WARNING *****

    Using this function is considered very harmful, and it should only
    ever be used to protect data that could also be accessed in interrupts.

    It is quite possible to either crash the system, or to prevent
    normal activities (disk/port i/o) from occurring.

    INPUTS

    RESULT
    Interrupts will be disabled AFTER this call returns.

    NOTES
    This function preserves all registers.

    As the scheduler's pre-emption is interrupt driven,
    this function has the side effect of disabling multitasking.

    EXAMPLE
    In most userspace code, you will not want to use this function.

    BUGS
    The only architecture that you can rely on the registers being
    saved is on the Motorola mc68000 family.

    SEE ALSO
    Forbid(), Permit(), Enable(), Wait()

    INTERNALS

******************************************************************************/
{
#undef Exec

    AROS_LIBFUNC_INIT

    D(bug("[Exec] Disable()\n");)

    /*
     *  - INTENA = $4000 (clear + disable all interrupts)
     *  - IDNestCnt++
     */
    volatile UWORD *INTENA = (UWORD *)0xDFF09A;

    *INTENA = 0x4000;
    SysBase->IDNestCnt++;

    D(bug("[Exec] Disable: IDNESTCOUNT = %d\n", SysBase->IDNestCnt);)

    AROS_LIBFUNC_EXIT
} /* Disable() */
