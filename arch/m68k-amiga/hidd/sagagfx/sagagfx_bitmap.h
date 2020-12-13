#ifndef SAGAGFX_BITMAP_H
#define SAGAGFX_BITMAP_H

/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Bitmap class for SAGAGfx Hidd.
    Lang: English.
*/

#include "sagagfx_hidd.h"
#include "sagagfx_hw.h"

#include <hidd/gfx.h>

#define IS_BM_ATTR(attr, idx) (((idx) = (attr) - HiddBitMapAttrBase) < num_Hidd_BitMap_Attrs)

struct SAGARegs {
    ULONG       pixclk;
    UBYTE       pixfmt;
    UBYTE       modeid;
    UWORD       hpixel;
    UWORD       hsstart;
    UWORD       hsstop;
    UWORD       htotal;
    UWORD       vpixel;
    UWORD       vsstart;
    UWORD       vsstop;
    UWORD       vtotal;
    UWORD       hvsync;
    UWORD       modulo;
    IPTR        memptr;
};

/*
   This structure is used as instance data for the bitmap class.
*/

struct SAGAGfxBitmapData
{
    UBYTE           *VideoBuffer;    /* Start of framebuffer, as allocated */
    UBYTE           *VideoData;      /* Start of aligned framebuffer */
    LONG            width;           /* Bitmap Width */
    LONG            height;          /* Bitmap Height */
    UBYTE           bytesperpixel;	/* Bytes Per Pixel */
    UBYTE           bitsperpix;      /* Bits Per Pixel */
    ULONG           bytesperline;    /* Bytes Per Line */
    ULONG           *CLUT;           /* Hardware palette registers */
    struct SAGARegs hwregs;          /* Hardware registers */
    BYTE            bpp;             /* Cached bits per pixel */
    BYTE            disp;            /* !=0 - displayable */
    OOP_Object      *pixfmtobj;      /* Cached pixelformat object */
    OOP_Object      *gfxhidd;        /* Cached driver object */
    LONG            xoffset;         /* Bitmap X offset */
    LONG            yoffset;         /* Bitmap Y offset */
    struct SignalSemaphore	bmLock;		/* For locking the surface*/
    WORD 			locked;
};

#define LOCK_BITMAP(data)       {ObtainSemaphore(&(data)->bmLock);}
#define TRYLOCK_BITMAP(data)    (AttemptSemaphore(&(data)->bmLock))
#define UNLOCK_BITMAP(data)     {ReleaseSemaphore(&(data)->bmLock);}

#endif /* SAGAGFX_BITMAP_H */
