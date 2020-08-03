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

struct TagItem * LoadExternalSyncs(OOP_Class *cl)
{
    struct Process *pr      = NULL;
    struct TagItem *retVal  = NULL;
    struct TagItem *lastTag = NULL;
    
    pr = (struct Process *)FindTask(NULL);
    
    if (pr->pr_Task.tc_Node.ln_Type == NT_PROCESS)
    {
        struct DOSBase *DOSBase;
        
        APTR winptr = pr->pr_WindowPtr;
        pr->pr_WindowPtr = (APTR)-1;
        
        DOSBase = (struct DOSBase *)OpenLibrary("dos.library", 0UL);
        
        if (DOSBase)
        {
            BPTR fp = Open("DEVS:modelines.txt", MODE_OLDFILE);
            
            if (fp)
            {
                TEXT buf[512];
                
                while (FGets(fp, buf, 512))
                {
                    char *p = buf;
                    ULONG i, Modeline[ML_COUNT];
                    
                    if (p[0] != '#')
                    {
                        for (i = 0; i < ML_COUNT; i++)
                        {
                            LONG n = StrToLong(p, (LONG *)&Modeline[i]);
                            
                            if (n == -1 || Modeline[i] == 0 || (i != ML_PIXCLK && Modeline[i] > 4000))
                            {
                                break;
                            }
                            
                            p += n;
                        }
                        
                        if (i == ML_COUNT)
                        {
                            struct TagItem *tags = AllocVecPooled(XSD(cl)->mempool, sizeof(struct TagItem)*14);
                            char *desc = AllocVecPooled(XSD(cl)->mempool, 32);
                            
                            if (lastTag != NULL)
                            {
                                lastTag[1].ti_Tag  = TAG_MORE;
                                lastTag[1].ti_Data = (IPTR)tags;
                            }
                            
                            snprintf(desc, 32, "SAGA(User):%dx%d", Modeline[ML_HPIXEL], Modeline[ML_VPIXEL]);
                            
                            tags[0].ti_Tag   = aHidd_Gfx_SyncTags;
                            tags[0].ti_Data  = (IPTR)&tags[2];
                            
                            tags[1].ti_Tag   = TAG_DONE;
                            tags[1].ti_Data  = 0;
                            
                            tags[2].ti_Tag   = aHidd_Sync_PixelClock;
                            tags[2].ti_Data  = Modeline[ML_PIXCLK] * 1000;
                            
                            tags[3].ti_Tag   = aHidd_Sync_HDisp;
                            tags[3].ti_Data  = Modeline[ML_HPIXEL];
                            
                            tags[4].ti_Tag   = aHidd_Sync_HSyncStart;
                            tags[4].ti_Data  = Modeline[ML_HSSTRT];
                            
                            tags[5].ti_Tag   = aHidd_Sync_HSyncEnd;
                            tags[5].ti_Data  = Modeline[ML_HSSTOP];
                            
                            tags[6].ti_Tag   = aHidd_Sync_HTotal;
                            tags[6].ti_Data  = Modeline[ML_HTOTAL];
                            
                            tags[7].ti_Tag   = aHidd_Sync_VDisp;
                            tags[7].ti_Data  = Modeline[ML_VPIXEL];
                            
                            tags[8].ti_Tag   = aHidd_Sync_VSyncStart;
                            tags[8].ti_Data  = Modeline[ML_VSSTRT];
                            
                            tags[9].ti_Tag   = aHidd_Sync_VSyncEnd;
                            tags[9].ti_Data  = Modeline[ML_VSSTOP];
                            
                            tags[10].ti_Tag  = aHidd_Sync_VTotal;
                            tags[10].ti_Data = Modeline[ML_VTOTAL];
                            
                            tags[11].ti_Tag  = aHidd_Sync_Flags;
                            tags[11].ti_Data = Modeline[ML_HVSYNC];
                            
                            tags[12].ti_Tag  = aHidd_Sync_Description;
                            tags[12].ti_Data = (IPTR)desc;
                            
                            tags[13].ti_Tag  = TAG_DONE;
                            tags[13].ti_Data = 0UL;
                            
                            lastTag = tags;
                            
                            if (retVal == 0)
                            {
                                retVal = tags;
                            }
                        }
                    }
                }
                
                Close(fp);
            }
            
            CloseLibrary((struct Library *)DOSBase);
        }
        
        pr->pr_WindowPtr = winptr;
    }

    return(retVal);
}

