/*
    Copyright © 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class.
    Lang: English.
*/

#define __OOP_NOATTRBASES__

#undef DEBUG
#define DEBUG 0

#include <aros/debug.h>

#include <aros/asmcall.h>
#include <proto/exec.h>
#include <proto/oop.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <aros/symbolsets.h>
#include <devices/inputevent.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <hardware/custom.h>
#include <hidd/hidd.h>
#include <hidd/gfx.h>
#include <oop/oop.h>
#include <clib/alib_protos.h>
#include <string.h>
#include <stdio.h>

#include "sagagfx_hidd.h"
#include "sagagfx_bitmap.h"
#include "sagagfx_hw.h"

#define MAKE_SYNC(name,clock,hdisp,hstart,hend,htotal,vdisp,vstart,vend,vtotal,flags,descr)   \
    struct TagItem sync_ ## name[] = { \
        { aHidd_Sync_PixelClock,         clock*1000  }, \
        { aHidd_Sync_HDisp,              hdisp       }, \
        { aHidd_Sync_HSyncStart,         hstart      }, \
        { aHidd_Sync_HSyncEnd,           hend        }, \
        { aHidd_Sync_HTotal,             htotal      }, \
        { aHidd_Sync_VDisp,              vdisp       }, \
        { aHidd_Sync_VSyncStart,         vstart      }, \
        { aHidd_Sync_VSyncEnd,           vend        }, \
        { aHidd_Sync_VTotal,             vtotal      }, \
        { aHidd_Sync_Flags,              flags       }, \
        { aHidd_Sync_Description,  (IPTR)descr       }, \
        { TAG_DONE,                      0UL         }  \
    }

enum {
    ML_PIXCLK,
    ML_HPIXEL,
    ML_HSSTRT,
    ML_HSSTOP,
    ML_HTOTAL,
    ML_VPIXEL,
    ML_VSSTRT,
    ML_VSSTOP,
    ML_VTOTAL,
    ML_HVSYNC,
    ML_COUNT
};


