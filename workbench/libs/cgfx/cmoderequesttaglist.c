/*
    Copyright © 1995-2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include "cybergraphics_intern.h"

/*****************************************************************************

    NAME */
#include <proto/exec.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#include <proto/asl.h>
#include <proto/utility.h>
#include <intuition/intuitionbase.h>
#include <utility/hooks.h>
#include <clib/alib_protos.h>

static ULONG CModeCompatPixelFormat(const struct DisplayInfo *display, const struct DimensionInfo *dimensions)
{
    if (dimensions->MaxDepth <= 8) return PIXFMT_LUT8;
    if (dimensions->MaxDepth == 15) return PIXFMT_RGB15;
    if (dimensions->MaxDepth == 16) return PIXFMT_RGB16;
    if (dimensions->MaxDepth == 24) return PIXFMT_RGB24;
    if (dimensions->MaxDepth == 32) return PIXFMT_ARGB32;

    /* Some drivers report 15/16 bpp through the RGB component widths. */
    if (display->RedBits == 5 && display->GreenBits == 5 && display->BlueBits == 5)
        return PIXFMT_RGB15;
    if (display->RedBits == 5 && display->GreenBits == 6 && display->BlueBits == 5)
        return PIXFMT_RGB16;
    return (ULONG)-1;
}

#define CMODECOMPAT_PIXFMT_END (UWORD)0xffff
static BOOL CModeCompatPixelFormatAllowed(const UWORD *models, ULONG format)
{
    if (models == NULL) return TRUE;
    while (*models != CMODECOMPAT_PIXFMT_END) {
        if ((ULONG)*models == format) return TRUE;
        ++models;
    }
    return FALSE;
}

static ULONG CModeCompatFilterHook(struct Hook *hook, APTR object, APTR message)
{
    struct CModeCompatFilter {
       const UWORD *colorModels;
    };
    struct CModeCompatFilter *filter = (struct CModeCompatFilter *)hook->h_Data;
    ULONG modeID = (ULONG)message;
	
    struct DisplayInfo display;
    struct DimensionInfo dimensions;
    DisplayInfoHandle handle = FindDisplayInfo(modeID);
    (void)object;
    if((modeID & 0xF0000000) == 0) return FALSE;
    if (GetDisplayInfoData(handle, &display, sizeof(display), DTAG_DISP, modeID) != sizeof(display)) return FALSE;
    if (!CModeCompatIsCyberModeID(modeID, &display)) return FALSE;
    if (GetDisplayInfoData(handle, &dimensions, sizeof(dimensions), DTAG_DIMS, modeID) != sizeof(dimensions)) return FALSE;
        
    return TRUE; //CModeCompatPixelFormatAllowed(filter->colorModels, CModeCompatPixelFormat(&display, &dimensions));      
}

	AROS_LH2(ULONG, CModeRequestTagList,

/*  SYNOPSIS */
	AROS_LHA(APTR            , , A0),
	AROS_LHA(struct TagItem *, , A1),

/*  LOCATION */
	struct Library *, CyberGfxBase, 11, Cybergraphics)