OOP_Object *METHOD(SAGAGfx, Root, New)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    struct TagItem *userSyncs = NULL;
    
    // Predefined resolutions
    
    MAKE_SYNC(320x200, 28375, 320, 688,  720,  800, 200, 483, 487, 494, 1, "SAGA:320x200");
    MAKE_SYNC(320x240, 28375, 320, 688,  720,  800, 240, 483, 487, 494, 1, "SAGA:320x240");
    MAKE_SYNC(320x256, 28375, 320, 688,  720,  800, 256, 483, 487, 494, 1, "SAGA:320x256");
    MAKE_SYNC(640x400, 28375, 640, 688,  720,  800, 400, 483, 487, 494, 1, "SAGA:640x400");
    MAKE_SYNC(640x480, 28375, 640, 688,  720,  800, 480, 483, 487, 494, 1, "SAGA:640x480");
    MAKE_SYNC(640x512, 28375, 640, 648,  680,  720, 512, 516, 524, 530, 1, "SAGA:640x512");
    MAKE_SYNC(960x540, 28375, 960, 968, 1000, 1040, 540, 542, 550, 556, 1, "SAGA:960x540");
    
    struct TagItem syncs[] = 
    {
        { aHidd_Gfx_SyncTags,         (IPTR)sync_320x200         }, // syncs[0]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_320x240         }, // syncs[1]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_320x256         }, // syncs[2]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_640x400         }, // syncs[3]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_640x480         }, // syncs[4]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_640x512         }, // syncs[5]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_960x540         }, // syncs[6]
        { TAG_DONE,                   0UL                        }, // syncs[7] <<< INSERT USERSYNCS HERE
        { TAG_DONE,                   0UL                        }  // syncs[8]
    };
    
    struct TagItem pftags_32bpp[] = 
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
    
    struct TagItem pftags_24bpp[] = 
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
    
    struct TagItem pftags_16bpp[] = 
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
    
    struct TagItem pftags_8bpp[] = 
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
    
    struct TagItem modetags[] = 
    {
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_32bpp         },
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_24bpp         },
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_16bpp         },
        { aHidd_Gfx_PixFmtTags,       (IPTR)pftags_8bpp          },
/*
        { aHidd_Gfx_SyncTags,         (IPTR)sync_320x200         }, // syncs[0]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_320x240         }, // syncs[1]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_320x256         }, // syncs[2]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_640x400         }, // syncs[3]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_640x480         }, // syncs[4]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_640x512         }, // syncs[5]
        { aHidd_Gfx_SyncTags,         (IPTR)sync_960x540         }, // syncs[6]
*/
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
        D(bug("[SAGAGfx] New() => Already instanciated. \n"));
        return(NULL);
    }
    
    /*
        The instance of driver object is created by the wrapper from
        DEVS:Monitors through a call to AddDisplayDriver(). The wrapper
        has set the current directory properly and we can extract its name.

        We use this knowledge to eventually open the corresponding Icon and
        read the driver specific tooltypes. Eventually we parse those needed.
    */
    
    D(bug("[SAGAGfx] New() => Read Icon Tooltypes. \n"));
    
    struct Library *IconBase = OpenLibrary("icon.library", 0);
    
    data->useHWSprite = TRUE;
    
    if (IconBase)
    {
        struct DiskObject *icon;
        STRPTR myName = FindTask(NULL)->tc_Node.ln_Name;
        
        icon = GetDiskObject(myName);
        
        if (icon)
        {
            // TOOLTYPE: HWSPRITE=(YES|NO)
            
            STRPTR hwSprite = FindToolType(icon->do_ToolTypes, "HWSPRITE");
            
            if (hwSprite && MatchToolValue(hwSprite, "No"))
            {
                data->useHWSprite = FALSE;
            }
            
            // TOOLTYPE: ...
            
            // TOOLTYPE: ...
            
            // TOOLTYPE: ...
            
            FreeDiskObject(icon);
        }
        
        CloseLibrary(IconBase);
    }
    
    D(bug("[SAGAGfx] Load UserSyncs. \n"));
    
	switch (data->boardModel)
    {
        case VREG_BOARD_V4:
        case VREG_BOARD_V4SA:
            // V4 METHOD
            break;
        default:
            // V2 METHOD
            userSyncs = LoadExternalSyncs(cl);
            if (userSyncs)
            {
                syncs[7].ti_Tag  = TAG_MORE;
                syncs[7].ti_Data = (IPTR)userSyncs;
            }
            break;
	}
    
    D(bug("[SAGAGfx] New() called. \n"));
    
    data->SAGAGfx_SetSpriteHide();
    
    newmsg.mID = msg->mID;
    newmsg.attrList = saganewtags;
    msg = &newmsg;
    
    o = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    
    if (o)
    {
        data->sagagfxhidd = o;
    }
    
    D(bug("[SAGAGfx] New() = %p\n", o));
    
    return(o);
}

