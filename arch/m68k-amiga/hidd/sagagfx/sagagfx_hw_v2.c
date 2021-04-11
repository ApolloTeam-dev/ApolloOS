/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class (For V2 boards).
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
#include "sagagfx_hw_v2.h"

/*
 **************************************************************
 **  PRIVATE
 **************************************************************
*/

static const struct saga_v2_pll_data {
    ULONG freq[2];
    UBYTE data[18];
} saga_v2_pll[] = {
    { .freq = { 28217480, 28477269 },
      .data = { 0x08,0x40,0x60,0x00,0x02,0xD5,0x64,0x2F,0x16,0x02,0xC1,0x20,0x00,0x08,0x00,0x02,0x00,0x00 } },
    { .freq = { 28375120, 28636360 },
      .data = { 0x08,0x00,0x60,0x00,0x01,0x40,0xA0,0x14,0x0A,0x01,0x00,0xA0,0x00,0x08,0x00,0x02,0x00,0x00 } },
    { .freq = { 28532759, 28795450 },
      .data = { 0x08,0x40,0x60,0x00,0x02,0xDD,0x68,0x2F,0x16,0x02,0xC1,0x20,0x00,0x08,0x00,0x02,0x00,0x00 } },
    { .freq = { 56395551, 56914765 },
      .data = { 0x08,0x40,0x60,0x00,0x02,0x85,0x3C,0x14,0x0A,0x01,0x00,0xA0,0x00,0x08,0x00,0x02,0x00,0x00 } },
    { .freq = { 56750240, 57272720 },
      .data = { 0x08,0x00,0x60,0x00,0x01,0x40,0xA0,0x0A,0x05,0x00,0x80,0x60,0x00,0x08,0x00,0x02,0x00,0x00 } },
    { .freq = { 57104929, 57630674 },
      .data = { 0x08,0x40,0x60,0x00,0x02,0x8D,0x40,0x14,0x0A,0x01,0x00,0xA0,0x00,0x08,0x00,0x02,0x00,0x00 } },
};

#define SAGA_V2_VIDEO_PLL_CLOCKS (sizeof(saga_v2_pll)/sizeof(saga_v2_pll[0]))

int saga_v2_pll_clock_count(void)
{
    return(SAGA_V2_VIDEO_PLL_CLOCKS);
}

int saga_v2_pll_clock_freq(int id, BOOL is_ntsc, ULONG *freq)
{
    int type = is_ntsc ? 1 : 0;
    
    if (id < 0 || id >= SAGA_V2_VIDEO_PLL_CLOCKS)
    {
        return(-1);
    }
    
    *freq = saga_v2_pll[id].freq[type];
    
    return(0);
}

int saga_v2_pll_clock_lookup(BOOL is_ntsc, ULONG *freqp)
{
    int type = is_ntsc ? 1 : 0;
    int i;
    ULONG freq;
    
    if (!freqp)
    {
        return(-1);
    }
    
    freq = *freqp;
    
    /* Find the closest clock */
    for (i = 0; i < SAGA_V2_VIDEO_PLL_CLOCKS - 1; i++)
    {
        ULONG split;
        
        if (freq <= saga_v2_pll[i].freq[type])
        {
            break;
        }
        
        split = (saga_v2_pll[i].freq[type] + saga_v2_pll[i + 1].freq[type]) / 2;
        
        if (freq < split)
        {
            break;
        }
    }
    
    *freqp = saga_v2_pll[i].freq[type];
    
    /* No match, return the largest valid clock */
    return(i);
}

int saga_v2_pll_clock_program(int clock)
{
    int i;
    
    if (clock < 0 || clock >= SAGA_V2_VIDEO_PLL_CLOCKS)
    {
        return(-1);
    }
    
    for (i = 0; i < 18; i++)
    {
        UBYTE byte = saga_v2_pll[clock].data[17 - i];
        int j;
        
        WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                    SAGA_V2_VIDEO_PLLW_CS(0) |
                                    SAGA_V2_VIDEO_PLLW_CLK(0));
        
        for (j = 0; j < 8; j++, byte >>= 1)
        {
            WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                        SAGA_V2_VIDEO_PLLW_MOSI(byte & 1) |
                                        SAGA_V2_VIDEO_PLLW_CS(0) |
                                        SAGA_V2_VIDEO_PLLW_CLK(0));
            WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                        SAGA_V2_VIDEO_PLLW_MOSI(byte & 1) |
                                        SAGA_V2_VIDEO_PLLW_CS(0) |
                                        SAGA_V2_VIDEO_PLLW_CLK(1));
        }
    }
    
    WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                SAGA_V2_VIDEO_PLLW_CS(1) |
                                SAGA_V2_VIDEO_PLLW_CLK(0));
    WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                SAGA_V2_VIDEO_PLLW_CS(1) |
                                SAGA_V2_VIDEO_PLLW_CLK(1));
    WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                SAGA_V2_VIDEO_PLLW_CS(1) |
                                SAGA_V2_VIDEO_PLLW_CLK(0) |
                                SAGA_V2_VIDEO_PLLW_UPDATE(1));
    WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                SAGA_V2_VIDEO_PLLW_CS(1) |
                                SAGA_V2_VIDEO_PLLW_CLK(1) |
                                SAGA_V2_VIDEO_PLLW_UPDATE(1));
    
    /* Send 128 clock cycles to allow the PLL to update */
    
    for (i = 0; i < 128; i++)
    {
        WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                    SAGA_V2_VIDEO_PLLW_CS(1) |
                                    SAGA_V2_VIDEO_PLLW_CLK(0));
        WRITE32(SAGA_V2_VIDEO_PLLW, SAGA_V2_VIDEO_PLLW_MAGIC |
                                    SAGA_V2_VIDEO_PLLW_CS(1) |
                                    SAGA_V2_VIDEO_PLLW_CLK(1));
    }
    
    return(0);
}

