#ifndef SAGASD_INTERN_H
#define SAGASD_INTERN_H

#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/tasks.h>

#include "sdcmd.h"

#define SAGASD_UNITS        2
#define IO_TIMINGLOOP_MSEC  100000

#define SDU_STACK_SIZE      (4096 / sizeof(ULONG))

struct SAGASDBase
{
    struct Device       sd_Device;
    struct Library *    sd_ExecBase;
    IPTR                sd_SegList;
    struct SAGASDUnit
    {
        struct Unit sdu_Unit;
        struct Task sdu_Task;
        TEXT        sdu_Name[6];        /* "SDIOx" */
        ULONG       sdu_Stack[1024];    /* 4K stack */
        BOOL        sdu_Enabled;

        struct sdcmd sdu_SDCmd;
        struct MsgPort *sdu_MsgPort;

        BOOL sdu_Present;               /* Is a device detected? */
        BOOL sdu_Valid;                 /* Is the device ready for IO? */
        BOOL sdu_ReadOnly;              /* Is the device read-only? */
        BOOL sdu_Motor;                 /* TD_MOTOR state */
        ULONG sdu_ChangeNum;

        APTR sdu_AddChangeList[10];    /* Storage for TD_ADDCHANGEINT Pointers from Filesystems */
        int sdu_AddChangeListItems;
        
        struct Library *sdu_ExecBase;
    } sd_Unit[SAGASD_UNITS];
};

#endif 
