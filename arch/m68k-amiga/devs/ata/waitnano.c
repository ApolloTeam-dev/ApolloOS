/*
    Copyright © 2013, The AROS Development Team. All rights reserved
    $Id$
*/

#include <exec/types.h>
#include <exec/io.h>
#include <hardware/cia.h>
#include <aros/debug.h>

#include "timer.h"
#include "ata.h"

BOOL ata_Calibrate(struct IORequest* tmr, struct ataBase *base)
{
    base->ata_ItersPer100ns = 1;
    return TRUE;
}

static void busywait(UWORD cnt)
{
}

/* Single CIA access = 1 E-clock */
void ata_WaitNano(ULONG ns, struct ataBase *base)
{
}
