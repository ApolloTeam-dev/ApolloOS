/*
    Copyright  2003-2020, The AROS Development Team. All rights reserved.
    $Id$
*/

// #define MUIMASTER_YES_INLINE_STDARG

#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include <libraries/mui.h>
#include <prefs/input.h>

#include <zune/systemprefswindow.h>

#include "locale.h"
#include "args.h"
#include "ipeditor.h"
#include "prefs.h"

#include <aros/debug.h>

#define VERSION "$VER: Input 1.2 ("ADATE") The AROS Dev Team"
#define EXCLUDEPATTERN "~(#?.info|#?.dbg)"

/*********************************************************************************************/

int main(int argc, char **argv)
{
    Object *application,  *window;

    bug("[INPUT] Start\n");

    Locale_Initialize();

    if (ReadArguments(argc, argv))
    {
        if (ARG(USE) || ARG(SAVE))
        {
            Prefs_HandleArgs((STRPTR)ARG(FROM), ARG(USE), ARG(SAVE));
        }
        else
        {
            Prefs_Default();

            NewList(&keymap_list);

            mempool = CreatePool(MEMF_PUBLIC | MEMF_CLEAR, 2048, 2048);
            if (mempool != 0)
            {
                struct Screen *pScreen = NULL;

                if (ARG(PUBSCREEN))
                    pScreen = LockPubScreen((CONST_STRPTR)ARG(PUBSCREEN));

                Prefs_ScanDirectory("DEVS:Keymaps/" EXCLUDEPATTERN, &keymap_list, sizeof(struct KeymapEntry));

                application = (Object *)ApplicationObject,
                    MUIA_Application_Author, (IPTR)"Jason McMullan <jason.mcmullan@gmail.com>",
                    MUIA_Application_Copyright, (IPTR)"2012, AROS Team",
                    MUIA_Application_Title, __(MSG_NAME),
                    MUIA_Application_Version, (IPTR) VERSION,
                    MUIA_Application_Description,  __(MSG_NAME),
                    MUIA_Application_SingleTask, TRUE,
                    MUIA_Application_Base, (IPTR) "INPUTPREF",
                    SubWindow, (IPTR) (window = SystemPrefsWindowObject,
                            MUIA_Window_Screen, (IPTR)pScreen,
                            MUIA_Window_ID, MAKE_ID('I','W','I','N'),
                        WindowContents, (IPTR) IPEditorObject,
                        TAG_DONE),
                        End),
                End;

                if (application != NULL)
                {
                    SET(window, MUIA_Window_Open, TRUE);
                    DoMethod(application, MUIM_Application_Execute);
                    SET(window, MUIA_Window_Open, FALSE);

                    MUI_DisposeObject(application);
                } else {
                    bug("[INPUT] Failed to open MUI application\n");
                }

                if (pScreen)
                    UnlockPubScreen(NULL, pScreen);

                DeletePool((APTR)mempool);
            }
        }
        FreeArguments();
    }

    Locale_Deinitialize();

    return 0;
}
