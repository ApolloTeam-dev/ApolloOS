/*
    Copyright © 1995-2014, The AROS Development Team. All rights reserved.
    $Id$
*/

#define DEBUG 1
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

static struct Screen *OpenBootScreenType(struct DOSBootBase *DOSBootBase, BYTE SquarePixels)
{
    UWORD height;
    ULONG mode;
    // It turns out that in AROS, the minimum bitdepth that actually works right is 4.

    BYTE MinDepth = 4;

    GfxBase = (void *)TaggedOpenLibrary(TAGGEDOPEN_GRAPHICS);
    IntuitionBase = (void *)TaggedOpenLibrary(TAGGEDOPEN_INTUITION);

    if ((!IntuitionBase) || (!GfxBase))
	/* We failed to open one of system libraries. AROS is in utterly broken state */
	Alert(AT_DeadEnd|AN_BootStrap|AG_OpenLib);

// (Alynna): New boot screen init code
//  Attempt to get a 640x480 mode
    height = 480;
    mode = BestModeID(BIDTAG_DesiredWidth, 640, BIDTAG_DesiredHeight, height,
	BIDTAG_Depth, MinDepth, TAG_DONE);
// On failure, try to get a PAL or NTSC mode (In resolution size order)
    if (mode == INVALID_ID)
	if (GfxBase->DisplayFlags & PAL) {
    	    height = 256;
	    if (SquarePixels) { height = height << 1; } // promote mode for bootanim
	    mode = BestModeID(BIDTAG_DesiredWidth, 640, BIDTAG_DesiredHeight, height,
		BIDTAG_Depth, MinDepth, TAG_DONE);
	} else {
    	    height = 200;
	    if (SquarePixels) { height = height << 1; } // promote mode for bootanim
	    mode = BestModeID(BIDTAG_DesiredWidth, 640, BIDTAG_DesiredHeight, height,
		BIDTAG_Depth, MinDepth, TAG_DONE);
	}

/* (Alynna): Redundant
    height = 480;
    mode = BestModeID(BIDTAG_DesiredWidth, 640, BIDTAG_DesiredHeight, height,
	BIDTAG_Depth, MinDepth, TAG_DONE);
    if (mode == INVALID_ID)
	Alert(AN_SysScrnType);

    /* Set PAL or NTSC default height if we are running on Amiga(tm) hardware.
     * We also need to check if this is really PAL or NTSC mode because we have to
     * use PC 640x480 mode if user has Amiga hardware + RTG board.
     * Check DisplayFlags first because non-Amiga modeIDs use different format.
     *
    if (GfxBase->DisplayFlags & (NTSC | PAL)) {
    	if ((mode & MONITOR_ID_MASK) == NTSC_MONITOR_ID)
	    height = SquarePixels ? 400 : 200;
	else if ((mode & MONITOR_ID_MASK) == PAL_MONITOR_ID)
	    height = SquarePixels ? 512 : 256;
    }

    /* We want the screen to occupy the whole display, so we find best maching
       mode ID and then open a screen with that mode *
    mode = BestModeID(BIDTAG_DesiredWidth, 640, BIDTAG_DesiredHeight, height,
	BIDTAG_Depth, MinDepth, TAG_DONE);
*/
    D(bug("[bootscreen.c] Requesting %dx%dx%d, modeid %08lx ... ",640,height,MinDepth,mode));

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
    return OpenBootScreenType(DOSBootBase, FALSE);
}

struct Screen *NoBootMediaScreen(struct DOSBootBase *DOSBootBase)
{
    /* Boot anim requires 16+ color screen and 1:1 pixels */
    struct Screen *scr = OpenBootScreenType(DOSBootBase, TRUE);

    if (!anim_Init(scr, DOSBootBase))
    {
    	SetAPen(&scr->RastPort, 1);
    	Move(&scr->RastPort, 215, 120);
    	Text(&scr->RastPort, "No bootable media found...", 26);
    }

    return scr;
}

void CloseBootScreen(struct Screen *scr, struct DOSBootBase *DOSBootBase)
{
    CloseScreen(scr);

    CloseLibrary(&IntuitionBase->LibNode);
    CloseLibrary(&GfxBase->LibNode);
}
