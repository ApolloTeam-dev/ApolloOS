#ifndef SAGAGFX_HW_V2_H
#define SAGAGFX_HW_V2_H

/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class (For V2 boards).
    Lang: English.
*/

#undef DEBUG
#define DEBUG 1

#include <aros/debug.h>
#include <exec/types.h>

/*
 **************************************************************
 **  DEFINES
 **************************************************************
*/

// HARDWARE SPRITE (MOUSE)

#define SAGA_V2_VIDEO_SPRITE_DELTAX      16        // Mouse Sprite Start X
#define SAGA_V2_VIDEO_SPRITE_DELTAY      8         // Mouse Sprite Start Y

#define SAGA_V2_VIDEO_SPRITE_POSX        0xDFF1D0  // [16-bits] Mouse Sprite X
#define SAGA_V2_VIDEO_SPRITE_POSY        0xDFF1D2  // [16-bits] Mouse Sprite Y
#define SAGA_V2_VIDEO_SPRITE_CLUT        0xDFF3A0  // [16-bits] Mouse Sprite CLUT[4]
#define SAGA_V2_VIDEO_SPRITE_DATA        0xDFF800  // [16-bits] Mouse Sprite DATA[16*16]

// PIXEL FORMATS

#define SAGA_V2_VIDEO_PIXFMT_AMIGA       0x00      // Planar
#define SAGA_V2_VIDEO_PIXFMT_CLUT8       0x01      // Chunky
#define SAGA_V2_VIDEO_PIXFMT_RGB16       0x02      // R5G6B5   [---- ---- ---- ---- rrrr rggg gggb bbbb]
#define SAGA_V2_VIDEO_PIXFMT_RGB15       0x03      // R5G5B5   [---- ---- ---- ---- 0rrr rrgg gggb bbbb]
#define SAGA_V2_VIDEO_PIXFMT_RGB24       0x04      // R8G8B8   [---- ---- rrrr rrrr gggg gggg bbbb bbbb]
#define SAGA_V2_VIDEO_PIXFMT_RGB32       0x05      // A8R8G8B8 [aaaa aaaa rrrr rrrr gggg gggg bbbb bbbb]
#define SAGA_V2_VIDEO_PIXFMT_YUV422      0x06      // Y4U2V2   [---- ---- ---- ---- yyyy uuuu yyyy vvvv]

// VIDEO MODELINE

#define SAGA_V2_VIDEO_HPIXEL             0xDFF300  // [16-bits] HPIXEL
#define SAGA_V2_VIDEO_HSSTRT             0xDFF302  // [16-bits] HSSTRT
#define SAGA_V2_VIDEO_HSSTOP             0xDFF304  // [16-bits] HSSTOP
#define SAGA_V2_VIDEO_HTOTAL             0xDFF306  // [16-bits] HTOTAL
#define SAGA_V2_VIDEO_VPIXEL             0xDFF308  // [16-bits] VPIXEL
#define SAGA_V2_VIDEO_VSSTRT             0xDFF30A  // [16-bits] VSSTRT
#define SAGA_V2_VIDEO_VSSTOP             0xDFF30C  // [16-bits] VSSTOP
#define SAGA_V2_VIDEO_VTOTAL             0xDFF30E  // [16-bits] VTOTAL
#define SAGA_V2_VIDEO_HVSYNC             0xDFF310  // [16-bits] HVSYNC

// VIDEO PLL

#define SAGA_V2_VIDEO_PLLW               0xDFF1F8  // [32-bits] PLL Write
#define SAGA_V2_VIDEO_PLLR               0xDFF1FA  // [32-bits] PLL Read

#define SAGA_V2_VIDEO_PLLW_MAGIC         0x43430000
#define SAGA_V2_VIDEO_PLLW_CS(x)         (((x) & 1) << 0)
#define SAGA_V2_VIDEO_PLLW_CLK(x)        (((x) & 1) << 1)
#define SAGA_V2_VIDEO_PLLW_MOSI(x)       (((x) & 1) << 2)
#define SAGA_V2_VIDEO_PLLW_UPDATE(x)     (((x) & 1) << 3)

// VIDEO DAC

#define SAGA_V2_VIDEO_MAXH               16384     // Maximum Horizontal Size
#define SAGA_V2_VIDEO_MAXV               16384     // Maximum Vertical Size

#define SAGA_V2_VIDEO_HMOD               0xDFF1E6  // [16-bits] Horizontal Modulo
#define SAGA_V2_VIDEO_DATA               0xDFF1EC  // [32-bits] Memory Data
#define SAGA_V2_VIDEO_MODE               0xDFF1F4  // [16-bits] DAC Mode
#define SAGA_V2_VIDEO_CLUT               0xDFF400  // [16-bits] Palette

#define SAGA_V2_VIDEO_DBLSCAN_OFF        0
#define SAGA_V2_VIDEO_DBLSCAN_X          1
#define SAGA_V2_VIDEO_DBLSCAN_Y          2
#define SAGA_V2_VIDEO_DBLSCAN_XY         (SAGA_V2_VIDEO_DBLSCAN_X | SAGA_V2_VIDEO_DBLSCAN_Y)

#define SAGA_V2_VIDEO_PIXFMT_BE          0
#define SAGA_V2_VIDEO_PIXFMT_LE          1
#define SAGA_V2_VIDEO_PIXFMT_ENDIAN      7
#define SAGA_V2_VIDEO_BYTESWAP           (SAGA_V2_VIDEO_PIXFMT_LE << SAGA_V2_VIDEO_PIXFMT_ENDIAN)

#define SAGA_V2_VIDEO_MODE_PIXFMT(x)     (((x) & 0xFF) << 0)
#define SAGA_V2_VIDEO_MODE_DBLSCN(x)     (((x) & 0xFF) << 8)

#define SAGA_V2_VIDEO_IS_DBLX(w)         ((w) <= 500)
#define SAGA_V2_VIDEO_IS_DBLY(h)         ((h) <= 300)

/*
 **************************************************************
 **  METHODS
 **************************************************************
*/

UBYTE SAGAHW_V2_GetPixFmt                (UBYTE bpp);
UBYTE SAGAHW_V2_GetModeID                (UWORD w, UWORD h);
VOID  SAGAHW_V2_SetColors                (ULONG *colors, UWORD startIndex, UWORD count);
VOID  SAGAHW_V2_SetModulo                (WORD modulo);
VOID  SAGAHW_V2_SetMemory                (ULONG memory);
VOID  SAGAHW_V2_SetMode                  (UBYTE modeid, UBYTE pixfmt);
VOID  SAGAHW_V2_SetModeline              (UWORD a, UWORD b, UWORD c, UWORD d, UWORD e, UWORD f, UWORD g, UWORD h, UWORD i);
VOID  SAGAHW_V2_SetPLL                   (ULONG clock);
VOID  SAGAHW_V2_SetSpriteHide            (VOID);
VOID  SAGAHW_V2_SetSpriteColors          (UWORD *colors);
VOID  SAGAHW_V2_SetSpriteMemory          (UBYTE *memory);
VOID  SAGAHW_V2_SetSpritePosition         (WORD x, WORD y);

/*
 **************************************************************
 **  END OF FILE
 **************************************************************
*/

#endif /* SAGAGFX_HW_V2_H */
