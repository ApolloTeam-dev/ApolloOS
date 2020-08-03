/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Bitmap class for SAGAGfx Hidd.
    Lang: English.
*/

#define __OOP_NOATTRBASES__

#undef DEBUG
#define DEBUG 0

#include <aros/debug.h>

#include <proto/oop.h>
#include <proto/utility.h>
#include <assert.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <graphics/rastport.h>
#include <graphics/gfx.h>
#include <hidd/hidd.h>
#include <hidd/gfx.h>
#include <oop/oop.h>
#include <aros/symbolsets.h>
#include <aros/debug.h>

#include <string.h>

#include "sagagfx_bitmap.h"
#include "sagagfx_hidd.h"

#include LC_LIBDEFS_FILE

/*********** BitMap::New() *************************************/

OOP_Object *METHOD(SAGABitMap, Root, New)
{
    D(bug("[SAGABitMap] BitMap::New()\n"));

    o = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg) msg);
    
    if (o)
    {
        OOP_MethodID             disp_mid;
        struct SAGAGfxBitmapData *data;
        HIDDT_ModeID             modeid;
        OOP_Object               *sync;
        OOP_Object               *pf;
        
        struct TagItem attrs[] = 
        {
            { aHidd_ChunkyBM_Buffer, 0UL },
            { TAG_DONE,              0UL }
        };
        
        data = OOP_INST_DATA(cl, o);
        
        OOP_GetAttr(o, aHidd_BitMap_GfxHidd, (APTR)&data->gfxhidd);
        OOP_GetAttr(o, aHidd_BitMap_PixFmt,  (APTR)&data->pixfmtobj);
        OOP_GetAttr(o, aHidd_BitMap_ModeID,  &modeid);
        
        HIDD_Gfx_GetMode(data->gfxhidd, modeid, &sync, &pf);
        
        data->width          = OOP_GET(o, aHidd_BitMap_Width);
        data->height         = OOP_GET(o, aHidd_BitMap_Height);
        data->bytesperline   = OOP_GET(o, aHidd_BitMap_BytesPerRow);
        data->bytesperpix    = OOP_GET(data->pixfmtobj, aHidd_PixFmt_BytesPerPixel);
        data->bitsperpix     = OOP_GET(data->pixfmtobj, aHidd_PixFmt_BitsPerPixel);
        
        data->VideoBuffer    = AllocVecPooled(XSD(cl)->mempool, 64 + data->bytesperline * (data->height + 10));
        data->VideoData      = (UBYTE *)(((IPTR)data->VideoBuffer + 31) & ~31);
        
        data->hwregs.pixfmt  = XSD(cl)->SAGAGfx_GetPixFmt(data->bitsperpix);
        data->hwregs.modeid  = XSD(cl)->SAGAGfx_GetModeID((UWORD)data->width, (UWORD)data->height);
        
        data->hwregs.pixclk  = OOP_GET(sync, aHidd_Sync_PixelClock);
        data->hwregs.hpixel  = OOP_GET(sync, aHidd_Sync_HDisp);
        data->hwregs.hsstart = OOP_GET(sync, aHidd_Sync_HSyncStart);
        data->hwregs.hsstop  = OOP_GET(sync, aHidd_Sync_HSyncEnd);
        data->hwregs.htotal  = OOP_GET(sync, aHidd_Sync_HTotal);
        data->hwregs.vpixel  = OOP_GET(sync, aHidd_Sync_VDisp);
        data->hwregs.vsstart = OOP_GET(sync, aHidd_Sync_VSyncStart);
        data->hwregs.vsstop  = OOP_GET(sync, aHidd_Sync_VSyncEnd);
        data->hwregs.vtotal  = OOP_GET(sync, aHidd_Sync_VTotal);
        data->hwregs.hvsync  = OOP_GET(sync, aHidd_Sync_Flags);
        
        attrs[0].ti_Data = (IPTR)data->VideoData;
        OOP_SetAttrs(o, attrs);
        
        D(bug("[SAGABitMap] Bitmap %ld x % ld, %u bits per pixel, %u bytes per pixel, %u bytes per line\n",
            data->width, 
            data->height, 
            data->bitsperpix, 
            data->bytesperpix, 
            data->bytesperline));
        
        D(bug("[SAGABitMap] Video data at 0x%p (%u bytes)\n", 
            data->VideoData, 
            data->bytesperline * 
            data->height));
        
        if (OOP_GET(data->pixfmtobj, aHidd_PixFmt_ColorModel) == vHidd_ColorModel_Palette)
        {
            data->CLUT = AllocVecPooled(XSD(cl)->mempool, 256 * sizeof(ULONG));
            
            D(bug("[SAGABitMap] CLUT at %p\n", data->CLUT));
            
            if (!data->CLUT)
            {
                disp_mid = OOP_GetMethodID(IID_Root, moRoot_Dispose);
                OOP_CoerceMethod(cl, o, (OOP_Msg)&disp_mid);
                return(NULL);
            }
        }
    }
    
    D(bug("[SAGABitMap] Returning object %p\n", o));
    
    return(o);
}

/**********  Bitmap::Dispose()  ***********************************/

VOID METHOD(SAGABitMap, Root, Dispose)
{
    struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);
    
    D(bug("[SAGABitMap] BitMap::Dispose(0x%p)\n", o));
    
    if (data->CLUT)
    {
        FreeVecPooled(XSD(cl)->mempool, data->CLUT);
    }
    
    if (data->VideoBuffer)
    {
        FreeVecPooled(XSD(cl)->mempool, data->VideoBuffer);
    }
    
    OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
}

/*** BitMap::Get() *******************************************/

VOID METHOD(SAGABitMap, Root, Get)
{
    struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);
    
    ULONG idx;

    D(bug("[SAGABitMap] BitMap::Get(0x%p)\n", o));
    
    if (IS_BM_ATTR(msg->attrID, idx))
    {
        switch (idx)
        {
        case aoHidd_BitMap_Visible:
            *msg->storage = data->disp;
            return;
            
        case aoHidd_BitMap_LeftEdge:
            *msg->storage = data->xoffset;
            return;
            
        case aoHidd_BitMap_TopEdge:
            *msg->storage = data->yoffset;
            return;
        }
    }
    
    OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
}

/*** BitMap::SetColors() *************************************/

BOOL METHOD(SAGABitMap, Hidd_BitMap, SetColors)
{
    struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);
    
    D(bug("[SAGABitMap] BitMap::SetColors(%u, %u)\n", 
        msg->firstColor, 
        msg->numColors));
    
    if (!OOP_DoSuperMethod(cl, o, (OOP_Msg)msg))
    {
        D(bug("[SAGABitMap] DoSuperMethod() failed\n"));
        return(FALSE);
    }
    
    if ((msg->firstColor + msg->numColors) > 256)
    {
        return(FALSE);
    }
    
    if (data->CLUT)
    {
        ULONG i, j;
        
        for (i = msg->firstColor, j = 0; j < msg->numColors; i++, j++)
        {
            UWORD red   = msg->colors[j].red   >> 8;
            UWORD green = msg->colors[j].green >> 8;
            UWORD blue  = msg->colors[j].blue  >> 8;
            
            D(bug("[SAGABitMap] CLUT[%d] R=%d, G=%d, B=%d\n", i, red, green, blue));
            
            data->CLUT[i] = (red << 16) | (green << 8) | blue;
        }
        
        if (XSD(cl)->visible == o)
        {
            XSD(cl)->SAGAGfx_SetColors(data->CLUT, msg->firstColor, msg->numColors);
        }
    }
    
    return(TRUE);
}

/* END OF FILE */
