/* gadtoolsgadgets.c
** Simple example of using a number of gadtools gadgets.
**
** Here's a working example showing how to set up and use a linked list
** of GadTools gadgets complete with keyboard shortcuts.
**
** Compiled with SAS C v5.10a
** lc -b1 -cfistq -v -y gadtoolsgadgets
** blink FROM LIB:c.o gadtoolsgadgets.o TO gadtoolsgadgets LIB LIB:lc.lib LIB:amiga.lib
*/
#define INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <stdio.h>

#ifdef LATTICE
int CXBRK(void)    { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

/* Gadget defines of our choosing, to be used as GadgetID's,
** also used as the index into the gadget array my_gads[].
*/
#define MYGAD_SLIDER    (0)
#define MYGAD_STRING1   (1)
#define MYGAD_STRING2   (2)
#define MYGAD_STRING3   (3)
#define MYGAD_BUTTON    (4)

/* Range for the slider: */
#define SLIDER_MIN  (1)
#define SLIDER_MAX (20)

struct Library      *IntuitionBase;
struct GfxBase      *GfxBase;
struct Library      *GadToolsBase;

/* Print any error message.  We could do more fancy handling (like
** an EasyRequest()), but this is only a demo.
*/
void errorMessage(STRPTR error)
{
    if (error)
        printf("Error: %s\n", error);
}

/*
** Function to handle a GADGETUP or GADGETDOWN event.  For GadTools gadgets,
** it is possible to use this function to handle MOUSEMOVEs as well, with
** little or no work.
*/
VOID handleGadgetEvent(struct Window *win, struct Gadget *gad, UWORD code,
                       WORD *slider_level, struct Gadget *my_gads[])
{
    switch (gad->GadgetID)
    {
        case MYGAD_SLIDER:
            /* Sliders report their level in the IntuiMessage Code field: */
            printf("Slider at level %ld\n", code);
            *slider_level = code;
            break;
        case MYGAD_STRING1:
            /* String gadgets report GADGETUP's */
            printf("String gadget 1: '%s'.\n",
                   ((struct StringInfo *)gad->SpecialInfo)->Buffer);
            break;
        case MYGAD_STRING2:
            /* String gadgets report GADGETUP's */
            printf("String gadget 2: '%s'.\n",
                   ((struct StringInfo *)gad->SpecialInfo)->Buffer);
            break;
        case MYGAD_STRING3:
            /* String gadgets report GADGETUP's */
            printf("String gadget 3: '%s'.\n",
                   ((struct StringInfo *)gad->SpecialInfo)->Buffer);
            break;
        case MYGAD_BUTTON:
            /* Buttons report GADGETUP's (button resets slider to 10) */
            printf("Button was pressed, slider reset to 10.\n");
            *slider_level = 10;
            GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER], win, NULL,
                              GTSL_Level, *slider_level,
                              TAG_END);
            break;
    }
}


/*
** Function to handle vanilla keys.
*/
VOID handleVanillaKey(struct Window *win, UWORD code,
                      WORD *slider_level, struct Gadget *my_gads[])
{
    switch (code)
    {
        case 'v':
            /* increase slider level, but not past maximum */
            if (++*slider_level > SLIDER_MAX)
                *slider_level = SLIDER_MAX;
            GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER], win, NULL,
                              GTSL_Level, *slider_level,
                              TAG_END);
            break;
        case 'V':
            /* decrease slider level, but not past minimum */
            if (--*slider_level < SLIDER_MIN)
                *slider_level = SLIDER_MIN;
            GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER], win, NULL,
                              GTSL_Level, *slider_level,
                              TAG_END);
            break;
        case 'c':
        case 'C':
            /* button resets slider to 10 */
            *slider_level = 10;
            GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER], win, NULL,
                              GTSL_Level, *slider_level,
                              TAG_END);
            break;
        case 'f':
        case 'F':
            ActivateGadget(my_gads[MYGAD_STRING1], win, NULL);
            break;
        case 's':
        case 'S':
            ActivateGadget(my_gads[MYGAD_STRING2], win, NULL);
            break;
        case 't':
        case 'T':
            ActivateGadget(my_gads[MYGAD_STRING3], win, NULL);
            break;
    }
}


