/*
    Copyright © 1995-2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Add interrupt client to chain of interrupt servers
    Lang: english
*/

#include <aros/debug.h>
#include <aros/libcall.h>
#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>

#include "exec_intern.h"
#include "exec_debug.h"
#include "chipset.h"
#include "exec_locks.h"

static void krnIRQwrapper(void *data1,  void *data2)
{
    struct Interrupt *irq = (struct Interrupt *)data1;
    struct ExecBase *SysBase = (struct ExecBase *)data2;

    AROS_INTC1(irq->is_Code, irq->is_Data);
}

/*****************************************************************************

    NAME */

	AROS_LH2(void, AddIntServer,

/*  SYNOPSIS */
	AROS_LHA(ULONG,              intNumber, D0),
	AROS_LHA(struct Interrupt *, interrupt, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 28, Exec)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES
	This function also enables the corresponding chipset interrupt if
	run on a native Amiga.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    ExecLog(SysBase, EXECDEBUGF_EXCEPTHANDLER, "AddIntServer: Int %d, Interrupt %p\n", intNumber, interrupt);

    /* ------------------------------------------------------------
     * Kernel IRQs are NOT handled here.
     * ------------------------------------------------------------ */
    if (intNumber >= INTB_KERNEL) {
        /* N.B. ln_Succ is being re-purposed/abused here */
        interrupt->is_Node.ln_Succ = KrnAddIRQHandler(intNumber - INTB_KERNEL, krnIRQwrapper, interrupt, SysBase);
        return;
    }

    struct List *list = (struct List *)SysBase->IntVects[intNumber].iv_Data;
    volatile UWORD *INTENA = (UWORD *)0xDFF09A;

    /* ------------------------------------------------------------
     * Disable() — classic Exec
     * ------------------------------------------------------------ */
    Disable();

    /* ------------------------------------------------------------
     * Insert interrupt server into vector list
     * ------------------------------------------------------------ */
    Enqueue(list, &interrupt->is_Node);

    /* ------------------------------------------------------------
     * Enable the corresponding hardware interrupt
     * ------------------------------------------------------------ */
    *INTENA = 0x8000 | (1 << intNumber);

    /* ------------------------------------------------------------
     * Enable() — classic Exec
     * ------------------------------------------------------------ */
    Enable();

    AROS_LIBFUNC_EXIT
} /* AddIntServer */
