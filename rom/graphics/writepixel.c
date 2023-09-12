/*
    Copyright © 1995-2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Graphics function WritePixel()
    Lang: english
*/

#include <graphics/rastport.h>
#include <graphics/clip.h>
#include <proto/graphics.h>

#include "graphics_intern.h"
#include "gfxfuncsupport.h"

static LONG pix_write(APTR pr_data, OOP_Object *bm, OOP_Object *gc,
    	    	      WORD x, WORD y, struct GfxBase *GfxBase);

/*****************************************************************************

    NAME */

	AROS_LH3(LONG, WritePixel,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A1),
	AROS_LHA(WORD             , x, D0),
	AROS_LHA(WORD             , y, D1),

/*  LOCATION */
	struct GfxBase *, GfxBase, 54, Graphics)

/*  FUNCTION
	Write the primary (A) pen colour to the given coordinates of a
	RastPort.

    INPUTS
	rp  - destination RastPort
	x,y - coordinate

    RESULT
	 0: pixel could be written
	-1: coordinate was outside rastport

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS
        This function takes layers into account. Some pixel that is
        being read is not found on the display-bitmap but in some
        clipped rectangle (cliprect) in a layer structure.

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    #define MODUS_XOR  0x1
    #define MODUS_SET  0x3
    #define MODUS_CLR  0x0
    #define MODUS_NON  0x2

    IPTR pix;

    FIX_GFXCOORD(x);
    FIX_GFXCOORD(y);

    // Find something else later. It seems this works but is ugly...
    if(rp->BitMap->Planes[0] == 0x00000000)
    {
        if ((rp->Flags & RPF_NO_PENS) != 0)
        {
            HIDDT_GC_Intern *_gc = GCINT(&((rp)->longreserved[1]));
            pix = _gc->fg;
        }
        else
            pix = BM_PIXEL(rp->BitMap, (UBYTE)rp->FgPen);

        return do_pixel_func(rp, x, y, pix_write, (APTR)pix, TRUE, GfxBase);
    }
    else
    {
	    // New method, without hidd
            UBYTE *minterms = rp->minterms;
            UBYTE planeIndex;
	    UBYTE WriteMode = MODUS_NON;
	    UBYTE *planePtr;
	    UBYTE thisbyte;
	    UBYTE pixel;
	    UBYTE planeMask = rp->Mask;
	    struct Layer* layer;
	    struct BitMap *bm = rp->BitMap;
	    struct ClipRect *clipRect;
	    struct BitMap *superBitMap;
	    ULONG offset;
	    
	    layer = rp->Layer;
	    
	    if (layer) {
		ObtainSemaphore((struct SignalSemaphore *)&layer->Lock);

		x = (WORD)x + layer->bounds.MinX - layer->Scroll_X;
		y = (WORD)y + layer->bounds.MinY - layer->Scroll_Y;

		clipRect = layer->ClipRect;
		while (clipRect)
		{
		    if (x >= clipRect->bounds.MinX && x <= clipRect->bounds.MaxX) {
		        if (y >= clipRect->bounds.MinY && y <= clipRect->bounds.MaxY) {
			    if (!(clipRect->lobs)) {
			        break; // we can draw directly to the screen
			    }
			    x -= (clipRect->bounds.MinX & 0xfff0);
			    y -= clipRect->bounds.MinY;

			    if (clipRect->BitMap) {
			        bm = (struct BitMap *)clipRect->BitMap;
			        break;
			    }
			    ReleaseSemaphore((struct SignalSemaphore *)&layer->Lock);
			    return -1; // no Bitmap
			}
		    }
		    clipRect = clipRect->Next;
		}
		if (!clipRect) {
		    // Try super bitmap
		    superBitMap = layer->SuperBitMap;
		    if (superBitMap) {
		        x = x + layer->Scroll_X - layer->bounds.MinX;
			y = y + layer->Scroll_Y - layer->bounds.MinY;
			clipRect = layer->SuperClipRect;
			while (clipRect) {
		            if (x >= clipRect->bounds.MinX && x <= clipRect->bounds.MaxX) {
			        if (y >= clipRect->bounds.MinY && y <= clipRect->bounds.MaxY) {
			            bm = superBitMap;
			            break;
			        }
			    }
			    clipRect = clipRect->Next;
		        }
		    }
		}
                ReleaseSemaphore((struct SignalSemaphore *)&layer->Lock);
		    
		if (!clipRect) {
		    return -1; // No Clip Rects found
		}
	 }

	UBYTE **plane = bm->Planes;
        offset    = (x >>3) + y * bm->BytesPerRow;
        pixel     = 128 >> (x & 0x7);
       
	for (planeIndex = 0 ;planeIndex < bm->Depth; planeIndex++)
	{
	    if (planeMask & (1 << planeIndex)) {
	        WriteMode = minterms[planeIndex] >> 6; 
		planePtr = (UBYTE *)(*(plane + planeIndex) + offset);
		thisbyte = *planePtr;
		if (WriteMode == MODUS_SET) {
		    thisbyte |= pixel;
		} else if (WriteMode == MODUS_CLR) {
		    thisbyte &= ~pixel;
		} else if (WriteMode == MODUS_XOR) {
		    thisbyte ^= pixel;
		}
		*planePtr = thisbyte;
	    }
	}
        return 0L; 
    }
	
    AROS_LIBFUNC_EXIT
} /* WritePixel */


static LONG pix_write(APTR pr_data, OOP_Object *bm, OOP_Object *gc,
    	    	      WORD x, WORD y, struct GfxBase *GfxBase)
{
    HIDD_BM_PutPixel(bm, x, y, (IPTR)pr_data);

    return 0;
}
