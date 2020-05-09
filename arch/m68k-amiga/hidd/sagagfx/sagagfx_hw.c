/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
     $Id$

    Desc: SAGA Gfx Hidd for AROS
    Lang: english
*/

#define DEBUG 0

#include <aros/debug.h>

#include <exec/exec.h>
#include <devices/inputevent.h>
#include <proto/exec.h>
#include <proto/input.h>

#include "sagagfx_hw.h"

void SAGA_SetPLL(ULONG clock)
{
	UBYTE model = (READ16(VREG_BOARD) >> 8);
	
	switch (model)
	{
		case VREG_BOARD_V4:
		case VREG_BOARD_V4SA:
			// V4 VIDEO PLL METHOD
			SAGA_SetPLL_V4(clock);
			break;
		default:
			// V2 VIDEO PLL METHOD
			SAGA_SetPLL_V2(clock);
			break;
	}
}

void SAGA_LoadCLUT(ULONG *palette, UWORD startIndex, UWORD count)
{
    if (palette)
    {
        if(startIndex > 255)
            return;

        if(startIndex + count > 256)
            count = 256 - startIndex;

        for (int i=0; i < count; i++)
        {
            WRITE32(SAGA_VIDEO_CLUT(startIndex + i), palette[startIndex + i]);
        }
    }
}

/* Attempts to detect SAGA. */
BOOL SAGA_Init()
{
    struct IORequest io;

    /* Read Vampire BOARD register */
	UBYTE model = (READ16(VREG_BOARD) >> 8);
	
    /* Do we have Vampire detected? */
	if (!(model > VREG_BOARD_Unknown && model < VREG_BOARD_Future))
	{
		// Probably not a Vampire hardware, better exit.
		return FALSE;
	}
	
    /* If SHIFT key was pressed during boot, do not initialize SAGA Gfx hidd */
    if (0 == OpenDevice("input.device", 0, &io, 0))
    {
        struct Library *InputBase = (struct Library *)io.io_Device;
        UWORD qual = PeekQualifier();
        CloseDevice(&io);

        if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
        {
            return FALSE;
        }
    }

    return TRUE;
}
