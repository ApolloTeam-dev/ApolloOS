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

#define ABS(x)           ((x) < 0) ? -(x) : (x)

#include LC_LIBDEFS_FILE

static inline void _PutPixel( struct SAGAGfxBitmapData *data, UWORD x, UWORD y, HIDDT_Pixel pixelToDraw )
{

#if 0
	//This check is already done in graphics library
	if( !data || !data->VideoData )	return;
	if( x >= data->width || y >= data->height )	return;		//No you don't :-)
#endif


	switch( data->bitsperpix )
	{
		case 8:
		{

			*(UBYTE*)(data->VideoData + ( y * data->bytesperline ) + ( x * data->bytesperpixel )) = pixelToDraw;
			//*pixel = pixelToDraw;

			break;
		}
		case 15:
		case 16:
		{
			*(UWORD*)( data->VideoData + ( y * data->bytesperline ) + ( x * data->bytesperpixel ) ) = pixelToDraw;
			break;
		}
		case 24:
		{
			UBYTE *pixel = (UBYTE*)( data->VideoData + ( y * data->bytesperline ) + ( x * data->bytesperpixel ) );
            *(UBYTE *)(pixel) = pixelToDraw >> 16;
            *(UBYTE *)(pixel + 1) = pixelToDraw >> 8;
            *(UBYTE *)(pixel + 2) = pixelToDraw;
			break;
		}
		case 32:
		{
			*(ULONG*)( data->VideoData + ( y * data->bytesperline ) + ( x * data->bytesperpixel ) ) = pixelToDraw;
			break;
		}
		default:
			//Not supported
		break;
	}
}

static inline void _DrawLine( struct SAGAGfxBitmapData *data, UWORD x1, UWORD x2, UWORD y1, UWORD y2, HIDDT_Pixel pixelToDraw)
{
	// THE EXTREMELY FAST LINE ALGORITHM Variation D (Addition Fixed Point)
	WORD xLength = ABS( x2 - x1 );
	WORD yLength = ABS( y2 - y1 );

#if 0
	//Some bounds checks
	if( x1 >= data->width )	x1=data->width-1;
	if( x2 >= data->width )	x2=data->width-1;
	if( y1 >= data->height )	y1=data->height-1;
	if( y2 >= data->height )	y2=data->height-1;
#endif

	//Special case vertical lines
	if( xLength == 0 )
	{
		if( y2 > y1 )
		{
			for( UWORD y = y1; y <= y2; y++ )
			{
				_PutPixel( data, x1, y, pixelToDraw );
			}
		}else
		{
			for( UWORD y = y2; y <= y1; y++ )
			{
				_PutPixel( data, x1, y, pixelToDraw );
			}
		}

		return;
	}

	//special case horizontal lines
	if( yLength == 0 )
	{
		if( x2 > x1 )
		{
			for( UWORD x = x1; x <= x2; x++ )
			{
				_PutPixel( data, x, y1, pixelToDraw );
			}
		}else
		{
			for( UWORD x = x2; x <= x1; x++ )
			{
				_PutPixel( data, x, y1, pixelToDraw );
			}
		}

		return;
	}

	//The other lines
	if( xLength >= yLength )
	{
		if( x2 > x1 )
		{
			if( y2 > y1 )
			{
				for( UWORD x = 0; x < xLength; x++ )
				{
					UWORD y = y1 + ( yLength * x / xLength );
					_PutPixel( data, x1 + x, y, pixelToDraw );
				}
			}else
			{
				for( UWORD x = 0; x < xLength; x++ )
				{
					UWORD y = y1 - ( yLength * x / xLength );
					_PutPixel( data, x1 + x, y, pixelToDraw );
				}
			}
		}else
		{
			if( y2 > y1 )
			{
				for( UWORD x = 0; x < xLength; x++ )
				{
					UWORD y = y1 + ( yLength * x / xLength );
					_PutPixel( data, x1 - x, y, pixelToDraw );
				}
			}else
			{
				for( UWORD x = 0; x < xLength; x++ )
				{
					UWORD y = y1 - ( yLength * x / xLength );
					_PutPixel( data, x1 - x, y, pixelToDraw );
				}
			}
		}
	}else
	{
		if( y2 > y1 )
		{
			if( x2 > x1 )
			{
				for( UWORD y = 0; y < yLength; y++ )
				{
					UWORD x = x1 + ( xLength * y / yLength );
					_PutPixel( data, x, y1 + y, pixelToDraw );
				}
			}else
			{
				{
					for( UWORD y = 0; y < yLength; y++ )
					{
						UWORD x = x1 - ( xLength * y / yLength );
						_PutPixel( data, x, y1 + y, pixelToDraw );
					}
				}
			}
		}else
		{
			if( x2 > x1 )
			{
				//bug( "DrawLine: (%d,%d)->(%d,%d)\n", x1, y1, x2, y2 );
				//bug( "xLength: %d    yLength: %d\n", xLength, yLength );
				for( UWORD y = 0; y < yLength; y++ )
				{
					UWORD x = x1 + ( xLength * y / yLength );
					_PutPixel( data, x, y1 - y, pixelToDraw );
				}
			}else
			{
				for( UWORD y = 0; y < yLength; y++ )
				{
					UWORD x = x1 - ( xLength * y / yLength );
					_PutPixel( data, x, y1 - y, pixelToDraw );
				}
			}
		}
	}
}

