/*
   Copyright � 2001-2020, The AROS Development Team. All rights reserved.
   $Id$

   Desc: Partition initialization code
   Lang: English
*/

#include <aros/symbolsets.h>
#include <aros/debug.h>
#include <proto/alib.h>
#include "debug.h"

#include "partition_intern.h"
#include "partition_support.h"
#include LC_LIBDEFS_FILE

static int PartitionInit(LIBBASETYPEPTR LIBBASE)
{
    D(bug("[PART] PartitionInit\n"));
    
    LIBBASE->partbase.tables =  (struct PartitionTableInfo **)PartitionSupport;
    NewList(&LIBBASE->bootList);
    InitSemaphore(&LIBBASE->bootSem);

    /*
     * This is intentionally allowed to fail.
     * It will fail if we are in kickstart; partition.library is initialized
     * long before dos.library.
     */
    LIBBASE->pb_DOSBase = TaggedOpenLibrary(TAGGEDOPEN_DOS);

    return TRUE;
}

static int PartitionCleanup(struct PartitionBase_intern *base)
{
    D(bug("[PART] PartitionCleanup\n"));
    
    /* If there's something in our boot list, we can't quit without losing it */
    if (!IsListEmpty(&base->bootList))
    	return FALSE;

    if (base->pb_DOSBase)
    	CloseLibrary(base->pb_DOSBase);

    return TRUE;
}

ADD2INITLIB(PartitionInit, 0);
ADD2EXPUNGELIB(PartitionCleanup, 0);