OOP_Object *METHOD(SAGAGfx, Root, New)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    struct TagItem *userSyncs = NULL;
    
    // Predefined resolutions

   MAKE_SYNC(320x200, 28375, 640/2, 753/2, 817/2, 908/2,  400/2, 490/2, 492/2, 524/2, 1, "SAGA:320x200");
   MAKE_SYNC(320x240, 28375, 640/2, 753/2, 817/2, 908/2,  480/2, 490/2, 492/2, 524/2, 1, "SAGA:320x240");
   MAKE_SYNC(320x256, 28375, 640/2, 704/2, 763/2, 800/2,  512/2, 544/2, 554/2, 592/2, 1, "SAGA:320x256");
   MAKE_SYNC(640x400, 28375, 640, 753, 817, 908,  400, 490, 492, 524, 1, "SAGA:640x400");
   MAKE_SYNC(640x480, 28375, 640, 753, 817, 908,  480, 490, 492, 524, 1, "SAGA:640x480");
   MAKE_SYNC(640x512, 28375, 640, 704, 763, 800,  512, 544, 554, 592, 1, "SAGA:640x512");
    MAKE_SYNC(960x540, 28375*2, 960, 1024, 1124, 1516,  540, 552, 556, 626, 1, "SAGA:960x540");
    MAKE_SYNC(480x270, 28375*2, 960/2, 1024/2, 1124/2, 1516/2,  540/2, 552/2, 556/2, 626/2, 1, "SAGA:480x270");
    MAKE_SYNC(304x224, 28375, 304, 753/2, 817/2, 908/2,  224, 490/2, 492/2, 524/2, 1, "SAGA:304x224");
    MAKE_SYNC(1280x720,28375*2,1280,1390,1430,1516,  720, 725, 730, 750, 1, "SAGA1280x720");
    MAKE_SYNC(640x360, 28375*2, 640, 1390, 1430,1516,  360, 725, 730, 750, 1, "SAGA:640x360");
    MAKE_SYNC(800x600, 28375*2, 800, 900, 1000, 1352,  600, 620, 640, 700, 1, "SAGA:800x600");
    MAKE_SYNC(1024x768, 28375*2,1024,1100,1150,1216,  768, 770, 774, 780, 1, "SAGA1024x768");
    MAKE_SYNC(720x576, 28375, 720, 753, 817, 908,  576, 582, 586, 626, 1, "SAGA:720x576");
    MAKE_SYNC(848x480, 28375*2, 848, 936, 984, 1516,  480, 490, 492, 626, 1, "SAGA:848x480");
    MAKE_SYNC(640x200, 28375, 640, 753, 817, 908,  400/2, 490/2, 492/2, 524/2, 1, "SAGA:640x200");
    MAKE_SYNC(1920x1080, 28375*3, 1920, 2085, 2145, 2274, 1080, 1088, 1095, 1125, 1, "SAGA:1920x1080");
    MAKE_SYNC(1280x1024, 28375*3, 1280, 1390, 1430, 1516, 1024, 1088, 1108, 1184, 1, "SAGA:1280x1024");
	
    const struct TagItem syncs[] =
	{
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_320x200         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_320x240         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_320x256         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_640x400         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_640x480         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_640x512         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_960x540         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_480x270         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_304x224         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_1280x720        },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_640x360         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_800x600         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_1024x768        },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_720x576         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_848x480         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_640x200         },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_1920x1080       },
		{ aHidd_Gfx_SyncTags,         (IPTR)sync_1280x1024       },
		{ TAG_DONE,                   0UL                        }
	};

    
    const struct TagItem pftags_32bpp[] = 
    {
        { aHidd_PixFmt_RedShift,      8                          }, /* 0 */
        { aHidd_PixFmt_GreenShift,    16                         }, /* 1 */
        { aHidd_PixFmt_BlueShift,  	  24                         }, /* 2 */
        { aHidd_PixFmt_AlphaShift,    0                          }, /* 3 */
        { aHidd_PixFmt_RedMask,       0x00ff0000                 }, /* 4 */
        { aHidd_PixFmt_GreenMask,     0x0000ff00                 }, /* 5 */
        { aHidd_PixFmt_BlueMask,      0x000000ff                 }, /* 6 */
        { aHidd_PixFmt_AlphaMask,     0xff000000                 }, /* 7 */
        { aHidd_PixFmt_ColorModel,    vHidd_ColorModel_TrueColor }, /* 8 */
        { aHidd_PixFmt_Depth,         32                         }, /* 9 */
        { aHidd_PixFmt_BytesPerPixel, 4                          }, /* 10 */
        { aHidd_PixFmt_BitsPerPixel,  32                         }, /* 11 */
        { aHidd_PixFmt_StdPixFmt,     vHidd_StdPixFmt_BGRA32     }, /* 12 Native */
        { aHidd_PixFmt_BitMapType,    vHidd_BitMapType_Chunky    }, /* 15 */
        { TAG_DONE,                   0UL                        }
    };
    
    const struct TagItem pftags_24bpp[] = 
    {
        { aHidd_PixFmt_RedShift,      8                          }, /* 0 */
        { aHidd_PixFmt_GreenShift,    16                         }, /* 1 */
        { aHidd_PixFmt_BlueShift,     24                         }, /* 2 */
        { aHidd_PixFmt_AlphaShift,    0                          }, /* 3 */
        { aHidd_PixFmt_RedMask,       0x00ff0000                 }, /* 4 */
        { aHidd_PixFmt_GreenMask,     0x0000ff00                 }, /* 5 */
        { aHidd_PixFmt_BlueMask,      0x000000ff                 }, /* 6 */
        { aHidd_PixFmt_AlphaMask,     0x00000000                 }, /* 7 */
        { aHidd_PixFmt_ColorModel,    vHidd_ColorModel_TrueColor }, /* 8 */
        { aHidd_PixFmt_Depth,         24                         }, /* 9 */
        { aHidd_PixFmt_BytesPerPixel, 3                          }, /* 10 */
        { aHidd_PixFmt_BitsPerPixel,  24                         }, /* 11 */
        { aHidd_PixFmt_StdPixFmt,     vHidd_StdPixFmt_BGR24      }, /* 12 Native */
        { aHidd_PixFmt_BitMapType,    vHidd_BitMapType_Chunky    }, /* 15 */
        { TAG_DONE,                   0UL                        }
    };
    
    const struct TagItem pftags_16bpp[] = 
    {
        { aHidd_PixFmt_RedShift,      16                         }, /* 0 */
        { aHidd_PixFmt_GreenShift,    21                         }, /* 1 */
        { aHidd_PixFmt_BlueShift,     27                         }, /* 2 */
        { aHidd_PixFmt_AlphaShift,    0                          }, /* 3 */
        { aHidd_PixFmt_RedMask,       0x0000f800                 }, /* 4 */
        { aHidd_PixFmt_GreenMask,     0x000007e0                 }, /* 5 */
        { aHidd_PixFmt_BlueMask,      0x0000001f                 }, /* 6 */
        { aHidd_PixFmt_AlphaMask,     0x00000000                 }, /* 7 */
        { aHidd_PixFmt_ColorModel,    vHidd_ColorModel_TrueColor }, /* 8 */
        { aHidd_PixFmt_Depth,         16                         }, /* 9 */
        { aHidd_PixFmt_BytesPerPixel, 2                          }, /* 10 */
        { aHidd_PixFmt_BitsPerPixel,  16                         }, /* 11 */
        { aHidd_PixFmt_StdPixFmt,     vHidd_StdPixFmt_RGB16      }, /* 12 */
        { aHidd_PixFmt_BitMapType,    vHidd_BitMapType_Chunky    }, /* 15 */
        { TAG_DONE,                   0UL                        }
    };
    
    const struct TagItem pftags_8bpp[] = 
    {
        { aHidd_PixFmt_RedShift,      8                          }, /* 0 */
        { aHidd_PixFmt_GreenShift,    16                         }, /* 1 */
        { aHidd_PixFmt_BlueShift,     24                         }, /* 2 */
        { aHidd_PixFmt_AlphaShift,    0                          }, /* 3 */
        { aHidd_PixFmt_RedMask,       0x00ff0000                 }, /* 4 */
        { aHidd_PixFmt_GreenMask,     0x0000ff00                 }, /* 5 */
        { aHidd_PixFmt_BlueMask,      0x000000ff                 }, /* 6 */
        { aHidd_PixFmt_AlphaMask,     0x00000000                 }, /* 7 */
        { aHidd_PixFmt_ColorModel,    vHidd_ColorModel_Palette   }, /* 8 */
        { aHidd_PixFmt_CLUTMask,      0x000000ff                 },
        { aHidd_PixFmt_CLUTShift,     0x00000000                 },
        { aHidd_PixFmt_Depth,         8                          }, /* 9 */
        { aHidd_PixFmt_BytesPerPixel, 1                          }, /* 10 */
        { aHidd_PixFmt_BitsPerPixel,  8                          }, /* 11 */
        { aHidd_PixFmt_StdPixFmt,     vHidd_StdPixFmt_LUT8       }, /* 12 */
        { aHidd_PixFmt_BitMapType,    vHidd_BitMapType_Chunky    }, /* 15 */
        { TAG_DONE,                   0UL                        }
    };
    
    const struct TagItem modetags[] = 
    {
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_32bpp         },
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_24bpp         },
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_16bpp         },
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_8bpp          },

        { TAG_MORE,                   (IPTR)syncs                },
        { TAG_DONE,                   0UL                        }
    };
    
    struct TagItem saganewtags[] = 
    {
        { aHidd_Gfx_ModeTags,         (IPTR)modetags             },
        { aHidd_Name,                 (IPTR)"SAGA"               },
        { aHidd_HardwareName,         (IPTR)"SAGA Graphics Chip" },
        { aHidd_ProducerName,         (IPTR)"APOLLO-Team"        },
        { TAG_MORE,                   (IPTR)msg->attrList        }
    };
    
    struct pRoot_New newmsg;
    
    /* Are we already there? Don't init it for the second time! */
    
    if (data->sagagfxhidd)
    {
        return(NULL);
    }
    
    /*
        The instance of driver object is created by the wrapper from
        DEVS:Monitors through a call to AddDisplayDriver(). The wrapper
        has set the current directory properly and we can extract its name.

        We use this knowledge to eventually open the corresponding Icon and
        read the driver specific tooltypes. Eventually we parse those needed.
    */
    
    data->useHWSprite = TRUE;
    
    
    
    data->SAGAGfx_SetSpriteHide();
    
    newmsg.mID = msg->mID;
    newmsg.attrList = saganewtags;
    msg = &newmsg;
    
    o = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    
    if (o)
    {
        data->sagagfxhidd = o;
    }
    
    return(o);
}

