/*
    Copyright � 1995-2019, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Discover all mountable partitions
    Lang: english
*/

#include <string.h>
#include <stdlib.h>

#include <aros/debug.h>
#include <exec/alerts.h>
#include <aros/asmcall.h>
#include <aros/bootloader.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/types.h>
#include <libraries/configvars.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <libraries/partition.h>
#include <utility/tagitem.h>
#include <devices/bootblock.h>
#include <devices/timer.h>
#include <dos/dosextens.h>
#include <resources/filesysres.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/partition.h>
#include <proto/bootloader.h>
#include <clib/alib_protos.h>

#include LC_LIBDEFS_FILE

#include "dosboot_intern.h"
#include "../expansion/expansion_intern.h"

#define uppercase(x) ((x >= 'a' && x <= 'z') ? (x & 0xdf) : x)


static ULONG GetOffset(struct Library *PartitionBase, struct PartitionHandle *ph)
{
    IPTR tags[3];
    struct DosEnvec de;
    ULONG offset = 0;

    tags[0] = PT_DOSENVEC;
    tags[1] = (IPTR)&de;
    tags[2] = TAG_DONE;
    ph = ph->root;
    while (ph->root)
    {
        GetPartitionAttrs(ph, (struct TagItem *)tags);
        offset += de.de_LowCyl * de.de_Surfaces * de.de_BlocksPerTrack;
        ph = ph->root;
    }
    return offset;
}

static VOID AddPartitionVolume(struct ExpansionBase *ExpansionBase, struct Library *PartitionBase,
			       struct FileSysStartupMsg *fssm, struct PartitionHandle *table,
			       struct PartitionHandle *pn, struct ExecBase *SysBase)
{
    UBYTE name[32];
    ULONG i, blockspercyl;
    const struct PartitionAttribute *attrs;
    IPTR tags[9];
    IPTR pp[4 + DE_BOOTBLOCKS + 1] = { };
    struct DeviceNode *devnode;
    LONG ppos;
    TEXT *devname;
    LONG bootable;
    LONG automount;
    ULONG pttype = PHPTT_UNKNOWN;
    BOOL appended, changed;
    struct Node *fsnode;

    D(bug("[BOOT] AddPartitionVolume\n"));
    GetPartitionTableAttrsTags(table, PTT_TYPE, &pttype, TAG_DONE);

    attrs = QueryPartitionAttrs(table);
    while ((attrs->attribute != TAG_DONE) && (attrs->attribute != PT_NAME))
        attrs++;  /* look for name attr */

    if (attrs->attribute != TAG_DONE)
    {
        D(bug("[BOOT] RDB/GPT partition\n"));

        /* partition has a name => RDB/GPT partition */
        tags[0] = PT_NAME;
        tags[1] = (IPTR)name;
        tags[2] = PT_DOSENVEC;
        tags[3] = (IPTR)&pp[4];
        tags[4] = PT_BOOTABLE;
        tags[5] = (IPTR)&bootable;
        tags[6] = PT_AUTOMOUNT;
        tags[7] = (IPTR)&automount;
        tags[8] = TAG_DONE;
        GetPartitionAttrs(pn, (struct TagItem *)tags);

        D(bug("[BOOT] Partition name: %s | Bootable: %d | Automount: %d\n", name, bootable, automount));

        if (automount == FALSE)
        {
        	D(bug("[BOOT] Skipping %s so NOMOUNT is SET\n", name));
        	return;
        }
    }
    else
    {
        D(bug("[BOOT] MBR/EBR partition\n"));

        /* partition doesn't have a name => MBR/EBR partition */
        tags[0] = PT_POSITION;
        tags[1] = (IPTR)&ppos;
        tags[2] = PT_DOSENVEC;
        tags[3] = (IPTR)&pp[4];
        tags[4] = TAG_DONE;
        GetPartitionAttrs(pn, (struct TagItem *)tags);

        // Default behaviour is to give BOOT priority to RDB disks (user can choose in Early Startup)
        bootable = TRUE;
        pp[4 + DE_BOOTPRI] = -1;

        /* make the name */
        devname = AROS_BSTR_ADDR(fssm->fssm_Device);
        for (i = 0; i < 26; i++)
        {
            if (*devname == '.' || *devname == '\0')
                break;
            name[i] = (UBYTE)uppercase(*devname);
            devname++;
        }

        if ((fssm->fssm_Unit / 10))
            name[i++] = '0' + (UBYTE)(fssm->fssm_Unit / 10);
        
        name[i++] = '0' + (UBYTE)(fssm->fssm_Unit % 10);
        name[i++] = 'P';
        
        if (table->table->type == PHPTT_EBR)
            ppos += 4;
        
        if ((ppos / 10))
            name[i++] = '0' + (UBYTE)(ppos / 10);
        
            name[i++] = '0' + (UBYTE)(ppos % 10);
        name[i] = '\0';

        D(bug("[BOOT] Partition name: %s\n", name));
    }

