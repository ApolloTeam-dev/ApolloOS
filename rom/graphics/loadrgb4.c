/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include <aros/debug.h>
#include "graphics_intern.h"

/*****************************************************************************

    NAME */
	#include <proto/graphics.h>

	AROS_LH3(void, LoadRGB4,

/*  SYNOPSIS */
	AROS_LHA(struct ViewPort *, vp, A0),
	AROS_LHA(UWORD           *, colors, A1),
	AROS_LHA(WORD             , count, D0),

/*  LOCATION */
	struct GfxBase *, GfxBase, 32, Graphics)

/*  FUNCTION
	Load RGB color values from table.

    INPUTS
	vp     - ViewPort
	colors - pointer to table of RGB values (0...15)
		 	background--  0x0RGB
			color1	  --  0x0RGB
			color2    --  0x0RGB
			...
	count	- number of UWORDs in the table
	
    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	LoadRGB32()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    WORD t;

    if (!vp)
        return;

    ASSERT_VALID_PTR(vp);
    ASSERT_VALID_PTR(colors);

    /* TODO: Optimization */

    for (t = 0; t < count; t ++ )
    {
    	UWORD r = (colors[t] & 0xF00) >> 8;
    	UWORD g = (colors[t] & 0x0F0) >> 4;
    	UWORD b = (colors[t] & 0x00F);

        r = (( r & 0xF) << 4) | (r & 0xFF );
        r = r | (r << 8) | ( r << 16 ) | (r << 24 );
        g = (( g & 0xF) << 4) | (g & 0xFF );
        g = g | (g << 8) | ( g << 16 ) | (g << 24 );
        b = (( b & 0xF) << 4) | (b & 0xFF );
        b = b | (b << 8) | ( b << 16 ) | (b << 24 );

        SetRGB32( vp, t, r, g, b );
    }

    AROS_LIBFUNC_EXIT
    
} /* LoadRGB4 */
