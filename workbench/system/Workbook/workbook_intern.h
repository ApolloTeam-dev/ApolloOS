/*
    Copyright © 2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Workbook headers
    Lang: english
*/

#ifndef WORKBOOK_H
#define WORKBOOK_H

#define WB_VERSION	1
#define WB_REVISION	0

#include <dos/bptr.h>
#include <intuition/classes.h>
#include <intuition/intuition.h>

struct WorkbookBase {
    APTR wb_IntuitionBase;
    APTR wb_DOSBase;
    APTR wb_UtilityBase;
    APTR wb_GadToolsBase;
    APTR wb_IconBase;
    APTR wb_WorkbenchBase;
    APTR wb_GfxBase;
    APTR wb_LayersBase;

    Class  *wb_WBApp;
    Class  *wb_WBWindow;
    Class  *wb_WBVirtual;
    Class  *wb_WBIcon;
    Class  *wb_WBSet;

    Object *wb_App;

    /* Create a new task that simply OpenWorkbenchObject()'s
     * it's argment.
     */
    BPTR wb_OpenerSegList;
};

/* FIXME: Remove these #define xxxBase hacks
   Do not use this in new code !
*/
#define DOSBase       wb->wb_DOSBase
#define WorkbenchBase wb->wb_WorkbenchBase

extern struct ExecBase *SysBase;

#include <string.h>
#include <proto/exec.h>

struct Region *wbClipWindow(struct WorkbookBase *wb, struct Window *win);
void wbUnclipWindow(struct WorkbookBase *wb, struct Window *win, struct Region *clip);

#endif /* WORKBOOK_H */