/*
** Here is where all the initialization and creation of GadTools gadgets
** take place.  This function requires a pointer to a NULL-initialized
** gadget list pointer.  It returns a pointer to the last created gadget,
** which can be checked for success/failure.
*/
struct Gadget *
createAllGadgets(struct Gadget **glistptr, void *vi, UWORD topborder, WORD slider_level, struct Gadget *my_gads[],
                 struct TextAttr *textAttr)
{
    struct NewGadget ng;
    struct Gadget *gad;

/* All the gadget creation calls accept a pointer to the previous gadget, and
** link the new gadget to that gadget's NextGadget field.  Also, they exit
** gracefully, returning NULL, if any previous gadget was NULL.  This limits
** the amount of checking for failure that is needed.  You only need to check
** before you tweak any gadget structure or use any of its fields, and finally
** once at the end, before you add the gadgets.
*/

/* The following operation is required of any program that uses GadTools.
** It gives the toolkit a place to stuff context data.
*/
    gad = CreateContext(glistptr);

/* Since the NewGadget structure is unmodified by any of the CreateGadget()
** calls, we need only change those fields which are different.
*/
    ng.ng_LeftEdge   = 140;
    ng.ng_TopEdge    = 20+topborder;
    ng.ng_Width      = 200;
    ng.ng_Height     = GfxBase->DefaultFont->tf_YSize+6;
    ng.ng_GadgetText = "_Volume:   ";
    ng.ng_TextAttr   = textAttr;
    ng.ng_VisualInfo = vi;
    ng.ng_GadgetID   = MYGAD_SLIDER;
    ng.ng_Flags      = NG_HIGHLABEL;

    my_gads[MYGAD_SLIDER] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
                                               GTSL_Min,         SLIDER_MIN,
                                               GTSL_Max,         SLIDER_MAX,
                                               GTSL_Level,       slider_level,
                                               GTSL_LevelFormat, "%2ld",
                                               GTSL_MaxLevelLen, 2,
                                               GT_Underscore,    '_',
                                               TAG_END);

    ng.ng_TopEdge   += 22;
    //ng.ng_Height     = GfxBase->DefaultFont->tf_YSize+6;
    ng.ng_GadgetText = "_First:";
    ng.ng_GadgetID   = MYGAD_STRING1;
    my_gads[MYGAD_STRING1] = gad = CreateGadget(STRING_KIND, gad, &ng,
                                                GTST_String,   "Try pressing",
                                                GTST_MaxChars, 50,
                                                GT_Underscore, '_',
                                                TAG_END);

    ng.ng_TopEdge   += 22;
    ng.ng_GadgetText = "_Second:";
    ng.ng_GadgetID   = MYGAD_STRING2;
    my_gads[MYGAD_STRING2] = gad = CreateGadget(STRING_KIND, gad, &ng,
                                                GTST_String,   "TAB or Shift-TAB",
                                                GTST_MaxChars, 50,
                                                GT_Underscore, '_',
                                                TAG_END);

    ng.ng_TopEdge   += 22;
    ng.ng_GadgetText = "_Third:";
    ng.ng_GadgetID   = MYGAD_STRING3;
    my_gads[MYGAD_STRING3] = gad = CreateGadget(STRING_KIND, gad, &ng,
                                                GTST_String,   "To see what happens!",
                                                GTST_MaxChars, 50,
                                                GT_Underscore, '_',
                                                TAG_END);

    ng.ng_LeftEdge  += 50;
    ng.ng_TopEdge   += 22;
    ng.ng_Width      = 100;
    ng.ng_Height     = GfxBase->DefaultFont->tf_YSize+6;
    ng.ng_GadgetText = "_Click Here";
    ng.ng_GadgetID   = MYGAD_BUTTON;
    ng.ng_Flags      = 0;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
                       GT_Underscore, '_',
                       TAG_END);
    return(gad);
}

