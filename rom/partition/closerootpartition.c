/*
    Copyright � 1995-2017, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <proto/exec.h>
#include <exec/memory.h>

#include "partition_intern.h"
#include "partition_support.h"
#include "platform.h"
#include "debug.h"

/*****************************************************************************

    NAME */
#include <libraries/partition.h>

        AROS_LH1(void, CloseRootPartition,

/*  SYNOPSIS */
        AROS_LHA(struct PartitionHandle *, ph,       A1),

/*  LOCATION */
        struct Library *, PartitionBase, 6, Partition)

/*  FUNCTION
        Close a root handle allocated by OpenRootPartition().

    INPUTS
        ph - root handle created by OpenRootPartition()

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    D(bug("[PART:ROOT] CloseRootPartition\n"));

    if (ph->table)
    {
        struct PTFunctionTable *handler = ph->table->handler;

        if (handler->closePartitionTable)
            handler->closePartitionTable(PartitionBase, ph);
    }

    CloseDevice((struct IORequest *)ph->bd->ioreq);
    DeleteIORequest((struct IORequest *)ph->bd->ioreq);
    DeleteMsgPort(ph->bd->port);
    FreeMem(ph->bd, sizeof(struct PartitionBlockDevice));
    FreeMem(ph, sizeof(struct PartitionHandle));

    AROS_LIBFUNC_EXIT
}
