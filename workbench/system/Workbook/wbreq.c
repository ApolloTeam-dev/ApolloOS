//
// Created by jarokuczi on 28/04/2019.
//

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <exec/types.h>
#include <stdio.h>
#include "wbreq.h"

struct GfxBase      *gfxBase;

char *toReturnBuffer;

void errorMessage(STRPTR error)
{
    if (error)
        printf("Error: %s\n", error);
}

BOOL handleGadgetEvent(struct Window *win, struct Gadget *gad, UWORD code,
                       struct Gadget *my_gads[])
{
    switch (gad->GadgetID) {
        case GAD_TEXT:
        case GAD_PROCEED:
            toReturnBuffer =  ((struct StringInfo *) my_gads[GAD_TEXT]->SpecialInfo)->Buffer;
            return TRUE;
        case GAD_CANCEL:
            return TRUE;
    }

}

struct Gadget *createAllGadgets(struct Gadget **glistptr, void *vi, UWORD topborder, char *inputLabel[], char *buttonLabel[], struct Gadget *my_gads[],
                 struct TextAttr *textAttr)
{
    struct NewGadget ng;
    struct Gadget *gad;

    gad = CreateContext(glistptr);

    ng.ng_LeftEdge   = 70;
    ng.ng_TopEdge    = 20+topborder;
    ng.ng_Width      = 200;
    ng.ng_Height     = gfxBase->DefaultFont->tf_YSize+6;
    ng.ng_GadgetText = *inputLabel;
    ng.ng_TextAttr   = textAttr;
    ng.ng_VisualInfo = vi;
    ng.ng_GadgetID   = GAD_TEXT;
    ng.ng_Flags      = NG_HIGHLABEL;
    my_gads[GAD_TEXT] = gad = CreateGadget(STRING_KIND, gad, &ng,
                                                GTST_String,   "",
                                                GTST_MaxChars, 50,
                                                GT_Underscore, '_',
                                                TAG_END);

    ng.ng_TopEdge   += 22;
    ng.ng_Width      = 70;
    ng.ng_Height     = gfxBase->DefaultFont->tf_YSize+6;
    ng.ng_GadgetText = *buttonLabel;
    ng.ng_GadgetID   = GAD_PROCEED;
    ng.ng_Flags      = 0;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
                       GT_Underscore, '_',
                       TAG_END);

    ng.ng_LeftEdge  += 75;
    ng.ng_Width      = 70;
    ng.ng_Height     = gfxBase->DefaultFont->tf_YSize+6;
    ng.ng_GadgetText = "_Cancel";
    ng.ng_GadgetID   = GAD_CANCEL;
    ng.ng_Flags      = 0;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
                       GT_Underscore, '_',
                       TAG_END);
    return(gad);
}


VOID process_window_events(struct Window *mywin, struct Gadget *my_gads[])
{
    struct IntuiMessage *imsg;
    ULONG imsgClass;
    UWORD imsgCode;
    struct Gadget *gad;
    BOOL terminated = FALSE;

    while (!terminated)
    {
        Wait (1 << mywin->UserPort->mp_SigBit);

        while ((!terminated) &&
               (imsg = GT_GetIMsg(mywin->UserPort)))
        {

            gad = (struct Gadget *)imsg->IAddress;

            imsgClass = imsg->Class;
            imsgCode = imsg->Code;

            GT_ReplyIMsg(imsg);

            switch (imsgClass)
            {

                case IDCMP_GADGETDOWN:
                case IDCMP_MOUSEMOVE:
                case IDCMP_VANILLAKEY:
                case IDCMP_GADGETUP:
                    terminated = handleGadgetEvent(mywin, gad, imsgCode, my_gads);
                    break;
                case IDCMP_CLOSEWINDOW:
                    terminated = TRUE;
                    break;
                case IDCMP_REFRESHWINDOW:
                    GT_BeginRefresh(mywin);
                    GT_EndRefresh(mywin, TRUE);
                    break;
            }
        }
    }
}


void gadtoolsWindow(struct TextAttr * textAttr, char *windowTitle[] , char *inputLabel[], char *buttonTitle[]  )
{
    struct TextFont *font;
    struct Screen   *mysc;
    struct Window   *mywin;
    struct Gadget   *glist, *my_gads[3];
    void            *vi;
    UWORD           topborder;

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
                topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);

                if (NULL == createAllGadgets(&glist, vi, topborder,
                                             inputLabel, buttonTitle, my_gads, textAttr))
                    errorMessage( "createAllGadgets() failed");
                else
                {
                    if (NULL == (mywin = OpenWindowTags(NULL,
                                                        WA_Title, *windowTitle,
                                                        WA_Gadgets, glist, WA_AutoAdjust, TRUE,
                                                        WA_Width, 300, WA_MinWidth, 50,
                                                        WA_InnerHeight, 80, WA_MinHeight, 50,
                                                        WA_DragBar, TRUE, WA_DepthGadget, TRUE,
                                                        WA_Activate, TRUE, WA_CloseGadget, TRUE,
                                                        WA_SizeGadget, TRUE, WA_SimpleRefresh, TRUE,
                                                        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
                                                                  IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
                                                                  BUTTONIDCMP,
                                                        WA_PubScreen, mysc,
                                                        TAG_END)))
                        errorMessage( "OpenWindow() failed");
                    else
                    {
                       GT_RefreshWindow(mywin, NULL);

                        process_window_events(mywin, my_gads);

                        CloseWindow(mywin);
                    }
                }
                FreeGadgets(glist);
                FreeVisualInfo(vi);
            }
            UnlockPubScreen(NULL, mysc);
        }
        CloseFont(font);
    }
}

char *RequestText(char windowTitle[], char inputLabel[], char buttonTitle[], struct WorkbookBase *wbBase)
{
    gfxBase = wbBase->wb_GfxBase;
    struct TextAttr font = { gfxBase->DefaultFont->tf_Message.mn_Node.ln_Name, 8, 0, 0, };
    gadtoolsWindow(&font, &windowTitle, &inputLabel, &buttonTitle);
    return toReturnBuffer;
}