VOID SAGABitMap__Hidd_BitMap__PutPixel(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutPixel *msg )
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);

	_PutPixel( data, msg->x, msg->y, msg->pixel );

    return;
}

ULONG SAGABitMap__Hidd_BitMap__GetPixel(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_GetPixel *msg)
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);
    HIDDT_Pixel pixel = 0;
    UBYTE *mem = 0;

    mem = data->VideoData + ( msg->x * data->bytesperpixel )  + (msg->y * data->bytesperline);

    switch(data->bytesperpixel)
    {
        case 1:
            pixel = *(UBYTE *)mem;
            break;

        case 2:
            pixel = *(UWORD *)mem;
            break;

        case 3:
            pixel = (mem[0] << 16) | (mem[1] << 8) | mem[2];
            break;

        case 4:
            pixel = *(ULONG *)mem;
            break;
    }

    return pixel;
}

BOOL SAGABitMap__Hidd_BitMap__ObtainDirectAccess(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_ObtainDirectAccess *msg)
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);

	bug( "Locking a bitmap\n" );

    LOCK_BITMAP(data)

    *msg->addressReturn = data->VideoData;
    *msg->widthReturn = data->width;
    *msg->heightReturn = data->height;
    /* undocumented, just a guess.. */
    *msg->bankSizeReturn = *msg->memSizeReturn = data->bytesperline * data->height;
    data->locked++;

    return TRUE;
}

BOOL SAGABitMap__Hidd_BitMap__ReleaseDirectAccess(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_ObtainDirectAccess *msg)
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);

	bug( "Unlocking a bitmap\n" );
    UNLOCK_BITMAP(data)

    return TRUE;
}


VOID SAGABitMap__Hidd_BitMap__DrawLine(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_DrawLine *msg)
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);
	HIDDT_Pixel pixelToDraw = GC_FG(msg->gc);

	_DrawLine( data, msg->x1, msg->x2, msg->y1, msg->y2, pixelToDraw );
}

VOID SAGABitMap__Hidd_BitMap__FillRect(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_DrawRect *msg)
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);
	HIDDT_Pixel pixelToDraw = GC_FG(msg->gc);

#if 1
	UWORD y = msg->minY;
	for( ; y <= msg->maxY - 8; )
	{
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
		_DrawLine( data, msg->minX, msg->maxX, y, y++, pixelToDraw );
	}
	for( ; y <= msg->maxY; y++ )
	{
		_DrawLine( data, msg->minX, msg->maxX, y, y, pixelToDraw );
	}
#else
	UWORD xLength = ABS( msg->maxX - msg->minX );
	UWORD yLength = ABS( msg->maxY - msg->minY );

	if( xLength > yLength )
	{
		for( UWORD y = msg->minY; y <= msg->maxY; y++ )
		{
			_DrawLine( data, msg->minX, msg->maxX, y, y, pixelToDraw );
		}
	}else
	{
		for( UWORD x = msg->minX; x <= msg->maxX; x++ )
		{
			_DrawLine( data, x, x, msg->minY, msg->maxY, pixelToDraw );
		}
	}
#endif
}