VOID METHOD(SAGAGfx, Root, Dispose)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    data->sagagfxhidd = NULL;
    DeletePool(data->mempool);
    OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
}

VOID METHOD(SAGAGfx, Root, Get)
{
    ULONG idx;
    int found = FALSE;

    if (IS_GFX_ATTR(msg->attrID, idx))
    {
        switch (idx)
        {
            case aoHidd_Gfx_NoFrameBuffer:
                found = TRUE;
                *msg->storage = TRUE;
                break;

            case aoHidd_Gfx_HWSpriteTypes:
                found = TRUE;
                *msg->storage = XSD(cl)->useHWSprite ? vHidd_SpriteType_3Plus1 : 0;
                break;

#if 0
            /* Not implemented yet */
            case aoHidd_Gfx_MaxHWSpriteWidth:
                found = TRUE;
                *msg->storage = 16;
                break;

            case aoHidd_Gfx_MaxHWSpriteHeight:
                found = TRUE;
                *msg->storage = 16;
                break;
#endif
        }
    }

    if (FALSE == found)
    {
        OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    }
}

BOOL METHOD(SAGAGfx, Hidd_Gfx, SetCursorPos)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    if (data->visible)
    {
        struct SAGAGfxBitmapData *bmdata = OOP_INST_DATA(data->bmclass, data->visible);
        
        WORD x = msg->x;
        WORD y = msg->y;
        
        data->cursorX = x;
        data->cursorY = y;
        
        if (data->cursor_visible)
        {
            x += data->hotX;
            y += data->hotY;
            
            data->SAGAGfx_SetSpritePosition(x, y);
        }
    }
    return TRUE;
}