    if ((pp[4 + DE_TABLESIZE] < DE_DOSTYPE) || (pp[4 + DE_DOSTYPE] == 0))
    {
    	/*
    	 * partition.library reports DosType == 0 for unknown filesystems.
    	 * However dos.library will mount such DeviceNodes using rn_DefaultHandler
    	 * (FFS). This is done for compatibility with 3rd party expansion ROMs.
    	 * Here we ignore partitions with DosType == 0 and won't enter them into
    	 * mountlist.
    	 */
    	D(bug("[BOOT] Unknown DosType for %s, skipping partition\n"));
    	return;
    }

    if (pttype != PHPTT_RDB)
    {
        /*
         * Only RDB partitions can store the complete DosEnvec.
         * For other partition types partition.library puts some defaults
         * into these fields, however they do not have anything to do with
         * real values, which are device-dependent.
         * However, the device itself knows them. Here we inherit these settings
         * from the original DeviceNode which represents the whole drive.
         * Note that we don't change DosEnvec size. If these fields are not included,
         * it will stay this way.
         * Copy members only if they are present in device's DosEnvec.
         */
        struct DosEnvec *devenv = BADDR(fssm->fssm_Environ);

        if (devenv->de_TableSize >= DE_MAXTRANSFER)
        {
            pp[4 + DE_MAXTRANSFER] = devenv->de_MaxTransfer;

            if (devenv->de_TableSize >= DE_MASK)
                pp[4 + DE_MASK] = devenv->de_Mask;
        }
    }

    /*
     * BHFormat complains if this bit is not set, and it's really wrong to have it unset.
     * So we explicitly set it here. Pavel Fedin <pavel.fedin@mail.ru>
     */
    pp[4 + DE_BUFMEMTYPE] |= MEMF_PUBLIC;

    pp[0] = (IPTR)name;
    pp[1] = (IPTR)AROS_BSTR_ADDR(fssm->fssm_Device);
    pp[2] = fssm->fssm_Unit;
    pp[3] = fssm->fssm_Flags;

    i = GetOffset(PartitionBase, pn);
    blockspercyl = pp[4 + DE_BLKSPERTRACK] * pp[4 + DE_NUMHEADS];
    if (i % blockspercyl != 0)
    {
        D(bug("[BOOT] Start block of subtable not on cylinder boundary: "
            "%ld (Blocks per Cylinder = %ld)\n", i, blockspercyl));
        return;
    }
    i /= blockspercyl;
    pp[4 + DE_LOWCYL] += i;
    pp[4 + DE_HIGHCYL] += i;

    /* Append .n if same device name already exists */
    appended = FALSE;
    changed = TRUE;
    while (changed)
    {
        struct BootNode *bn;
        changed = FALSE;

        /* Note that we already have the mount list semaphore */
        ForeachNode(&ExpansionBase->MountList, bn)
        {
            if (stricmp(AROS_BSTR_ADDR(((struct DeviceNode*)bn->bn_DeviceNode)->dn_Name), name) == 0)
            {
                if (!appended)
                    strcat(name, ".1");
                else
                    name[strlen(name) - 1]++;
                appended = TRUE;
                changed = TRUE;
            }
        }
    }

    fsnode = FindFileSystem(table, FST_ID, pp[4 + DE_DOSTYPE], TAG_DONE);
    if (fsnode) {
        D(bug("[BOOT] Found on-disk filesystem 0x%08x\n", pp[4 + DE_DOSTYPE]));
        AddBootFileSystem(fsnode);
    }

    devnode = MakeDosNode(pp);

    struct FileSysStartupMsg *fssm_devnode = BADDR(devnode->dn_Startup);
    struct DosEnvec *de_devnode = BADDR(fssm->fssm_Environ);

    D(bug("\n[BOOT] [%s] Current glob->fssm->fssm_Environ (DOSENVEC):\n",__FUNCTION__));
    D(bug("[BOOT] [%s] de_SizeBlock      : %d\n",__FUNCTION__ , de_devnode->de_SizeBlock));
    D(bug("[BOOT] [%s] de_Surfaces       : %d\n",__FUNCTION__ ,  de_devnode->de_Surfaces));
    D(bug("[BOOT] [%s] de_BlocksPerTrack : %d\n",__FUNCTION__ ,  de_devnode->de_BlocksPerTrack));
    D(bug("[BOOT] [%s] dg_LowCyl         : %d\n",__FUNCTION__ ,  de_devnode->de_LowCyl));
    D(bug("[BOOT] [%s] dg_HighCyl        : %d\n\n",__FUNCTION__ ,  de_devnode->de_HighCyl));