/*
** Standard message handling loop with GadTools message handling functions
** used (GT_GetIMsg() and GT_ReplyIMsg()).
*/
VOID process_window_events(struct Window *mywin,
                           WORD *slider_level, struct Gadget *my_gads[])
{
    struct IntuiMessage *imsg;
    ULONG imsgClass;
    UWORD imsgCode;
    struct Gadget *gad;
    BOOL terminated = FALSE;

    while (!terminated)
    {
        Wait (1 << mywin->UserPort->mp_SigBit);

        /* GT_GetIMsg() returns an IntuiMessage with more friendly information for
        ** complex gadget classes.  Use it wherever you get IntuiMessages where
        ** using GadTools gadgets.
        */
        while ((!terminated) &&
               (imsg = GT_GetIMsg(mywin->UserPort)))
        {
            /* Presuming a gadget, of course, but no harm...
            ** Only dereference this value (gad) where the Class specifies
            ** that it is a gadget event.
            */
            gad = (struct Gadget *)imsg->IAddress;

            imsgClass = imsg->Class;
            imsgCode = imsg->Code;

            /* Use the toolkit message-replying function here... */
            GT_ReplyIMsg(imsg);

            switch (imsgClass)
            {
                /*  --- WARNING --- WARNING --- WARNING --- WARNING --- WARNING ---
                ** GadTools puts the gadget address into IAddress of IDCMP_MOUSEMOVE
                ** messages.  This is NOT true for standard Intuition messages,
                ** but is an added feature of GadTools.
                */
                case IDCMP_GADGETDOWN:
                case IDCMP_MOUSEMOVE:
                case IDCMP_GADGETUP:
                    handleGadgetEvent(mywin, gad, imsgCode, slider_level, my_gads);
                    break;
                case IDCMP_VANILLAKEY:
                    handleVanillaKey(mywin, imsgCode, slider_level, my_gads);
                    break;
                case IDCMP_CLOSEWINDOW:
                    terminated = TRUE;
                    break;
                case IDCMP_REFRESHWINDOW:
                    /* With GadTools, the application must use GT_BeginRefresh()
                    ** where it would normally have used BeginRefresh()
                    */
                    GT_BeginRefresh(mywin);
                    GT_EndRefresh(mywin, TRUE);
                    break;
            }
        }
    }
}

/*
** Prepare for using GadTools, set up gadgets and open window.
** Clean up and when done or on error.
*/
void gadtoolsWindow(struct TextAttr * textAttr)
{
    struct TextFont *font;
    struct Screen   *mysc;
    struct Window   *mywin;
    struct Gadget   *glist, *my_gads[4];
    void            *vi;
    WORD            slider_level = 5;
    UWORD           topborder;

/* Open topaz 8 font, so we can be sure it's openable
** when we later set ng_TextAttr to &Topaz80:
*/
    if (NULL == (font = OpenFont(textAttr)))
        errorMessage( "Failed to open Topaz 80");
    else
    {
        if (NULL == (mysc = LockPubScreen(NULL)))
            errorMessage( "Couldn't lock default public screen");
        else
        {
            if (NULL == (vi = GetVisualInfo(mysc, TAG_END)))
                errorMessage( "GetVisualInfo() failed");
            else
            {
                /* Here is how we can figure out ahead of time how tall the  */
                /* window's title bar will be:                               */
                topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);

                if (NULL == createAllGadgets(&glist, vi, topborder,
                                             slider_level, my_gads, textAttr))
                    errorMessage( "createAllGadgets() failed");
                else
                {
                    if (NULL == (mywin = OpenWindowTags(NULL,
                                                        WA_Title,     "GadTools Gadget Demo",
                                                        WA_Gadgets,   glist,      WA_AutoAdjust,    TRUE,
                                                        WA_Width,       400,      WA_MinWidth,        50,
                                                        WA_InnerHeight, 140,      WA_MinHeight,       50,
                                                        WA_DragBar,    TRUE,      WA_DepthGadget,   TRUE,
                                                        WA_Activate,   TRUE,      WA_CloseGadget,   TRUE,
                                                        WA_SizeGadget, TRUE,      WA_SimpleRefresh, TRUE,
                                                        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
                                                                  IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
                                                                  BUTTONIDCMP,
                                                        WA_PubScreen, mysc,
                                                        TAG_END)))
                        errorMessage( "OpenWindow() failed");
                    else
                    {
                        /* After window is open, gadgets must be refreshed with a
                        ** call to the GadTools refresh window function.
                        */
                        GT_RefreshWindow(mywin, NULL);

                        process_window_events(mywin, &slider_level, my_gads);

                        CloseWindow(mywin);
                    }
                }
                /* FreeGadgets() even if createAllGadgets() fails, as some
                ** of the gadgets may have been created...If glist is NULL
                ** then FreeGadgets() will do nothing.
                */
                FreeGadgets(glist);
                FreeVisualInfo(vi);
            }
            UnlockPubScreen(NULL, mysc);
        }
        CloseFont(font);
    }
}


/*
** Open all libraries and run.  Clean up when finished or on error..
*/
void main(void)
{
    if (NULL == (IntuitionBase = OpenLibrary("intuition.library", 37)))
        errorMessage( "Requires V37 intuition.library");
    else
    {
        if (NULL == (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37)))
            errorMessage( "Requires V37 graphics.library");
        else
        {

            struct TextAttr Topaz80 = { GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name, 8, 0, 0, };
            if (NULL == (GadToolsBase = OpenLibrary("gadtools.library", 37)))
                errorMessage( "Requires V37 gadtools.library");
            else
            {
                gadtoolsWindow(&Topaz80);

                CloseLibrary(GadToolsBase);
            }
            CloseLibrary((struct Library *)GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }
}