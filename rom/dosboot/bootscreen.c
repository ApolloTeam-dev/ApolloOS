/*
    Copyright � 1995-2014, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <aros/debug.h>
#include <exec/alerts.h>
#include <exec/libraries.h>
#include <graphics/gfxbase.h>
#include <graphics/modeid.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include "dosboot_intern.h"

#define VREG_SAGACTRL1  0xdff3ec /* AGA, Scanlines, Zoom modes */
#define VREG_SAGACTRL2  0xdff3ee /* AGA, Zoom shift            */

static struct Screen *OpenBootScreenType(struct DOSBootBase *DOSBootBase, BYTE MinDepth, BYTE SquarePixels)
{
    ULONG mode;

    GfxBase = (void *)TaggedOpenLibrary(TAGGEDOPEN_GRAPHICS);
    IntuitionBase = (void *)TaggedOpenLibrary(TAGGEDOPEN_INTUITION);

    if ((!IntuitionBase) || (!GfxBase))	Alert(AT_DeadEnd|AN_BootStrap|AG_OpenLib);

    mode = BestModeID(BIDTAG_DesiredWidth, 640, BIDTAG_DesiredHeight, 256, BIDTAG_Depth, MinDepth, TAG_DONE);

    *((volatile UWORD*)VREG_SAGACTRL1) = 0x0024;
    *((volatile UWORD*)VREG_SAGACTRL1) = 0x8024;        // SAGA ZoomMode 320x256 
    *((volatile UWORD*)VREG_SAGACTRL2) = 0x1010;        // SAGA Vertical/Horizontal Shift

    if (mode != INVALID_ID)
    {
	    struct Screen *scr = OpenScreenTags(NULL, SA_DisplayID, mode, SA_Draggable, FALSE, SA_Quiet, TRUE, SA_Depth, MinDepth, TAG_DONE);

	    if (scr) return scr;
    } 
    
    Alert(AN_SysScrnType);
    return NULL;
}

static struct Screen *OpenBootScreenType2(struct DOSBootBase *DOSBootBase, BYTE MinDepth, BYTE SquarePixels)
{
    UWORD height;
    ULONG mode;

    GfxBase = (void *)TaggedOpenLibrary(TAGGEDOPEN_GRAPHICS);
    IntuitionBase = (void *)TaggedOpenLibrary(TAGGEDOPEN_INTUITION);

    if ((!IntuitionBase) || (!GfxBase))
	/* We failed to open one of system libraries. AROS is in utterly broken state */
	Alert(AT_DeadEnd|AN_BootStrap|AG_OpenLib);

    height = 240;
    mode = BestModeID(BIDTAG_DesiredWidth, 320, BIDTAG_DesiredHeight, height,
	BIDTAG_Depth, MinDepth, TAG_DONE);
    if (mode == INVALID_ID)
	Alert(AN_SysScrnType);

    mode = BestModeID(BIDTAG_DesiredWidth, 320, BIDTAG_DesiredHeight, height,
	BIDTAG_Depth, MinDepth, TAG_DONE);

    if (mode != INVALID_ID)
    {
	struct Screen *scr = OpenScreenTags(NULL, SA_DisplayID, mode, SA_Draggable, FALSE, 
					    SA_Quiet, TRUE, SA_Depth, MinDepth, TAG_DONE);

	if (scr)
	    return scr;
    }
    /* We can't open a screen. Likely there are no display modes in the database at all */
    Alert(AN_SysScrnType);
    return NULL;
}


struct Screen *OpenBootScreen(struct DOSBootBase *DOSBootBase)
{   
    /* Boot menu requires basic 4+ color screen */
    return OpenBootScreenType(DOSBootBase, 2, FALSE);
}

/*
struct Screen *NoBootMediaScreen(struct DOSBootBase *DOSBootBase)
{
    // Boot anim requires 16+ color screen and 1:1 pixels
    struct Screen *scr = OpenBootScreenType2(DOSBootBase, 4, TRUE);

    if (!anim_Init(scr, DOSBootBase))
    {
    	SetAPen(&scr->RastPort, 1);
    	Move(&scr->RastPort, 215, 120);
    	Text(&scr->RastPort, "No bootable media found...", 26);
    }

    return scr;
}*/

void CloseBootScreen(struct Screen *scr, struct DOSBootBase *DOSBootBase)
{
    CloseScreen(scr);

    CloseLibrary(&IntuitionBase->LibNode);
    CloseLibrary(&GfxBase->LibNode);
}