    if (devnode != NULL) {
        AddBootNode(bootable ? pp[4 + DE_BOOTPRI] : -128, ADNF_STARTPROC, devnode, NULL);
        D(bug("[BOOT] AddBootNode(%b, 0, 0x%p, NULL)\n",  devnode->dn_Name, pp[4 + DE_DOSTYPE]));
        return;
    }
}

static BOOL CheckTables(struct ExpansionBase *ExpansionBase, struct Library *PartitionBase, struct FileSysStartupMsg *fssm,	struct PartitionHandle *table, struct ExecBase *SysBase)
{
    BOOL retval = FALSE;
    struct PartitionHandle *ph;

    D(bug("\n[BOOT] CheckTables - Start\n"));

    /* Traverse partition tables recursively, and attempt to add a BootNode for any non-subtable partitions found */
    if (OpenPartitionTable(table) == 0)
    {
        ph = (struct PartitionHandle *)table->table->list.lh_Head;

        D(bug("\n[BOOT] CheckTables - Table Type = %d\n", table->table->type));
        if (table->table->type == 2) return FALSE;                                      // Skipping FAT tables (handled by fat-handler)

        while (ph->ln.ln_Succ)
        {
                      
            /* Attempt to add partition to system if it isn't a subtable or a FAT table */
            if (!CheckTables(ExpansionBase, PartitionBase, fssm, ph, SysBase)) AddPartitionVolume(ExpansionBase, PartitionBase, fssm, table, ph, SysBase);
            ph = (struct PartitionHandle *)ph->ln.ln_Succ;
        }
        retval = TRUE;
        ClosePartitionTable(table);
    }
    return retval;
}

static VOID CheckPartitions(struct ExpansionBase *ExpansionBase, struct Library *PartitionBase, struct ExecBase *SysBase, struct BootNode *bn)
{
    struct DeviceNode *dn = bn->bn_DeviceNode;
    BOOL res = FALSE;

    D(bug("\n[BOOT] CheckPartitions('%b') handler seglist = %x, handler = %s\n", dn->dn_Name, dn->dn_SegList, AROS_BSTR_ADDR(dn->dn_Handler)));

    /* If we already have filesystem handler, don't do anything */
    if (dn->dn_SegList == BNULL && dn->dn_Handler == BNULL)
    {
    	struct FileSysStartupMsg *fssm = BADDR(dn->dn_Startup);

        if (fssm && fssm->fssm_Device)
        {
            struct PartitionHandle *pt = OpenRootPartition(AROS_BSTR_ADDR(fssm->fssm_Device), fssm->fssm_Unit);
            
            if (pt)
            {
                res = CheckTables(ExpansionBase, PartitionBase, fssm, pt, SysBase);
                CloseRootPartition(pt);
            }

        }
    }
    
    //* Cleanup Device Node and if no partitions were found for the DeviceNode or device is SDx: then we put it back 
    Remove(&bn->bn_Node);
    if (!res)
    // || (AROS_BSTR_ADDR(dn->dn_Name)[0] == 'S' && AROS_BSTR_ADDR(dn->dn_Name)[1] == 'D'  
    //    && AROS_BSTR_ADDR(dn->dn_Name)[2] == 'R' && AROS_BSTR_ADDR(dn->dn_Name)[3] == 'O' && AROS_BSTR_ADDR(dn->dn_Name)[4] == 'M') )
    {
        Enqueue(&ExpansionBase->MountList, &bn->bn_Node);
    }
}

/* Scan all partitions manually for additional volumes that can be mounted. */
void dosboot_BootScan(LIBBASETYPEPTR DOSBootBase)
{
    struct ExpansionBase *ExpansionBase = DOSBootBase->bm_ExpansionBase;
    APTR PartitionBase;
    struct BootNode *bootNode, *temp;
    struct DeviceNode *deviceNode;
    struct List rootList;

    D(bug("\n[BOOT] dosboot_BootScan START\n"));

    /* If we have partition.library, we can look for partitions */
    PartitionBase = OpenLibrary("partition.library", 2);
    if (PartitionBase)
    {
        ObtainSemaphore(&IntExpBase(ExpansionBase)->BootSemaphore);

        /* Transfer all bootnodes in the mountlist into a temporary list. The assumption is that all bootnodes created before now represent entire disks */
        NewList(&rootList);
        while ((temp = (struct BootNode *)RemHead(&ExpansionBase->MountList)) != NULL)
            AddTail(&rootList, (struct Node *) temp);

        ForeachNodeSafe (&rootList, bootNode, temp)
        {
            CheckPartitions(ExpansionBase, PartitionBase, SysBase, bootNode);
        }

        ReleaseSemaphore(&IntExpBase(ExpansionBase)->BootSemaphore);

	    CloseLibrary(PartitionBase);
    }

    D(bug("[BOOT] dosboot_BootScan FINISH\n\n"));
}