VOID SAGABitMap__Hidd_BitMap__GetImage(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_GetImage *msg)
{
	//bug( "%s()\n", __FUNCTION__ );
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);

    switch(msg->pixFmt)
    {
        case vHidd_StdPixFmt_Native:
            switch(data->bytesperpixel)
            {
                case 1:
                    HIDD_BM_CopyMemBox8(o,
                                        data->VideoData,
                                        msg->x,
                                        msg->y,
                                        msg->pixels,
                                        0,
                                        0,
                                        msg->width,
                                        msg->height,
                                        data->bytesperline,
                                        msg->modulo);
                    break;

                case 2:
                    HIDD_BM_CopyMemBox16(o,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->pixels,
                                         0,
                                         0,
                                         msg->width,
                                         msg->height,
                                         data->bytesperline,
                                         msg->modulo);
                    break;

                case 3:
                    HIDD_BM_CopyMemBox24(o,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->pixels,
                                         0,
                                         0,
                                         msg->width,
                                         msg->height,
                                         data->bytesperline,
                                         msg->modulo);
                    break;

                case 4:
                    HIDD_BM_CopyMemBox32(o,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->pixels,
                                         0,
                                         0,
                                         msg->width,
                                         msg->height,
                                         data->bytesperline,
                                         msg->modulo);
                    break;
             } /* switch(data->bytesperpix) */
            break;

        case vHidd_StdPixFmt_Native32:
            switch(data->bytesperpixel)
            {
                case 1:
                    HIDD_BM_GetMem32Image8(o,
                                           data->VideoData,
                                           msg->x,
                                           msg->y,
                                           msg->pixels,
                                           msg->width,
                                           msg->height,
                                           data->bytesperline,
                                           msg->modulo);
                    break;

                case 2:
                    HIDD_BM_GetMem32Image16(o,
                                            data->VideoData,
                                            msg->x,
                                            msg->y,
                                            msg->pixels,
                                            msg->width,
                                            msg->height,
                                            data->bytesperline,
                                            msg->modulo);
                    break;

                case 3:
                    HIDD_BM_GetMem32Image24(o,
                                            data->VideoData,
                                            msg->x,
                                            msg->y,
                                            msg->pixels,
                                            msg->width,
                                            msg->height,
                                            data->bytesperline,
                                            msg->modulo);
                    break;

                case 4:
                    HIDD_BM_CopyMemBox32(o,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->pixels,
                                         0,
                                         0,
                                         msg->width,
                                         msg->height,
                                         data->bytesperline,
                                         msg->modulo);
                    break;
            } /* switch(data->bytesperpix) */
            break;

        default:
            {
                APTR 	    pixels = msg->pixels;
                APTR 	    srcPixels = data->VideoData + msg->y * data->bytesperline + msg->x * data->bytesperpixel;
                OOP_Object *dstpf;

                dstpf = HIDD_Gfx_GetPixFmt(data->gfxhidd, msg->pixFmt);

                HIDD_BM_ConvertPixels(o, &srcPixels, (HIDDT_PixelFormat *)data->pixfmtobj, data->bytesperline,
                                      &pixels, (HIDDT_PixelFormat *)dstpf, msg->modulo,
                                      msg->width, msg->height, NULL);
            }
            break;
    }
}

VOID SAGABitMap__Hidd_BitMap__PutImage(OOP_Class *cl, OOP_Object *o,
                                struct pHidd_BitMap_PutImage *msg)
{
	struct SAGAGfxBitmapData *data = OOP_INST_DATA(cl, o);

    //bug("%s()\n", __func__);

    LOCK_BITMAP(data)

    //bug("%s() bitmap locked\n", __func__);

    switch(msg->pixFmt)
    {
        case vHidd_StdPixFmt_Native:
            switch(data->bytesperpixel)
            {
                case 1:
                    HIDD_BM_CopyMemBox8(o,
                                        msg->pixels,
                                        0,
                                        0,
                                        data->VideoData,
                                        msg->x,
                                        msg->y,
                                        msg->width,
                                        msg->height,
                                        msg->modulo,
                                        data->bytesperline);
                    break;

                case 2:
                	//bug( "Here!\n" );
                    HIDD_BM_CopyMemBox16(o,
                                         msg->pixels,
                                         0,
                                         0,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->width,
                                         msg->height,
                                         msg->modulo,
                                         data->bytesperline);
                    break;

                case 3:
                    HIDD_BM_CopyMemBox24(o,
                                         msg->pixels,
                                         0,
                                         0,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->width,
                                         msg->height,
                                         msg->modulo,
                                         data->bytesperline);
                    break;

                case 4:
                    HIDD_BM_CopyMemBox32(o,
                                         msg->pixels,
                                         0,
                                         0,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->width,
                                         msg->height,
                                         msg->modulo,
                                         data->bytesperline);
                    break;
            } /* switch(data->bytesperpix) */
            break;

        case vHidd_StdPixFmt_Native32:
            switch(data->bytesperpixel)
            {
                case 1:
                    HIDD_BM_PutMem32Image8(o,
                                           msg->pixels,
                                           data->VideoData,
                                           msg->x,
                                           msg->y,
                                           msg->width,
                                           msg->height,
                                           msg->modulo,
                                           data->bytesperline);
                    break;

                case 2:
                	//bug( "Maybe here?\n" );
                    HIDD_BM_PutMem32Image16(o,
                                            msg->pixels,
                                            data->VideoData,
                                            msg->x,
                                            msg->y,
                                            msg->width,
                                            msg->height,
                                            msg->modulo,
                                            data->bytesperline);
                    break;

                case 3:
                    HIDD_BM_PutMem32Image24(o,
                                            msg->pixels,
                                            data->VideoData,
                                            msg->x,
                                            msg->y,
                                            msg->width,
                                            msg->height,
                                            msg->modulo,
                                            data->bytesperline);
                    break;

                case 4:
                    HIDD_BM_CopyMemBox32(o,
                                         msg->pixels,
                                         0,
                                         0,
                                         data->VideoData,
                                         msg->x,
                                         msg->y,
                                         msg->width,
                                         msg->height,
                                         msg->modulo,
                                         data->bytesperline);
                    break;
            } /* switch(data->bytesperpix) */
            break;

        default:
            {
#if 0
            	bug( "Hopefully not here.\n" );
            	bug( "Message Pixel format %ld\n", msg->pixFmt );
            	bug( "Bitmap pixel format %ld\n",((HIDDT_PixelFormat *)data->pixfmtobj)->stdpixfmt );
#endif
                APTR 	    pixels = msg->pixels;
                APTR 	    dstBuf = data->VideoData + msg->y * data->bytesperline + msg->x * data->bytesperpixel;
                OOP_Object *srcpf;

                srcpf = HIDD_Gfx_GetPixFmt(data->gfxhidd, msg->pixFmt);

                HIDD_BM_ConvertPixels(o, &pixels, (HIDDT_PixelFormat *)srcpf, msg->modulo,
                                      &dstBuf, (HIDDT_PixelFormat *)data->pixfmtobj, data->bytesperline,
                                      msg->width, msg->height, NULL);
            }
            break;
    } /* switch(msg->pixFmt) */

