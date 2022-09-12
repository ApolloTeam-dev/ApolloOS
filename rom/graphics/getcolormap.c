/*
    Copyright © 1995-2016, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Graphics function GetColorMap()
    Lang: english
*/
#include <exec/memory.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <graphics/view.h>
#include "graphics_intern.h"

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

	AROS_LH1(struct ColorMap *, GetColorMap,

/*  SYNOPSIS */
	AROS_LHA(ULONG, entries, D0),

/*  LOCATION */
	struct GfxBase *, GfxBase, 95, Graphics)

/*  FUNCTION
	Allocates and initializes a ColorMap structure and passes back the
	pointer. This enables you to do calls to SetRGB4() and LoadRGB4()
	to load colors for a view port.
	The ColorTable pointer in the ColorMap structure points to a hardware
	specific colormap data structure which you should not interpret.

    INPUTS
	entries	- the number of entries for the colormap

    RESULT
	NULL  - not enough memory could be allocated for the necessary
	        data structures
	other - pointer to an initialized ColorMap structure that may be
	        stored into the ViewPort.ColorMap pointer.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	FreeColorMap(), SetRGB4(), graphics/view.h

    INTERNALS
	RGB Colortable with preference values is incomplete.

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct ColorMap * NewCM = (struct ColorMap *)AllocMem(sizeof(struct ColorMap),
                                                          MEMF_PUBLIC|MEMF_CLEAR);
    UWORD 	    * ptr1, * ptr2;
							  
#if 1
    /* ColorTable with some preference values;*/
    const WORD 	      RGBColorTable[] = {0x0000,0x0f00,0x00f0,0x0ff0,0x000f,0x0f0f,0x00ff,0x0fff,
    					 0x0620,0x0e50,0x09f1,0x0eb0,0x055f,0x092f,0x00f8,0x0ccc,
					 0x0000,0x0111,0x0222,0x0333,0x0444,0x0555,0x0666,0x0777,
					 0x0888,0x0999,0x0aaa,0x0bbb,0x0ccc,0x0ddd,0x0eee,0x0fff,
					 0x0fff,0x0888,0x0fff,0x0ccc,0x0444,0x0fff,0x0fff,0x08ff,
					 0x0488,0x0cff,0x06cc,0x0244,0x0aff,0x0eff,0x0cff,0x0688,
					 0x0fff,0x09cc,0x0344,0x0fff,0x0fff,0x04ff,0x0288,0x06ff,
					 0x03cc,0x0144,0x05ff,0x07ff,0x0f8f,0x0848,0x0fcf,0x0c6c,
					 0x0424,0x0faf,0x0fef,0x088f,0x0448,0x0ccf,0x066c,0x0224,
					 0x0aaf,0x0eef,0x0c8f,0x0648,0x0fcf,0x096c,0x0324,0x0faf,
					 0x0fef,0x048f,0x0248,0x06cf,0x036c,0x0124,0x05af,0x07ef,
					 0x0fcf,0x0868,0x0fff,0x0c9c,0x0434,0x0fff,0x0fff,0x08cf,
					 0x0468,0x0cff,0x069c,0x0234,0x0aff,0x0eff,0x0ccf,0x0668,
					 0x0fff,0x099c,0x0334,0x0fff,0x0fff,0x04cf,0x0268,0x06ff,
					 0x039c,0x0134,0x05ff,0x07ff,0x0f4f,0x0828,0x0f6f,0x0c3c,
					 0x0414,0x0f5f,0x0f7f,0x084f,0x0428,0x0c6f,0x063c,0x0214,
					 0x0a5f,0x0e7f,0x0c4f,0x0628,0x0f6f,0x093c,0x0314,0x0f5f,
					 0x0f7f,0x044f,0x0228,0x066f,0x033c,0x0114,0x055f,0x077f,
					 0x0ff8,0x0884,0x0ffc,0x0cc6,0x0442,0x0ffa,0x0ffe,0x08f8,
					 0x0484,0x0cfc,0x06c6,0x0242,0x0afa,0x0efe,0x0cf8,0x0684,
					 0x0ffc,0x09c6,0x0342,0x0ffa,0x0ffe,0x04f8,0x0284,0x06fc,
					 0x03c6,0x0142,0x05fa,0x07fe,0x0f88,0x0844,0x0fcc,0x0c66,
					 0x0422,0x0faa,0x0fee,0x0888,0x0444,0x0ccc,0x0666,0x0222,
					 0x0aaa,0x0eee,0x0c88,0x0644,0x0fcc,0x0966,0x0322,0x0faa,
					 0x0fee,0x0488,0x0244,0x06cc,0x0366,0x0122,0x05aa,0x07ee,
					 0x0fc8,0x0864,0x0ffc,0x0c96,0x0432,0x0ffa,0x0ffe,0x08c8,
					 0x0464,0x0cfc,0x0696,0x0232,0x0afa,0x0efe,0x0cc8,0x0664,
					 0x0ffc,0x0996,0x0332,0x0ffa,0x0ffe,0x04c8,0x0264,0x06fc,
					 0x0396,0x0132,0x05fa,0x07fe,0x0f48,0x0824,0x0f6c,0x0c36,
					 0x0412,0x0f5a,0x0f7e,0x0848,0x0424,0x0c6c,0x0636,0x0212,
					 0x0a5a,0x0e7e,0x0c48,0x0624,0x0f6c,0x0936,0x0312,0x0f5a,
					 0x0f7e,0x0448,0x0224,0x066c,0x0336,0x0112,0x055a,0x077e};
