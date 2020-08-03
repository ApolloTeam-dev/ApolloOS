#ifndef SAGAGFX_HW_V4_H
#define SAGAGFX_HW_V4_H

/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class (For V4 boards).
    Lang: English.
*/

#undef DEBUG
#define DEBUG 0

#include <aros/debug.h>
#include <exec/types.h>

/*
 **************************************************************
 **  DEFINES
 **************************************************************
*/

// HARDWARE SPRITE (MOUSE)

#define SAGA_V4_VIDEO_SPRITE_DELTAX      16        // Mouse Sprite Start X
#define SAGA_V4_VIDEO_SPRITE_DELTAY      8         // Mouse Sprite Start Y

#define SAGA_V4_VIDEO_SPRITE_POSX        0xDFF1D0  // [16-bits] Mouse Sprite X
#define SAGA_V4_VIDEO_SPRITE_POSY        0xDFF1D2  // [16-bits] Mouse Sprite Y
#define SAGA_V4_VIDEO_SPRITE_CLUT        0xDFF3A0  // [16-bits] Mouse Sprite CLUT[4]
#define SAGA_V4_VIDEO_SPRITE_DATA        0xDFF800  // [16-bits] Mouse Sprite DATA[16*16]

// PIXEL FORMATS

#define SAGA_V4_VIDEO_PIXFMT_AMIGA       0x00      // Planar
#define SAGA_V4_VIDEO_PIXFMT_CLUT8       0x01      // Chunky
#define SAGA_V4_VIDEO_PIXFMT_RGB16       0x02      // R5G6B5   [---- ---- ---- ---- rrrr rggg gggb bbbb]
#define SAGA_V4_VIDEO_PIXFMT_RGB15       0x03      // R5G5B5   [---- ---- ---- ---- 0rrr rrgg gggb bbbb]
#define SAGA_V4_VIDEO_PIXFMT_RGB24       0x04      // R8G8B8   [---- ---- rrrr rrrr gggg gggg bbbb bbbb]
#define SAGA_V4_VIDEO_PIXFMT_RGB32       0x05      // A8R8G8B8 [aaaa aaaa rrrr rrrr gggg gggg bbbb bbbb]
#define SAGA_V4_VIDEO_PIXFMT_YUV422      0x06      // Y4U2V2   [---- ---- ---- ---- yyyy uuuu yyyy vvvv]

// RESOLUTION MODE IDS

#define SAGA_V4_VIDEO_MODEID_320x200     0x01      // [x2, x2]
#define SAGA_V4_VIDEO_MODEID_320x240     0x02      // [x2, x2]
#define SAGA_V4_VIDEO_MODEID_320x256     0x03      // [x2, x2]
#define SAGA_V4_VIDEO_MODEID_640x400     0x04      // [x1, x1]
#define SAGA_V4_VIDEO_MODEID_640x480     0x05      // [x1, x1]
#define SAGA_V4_VIDEO_MODEID_640x512     0x06      // [x1, x1]
#define SAGA_V4_VIDEO_MODEID_960x540     0x07      // [x1, x1]

// VIDEO DAC

#define SAGA_V4_VIDEO_MAXH               16384     // Maximum Horizontal Size
#define SAGA_V4_VIDEO_MAXV               16384     // Maximum Vertical Size

#define SAGA_V4_VIDEO_HMOD               0xDFF1E6  // [16-bits] Horizontal Modulo
#define SAGA_V4_VIDEO_DATA               0xDFF1EC  // [32-bits] Memory Data
#define SAGA_V4_VIDEO_MODE               0xDFF1F4  // [16-bits] DAC Mode
#define SAGA_V4_VIDEO_CLUT               0xDFF388  // [16-bits] Palette

/*
 **************************************************************
 **  METHODS
 **************************************************************
*/

UBYTE SAGAHW_V4_GetPixFmt                (UBYTE bpp);
UBYTE SAGAHW_V4_GetModeID                (UWORD w, UWORD h);
VOID  SAGAHW_V4_SetColors                (ULONG *colors, UWORD startIndex, UWORD count);
VOID  SAGAHW_V4_SetModulo                (WORD modulo);
VOID  SAGAHW_V4_SetMemory                (ULONG memory);
VOID  SAGAHW_V4_SetMode                  (UBYTE modeid, UBYTE pixfmt);
VOID  SAGAHW_V4_SetModeline              (UWORD a, UWORD b, UWORD c, UWORD d, UWORD e, UWORD f, UWORD g, UWORD h, UWORD i);
VOID  SAGAHW_V4_SetPLL                   (ULONG clock);
VOID  SAGAHW_V4_SetSpriteHide            (VOID);
VOID  SAGAHW_V4_SetSpriteColors          (UWORD *colors);
VOID  SAGAHW_V4_SetSpriteMemory          (UBYTE *memory);
VOID  SAGAHW_V4_SetSpritePosition        (WORD x, WORD y);

/*
 **************************************************************
 **  END OF FILE
 **************************************************************
*/

#endif /* SAGAGFX_HW_V4_H */
