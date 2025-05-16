/*
    Copyright � 1995-2014, The AROS Development Team. All rights reserved.
    $Id$
*/

/*
 * This hook is called after dos.library wakes up.
 * Its job is to load all queued filesystems and add to the system.
 */

#include <aros/asmcall.h>
#include <aros/debug.h>
#include <exec/resident.h>
#include <resources/filesysres.h>
#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/partition.h>

#include "partition_support.h"
#include "fsloader.h"
#include "debug.h"

static struct FileSysEntry *FindResidentFS(struct FileSysResource *fsr, ULONG dostype, ULONG version)
{
    struct FileSysEntry *fsrnode;

    D(bug("[PART] FindResidentFS . . . "));

    ForeachNode(&fsr->fsr_FileSysEntries, fsrnode)
    {
    	if (fsrnode->fse_DosType == dostype)
        {
            if (fsrnode->fse_Version >= version)
            {
                D(bug("FOUND a HIGHER version Resident, so NO Loading version from RDB\n"));
                return fsrnode;
            } else {
                D(bug("FOUND a LOWER version Resident, so continue with Loading version from RDB\n"));
            }
        }
    }

    return NULL;
}

ULONG AddFS(struct Library *PartitionBase, struct FileSysHandle *fs)
{
    struct DosLibrary *DOSBase = (struct DosLibrary *)((struct PartitionBase_intern *)PartitionBase)->pb_DOSBase;
    struct FileSysResource *fsr;
    struct FileSysEntry *fsrnode;
    ULONG dostype;
    ULONG version;

    D(bug("[PART] AddFS: FileSystem=%s", fs->ln.ln_Name));

    fsr = OpenResource("FileSystem.resource");
    if (!fsr)
    	return ERROR_INVALID_RESIDENT_LIBRARY;

    GetFileSystemAttrs(&fs->ln, FST_ID, &dostype, FST_VERSION, &version, TAG_DONE);

    /*
     * First we want to check if we already have this filesystem in the resource.
     * Unfortunately the resource doesn't have any locking, so we have to use
     * Forbid()/Permit() pair. In order not to hold it for a while, we repeat
     * the check below after loading the handler (to eliminate race condition
     * when someone loads newer version of the filesystem that we are loading
     * at the moment.
     */
    D(ULONG version_major);
    D(ULONG version_minor);
    D(version_minor = version % 65536);
    D(version_major = ( (version - version_minor) / 65536 )); 
    D(bug("[PART] AddFS Stage #1: DosType=0x%8lx | version=%u.%u | Checking if Resident in FileSystem.resource\n", dostype, version_major, version_minor));

    Forbid();
    fsrnode = FindResidentFS(fsr, dostype, version);
    Permit();

    if (fsrnode)
    	return ERROR_OBJECT_EXISTS;

    fsrnode = AllocVec(sizeof(struct FileSysEntry), MEMF_PUBLIC | MEMF_CLEAR);
    if (!fsrnode)
        return ERROR_NO_FREE_STORE;

    GetFileSystemAttrs(&fs->ln, FST_FSENTRY, fsrnode, TAG_DONE);
    fsrnode->fse_SegList = LoadFileSystem(&fs->ln);

    D(version_minor = fsrnode->fse_Version % 65536);
    D(version_major = ( (fsrnode->fse_Version - version_minor) / 65536 ));     

    if (fsrnode->fse_SegList)
    {
    	struct FileSysEntry *dup;

        /*
        * Repeat checking, and insert the filesystem only if still not found.
        * If found, unload our seglist and return error.
        * This really sucks but nothing can be done with it. Even if we implement
        * a global semaphore on the resource original m68k software won't know
        * about it.
        */
        Forbid();

        dup = FindResidentFS(fsr, dostype, version);

        D(bug("[PART] AddFS Stage #2: DosType=0x%8lx | version=%u.%u | Handler=%s | SegList=0x%8xl | Loading Filesystem from RDB . . . ",
            fsrnode->fse_DosType, version_major, version_minor, fsrnode->fse_Handler, fsrnode->fse_SegList));

        if (!dup)
            /*
            * Entries in the list are not sorted by priority.
            * Adding to head makes them sorted by version.
            */
            AddHead(&fsr->fsr_FileSysEntries, &fsrnode->fse_Node);

        Permit();

        if (dup)
        {
            UnLoadSeg(fsrnode->fse_SegList);
            FreeVec(fsrnode);

            D(bug("NOT LOADED (Duplicate Found)\n"));
            return ERROR_OBJECT_EXISTS;
        }

        D(bug("LOADED\n"));
	    return 0;

    }

    D(bug("[PART] AddFS Stage #2: DosType=0x%8lx | version=%u.%u | Handler=%s | SegList=0x%8xl | Loading Filesystem from RDB . . .",
        fsrnode->fse_DosType, version_major, version_minor, fsrnode->fse_Handler, fsrnode->fse_SegList));
    D(bug("NOT LOADED (FS Not Found)\n"));

    /* InternalLoadSeg() will leave its error code in IoErr() */
    return IoErr();
}