#endif


    /* go on if we got the memory for the ColorMap */
    if (NULL != NewCM)
    {
	/* get memory for the ColorTable */
	NewCM -> ColorTable = AllocMem(entries * sizeof(UWORD), MEMF_CLEAR|MEMF_PUBLIC);

	/* get memory for LowColorBits */
	NewCM -> LowColorBits = AllocMem(entries * sizeof(UWORD), MEMF_CLEAR|MEMF_PUBLIC);

	ptr1 = NewCM -> ColorTable;
	ptr2 = NewCM -> LowColorBits;

	/* did we get all the memory we wanted? */
	if ( (NULL != ptr1) && (NULL != ptr2) )
	{
#if 1
	    ULONG i;
	    LONG  * L_RGBColorTable = (LONG *)&RGBColorTable[0];
#endif

	    /* further init the GetColorMap structure */
	    NewCM->Type             = COLORMAP_TYPE_V39;
	    NewCM->Count            = entries;
	    NewCM->SpriteResolution = SPRITERESN_DEFAULT;
	    NewCM->SpriteResDefault = SPRITERESN_ECS;
	    NewCM->AuxFlags         = CMAF_FULLPALETTE;
	    NewCM->VPModeID         = -1;

	    /* FIXME: Shouldn't these be different? */
	    NewCM->SpriteBase_Even  = 0x0010;
	    NewCM->SpriteBase_Odd   = 0x0010;

	    NewCM->Bp_1_base        = 0x0008;

#if 1
	    /* Fill the ColorTable and the LowColorBits with the appropriate Data */

	    /* as we`re clever we`re doing some 32 bit copying with the 16 bit data */
	    for (i = 0; i < (entries >> 1); i++)
	    {
        	LONG ColorValue = L_RGBColorTable[i];
        	*ptr1++ = ColorValue;
        	*ptr2++ = ColorValue;
	    }
	    /* is there one WORD left to copy? */
	    if (1 == (entries & 1) )
	    {
        	WORD ColorValue = RGBColorTable[entries-1];
        	*(WORD *)ptr1 = ColorValue;
        	*(WORD *)ptr2 = ColorValue;
	    }
#endif

	}
	else /* not enough memory for the tables */
	{
	    if (NULL != NewCM -> ColorTable)
        	FreeMem(NewCM -> ColorTable, entries * sizeof(UWORD));
	    if (NULL != NewCM -> LowColorBits)
        	FreeMem(NewCM -> LowColorBits, entries * sizeof(UWORD));

	    FreeMem(NewCM, sizeof(struct ColorMap));
	    /* make return value invalid */
	    NewCM = NULL;
	}
	
    } /* if (NULL != NewCM) */

    return NewCM;

    AROS_LIBFUNC_EXIT
  
} /* GetColorMap */
