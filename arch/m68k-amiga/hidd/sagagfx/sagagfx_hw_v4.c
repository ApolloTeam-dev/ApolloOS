/*
    Copyright © 1995-2020, The AROS Development Team. All rights reserved.
     $Id$

    Desc: SAGA Gfx Hidd for AROS (V4)
    Lang: english
*/

#define DEBUG 0

#include <aros/debug.h>

#include <exec/exec.h>
#include <devices/inputevent.h>
#include <proto/exec.h>
#include <proto/input.h>

#include "sagagfx_hw.h"

void SAGA_SetPLL_V4(ULONG clock)
{
    D(bug("[SAGA] SAGA_SetPLL_V4(%d)\n", clock));
}



/* END OF FILE */
