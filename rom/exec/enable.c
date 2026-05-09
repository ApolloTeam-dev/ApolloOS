/*
    Copyright © 1995-2018, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Enable() - Allow interrupts to occur after Disable().
    Lang: english
*/

#define DEBUG 0

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

        AROS_LH0(void, Enable,

/*  LOCATION */
        struct ExecBase *, SysBase, 21, Exec)

/*  FUNCTION
        This function will allow interrupts to occur after they have
        been disabled by Disable().

        Note that calls to Disable() nest, and for every call to
        Disable() you need a matching call to Enable().

        ***** WARNING *****

        Using this function is considered very harmful, and it should only
        ever be used to protect data that could also be accessed in interrupts.

    INPUTS
        None.

    RESULT
        Interrupts will be enabled again when this call returns.

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
        Forbid(), Permit(), Disable(), Wait()

    INTERNALS

******************************************************************************/
{
#undef Exec

    AROS_LIBFUNC_INIT

    D(bug("[Exec] Enable()\n");)

    /* Classic AmigaOS 1.2–3.1 Enable():
     *  - IDNestCnt--
     *  - If negative, INTENA = $C000 (enable all)
     */
    volatile UWORD *INTENA = (UWORD *)0xDFF09A;

    SysBase->IDNestCnt--;

    D(bug("[Exec] Enable: IDNESTCOUNT = %d\n", SysBase->IDNestCnt);)

    if ((BYTE)SysBase->IDNestCnt < 0)
    {
        D(bug("[Exec] Enable: Enabling interrupts\n");)
        *INTENA = 0xC000;
    }

    AROS_LIBFUNC_EXIT
} /* Enable() */
