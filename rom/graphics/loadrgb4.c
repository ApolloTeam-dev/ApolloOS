/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include <aros/debug.h>
#include "graphics_intern.h"

static void pokeCL(UWORD *ci, UWORD target, UWORD table)
{
    ULONG targ = (ULONG)target;
    if(!ci) return;
    targ &= 0x1fe;
    for(;ci;ci+=2)
    {
        if( ((*ci)==0xffff) && ((*(ci+1) == 0xfffe))) return;
        if(*ci == targ) break;
    }
    if (((*ci) == 0xffff) && ((*(ci+1)==0xfffe))) return;
    if(*ci == targ)
    {
       ci++;
       *ci++ = table;
    }  
}

static struct CopIns *pokeCI(struct CopIns *ci, UWORD *field1, short field2)
{
    struct CopIns *c;
    UWORD op=COPPER_MOVE;
    c = ci;
    if(c)
    {
        short out = FALSE;
        while(!out)
        {
            switch(c->OpCode & 3)
            {
                case COPPER_MOVE:
                {
                    if(c->DESTADDR == (((UWORD)field1) & 0x1fe))
                    {
                        short mask;
                        if((mask = op&0xC000))
                        {
                            if(c->OpCode & mask)
                            {
                                c->DESTDATA = field2;
                                return c;
                            }   
                        }
                        else
                        {
                            c->DESTDATA = field2;
                            return c;
                        }
                    }
                    c++;
                    break;
                }
                case COPPER_WAIT:
                {
                    if(c->HWAITPOS == 255)
                    {
                        return 0;
                    }
                    else c++;
                    break;
                }
                case CPRNXTBUF:
                {
                    if(c->NXTLIST == NULL)
                    {
                        out = TRUE;
                    }
                    else
                    {
                        if((c = c->NXTLIST->CopIns) == NULL)
                        {
                            out = TRUE;
                        }
                    }
                    break;
                }
                default:
                {  
                    out=TRUE;
                    break;
                }
            }
        }
    }
    return 0;
}

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
    int i;
    volatile struct Custom *custom = (struct Custom *)0xdff000;

    if (!vp)
    {
	// No ViewPort? Then alter the colors directly
	for (i = 0; i < count ; i++)
       	    custom->color[i] = *colors++;    
        return;
    }
    ASSERT_VALID_PTR(vp);
    ASSERT_VALID_PTR(colors);

    /* TODO: Optimization */
    struct ColorMap *cm;
    UWORD *ct;

    // Why do we need this? Answer: For non hidd screens SetRGB32 might fail to fill the color table. This is a good place, I think
    if(vp->ColorMap)
    {
        cm = vp->ColorMap;
        ct = cm->ColorTable;
        if(count > cm->Count) count = cm->Count;
        for(i = 0; i<count; i++)
        {
            *ct++ = *colors++;
        }
        colors -= count; // back to first color
    }

    for (t = 0; t < count; t ++ )
    {
    	ULONG r = (colors[t] & 0xF00) >> 8;
    	ULONG g = (colors[t] & 0x0F0) >> 4;
    	ULONG b = (colors[t] & 0x00F);

        r = (( r & 0xF) << 4) | (r & 0xFF );
        r = r | (r << 8) | ( r << 16 ) | (r << 24 );
        g = (( g & 0xF) << 4) | (g & 0xFF );
        g = g | (g << 8) | ( g << 16 ) | (g << 24 );
        b = (( b & 0xF) << 4) | (b & 0xFF );
        b = b | (b << 8) | ( b << 16 ) | (b << 24 );

        SetRGB32( vp, t, r, g, b );
    }
	
    ObtainSemaphore(GfxBase->ActiViewCprSemaphore);
    for (t = 0; t < count; t ++ )
    {
        // Remaining question: Do we need to check VP_HIDE?
        if(vp->DspIns)
        {		
   	    // we need to store it into the intermediate CopperList too
	    pokeCL(vp->DspIns->CopLStart, &custom->color[t], colors[t]);
	    pokeCL(vp->DspIns->CopSStart, &custom->color[t], colors[t]);
	    pokeCI(vp->DspIns->CopIns, &custom->color[t], colors[t]);
	}
    }    
    ReleaseSemaphore(GfxBase->ActiViewCprSemaphore);

    // To make it visible immediately we need to update the view. To simplify it, we call MrgCop
    MrgCop(GfxBase->ActiView);
    LoadView(GfxBase->ActiView);

    AROS_LIBFUNC_EXIT
    
} /* LoadRGB4 */
