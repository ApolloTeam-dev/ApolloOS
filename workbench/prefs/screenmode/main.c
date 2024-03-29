/*
    Copyright � 2003-2020, The AROS Development Team. All rights reserved.
    $Id$
*/

#define MUIMASTER_YES_INLINE_STDARG
#define DEBUG 0

#include <aros/debug.h>
#include <dos/dos.h>

#include <proto/alib.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>

#include <zune/systemprefswindow.h>

#include "args.h"
#include "locale.h"
#include "prefs.h"
#include "smeditor.h"

#define VERSION "ScreenMode Preferences 1.12 (10.08.2021)"
#define COPYRIGHT "Copyright � 1995-2020, The AROS Development Team"

static const char vers[] = VERSION;
static const char version[] __attribute__((used)) = "$VER: " VERSION "\n";


int main(int argc, char **argv)
{
    Object *app, *win;

    Locale_Initialize();

    if (ReadArguments(argc, argv))
    {
        if (ARG(USE) || ARG(SAVE))
        {
            Prefs_HandleArgs((STRPTR)ARG(FROM), ARG(USE), ARG(SAVE));
        }
        else
        {
            struct Screen *pScreen = NULL;

            if (ARG(PUBSCREEN))
                pScreen = LockPubScreen((CONST_STRPTR)ARG(PUBSCREEN));

            app = (Object *) ApplicationObject,
                MUIA_Application_Title, (IPTR) __(MSG_NAME),
                MUIA_Application_Version, (IPTR) vers,
                MUIA_Application_Copyright, (IPTR) COPYRIGHT,
                MUIA_Application_Author, (IPTR) "The AROS Development Team",
                MUIA_Application_Description, (IPTR) __(MSG_NAME),
                MUIA_Application_SingleTask, TRUE,
                MUIA_Application_Base, (IPTR) "SCREENMODEPREF",
                SubWindow, (IPTR)(win = (Object *) SystemPrefsWindowObject,
                  MUIA_Window_Screen, (IPTR)pScreen,
                  MUIA_Window_ID, MAKE_ID('S','W','I','N'),
                    WindowContents, (IPTR) SMEditorObject,
                    End,
                End),
            End;

            if (app)
            {
                set(win, MUIA_Window_Open, TRUE);
                
                DoMethod(app, MUIM_Application_Execute);
            
                MUI_DisposeObject(app);
            }
            if (pScreen)
                UnlockPubScreen(NULL, pScreen);
        }
        FreeArguments();
    }

    Locale_Deinitialize();

    return RETURN_OK;
}
