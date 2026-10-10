/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id$	  $Log

    Desc: Graphics function SetFont()
    Lang: english
*/
#include "graphics_intern.h"
#include <graphics/rastport.h>
#include <graphics/text.h>

/*****************************************************************************

    NAME */
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <proto/graphics.h>

	AROS_LH2(void, SetFont,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A1),
	AROS_LHA(struct TextFont *, textFont, A0),

/*  LOCATION */
	struct GfxBase *, GfxBase, 11, Graphics)

/*  FUNCTION
	Select a new font for rendering strings in a RastPort.

    INPUTS
	rp - Change this RastPort
	textFont - This is the new font

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

	if(!rp)
        return;

    if(textFont)
    {
    	if(ExtendFont(textFont, 0))
    	{
    		rp->TxWidth    = textFont->tf_XSize;
	 		rp->TxHeight   = textFont->tf_YSize;
			rp->TxBaseline = textFont->tf_Baseline;	
    	}
    	else return;
    }
    else
    {
        rp->TxWidth = 0;
        rp->TxHeight = 0;      
        rp->TxBaseline = 0; 
    }
    rp->AlgoStyle = 0;
    rp->Font = textFont;
  
    AROS_LIBFUNC_EXIT
    
} /* SetFont */
