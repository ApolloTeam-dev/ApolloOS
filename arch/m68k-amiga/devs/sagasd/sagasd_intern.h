/*
 * Copyright (C) 2016, Netronome Systems, Inc.
 * All rights reserved.
 *
 */

#ifndef SAGASD_INTERN_H
#define SAGASD_INTERN_H

#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/tasks.h>

#include "sdcmd.h"

#define SAGASD_UNITS    1       /* Only one chip select for now */
#define SDU_STACK_SIZE  (4096 / sizeof(ULONG))
#define SAGASD_VERSION 3
#define SAGASD_REVISION 1
#define SAGASD_VSTRING "sagasd.device v#VERSION##.#REVISION"

// How long to wait for IO before checking SD_Detect again
#define SD_DETECT_MSEC 1000000
#define IO_TIMINGLOOP_MSEC 10000


struct SAGASDBase {
    struct Device       sd_Device;
    struct Library *    sd_ExecBase;
    IPTR                sd_SegList;
    BOOL		sd_IsAROS;
    struct SAGASDUnit {
        struct Unit sdu_Unit;
        struct Task sdu_Task;
        TEXT        sdu_Name[6];                /* "SDIOx" */
        ULONG       sdu_Stack[1024];            /* 4K stack */
        BOOL        sdu_Enabled;

        struct sdcmd sdu_SDCmd;
        struct MsgPort *sdu_MsgPort;

        BOOL sdu_Present;               /* Is a device detected? */
        BOOL sdu_Valid;                 /* Is the device ready for IO? */
        BOOL sdu_ReadOnly;              /* Is the device read-only? */
        BOOL sdu_Motor;                 /* TD_MOTOR state */
        ULONG sdu_ChangeNum;

        struct Library *sdu_ExecBase;
    } sd_Unit[SAGASD_UNITS];
};

#define SD_DETECT_DELTA SD_DETECT_MSEC / IO_TIMINGLOOP_MSEC
#endif /* SAGASD_INTERN_H */
/* vim: set shiftwidth=4 expandtab:  */
