/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class.
    Lang: English.
*/

#define __OOP_NOATTRBASES__

#undef DEBUG
#define DEBUG 1

#include <aros/debug.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/input.h>
#include <proto/oop.h>
#include <exec/exec.h>
#include <exec/types.h>
#include <exec/lists.h>
#include <devices/inputevent.h>
#include <graphics/driver.h>
#include <graphics/gfxbase.h>
#include <hidd/gfx.h>
#include <oop/oop.h>
#include <utility/utility.h>
#include <aros/symbolsets.h>

#include "sagagfx_hidd.h"
#include "sagagfx_hw_v2.h"
#include "sagagfx_hw_v4.h"

#include LC_LIBDEFS_FILE

/*
 * The following two functions are candidates for inclusion into oop.library.
 * For slightly other implementation see incomplete Android-hosted graphics driver.
 */
static void FreeAttrBases(const STRPTR *iftable, OOP_AttrBase *bases, ULONG num)
{
    for (int i = 0; i < num; i++)
    {
        if (bases[i])
        {
            OOP_ReleaseAttrBase(iftable[i]);
        }
    }
}

static BOOL GetAttrBases(const STRPTR *iftable, OOP_AttrBase *bases, ULONG num)
{
    for (int i = 0; i < num; i++)
    {
        bases[i] = OOP_ObtainAttrBase(iftable[i]);
        
        if (!bases[i])
        {
            FreeAttrBases(iftable, bases, i);
            return(FALSE);
        }
    }
    
    return(TRUE);
}

/* These must stay in the same order as attrBases[] entries assignment in sagagfx_hidd.h */
static const STRPTR interfaces[ATTRBASES_NUM] =
{
    IID_Hidd_ChunkyBM,
    IID_Hidd_BitMap,
    IID_Hidd_Gfx,
    IID_Hidd_PixFmt,
    IID_Hidd_Sync,
    IID_Hidd,
    IID_Hidd_ColorMap
};

static int SAGAGfx_Init(LIBBASETYPEPTR LIBBASE)
{
    struct SAGAGfx_staticdata *xsd = &LIBBASE->vsd;
    struct GfxBase *GfxBase;
    struct IORequest io;
    
    D(bug("[SAGAGfx_Init] \n"));
    
    /* SHIFT key pressed during boot => Skip */
    
    if (0 == OpenDevice("input.device", 0, &io, 0))
    {
        struct Library *InputBase = (struct Library *)io.io_Device;
        UWORD qual = PeekQualifier();
        CloseDevice(&io);
        
        if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
        {
            D(bug("[SAGAGfx_Init] SHIFT KEY pressed => Skipped \n"));
            return(FALSE);
        }
    }
    
    /* Vampire board model */
    
    xsd->boardModel = (READ16(VREG_BOARD) >> 8);
    
	switch (xsd->boardModel)
    {
        case VREG_BOARD_V500:
        case VREG_BOARD_V600:
        case VREG_BOARD_V666:
        case VREG_BOARD_V1200:
        case VREG_BOARD_V4000:
        case VREG_BOARD_VCD32:
            // SAGA V2 METHODS
            xsd->SAGAGfx_GetPixFmt         = SAGAHW_V2_GetPixFmt;
            xsd->SAGAGfx_GetModeID         = SAGAHW_V2_GetModeID;
            xsd->SAGAGfx_SetColors         = SAGAHW_V2_SetColors;
            xsd->SAGAGfx_SetModulo         = SAGAHW_V2_SetModulo;
            xsd->SAGAGfx_SetMemory         = SAGAHW_V2_SetMemory;
            xsd->SAGAGfx_SetMode           = SAGAHW_V2_SetMode;
            xsd->SAGAGfx_SetModeline       = SAGAHW_V2_SetModeline;
            xsd->SAGAGfx_SetPLL            = SAGAHW_V2_SetPLL;
            xsd->SAGAGfx_SetSpriteHide     = SAGAHW_V2_SetSpriteHide;
            xsd->SAGAGfx_SetSpriteColors   = SAGAHW_V2_SetSpriteColors;
            xsd->SAGAGfx_SetSpriteMemory   = SAGAHW_V2_SetSpriteMemory;
            xsd->SAGAGfx_SetSpritePosition = SAGAHW_V2_SetSpritePosition;
            break;
            
        case VREG_BOARD_V4:
        case VREG_BOARD_V4SA:
            // SAGA V4 METHODS
            xsd->SAGAGfx_GetPixFmt         = SAGAHW_V4_GetPixFmt;
            xsd->SAGAGfx_GetModeID         = SAGAHW_V4_GetModeID;
            xsd->SAGAGfx_SetColors         = SAGAHW_V4_SetColors;
            xsd->SAGAGfx_SetModulo         = SAGAHW_V4_SetModulo;
            xsd->SAGAGfx_SetMemory         = SAGAHW_V4_SetMemory;
            xsd->SAGAGfx_SetMode           = SAGAHW_V4_SetMode;
            xsd->SAGAGfx_SetModeline       = SAGAHW_V4_SetModeline;
            xsd->SAGAGfx_SetPLL            = SAGAHW_V4_SetPLL;
            xsd->SAGAGfx_SetSpriteHide     = SAGAHW_V4_SetSpriteHide;
            xsd->SAGAGfx_SetSpriteColors   = SAGAHW_V4_SetSpriteColors;
            xsd->SAGAGfx_SetSpriteMemory   = SAGAHW_V4_SetSpriteMemory;
            xsd->SAGAGfx_SetSpritePosition = SAGAHW_V4_SetSpritePosition;
            break;
            
        default:
            D(bug("[SAGAGfx_Init] Failed to detect a Vampire board. \n"));
            return(FALSE);
            break;
	}
    
    /* Create a memory pool */
    
    xsd->mempool = CreatePool(MEMF_FAST | MEMF_CLEAR, 32768, 16384);
    
    if (xsd->mempool == NULL)
    {
        D(bug("[SAGAGfx_Init] Failed to create memory pool. \n"));
        return(FALSE);
    }
    
    xsd->visible = NULL;
    
    /* Initialize lock */
    
    // InitSemaphore(&xsd->framebufferlock);
    
    /* Obtain AttrBases */
    
    if (!GetAttrBases(interfaces, xsd->attrBases, ATTRBASES_NUM))
    {
        D(bug("[SAGAGfx_Init] Failed to get attributes. \n"));
        return(FALSE);
    }
    
    /*
        Open graphics.library ourselves because we will close it after adding the driver.
        Autoinit code would close it only upon driver expunge.
    */
    
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 41);
    
    if (!GfxBase)
    {
        D(bug("[SAGAGfx_Init] Failed to open graphics.library!\n"));
        return(FALSE);
    }
    
    LIBBASE->vsd.basebm = OOP_FindClass(CLID_Hidd_BitMap);
    LIBBASE->library.lib_OpenCnt = 1;
    
    CloseLibrary(&GfxBase->LibNode);
    
    return(TRUE);
}

ADD2LIBS((STRPTR)"gfx.hidd", 0, static struct Library *, __gfxbase);
ADD2INITLIB(SAGAGfx_Init, 0)

/* END OF FILE */