/*  FUNCTION
        Displays a requester that allows the user to select an RTG screenmode.
        Some of the requester's properties may be set using the following
        tags:
            CYBRMREQ_Screen (struct Screen *) - the screen on which the
                requester should be opened.
            CYBRMREQ_WinTitle (STRPTR) - window title.
            CYBRMREQ_OKText (STRPTR) - label text for OK button.
            CYBRMREQ_CancelText (STRPTR) - label text for Cancel button.
            CYBRMREQ_MinWidth (IPTR) - Minimum acceptable display width
                (defaults to 320).
            CYBRMREQ_MaxWidth (IPTR) - Maximum acceptable display width.
                (defaults to 1600).
            CYBRMREQ_MinHeight (IPTR) - Minimum acceptable display height.
                (defaults to 240).
            CYBRMREQ_MaxHeight (IPTR) - Maximum acceptable display height.
                (defaults to 1200).
            CYBRMREQ_MinDepth (IPTR) - Minimum acceptable display depth
                (defaults to 8).
            CYBRMREQ_MaxDepth (IPTR) - Maximum acceptable display depth
                (defaults to 32).
            CYBRMREQ_CModelArray (UWORD *) - array of permitted pixel formats.
                Any of the PIXFMT_#? constants may be specified (see
                LockBitMapTagList()), and the array must be terminated by ~0.
                By default, all pixel formats are acceptable.

    INPUTS
        requester - not used. Must be NULL.
        tagItems - options for the requester that will be created (may be
            NULL).

    RESULT
        result - user-selected screenmode ID, or zero on failure or
            user-cancellation.

    NOTES

    EXAMPLE

    BUGS
        This function is not implemented.

    SEE ALSO
        asl.library/AslRequest()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    struct Library *AslBase = NULL;
    struct ScreenModeRequester *screenModeRequester = NULL;
    struct TagItem *tag, *tstate = tagItems;
    struct TagItem aslTags[12];
   
    struct Hook hook;
    UWORD *colModels = NULL;
    struct CModeCompatFilter {
       const UWORD *colorModels;
    };
    struct CModeCompatFilter filter;
    struct Screen *screen = NULL;
    STRPTR title = (STRPTR)"Select Screenmode";
    STRPTR oktext = (STRPTR)"OK";
    STRPTR canceltext = (STRPTR)"Cancle";
    
    UWORD minWidth  = 320;
    UWORD maxWidth  = 1600;
    UWORD minHeight = 240;
    UWORD maxHeight = 1200;
    UWORD minDepth  = 8;
    UWORD maxDepth  = 32;
      
    ULONG modeid = 0;
    
    AslBase = OpenLibrary("asl.library", 0L);
    if(!AslBase)
      return 0;
    
    while ((tag = NextTagItem(&tstate)))
    {
    	switch (tag->ti_Tag)
    	{
    	    case CYBRMREQ_Screen:
    	        screen  = (struct Screen *)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_WinTitle:
    	        title  = (STRPTR)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_OKText:
    	        oktext  = (STRPTR)tag->ti_Data;
    	      break;    
    	    case CYBRMREQ_CancelText:
    	        canceltext  = (STRPTR)tag->ti_Data;
    	      break; 	
    	    //----------
    	    case CYBRMREQ_MinWidth:
    	        minWidth  = (UWORD)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_MaxWidth:
    	        maxWidth  = (UWORD)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_MinHeight:
    	        minHeight  = (UWORD)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_MaxHeight:
    	        maxHeight  = (UWORD)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_MinDepth:
    	        minDepth  = (UWORD)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_MaxDepth:
    	        maxDepth  = (UWORD)tag->ti_Data;
    	      break;
    	    case CYBRMREQ_CModelArray:
    	        colModels = (UWORD *)tag->ti_Data;
    	      break;
    	}
    }
    
    if(!screen)
    {
        struct IntuitionBase *IntuitionBase;
        if(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L))
        {
            screen = IntuitionBase->FirstScreen;
            CloseLibrary((struct Library *)IntuitionBase);
        }
    }
    if(!screen)
      return 0;

    filter.colorModels = (const UWORD *)colModels;
    hook.h_Entry =  (HOOKFUNC)HookEntry;
    hook.h_SubEntry = (HOOKFUNC)CModeCompatFilterHook;
    hook.h_Data = &filter;
    
    aslTags[0].ti_Tag  = ASLSM_Screen;
    aslTags[0].ti_Data = (IPTR)screen;
    aslTags[1].ti_Tag  = ASLSM_TitleText;
    aslTags[1].ti_Data = (IPTR)title;
    aslTags[2].ti_Tag  = ASLSM_PositiveText;
    aslTags[2].ti_Data = (IPTR)oktext;
    aslTags[3].ti_Tag  = ASLSM_NegativeText;
    aslTags[3].ti_Data = (IPTR)canceltext;
    
    aslTags[4].ti_Tag = ASLSM_MinWidth;
    aslTags[4].ti_Data = minWidth;
    aslTags[5].ti_Tag = ASLSM_MaxWidth;
    aslTags[5].ti_Data = maxWidth;
    aslTags[6].ti_Tag = ASLSM_MinHeight;
    aslTags[6].ti_Data = minHeight;
    aslTags[7].ti_Tag = ASLSM_MaxHeight;
    aslTags[7].ti_Data = maxHeight;
    aslTags[8].ti_Tag = ASLSM_MinDepth;
    aslTags[8].ti_Data = minDepth;
    aslTags[9].ti_Tag = ASLSM_MaxDepth;
    aslTags[9].ti_Data = maxDepth;   

  /* To any magician out there, who can repair the hook, I would be very pleased */
  //  aslTags[10].ti_Tag  = ASLSM_FilterFunc;
  //  aslTags[10].ti_Data = (ULONG)&hook;

    aslTags[10].ti_Tag  = TAG_DONE;
    aslTags[10].ti_Data = 0;
    
    screenModeRequester = (struct ScreenModeRequester *)AllocAslRequest(ASL_ScreenModeRequest, aslTags);
  
    if (screenModeRequester != NULL && AslRequest(screenModeRequester, NULL)  )
        modeid = (LONG)screenModeRequester->sm_DisplayID;
    else modeid = 0;
    
    if (screenModeRequester) FreeAslRequest(screenModeRequester);
    
    CloseLibrary(AslBase);
    
    return modeid;

    AROS_LIBFUNC_EXIT
} /* CModeRequestTagList */
