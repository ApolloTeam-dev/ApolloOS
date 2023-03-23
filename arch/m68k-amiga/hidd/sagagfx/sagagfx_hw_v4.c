/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class (For V4 boards).
    Lang: English.
*/

#undef DEBUG
#define DEBUG 0

#include <aros/debug.h>

#include <exec/exec.h>
#include <devices/inputevent.h>
#include <proto/exec.h>
#include <proto/input.h>

#include "sagagfx_hw.h"
#include "sagagfx_hw_v4.h"

/*
 **************************************************************
 **  PRIVATE
 **************************************************************
*/


/*
 **************************************************************
 **  PUBLIC
 **************************************************************
*/

UBYTE SAGAHW_V4_GetModeID(UWORD w, UWORD h)
{
    
    if		(w == 304 && h == 224) return(SAGA_V4_VIDEO_MODEID_304x224);
    else if (w == 320 && h == 200) return(SAGA_V4_VIDEO_MODEID_320x200);
    else if (w == 320 && h == 240) return(SAGA_V4_VIDEO_MODEID_320x240);
    else if (w == 320 && h == 256) return(SAGA_V4_VIDEO_MODEID_320x256);
    else if (w == 480 && h == 270) return(SAGA_V4_VIDEO_MODEID_480x270);
    else if (w == 640 && h == 360) return(SAGA_V4_VIDEO_MODEID_640x360);
    else if (w == 640 && h == 200) return(SAGA_V4_VIDEO_MODEID_640x200);
    else if (w == 640 && h == 400) return(SAGA_V4_VIDEO_MODEID_640x400);
    else if (w == 640 && h == 480) return(SAGA_V4_VIDEO_MODEID_640x480);
    else if (w == 640 && h == 512) return(SAGA_V4_VIDEO_MODEID_640x512);
    else if (w == 480 && h == 270) return(SAGA_V4_VIDEO_MODEID_480x270);
    else if (w == 640 && h == 360) return(SAGA_V4_VIDEO_MODEID_640x360);
    else if (w == 960 && h == 540) return(SAGA_V4_VIDEO_MODEID_960x540);
    else if (w == 800 && h == 600) return(SAGA_V4_VIDEO_MODEID_800x600);
    else if (w == 1280 && h == 720) return(SAGA_V4_VIDEO_MODEID_1280x720);
    else if (w == 1280 && h == 1024) return(SAGA_V4_VIDEO_MODEID_1280x1024);
    else if (w == 1920 && h == 1080) return(SAGA_V4_VIDEO_MODEID_1920x1080);
    else if (w == 848 && h == 480) return(SAGA_V4_VIDEO_MODEID_848x480);
    else if (w == 1024 && h == 768) return(SAGA_V4_VIDEO_MODEID_1024x768);
    else if (w == 720 && h == 576) return(SAGA_V4_VIDEO_MODEID_720x576);
    else if (w == 848 && h == 480) return(SAGA_V4_VIDEO_MODEID_848x480);

    return(0);
}

UBYTE SAGAHW_V4_GetPixFmt(UBYTE bpp)
{
    
    if      (bpp == 15) return(SAGA_V4_VIDEO_PIXFMT_RGB15);
    else if (bpp == 16) return(SAGA_V4_VIDEO_PIXFMT_RGB16);
    else if (bpp == 24) return(SAGA_V4_VIDEO_PIXFMT_RGB24);
    else if (bpp == 32) return(SAGA_V4_VIDEO_PIXFMT_RGB32);
    
    return(SAGA_V4_VIDEO_PIXFMT_CLUT8);
}

VOID SAGAHW_V4_SetColors(ULONG *colors, UWORD startIndex, UWORD count)
{
    ULONG i;
    
    
    if (colors)
    {
        if(startIndex > 255)
        {
            return;
        }
        
        if(startIndex + count > 256)
        {
            count = 256 - startIndex;
        }
        
        for (i = 0; i < count; i++)
        {
            WRITE32(SAGA_V4_VIDEO_CLUT, 
                ( ( startIndex + i ) << 24 ) |  // Color Index
                colors[ startIndex + i ]       // Color Value
            );
        }
    }
}

VOID SAGAHW_V4_SetMemory(ULONG memory)
{
    WRITE32(SAGA_V4_VIDEO_DATA, memory);
}

VOID SAGAHW_V4_SetMode(UBYTE modeid, UBYTE pixfmt)
{
    WRITE16(SAGA_V4_VIDEO_MODE, (modeid << 8) | pixfmt);
}

VOID SAGAHW_V4_SetModulo(WORD modulo)
{
    WRITE16(SAGA_V4_VIDEO_HMOD, modulo);
}

VOID SAGAHW_V4_SetModeline(UWORD a, UWORD b, UWORD c, UWORD d, UWORD e, UWORD f, UWORD g, UWORD h, UWORD i)
{
    return;
}

VOID SAGAHW_V4_SetPLL(ULONG clock)
{
    return;
}

VOID SAGAHW_V4_SetSpriteHide(VOID)
{
    WRITE16(SAGA_V4_VIDEO_SPRITE_POSX, SAGA_V4_VIDEO_MAXH - 1);
    WRITE16(SAGA_V4_VIDEO_SPRITE_POSY, SAGA_V4_VIDEO_MAXV - 1);
}

VOID SAGAHW_V4_SetSpriteColors(UWORD *colors)
{
    for (int i = 1; i < 4; i++)
    {
        WRITE16(SAGA_V4_VIDEO_SPRITE_CLUT + (i << 1), colors[i]);
    }
}

VOID SAGAHW_V4_SetSpriteMemory(UBYTE *memory)
{
    IPTR ptr = SAGA_V4_VIDEO_SPRITE_DATA;
    
    ULONG x, y, pix, val;
    
    for (y = 0; y < 16; y++)
    {
        pix = 0x80008000;
        val = 0;
        
        for (x = 0; x < 16; x++)
        {
            switch (memory[y * 16 + x])
            {
                case 1:
                    val |= pix & 0xffff0000;
                    break;
                case 2:
                    val |= pix & 0x0000ffff;
                    break;
                case 3:
                    val |= pix;
                    break;
                default:
                    break;
            }
            pix >>= 1;
        }
        
        WRITE32(ptr, val);
        ptr += 4;
    }
}

VOID SAGAHW_V4_SetSpritePosition(WORD x, WORD y)
{
    WRITE16(SAGA_V4_VIDEO_ON, 0x0001);
    WRITE16(SAGA_V4_VIDEO_SPRITE_POSX, SAGA_V4_VIDEO_SPRITE_DELTAX + x);
    WRITE16(SAGA_V4_VIDEO_SPRITE_POSY, SAGA_V4_VIDEO_SPRITE_DELTAY + y);
}


/* END OF FILE */
