#ifndef WANDERER_FILESYSTEMS_H
#define WANDERER_FILESYSTEMS_H

#include <string.h>
#include <time.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

/* FILEINFO CONSTANTS */

#define OPMODE_ASK          0
#define OPMODE_YES          1
#define OPMODE_ALL          2
#define OPMODE_NO           3
#define OPMODE_NONE         4

#define ACCESS_SKIP         OPMODE_YES
#define ACCESS_BREAK        OPMODE_NONE

#define FILEINFO_DIR        1
#define FILEINFO_PROTECTED  2
#define FILEINFO_WRITE      4

#define DROP_MODE_MOVE      0
#define DROP_MODE_COPY      1

#define ACTION_COPY         (1 << 4)
#define ACTION_DELETE       (1 << 2)
#define ACTION_DIRTOABS     (1 << 3)
#define ACTION_MAKEDIRS     (1 << 1)
#define ACTION_GETINFO      (1 << 5)
#define ACTION_MOVE         (1 << 6)
#define ACTION_UPDATE       (1 << 7)

#define PATHBUFFERSIZE      1024
#define COPYLEN              100

struct MUIDisplayObjects
{
    Object              *fileObject;
    Object              *stopObject;
    Object              *copyApp;
    Object              *win;
    Object              *numFilesGauge;
    Object              *gauge;
    ULONG               stopflag;
    ULONG               numObjects;
    ULONG               totalObjects;
    CONST_STRPTR        currentObject;
    UWORD               action;
    DOUBLE              bytes;

    UBYTE               Buffer[PATHBUFFERSIZE];
    UBYTE               SpeedBuffer[32];
    UBYTE               NumberBuffer[PATHBUFFERSIZE];
};

struct FileCopyData
{
    CONST_STRPTR    spath;
    CONST_STRPTR    dpath;
    CONST_STRPTR    file;
    APTR            userdata;
    ULONG           flags;
    ULONG           filelen;
    ULONG           actlen;
    ULONG           totallen;
    UBYTE           type;
    UWORD           difftime;
};

struct OpModes
{
    WORD        deletemode;
    WORD        protectmode;
    WORD        overwritemode;
};

BOOL CopyContent(CONST_STRPTR sourcePath, CONST_STRPTR targetDir, struct Hook* displayHook, struct Hook* askHook, struct OpModes *opModes, APTR userdata, BOOL root);
BOOL MoveContent(CONST_STRPTR sourcePath, CONST_STRPTR targetDir);
BPTR CreateDirectory(CONST_STRPTR path);
BOOL DeleteContent(CONST_STRPTR path, struct OpModes *opModes, struct Hook *askHook, struct Hook* displayHook, APTR userdata);
BOOL IsOnSameDevice(CONST_STRPTR path1, CONST_STRPTR path2);

#endif /* WANDERER_FILESYSTEMS_H */
