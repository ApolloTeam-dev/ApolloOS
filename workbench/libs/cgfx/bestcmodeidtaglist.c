/*
    Copyright © 1995-2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <aros/debug.h>
#include <cybergraphx/cybergraphics.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#include "cybergraphics_intern.h"

#define MODE_IS_SAGA     0xF0000000

static STRPTR ids[] = {
    NULL,
    "CVision64",
    "Piccolo",
    "PicassoII",
    "Spectrum",
    "Domino",
    "RetinaZ3",
    "PiccoSD64",
    "A2410",
    NULL,
    NULL,
    NULL,
    NULL,
    "CVision3D",
    "Inferno",
    "PicassoIV"
};

#define MAX_ID 15

static BOOL CModeCompatBoardNameMatches(const struct NameInfo *name, const char *boardName)
{
    ULONG i;

    if (boardName == NULL /* || *boardName == '\0'*/) return TRUE;
    for (i = 0; i < DISPLAYNAMELEN; i++) {
        UBYTE modeChar = name->Name[i];
        UBYTE boardChar = (UBYTE)boardName[i];
        
        if(boardChar == '\0')
          break;

        /* Board labels in old mode databases are not consistently cased. */
        if (modeChar >= 'a' && modeChar <= 'z') modeChar -= 'a' - 'A';
        if (boardChar >= 'a' && boardChar <= 'z') boardChar -= 'a' - 'A';
        if (modeChar != boardChar) return FALSE;
    }
    if(i == 0) return TRUE; // boardName was empty string
    return name->Name[i] == '\0' || name->Name[i] == ':' ||
           name->Name[i] == ' ' || name->Name[i] == '-' ||
           name->Name[i] == '/';
}

static ULONG CModeCompatDistance(ULONG actual, ULONG wanted)
{
    return actual >= wanted ? actual - wanted : wanted - actual;
}

/*****************************************************************************

    NAME */
#include <proto/cybergraphics.h>

	AROS_LH1(ULONG, BestCModeIDTagList,

/*  SYNOPSIS */
	AROS_LHA(struct TagItem *, tags, A0),

/*  LOCATION */
	struct Library *, CyberGfxBase, 10, Cybergraphics)

/*  FUNCTION
	Finds best RTG display mode ID which matches parameters specified
	by the taglist.

    INPUTS
	tags - An optional pointer to a TagList containing requirements
	       for the display mode. Valid tags are:

	  CYBRBIDTG_Depth (ULONG) - depth the returned ModeID must support.
				    Defaults to 8.
	  CYBRBIDTG_NominalWidth  (UWORD),
	  CYBRBIDTG_NominalHeight (UWORD) - desired width and height for the
                                            display mode.
          CYBRBIDTG_MonitorID (ULONG) - Specify numeric driver ID to find only
					modes belonging to this driver. Useful
					for systems with several graphics cards.
					Defined board IDs are:
					  1 - CVision64
					  2 - Piccolo
					  3 - PicassoII
					  4 - Spectrum
					  5 - Domino
					  6 - RetinaZ3
					  7 - PiccoSD64
                                          8 - A2410
					 13 - CVision3D (V41)
					 14 - Inferno   (V41)
					 15 - PicassoIV (V41)
					Note that this tag exists only for
					compatibility with old software. New
					programs should use CYBRIDTG_BoardName
					tag instead.
	CYBRBIDTG_BoardName (STRPTR) - Specify the driver name directly. For
				       example, pass "CVision3D" to get a
				       CyberVision64/3D display mode ID

    RESULT
        ID - Best matching display mode ID or INVALID_ID if there is no match.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	graphics.library/BestModeIDA()

    INTERNALS
	This functions relies on processing CYBRIDTG_BoardName tag by
	graphics.library/BestModeIDA().

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    struct GfxBase *GfxBase;
    struct TagItem *tag;
    struct DisplayInfo display;
    struct DimensionInfo dimensions;
    struct NameInfo name;
    DisplayInfoHandle handle;
    ULONG modeID = INVALID_ID;
    UWORD bestDepth = 0;
    ULONG bestID = INVALID_ID;
    ULONG bestDistance = INVALID_ID;
	
	/* Get values from TagList */
    ULONG wantedDepth = GetTagData(CYBRBIDTG_Depth, 8, tags);
    ULONG wantedWidth = GetTagData(CYBRBIDTG_NominalWidth, 0, tags);
    ULONG wantedHeight = GetTagData(CYBRBIDTG_NominalHeight, 0, tags);
    ULONG wantedMonitor = GetTagData(CYBRBIDTG_MonitorID, 0, tags);
    char *wantedBoard = (const char *)GetTagData(CYBRBIDTG_BoardName, NULL, tags);
    
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
    if(!GfxBase)
      return modeID;

	/* Do we have a Monitor ID? */
    if((wantedMonitor <= MAX_ID) && (wantedMonitor > 0))
        *wantedBoard = (const char *)ids[wantedMonitor];

    while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID) {
        ULONG width, height, distance;

		/* Use only CGX Modes */
        if((modeID & MODE_IS_SAGA) == 0)
          continue;

        handle = FindDisplayInfo(modeID);
        if (handle == NULL ||
            GetDisplayInfoData(handle, &display, sizeof(display), DTAG_DISP, modeID) != sizeof(display) ||
            display.NotAvailable != 0 ||
            GetDisplayInfoData(handle, &dimensions, sizeof(dimensions), DTAG_DIMS, modeID) != sizeof(dimensions) ||
            dimensions.MaxDepth < wantedDepth)
         {
            continue;
         }
        if (wantedBoard != NULL && *wantedBoard != '\0')
		{
            if (GetDisplayInfoData(handle, &name, sizeof(name), DTAG_NAME, modeID) != sizeof(name) ||
                !CModeCompatBoardNameMatches(&name, wantedBoard))
            {
                continue;
            }
        }
       
        /* Get width and height and calculate the distance */
        width = (ULONG)(dimensions.Nominal.MaxX - dimensions.Nominal.MinX + 1);
        height = (ULONG)(dimensions.Nominal.MaxY - dimensions.Nominal.MinY + 1);
        distance = CModeCompatDistance(width, wantedWidth) +
                   CModeCompatDistance(height, wantedHeight) +
                   CModeCompatDistance(dimensions.MaxDepth, wantedDepth);

		/* distance smaller than old distance? Then it is better */
		if (bestID == INVALID_ID || distance < bestDistance )
		{
		    bestID = modeID;
		    bestDistance = distance;
		}
    }
    
    CloseLibrary((struct Library *)GfxBase);
   
    return bestID;
	
    AROS_LIBFUNC_EXIT
} /* BestCModeIDTagList */
