/*
    Copyright © 1995-2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Send a message to a port.
    Lang: english
*/

#define DEBUG 0
#include <aros/debug.h>

#include <aros/libcall.h>
#include <exec/ports.h>
#include <proto/exec.h>

#include "exec_intern.h"
#include "exec_util.h"

/*****************************************************************************

    NAME */

	AROS_LH2(void, PutMsg,

/*  SYNOPSIS */
	AROS_LHA(struct MsgPort *, port,    A0),
	AROS_LHA(struct Message *, message, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 61, Exec)

/*  FUNCTION
	Sends a message to a given message port. Messages are not copied
	from one task to another but must lie in shared memory instead.
	Therefore the owner of the message may generally not reuse it before
	it is returned. But this depends on the two tasks sharing the message.

    INPUTS
	port	- Pointer to messageport.
	message - Pointer to message.

    RESULT

    NOTES
	It is legal to send a message from within interrupts.

	Messages may either trigger a signal at the owner of the messageport
	or raise a software interrupt, depending on port->mp_Flags&PF_ACTION.

    EXAMPLE

    BUGS

    SEE ALSO
	WaitPort(), GetMsg()

    INTERNALS

******************************************************************************/
{
    AROS_LIBFUNC_INIT
    ASSERT_VALID_PTR(message);
    ASSERT_VALID_PTR(port);

    /* Set the node type to NT_MESSAGE == sent message. */
    message->mn_Node.ln_Type = NT_MESSAGE;

    InternalPutMsg(port, message, SysBase);

    AROS_LIBFUNC_EXIT
} /* PutMsg() */

void InternalPutMsg(struct MsgPort *port, struct Message *message, struct ExecBase *SysBase)
{
    /*
     * Add a message to the ports list.
     * NB : Messages may be sent from interrupts, therefore
     * the message list of the message port must be protected
     * with Disable() for the local core, and also a spinlock
     * on smp systems.
     */

    D(bug("[EXEC] PutMsg: Port @ 0x%p, Msg @ 0x%p\n", port, message);)

    /*
     * Capture port fields while under Disable() so they remain consistent
     * even if an interrupt causes the port owner to wake and modify/free
     * the port before we get to Signal()/Cause().
     */
    {
	APTR sigTask;
	UBYTE action;
	UBYTE sigBit;

	Disable();
#if defined(__AROSEXEC_SMP__)
	EXEC_SPINLOCK_LOCK(&port->mp_SpinLock, NULL, SPINLOCK_MODE_WRITE);
#endif
	AddTail(&port->mp_MsgList, &message->mn_Node);
	D(bug("[EXEC] PutMsg: Port MsgList->lh_TailPred =  0x%p\n", port->mp_MsgList.lh_TailPred);)

	sigTask = port->mp_SigTask;
	action  = port->mp_Flags & PF_ACTION;
	sigBit  = port->mp_SigBit;

#if defined(__AROSEXEC_SMP__)
	EXEC_SPINLOCK_UNLOCK(&port->mp_SpinLock);
#endif
	Enable();

	if (sigTask)
	{
	    ASSERT_VALID_PTR(sigTask);

	    /* And trigger the action. */
	    switch(action)
	    {
		case PA_SIGNAL:
		    D(bug("[EXEC] PutMsg: PA_SIGNAL -> Task 0x%p, Signal %08x\n", sigTask, (1 << sigBit));)

		    /* Send the signal */
		    Signal((struct Task *)sigTask, (1 << sigBit));
		    break;

		case PA_SOFTINT:
		    D(bug("[EXEC] PutMsg: PA_SOFTINT -> Int %s\n", ((struct Interrupt *)sigTask)->is_Node.ln_Name);)

		    /* Raise a software interrupt */
		    Cause((struct Interrupt *)sigTask);
		    break;

		case PA_IGNORE:
		    /* Do nothing. */
		    break;

		case PA_CALL:
		    D(bug("[EXEC] PutMsg: PA_CALL -> Func @ 0x%p, Port @ 0x%p\n", sigTask, port);)

#if defined(__AROSEXEC_SMP__)
		    //TODO! - the called function must be SMP safe.
#endif
		    /* Call the function in mp_SigTask. */
		    AROS_UFC2NR(void, sigTask,
			AROS_UFCA(struct MsgPort *,  port,    D0),
			AROS_UFCA(struct ExecBase *, SysBase, A6));
		    break;
	    }
	}
    }
}