VOID METHOD(SAGAGfx, Hidd_Gfx, SetCursorVisible)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    if (msg->visible)
    {
        data->SAGAGfx_SetSpritePosition(data->cursorX, data->cursorY);
    }
    else
    {
        data->SAGAGfx_SetSpriteHide();
    }
    
    data->cursor_visible = msg->visible;
}

BOOL METHOD(SAGAGfx, Hidd_Gfx, SetCursorShape)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    IPTR width, height, depth;
    OOP_Object *cmap = NULL;
    IPTR num_colors = 0;
    IPTR ptr;
    
    OOP_GetAttr(msg->shape, aHidd_BitMap_Width,    &width );
    OOP_GetAttr(msg->shape, aHidd_BitMap_Height,   &height);
    OOP_GetAttr(msg->shape, aHidd_BitMap_Depth,    &depth );
    OOP_GetAttr(msg->shape, aHidd_BitMap_ColorMap, &cmap  );
    OOP_GetAttr(cmap, aHidd_ColorMap_NumEntries, &num_colors);
    


    if (cmap)
    {
        if (num_colors > 4)
        {
            num_colors = 4;
        }
        
        for (int i = 0; i < num_colors; i++)
        {
            HIDDT_Color c;
            HIDD_CM_GetColor(cmap, i, &c);
            
            data->cursor_pal[i] = 
                ((c.red   >> 12) & 15) << 8 |
                ((c.green >> 12) & 15) << 4 |
                ((c.blue  >> 12) & 15) << 0;
            
        }
    }
    
    
    if (width  > 16) width  = 16;
    if (height > 16) height = 16;
    
    if (width != 16 || height != 16)
    {
        for (UWORD i = 0; i < 16 * 16; i++)
        {
            data->cursor_clut[i] = 0;
        }
    }
    
    HIDD_BM_GetImageLUT(msg->shape, data->cursor_clut, 16, 0, 0, width, height, NULL);
    
    data->SAGAGfx_SetSpriteColors(data->cursor_pal);
    data->SAGAGfx_SetSpriteMemory(data->cursor_clut);
    
    data->hotX = msg->xoffset;
    data->hotY = msg->yoffset;
    
    return(TRUE);
}

