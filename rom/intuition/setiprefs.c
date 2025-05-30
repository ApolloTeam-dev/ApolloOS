/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    Copyright � 2001-2003, The MorphOS Development Team. All Rights Reserved.
    $Id$
*/

#include <proto/exec.h>
#include <intuition/iprefs.h>
#include <intuition/pointerclass.h>
#include <prefs/pointer.h>
#include <prefs/palette.h>

#include <proto/intuition.h>
#include <proto/graphics.h>

#include "intuition_intern.h"

/*****************************************************************************

    NAME */
        AROS_LH3(ULONG, SetIPrefs,

/*  SYNOPSIS */
        AROS_LHA(APTR , data, A0),
        AROS_LHA(ULONG, length, D0),
        AROS_LHA(ULONG, type, D1),

/*  LOCATION */
        struct IntuitionBase *, IntuitionBase, 96, Intuition)

/*  FUNCTION

    INPUTS

    RESULT
        Depending on the operation.

    NOTES
        This function is currently considered private.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct Library *UtilityBase = GetPrivIBase(IntuitionBase)->UtilityBase;
    struct GfxBase *GfxBase = GetPrivIBase(IntuitionBase)->GfxBase;
    ULONG Result = TRUE;
    ULONG lock = LockIBase(0);

    DEBUG_SETIPREFS(bug("[Intuition] %s: data %p length %lu type %lu\n", __func__, data, length, type));

    switch (type)
    {
        case IPREFS_TYPE_ICONTROL_V37:
            DEBUG_SETIPREFS(bug("[Intuition] %s: IP_ICONTROL_V37\n", __func__));
            if (length > sizeof(struct IIControlPrefs))
                length = sizeof(struct IIControlPrefs);
            CopyMem(data, &GetPrivIBase(IntuitionBase)->IControlPrefs, length);

            DEBUG_SETIPREFS(bug("[Intuition] %s: Drag modes: 0x%04lX\n", __func__, GetPrivIBase(IntuitionBase)->IControlPrefs.ic_VDragModes[0]));

            break;

        case IPREFS_TYPE_SCREENMODE_V37:
        {
            struct IScreenModePrefs *smp;
            smp = data;
            D(bug("[SetiPrefs] ModeID: 0x%08lX, Size: %dx%d, Depth: %d, Control: 0x%08lX\n",
                smp->smp_DisplayID, smp->smp_Width, smp->smp_Height, smp->smp_Depth, smp->smp_Control));
                
            BOOL reopen = FALSE, closed = (GetPrivIBase(IntuitionBase)->WorkBench) ? FALSE : TRUE;

            DEBUG_SETIPREFS(bug("[Intuition] %s: IP_SCREENMODE_V37\n", __func__));
            if (length > sizeof(struct IScreenModePrefs))
                length = sizeof(struct IScreenModePrefs);

            if (!GetPrivIBase(IntuitionBase)->ScreenModePrefs)
                GetPrivIBase(IntuitionBase)->ScreenModePrefs = AllocMem(sizeof(struct IScreenModePrefs), MEMF_ANY);

            if (memcmp(GetPrivIBase(IntuitionBase)->ScreenModePrefs, data, sizeof(struct IScreenModePrefs)) == 0)
            {
                DEBUG_SETIPREFS(bug("[Intuition] %s: Copy Prefs Failed!\n", __func__));
                break;
            }

            if (!closed)
            {
                DEBUG_SETIPREFS(bug("[Intuition] %s: WB Not Closed . . .\n", __func__));
                
                BOOL try = TRUE;
                reopen = TRUE;

                UnlockIBase(lock);

                while (try && !(closed = CloseWorkBench()))
                {
                    DEBUG_SETIPREFS(bug("[Intuition] %s: Trying to Close WB\n", __func__));
                    
                    struct EasyStruct es =
                    {
                        sizeof(struct EasyStruct),
                        0,
                        "System Request",
                        "Intuition is attempting to reset the screen,\n"
                        "please close all windows except Wanderer's ones.",
                        "Retry|Cancel"
                    };

                    try = EasyRequestArgs(NULL, &es, NULL, NULL) == 1;
                }
            }

            if (closed)
            {
                DEBUG_SETIPREFS(bug("[Intuition] %s: WB is Closed !\n", __func__));
                CopyMem(data, GetPrivIBase(IntuitionBase)->ScreenModePrefs, sizeof(struct IScreenModePrefs));
            }
                
            if (reopen)
            {
                if (closed)
                {
                    if (!OpenWorkBench())
                    {
                        /* FIXME: handle the error condition if OpenWorkBench() fails */
                        /* What to do if OpenWorkBench() fails? Try until it succeeds?
                           Try for a finite amount of times? Don't try and do nothing 
                           at all? */
                    }
                }
                else
                    Result = FALSE;
                return Result;
            }

            break;
        }

        case IPREFS_TYPE_POINTER_V39:
        DEBUG_SETIPREFS(bug("[Intuition] %s: IP_POINTER_V39\n", __func__));
        {
            struct IPointerPrefs *fp = data;
            struct TagItem pointertags[] = {
                {POINTERA_BitMap , (IPTR)fp->BitMap},
                {POINTERA_XOffset, fp->XOffset     },
                {POINTERA_YOffset, fp->YOffset     },
                {TAG_DONE        , 0               }
            };

            Object *pointer = NewObjectA(
                          GetPrivIBase(IntuitionBase)->pointerclass,
                          NULL,
                          pointertags);

            Object **oldptr = fp->Which ?
                      &GetPrivIBase(IntuitionBase)->BusyPointer :
                      &GetPrivIBase(IntuitionBase)->DefaultPointer;

            InstallPointer(IntuitionBase, fp->Which, oldptr, pointer);
            /* return -1 so that WB3.x C:IPrefs is happy */
            Result = -1;
        }
        break;

        case IPREFS_TYPE_POINTER_V37:
        DEBUG_SETIPREFS(bug("[Intuition] %s: IP_POINTER_V37\n", __func__));
        {
            struct Preferences *ActivePrefs = &GetPrivIBase(IntuitionBase)->ActivePreferences;
            struct IPointerPrefsV37 *fp = data;
            UWORD size = fp->YSize * 2;
            Object *pointer;

            if (size > POINTERSIZE)
                size = POINTERSIZE;
            SetMem(ActivePrefs->PointerMatrix, 0, POINTERSIZE * sizeof (UWORD));
            CopyMem(fp->data, ActivePrefs->PointerMatrix, size * sizeof (UWORD));
            ActivePrefs->XOffset = fp->XOffset;
            ActivePrefs->YOffset = fp->YOffset;

            pointer = MakePointerFromPrefs(IntuitionBase, ActivePrefs);
            if (pointer)
                 InstallPointer(IntuitionBase, WBP_NORMAL, &GetPrivIBase(IntuitionBase)->DefaultPointer, pointer);
            /* return -1 so that WB2.x C:IPrefs is happy */
            Result = -1;
        }
        break;

        case IPREFS_TYPE_PALETTE_V39:
        case IPREFS_TYPE_PALETTE_V37:
        DEBUG_SETIPREFS(bug("[Intuition] %s: IP_PALETTE_V%d %p %d\n", __func__, type == IPREFS_TYPE_PALETTE_V39 ? 39 : 37, data, length));
        {
            BOOL closed = (GetPrivIBase(IntuitionBase)->WorkBench) ? FALSE : TRUE;
            struct Preferences *ActivePrefs = &GetPrivIBase(IntuitionBase)->ActivePreferences;
            struct Color32 *p = GetPrivIBase(IntuitionBase)->Colors;
            struct ColorSpec *pp = data;
            BOOL update_pointer = FALSE;

            DEBUG_SETIPREFS(bug("[Intuition] %s: Intuition Color32 Table 0x%p\n", __func__, p));

            while (pp->ColorIndex != -1)
            {
                WORD idx;
                    
                idx = pp->ColorIndex;
                DEBUG_SETIPREFS(bug("[Intuition] %s: Index %ld R 0x%04lX G 0x%04lX B 0x%04lX\n",
                                    __func__, idx, pp->Red, pp->Green, pp->Blue));
                if (type == IPREFS_TYPE_PALETTE_V37) {
                    /* v37 cursor colors are 17 to 19 */
                    if (idx >= 17)
                        idx = idx - 17 + 8;
                    else if (idx >= 8)
                        idx = -1;
                }
                if (idx >= 0 && idx < COLORTABLEENTRIES)
                {
                    UWORD red, green, blue;
                    if (type == IPREFS_TYPE_PALETTE_V37) {
                        /* 4-bit color components */
                        red = (pp->Red << 4) | pp->Red;
                        green = (pp->Green << 4) | pp->Green;
                        blue = (pp->Blue << 4) | pp->Blue;
                        red = (red << 8) | red;
                        green = (green << 8) | green;
                        blue = (blue << 8) | blue;
                    } else {
                        /* 8-bit color components */
                        red = pp->Red;
                        green = pp->Green;
                        blue = pp->Blue;
                    }
                   
                    p[idx].red   = (red << 16) | red;
                    p[idx].green = (green << 16) | green;
                    p[idx].blue  = (blue << 16) | blue;

                    /* Update oldstyle preferences */
                    if (ActivePrefs)
                    {
                        UWORD *cols = NULL;
                        UWORD baseindex;
                        
                        if (idx < 4) {
                            baseindex = 0;
                            cols = &ActivePrefs->color0;
                        } else if (idx >= 8 && idx <= 10) {
                            baseindex = 8;
                            cols=&ActivePrefs->color17;
                            update_pointer = TRUE;
                        }
                        
                        if (cols)
                            cols[idx - baseindex] = ((red >> 4) & 0xf00) | ((green >> 8) & 0x0f0) | ((blue >> 12));
                    }
                    DEBUG_SETIPREFS(bug("[Intuition] %s: Set Color32 %ld R 0x%08lx G 0x%08lx B 0x%08lx\n",
                                __func__,
                                (LONG) idx,
                                p[idx].red,
                                p[idx].green,
                                p[idx].blue));

                    /* update the wokrbench displays palette ... */
                    if (!closed)
                    {
                        UBYTE wbdepth = GetPrivIBase(IntuitionBase)->WorkBench->RastPort.BitMap->Depth;

                        if (idx < 4)
                            SetRGB32(&GetPrivIBase(IntuitionBase)->WorkBench->ViewPort, idx, p[idx].red, p[idx].green, p[idx].blue);
                        else if ((idx < 8) && (wbdepth >= 3))
                        {
                            ULONG lastcol = ((wbdepth > 8) ? 256 : (1 << wbdepth)) - 4;

                            SetRGB32(&GetPrivIBase(IntuitionBase)->WorkBench->ViewPort, lastcol + idx, p[idx].red, p[idx].green, p[idx].blue);
                        }
                    }
                }
                pp++;
            }
            
            if (update_pointer) {
                DEBUG_SETIPREFS(bug("[SetIPrefs] Updating pointer colors\n", __func__));
                SetPointerColors(IntuitionBase);
            }
        }
        break;

        case IPREFS_TYPE_PENS_V39:
        DEBUG_SETIPREFS(bug("[Intuition] %s: IP_PENS_V39\n", __func__));
        {
            BOOL closed = (GetPrivIBase(IntuitionBase)->WorkBench) ? FALSE : TRUE;
            struct IOldPenPrefs *fp = data;
            UWORD *defpenptr, *scrppenptr, *scrpenptr = NULL;
            struct IntScreen      *wbsint;
            UBYTE wbdepth;
            int i;

            DEBUG_SETIPREFS(bug("[Intuition] %s: Count %ld Type %ld\n",
                        __func__,
                        (LONG) fp->Count,
                        (LONG) fp->Type));

            if (fp->Type==0)
            {
                defpenptr = &GetPrivIBase(IntuitionBase)->DriPens4[0];
                DEBUG_SETIPREFS(bug("[Intuition] %s: Pens4[]\n", __func__));
                if (!closed)
                {
                    struct IntDrawInfo      *dri;

                    wbsint = GetPrivScreen(GetPrivIBase(IntuitionBase)->WorkBench);
                    wbdepth = GetPrivIBase(IntuitionBase)->WorkBench->RastPort.BitMap->Depth;
                    
                    if ((wbdepth < 3) && (dri = (struct IntDrawInfo *)GetScreenDrawInfo(GetPrivIBase(IntuitionBase)->WorkBench)))
                    {
                        DEBUG_SETIPREFS(bug("[Intuition] %s: updating wbscreen dri pens\n", __func__));
                        scrpenptr = dri->dri_Pens;
                        scrppenptr = wbsint->Pens;
                    }
                }
            }
            else
            {
                defpenptr = &GetPrivIBase(IntuitionBase)->DriPens8[0];
                DEBUG_SETIPREFS(bug("[Intuition] %s: Pens8[]\n", __func__));
                if (!closed)
                {
                    struct IntDrawInfo      *dri;

                    wbsint = GetPrivScreen(GetPrivIBase(IntuitionBase)->WorkBench);
                    wbdepth = GetPrivIBase(IntuitionBase)->WorkBench->RastPort.BitMap->Depth;

                    if ((wbdepth >= 3) && (dri = (struct IntDrawInfo *)GetScreenDrawInfo(GetPrivIBase(IntuitionBase)->WorkBench)))
                    {
                        DEBUG_SETIPREFS(bug("[Intuition] %s: updating wbscreen dri pens\n", __func__));
                        scrpenptr = dri->dri_Pens;
                        scrppenptr = wbsint->Pens;
                    }
                }
            }
            for (i=0;i<NUMDRIPENS;i++)
            {
                if (fp->PenTable[i]==(UWORD)~0UL)
                {
                    /*
                     * end of the array
                     */
                    DEBUG_SETIPREFS(bug("[Intuition] %s: PenTable end at entry %ld\n", __func__, (LONG) i));
                    break;
                }
                else
                {
                    DEBUG_SETIPREFS(bug("[Intuition] %s: Pens[%ld] %ld\n",
                                __func__,
                                (LONG) i,
                                (LONG) fp->PenTable[i]));

                    defpenptr[i] = fp->PenTable[i];

                    if (scrpenptr)
                    {
                        UWORD penval = fp->PenTable[i];
#if (0)
                        if ((penval > 3) && (wbdepth >= 3))
                        {
                            ULONG lastpen = ((wbdepth > 8) ? 252 : (1 << wbdepth)) - 8;
                            penval += lastpen;
                        }
#endif
                        scrpenptr[i] = penval;
                        scrppenptr[i] = penval;
                    }
                }
            }
        }
        break;


        case IPREFS_TYPE_POINTER_ALPHA:
            DEBUG_SETIPREFS(bug("[Intuition] %s: IP_POINTER_ALPHA\n", __func__));
            GetPrivIBase(IntuitionBase)->PointerAlpha = *(UWORD *)data;
        break;

        case IPREFS_TYPE_OVERSCAN_V37:
            DEBUG_SETIPREFS(bug("[Intuition] %s: IP_OVERSCAN_V37\n", __func__));
        break;

        case IPREFS_TYPE_FONT_V37:
        DEBUG_SETIPREFS(bug("[Intuition] %s: IP_FONT_V37\n", __func__));
        {
            struct IFontPrefs *fp = data;
            struct TextFont *font = OpenFont(&fp->fp_TextAttr);
            struct TextFont **fontptr;

            DEBUG_SETIPREFS(bug("[Intuition] %s: Type %d Name <%s> Size %d Font %p\n", __func__, fp->fp_ScrFont, fp->fp_Name, fp->fp_TextAttr.ta_YSize, font));

            if (font)
            {
                if (fp->fp_ScrFont==0)
                {
                    /*
                     * We can't free graphics defaultfont..it`s shared
                     */
                    fontptr = &GfxBase->DefaultFont;
                }
                else
                {
                    fontptr = &GetPrivIBase(IntuitionBase)->ScreenFont;
                    CloseFont(*fontptr);
                }
                *fontptr = font;
            }
        }
        break;

        default:
            DEBUG_SETIPREFS(bug("[Intuition] %s: Unknown Prefs Type\n", __func__));
            Result = FALSE;
            break;
    }

    UnlockIBase(lock);

    DEBUG_SETIPREFS(bug("[Intuition] %s: Result 0x%lx\n", __func__, Result));
    
    return(Result);
    
    AROS_LIBFUNC_EXIT
} /* private1 */