VOID METHOD(SAGAGfx, Root, Dispose)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    D(bug("[SAGA] Root::Dispose()\n"));
    
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
        
        D(bug("[SAGA] SetCursorPos(%d, %d)\n", msg->x, msg->y));
        
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
    
    D(bug("[SAGA] SetCursorVisible(%d)\n", msg->visible));
    
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
    
    if (cmap)
    {
        OOP_GetAttr(cmap, aHidd_ColorMap_NumEntries, &num_colors);
        
        if (num_colors > 4)
        {
            num_colors = 4;
        }
        
        D(bug("[SAGA] number of colors: %d\n", num_colors));
        
        for (int i = 0; i < num_colors; i++)
        {
            HIDDT_Color c;
            HIDD_CM_GetColor(cmap, i, &c);
            
            data->cursor_pal[i] = 
                ((c.red   >> 12) & 15) << 8 |
                ((c.green >> 12) & 15) << 4 |
                ((c.blue  >> 12) & 15) << 0;
            
            D(bug("[SAGA] c%02x: %x %x %x %x %08x\n", i, 
                c.red, 
                c.green, 
                c.blue, 
                c.alpha, 
                c.pixval));
        }
    }
    
    D(bug("[SAGA] SetCursorShape(%p, %d, %d, %d)\n", msg->shape, 
        width, 
        height, 
        depth));
    
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
    
    D(bug("[SAGA] Hidd_Gfx::CreateObject()\n"));
    
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
        
        D(bug("[SAGA] displayable=%d, framebuffer=%d\n", 
            displayable, 
            framebuffer));
        
        if (displayable)
        {
            D(bug("[SAGA] Displayable bitmap.\n"));
            
            /* Only displayable bitmaps are bitmaps of our class */
            tags[0].ti_Tag  = aHidd_BitMap_ClassPtr;
            tags[0].ti_Data = (IPTR)data->bmclass;
        }
        else
        {
            /* Non-displayable friends of our bitmaps are our bitmaps too */
            OOP_Object *friend = (OOP_Object *)GetTagData(aHidd_BitMap_Friend, 0, msg->attrList);
            
            D(bug("[SAGA] Not displayable. Friend=%p.\n", friend));
            
            if (friend && (OOP_OCLASS(friend) == data->bmclass))
            {
                D(bug("[SAGA] ClassID = ChunkyBM, friend is OK, returning correct class\n"));
                
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
    
    D(bug("[SAGA] CreateObject returns %p\n", object));

    return object;
}

/*********  GfxHidd::CopyBox()  ************************/

void METHOD(SAGAGfx, Hidd_Gfx, CopyBox)
{
    struct SAGAGfx_staticdata *data = XSD(cl);
    
    ULONG mode = GC_DRMD(msg->gc);
    IPTR  src  = 0;
    IPTR  dst  = 0;
    
    D(bug("[SAGA] CopyBox(%p, %p, dx:%d, dy:%d, sx:%d, sy:%d, w:%d, h:%d)\n", 
        msg->src, 
        msg->dest, 
        msg->destX, 
        msg->destY, 
        msg->srcX, 
        msg->srcY, 
        msg->width, 
        msg->height));
    
    if (OOP_OCLASS(msg->src ) != data->bmclass ||
        OOP_OCLASS(msg->dest) != data->bmclass)
    {
        D(bug("[SAGA] CopyBox - either source or dest is not SAGA bitmap\n"));
        
        D(bug("[SAGA] oclass src: %p, oclass dst: %p, bmclass: %p\n", 
            OOP_OCLASS(msg->src), 
            OOP_OCLASS(msg->dest), 
            data->bmclass));
        
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
            D(bug("[SAGA] bpp_src=%d, bpp_dst=%d\n", 
                bm_src->bitsperpix, 
                bm_dst->bitsperpix));
            
            OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
        }
        else
        {
            D(bug("[SAGA] both bitmaps compatible. drmd=%d\n", mode));
            
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
    
    D(bug("[SAGA] Show(0x%p), old visible 0x%p\n", msg->bitMap, data->visible));
    
    /* Remove old bitmap from the screen */
    
    if (data->visible)
    {
        D(bug("[SAGA] Hiding old bitmap\n"));
//      OOP_SetAttrs(data->visible, tags);
    }
    
    if (msg->bitMap)
    {
        struct SAGAGfxBitmapData *bmdata = OOP_INST_DATA(data->bmclass, msg->bitMap);
        
        /* If we have a bitmap to show, set it as visible */
        
        D(bug("[SAGA] Showing new bitmap\n"));
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
        D(bug("[SAGA] No bitmap to show? Falling back to AGA...\n"));
        
        data->SAGAGfx_SetMode(0,0);
    }
    
    data->visible = msg->bitMap;
    
    D(bug("[SAGA] Show() done\n"));
    
    return msg->bitMap;
}