OOP_Object *METHOD(SAGAGfx, Hidd_Gfx, CreateObject)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    OOP_Object *object = NULL;
    
    
    if (msg->cl == data->basebm)
    {
        BOOL displayable;
        BOOL framebuffer;
        
        struct TagItem tags[2] = 
        {
            { TAG_IGNORE, 0UL                 },
            { TAG_MORE  , (IPTR)msg->attrList }
        };
        
        struct pHidd_Gfx_CreateObject p;
        
        displayable = GetTagData(aHidd_BitMap_Displayable, FALSE, msg->attrList);
        framebuffer = GetTagData(aHidd_BitMap_FrameBuffer, FALSE, msg->attrList);
        
        
        if (displayable)
        {
            /* Only displayable bitmaps are bitmaps of our class */
            tags[0].ti_Tag  = aHidd_BitMap_ClassPtr;
            tags[0].ti_Data = (IPTR)data->bmclass;
        }
        else
        {
            /* Non-displayable friends of our bitmaps are our bitmaps too */
            OOP_Object *friend = (OOP_Object *)GetTagData(aHidd_BitMap_Friend, 0, msg->attrList);
            
            
            if (friend && (OOP_OCLASS(friend) == data->bmclass))
            {
                tags[0].ti_Tag  = aHidd_BitMap_ClassPtr;
                tags[0].ti_Data = (IPTR)data->bmclass;
            }
        }
        
        p.mID = msg->mID;
        p.cl  = msg->cl;
        p.attrList = tags;
        
        object = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)&p);
    }
    else
    {
        object = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    }
    
    return object;
}

/*********  GfxHidd::CopyBox()  ************************/

void METHOD(SAGAGfx, Hidd_Gfx, CopyBox)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    ULONG mode = GC_DRMD(msg->gc);
    IPTR  src  = 0;
    IPTR  dst  = 0;
    
    
    if (OOP_OCLASS(msg->src ) != data->bmclass ||
        OOP_OCLASS(msg->dest) != data->bmclass)
    {
        
        OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    }
    else
    {
        struct SAGAGfxBitmapData *bm_src = OOP_INST_DATA(OOP_OCLASS(msg->src),  msg->src );
        struct SAGAGfxBitmapData *bm_dst = OOP_INST_DATA(OOP_OCLASS(msg->dest), msg->dest);
        
        if (bm_src->bitsperpix <= 8 || 
            bm_dst->bitsperpix <= 8 || 
           (bm_src->bitsperpix != bm_dst->bitsperpix))
        {
            
            OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
        }
        else
        {
            
            OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
        }
    }
}

/*********  GfxHidd::Show()  ***************************/

OOP_Object *METHOD(SAGAGfx, Hidd_Gfx, Show)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    struct TagItem tags[] = 
    {
        { aHidd_BitMap_Visible, FALSE },
        { TAG_DONE,             0UL   }
    };
    
    
    /* Remove old bitmap from the screen */
    
    if (data->visible)
    {
//      OOP_SetAttrs(data->visible, tags);
    }
    
    if (msg->bitMap)
    {
        struct SAGAGfxBitmapData *bmdata = OOP_INST_DATA(data->bmclass, msg->bitMap);
        
        /* If we have a bitmap to show, set it as visible */
        
        tags[0].ti_Data = TRUE;
//      OOP_SetAttrs(msg->bitMap, tags);
        
        data->SAGAGfx_SetModeline(
            bmdata->hwregs.hpixel,
            bmdata->hwregs.hsstart,
            bmdata->hwregs.hsstop,
            bmdata->hwregs.htotal,
            bmdata->hwregs.vpixel,
            bmdata->hwregs.vsstart,
            bmdata->hwregs.vsstop,
            bmdata->hwregs.vtotal,
            bmdata->hwregs.hvsync);
        
        data->SAGAGfx_SetPLL(bmdata->hwregs.pixclk);
        data->SAGAGfx_SetColors(bmdata->CLUT, 0, 256);
        data->SAGAGfx_SetMemory((ULONG)bmdata->VideoData);
        data->SAGAGfx_SetMode(bmdata->hwregs.modeid, bmdata->hwregs.pixfmt);
        data->SAGAGfx_SetModulo(0);
        
        data->SAGAGfx_SetSpriteColors(data->cursor_pal);
        data->SAGAGfx_SetSpriteMemory(data->cursor_clut);
        
        if (data->cursor_visible)
        {
            data->SAGAGfx_SetSpritePosition(data->cursorX, data->cursorY);
        }
        else
        {
            data->SAGAGfx_SetSpriteHide();
        }
    }
    else
    {
        
        data->SAGAGfx_SetMode(0,0);
    }
    
    data->visible = msg->bitMap;
    
    return msg->bitMap;
}