    UNLOCK_BITMAP(data)
}


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
        memset(data, 0, sizeof  (*data));
        InitSemaphore(&data->bmLock);
        
        OOP_GetAttr(o, aHidd_BitMap_GfxHidd, (APTR)&data->gfxhidd);
        OOP_GetAttr(o, aHidd_BitMap_PixFmt,  (APTR)&data->pixfmtobj);
        OOP_GetAttr(o, aHidd_BitMap_ModeID,  &modeid);
        
        HIDD_Gfx_GetMode(data->gfxhidd, modeid, &sync, &pf);
        
        data->width          = OOP_GET(o, aHidd_BitMap_Width);
        data->height         = OOP_GET(o, aHidd_BitMap_Height);
        data->bytesperline   = OOP_GET(o, aHidd_BitMap_BytesPerRow);
        data->bytesperpixel  = OOP_GET(data->pixfmtobj, aHidd_PixFmt_BytesPerPixel);
        //bug( "Created bitmap with %ld bytes per line.\n", data->bytesperline );
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
        //case aoHidd_BitMap_GfxHidd:
        //	*msg->storage = data->gfxhidd;
        //	return;
        case aoHidd_BitMap_Visible:
			*msg->storage = data->disp;
            return;
        //case aoHidd_BitMap_Width:
        //	*msg->storage = data->width;
        //	return;
        //case aoHidd_BitMap_Height:
        //	*msg->storage = data->height;
        //	return;
        //case aoHidd_BitMap_Align:
        //	//What do we do here?
        //	OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
        //	return;
        //case aoHidd_BitMap_Depth:
        //	*msg->storage = OOP_GET(o, aoHidd_BitMap_Depth);
        //	bug( "Screen depth requested.  Answering with %ld.\n", *msg->storage );
        //	return;
        case aoHidd_BitMap_LeftEdge:
            *msg->storage = data->xoffset;
            return;
        case aoHidd_BitMap_TopEdge:
            *msg->storage = data->yoffset;
            return;
        case aoHidd_BitMap_BytesPerRow:
        {
        //Returning OP_GET(o, aHidd_BitMap_BytesPerRow) causes a crash.............
		//	*msg->storage = OOP_GET(o, aHidd_BitMap_BytesPerRow);
        //	*msg->storage = data->bytesperline;
        	ULONG bytesPerLine = data->width * data->bytesperpixel;
        	//bug( "Bytes per row: %ld\n", bytesPerLine );
        	OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
        	return;
        }
        case aoHidd_BitMap_StdPixFmt:
        	//bug( "Standard pixel format was asked.\n" );
        	//*msg->storage = vHidd_StdPixFmt_RGB16;
        	*msg->storage = OOP_GET(data->pixfmtobj, aoHidd_BitMap_StdPixFmt);
        	//bug( "Answered with %ld.\n", *msg->storage );
        	return;
        //case aoHidd_BitMap_PixFmt:
        //	*msg->storage = OOP_GET(o, aoHidd_BitMap_PixFmt);
        //	return;
        default:
        	//bug( "SAGAGFX was asked for aspect %ld\n", idx );
        	OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
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