/*
 **************************************************************
 **  PUBLIC
 **************************************************************
*/

UBYTE SAGAHW_V2_GetModeID(UWORD w, UWORD h)
{
    
    return(0);
}

UBYTE SAGAHW_V2_GetPixFmt(UBYTE bpp)
{
    
    if      (bpp == 15) return(SAGA_V2_VIDEO_PIXFMT_RGB15);
    else if (bpp == 16) return(SAGA_V2_VIDEO_PIXFMT_RGB16);
    else if (bpp == 24) return(SAGA_V2_VIDEO_PIXFMT_RGB24);
    else if (bpp == 32) return(SAGA_V2_VIDEO_PIXFMT_RGB32);
    
    return(SAGA_V2_VIDEO_PIXFMT_CLUT8);
}

VOID SAGAHW_V2_SetColors(ULONG *colors, UWORD startIndex, UWORD count)
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
            WRITE32(SAGA_V2_VIDEO_CLUT + 
                ( ( ( startIndex + i ) & 0xFF ) << 2 ),  // Color Index
                colors[ startIndex + i ]                // Color Value
            );
        }
    }
}

VOID SAGAHW_V2_SetMemory(ULONG memory)
{
    
    WRITE32(SAGA_V2_VIDEO_DATA, memory);
}

VOID SAGAHW_V2_SetMode(UBYTE modeid, UBYTE pixfmt)
{
    
    WRITE16(SAGA_V2_VIDEO_MODE, pixfmt);
}

VOID SAGAHW_V2_SetModulo(WORD modulo)
{
    
    WRITE16(SAGA_V2_VIDEO_HMOD, modulo);
}

VOID SAGAHW_V2_SetModeline(UWORD a, UWORD b, UWORD c, UWORD d, UWORD e, UWORD f, UWORD g, UWORD h, UWORD i)
{
    
    if (SAGA_V2_VIDEO_IS_DBLX(a))
        a <<= 1; // HPIXEL
    
    if (SAGA_V2_VIDEO_IS_DBLY(e))
        e <<= 1; // VPIXEL
    
    WRITE16(SAGA_V2_VIDEO_HPIXEL, a);
    WRITE16(SAGA_V2_VIDEO_HSSTRT, b);
    WRITE16(SAGA_V2_VIDEO_HSSTOP, c);
    WRITE16(SAGA_V2_VIDEO_HTOTAL, d);
    WRITE16(SAGA_V2_VIDEO_VPIXEL, e);
    WRITE16(SAGA_V2_VIDEO_VSSTRT, f);
    WRITE16(SAGA_V2_VIDEO_VSSTOP, g);
    WRITE16(SAGA_V2_VIDEO_VTOTAL, h);
    WRITE16(SAGA_V2_VIDEO_HVSYNC, i);
}

VOID SAGAHW_V2_SetPLL(ULONG clock)
{
    ULONG clk = clock;
    int idx = 0;
    
    
    idx = saga_v2_pll_clock_lookup(FALSE, &clk);
    
    saga_v2_pll_clock_program(idx);
}

VOID SAGAHW_V2_SetSpriteHide(VOID)
{
    
    WRITE16(SAGA_V2_VIDEO_SPRITE_POSX, SAGA_V2_VIDEO_MAXH - 1);
    WRITE16(SAGA_V2_VIDEO_SPRITE_POSY, SAGA_V2_VIDEO_MAXV - 1);
}

VOID SAGAHW_V2_SetSpriteColors(UWORD *colors)
{
    for (int i = 1; i < 4; i++)
    {
        WRITE16(SAGA_V2_VIDEO_SPRITE_CLUT + (i << 1), colors[i]);
    }
}

VOID SAGAHW_V2_SetSpriteMemory(UBYTE *memory)
{
    IPTR ptr = SAGA_V2_VIDEO_SPRITE_DATA;
    
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

VOID SAGAHW_V2_SetSpritePosition(WORD x, WORD y)
{
    WRITE16(SAGA_V2_VIDEO_SPRITE_POSX, SAGA_V2_VIDEO_SPRITE_DELTAX + x);
    WRITE16(SAGA_V2_VIDEO_SPRITE_POSY, SAGA_V2_VIDEO_SPRITE_DELTAY + y);
}

/* END OF FILE */
