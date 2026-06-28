#ifndef SAGASD_INTERN_H
#define SAGASD_INTERN_H

#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/tasks.h>

#include "sdcmd.h"

#define SAGASD_UNITS        2
#define IO_TIMINGLOOP_USEC  100000  /* timer tick in microseconds (100 ms) for the SD detect poll */

#define SDU_STACK_SIZE      (16384 / sizeof(ULONG))

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
        ULONG       sdu_Stack[SDU_STACK_SIZE];    /* 16K stack (see SDU_STACK_SIZE) */
        BOOL        sdu_Enabled;

        struct sdcmd sdu_SDCmd;
        struct MsgPort *sdu_MsgPort;

        BOOL sdu_Present;               /* Is a device detected? */
        BOOL sdu_Valid;                 /* Is the device ready for IO? */
        BOOL sdu_ReadOnly;              /* Is the device read-only? */
        BOOL sdu_Motor;                 /* TD_MOTOR state */
        ULONG sdu_ChangeNum;

        APTR sdu_AddChangeList[10];    /* TD_ADDCHANGEINT interrupt pointers; 0-based, NULL = reusable hole */
        int sdu_AddChangeListItems;    /* High-water count of used slots */
        BOOL sdu_MultiFS;              /* TRUE once a non-FAT handler is registered (was AddChangeList[0]) */
        
        struct Library *sdu_ExecBase;
    } sd_Unit[SAGASD_UNITS];
};

#endif 
