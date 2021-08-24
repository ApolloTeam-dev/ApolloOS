/*
    Copyright © 2013, The AROS Development Team. All rights reserved
    $Id$
*/

#include <aros/debug.h>

#include <proto/exec.h>

/* We want all other bases obtained from our base */
#define __NOLIBBASE__

#include <proto/timer.h>

#include <exec/types.h>
#include <devices/timer.h>
#include <exec/io.h>

#include "timer.h"
#include "ata.h"

BOOL ata_Calibrate(struct IORequest* tmr, struct ataBase *base)
{
    return TRUE;
}

void ata_WaitNano(register ULONG ns, struct ataBase *base)
{
}
