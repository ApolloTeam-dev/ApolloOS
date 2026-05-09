/*
    Copyright © 1995-2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Remove an interrupt handler.
    Lang:
*/

#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>
#include <aros/libcall.h>

#include "exec_intern.h"
#include "exec_debug.h"
#include "chipset.h"
#include "exec_locks.h"

/*****************************************************************************

    NAME */

    AROS_LH2(void, RemIntServer,

/*  SYNOPSIS */
    AROS_LHA(ULONG,              intNumber, D0),
    AROS_LHA(struct Interrupt *, interrupt, A1),

/*  LOCATION */
    struct ExecBase *, SysBase, 29, Exec)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    ExecLog(SysBase, EXECDEBUGF_EXCEPTHANDLER,
            "RemIntServer: Int %d, Interrupt %p\n",
            intNumber, interrupt);

    /* Kernel IRQs still go through the AROS kernel handler path */
    if (intNumber >= INTB_KERNEL) {
        KrnRemIRQHandler(interrupt->is_Node.ln_Succ);
        return;
    }

    volatile UWORD *INTENA = (UWORD *)0xDFF09A;

    /* ------------------------------------------------------------
     * Disable() — same logic as in AddIntServer()
     *  - Write INTENA = $4000 (clear + disable all interrupts)
     *  - Increment IDNestCnt
     *  - Does NOT modify CPU SR
     * ------------------------------------------------------------ */
    *INTENA = 0x4000;
    SysBase->IDNestCnt++;

    /* ------------------------------------------------------------
     * Remove the interrupt server from the list
     * ------------------------------------------------------------ */
    Remove(&interrupt->is_Node);

    /* ------------------------------------------------------------
     * Enable() — same logic as in AddIntServer()
     *  - Decrement IDNestCnt
     *  - If it becomes negative, write INTENA = $C000 (enable all)
     * ------------------------------------------------------------ */
    SysBase->IDNestCnt--;
    if ((BYTE)SysBase->IDNestCnt < 0)
        *INTENA = 0xC000;

    AROS_LIBFUNC_EXIT
} /* RemIntServer */
