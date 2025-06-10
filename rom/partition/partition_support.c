/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

*/

#include <proto/exec.h>
#include <devices/newstyle.h>
#include <utility/tagitem.h>

#include "partition_intern.h"
#include "partition_support.h"
#include "debug.h"

const struct PTFunctionTable * const PartitionSupport[] =
{
    //&PartitionGPT, /* The order of these two is important, GPT must be checked before MBR */
    &PartitionMBR,
    //&PartitionEBR,
    &PartitionRDB,
    NULL
};

/* Keep RDB last. RDB scans up to 16 blocks and in case of
 * MBR->EBR->RDB having RDB first detects RDB as root and
 * causes MBR and EBR offsets not be taken into account
 */

/* get geometry */
LONG PartitionGetGeometry(struct Library *PartitionBase, struct IOExtTD *ioreq, struct DriveGeometry *dg)
{
    D(bug("[PART:ROOT] PartitionGetGeometry\n"));

    ioreq->iotd_Req.io_Command = TD_GETGEOMETRY;
    ioreq->iotd_Req.io_Data = dg;
    ioreq->iotd_Req.io_Length = sizeof(struct DriveGeometry);
    return DoIO((struct IORequest *)ioreq);
}

/* query NSD commands */
void PartitionNsdCheck(struct Library *PartitionBase, struct PartitionHandle *root)
{
    struct NSDeviceQueryResult nsdq;
    struct IOExtTD *ioreq = root->bd->ioreq;
    UWORD *cmdcheck;

    D(bug("[PART:ROOT] PartitionNSDCheck\n"));

    if ((root->de.de_HighCyl * root->de.de_Surfaces * root->de.de_BlocksPerTrack * ((root->de.de_SizeBlock<<2)/512)) > 8388608)
    {
        nsdq.SizeAvailable=0;
        nsdq.DevQueryFormat=0;
        ioreq->iotd_Req.io_Command=NSCMD_DEVICEQUERY;
        ioreq->iotd_Req.io_Data=&nsdq;
        ioreq->iotd_Req.io_Length=sizeof(struct NSDeviceQueryResult);
        if (DoIO((struct IORequest *)ioreq)==0)
        {
            if (
                (ioreq->iotd_Req.io_Actual<=sizeof(struct NSDeviceQueryResult)) &&
                (ioreq->iotd_Req.io_Actual!=0) &&
                (ioreq->iotd_Req.io_Actual==nsdq.SizeAvailable)
            )
            {
            if (nsdq.DeviceType != NSDEVTYPE_TRACKDISK)
                D(bug("[PART:ROOT] NSDcheck: WARNING no trackdisk type\n"));
            for (cmdcheck=nsdq.SupportedCommands;*cmdcheck;cmdcheck++)
            {
                if (*cmdcheck == NSCMD_TD_READ64)
                    root->bd->cmdread = NSCMD_TD_READ64;
                if (*cmdcheck == NSCMD_TD_WRITE64)
                    root->bd->cmdwrite = NSCMD_TD_WRITE64;
            }
            }
            else
            D(bug("[PART:ROOT] NSDcheck: WARNING wrong io_Actual using NSD\n"));
        }
    }
}

/* get real first block of partition ph */
ULONG getStartBlock(struct PartitionHandle *ph)
{
    ULONG start = 0;

    while (ph)
    {
        start += ph->de.de_LowCyl*ph->de.de_BlocksPerTrack*ph->de.de_Surfaces;
        ph = ph->root;
    }

    //D(bug("[PART:%s] GetStartBlock: %u\n", ph->ln.ln_Name, start));

    return start;
}

LONG deviceError(LONG err)
{
    D(bug("[PART] deviceError\n"));
    
    switch (err)
    {
    case 0:
    	return 0;

    case TDERR_WriteProt:
    	return ERROR_DISK_WRITE_PROTECTED;

    case TDERR_SeekError:
    	return ERROR_SEEK_ERROR;
    
    default:
    	return ERROR_NOT_A_DOS_DISK;
    }
}

/*
 * Initialize partition handle based on parent's data.
 * Geometry will be inherited from parent and adjusted if needed
 * for given start block and length in blocks to fit in.
 */
void initPartitionHandle(struct PartitionHandle *root, struct PartitionHandle *ph, ULONG first_sector, ULONG count_sector)
{
    ULONG cylsecs = root->de.de_BlocksPerTrack * root->de.de_Surfaces;

    D(bug("[PART:%s] initPartitionHandle\n", ph->ln.ln_Name));

    /* Attach parent */
    ph->root = root;
    ph->bd   = root->bd;

    /* initialize DosEnvec */
    CopyMem(&root->de, &ph->de, sizeof(struct DosEnvec));

    D(bug("\n[PART] [%s] Disk geometry received from MBR:\n",__FUNCTION__ ));
    D(bug("[PART] [%s] first_sector     : %d\n",__FUNCTION__ , first_sector));
    D(bug("[PART] [%s] count_sector     : %d\n",__FUNCTION__ , count_sector));
    D(bug("[PART] [%s] cylsecs          : %d\n",__FUNCTION__ , cylsecs));

    /* Check if partition starts and ends on a cylinder boundary */
    if ((first_sector % cylsecs != 0) || (count_sector % cylsecs != 0))
    {
        /* Treat each track as a cylinder if possible */
        ph->de.de_Surfaces = 1;
        cylsecs = ph->de.de_BlocksPerTrack;
        D(bug("\n[PART] [%s] Disk geometry adjusted (flat LBA):\n",__FUNCTION__ ));
        D(bug("[PART] [%s] cylsecs          : %d\n",__FUNCTION__ , cylsecs));
        D(bug("[PART] [%s] de.de_Surfaces   : %d\n",__FUNCTION__ , ph->de.de_Surfaces));

        if ((first_sector % cylsecs != 0) || (count_sector % cylsecs != 0))
        {
            /* We can't. We could find the highest common factor of first_sector and count_sector here, but currently we simply use one block per cylinder (flat LBA) */
            ph->de.de_BlocksPerTrack = 1;
            cylsecs = 1;
            
            D(bug("[PART] [%s] cylsecs          : %d\n",__FUNCTION__ , cylsecs));
            D(bug("[PART] [%s] de.BlocksperTrack: %d\n",__FUNCTION__ , ph->de.de_BlocksPerTrack));
        }
    }

    /* initialize DriveGeometry */
    ph->dg.dg_DeviceType   = DG_DIRECT_ACCESS;
    ph->dg.dg_SectorSize   = ph->de.de_SizeBlock<<2;
    ph->dg.dg_Heads        = ph->de.de_Surfaces;
    ph->dg.dg_TrackSectors = ph->de.de_BlocksPerTrack;
    ph->dg.dg_Cylinders    = count_sector / cylsecs;
    ph->dg.dg_CylSectors   = cylsecs;
    ph->dg.dg_BufMemType   = ph->de.de_BufMemType;

    D(bug("\n[PART] [%s] Disk geometry received from Device Driver:\n",__FUNCTION__ ));
    D(bug("[PART] [%s] dg_SectorSize    : %d\n",__FUNCTION__ , ph->dg.dg_SectorSize));
    D(bug("[PART] [%s] dg_TotalSectors  : %d\n",__FUNCTION__ , ph->dg.dg_TotalSectors));
    D(bug("[PART] [%s] dg_Cylinders     : %d\n",__FUNCTION__ , ph->dg.dg_Cylinders));
    D(bug("[PART] [%s] dg_CylSectors    : %d\n",__FUNCTION__ , ph->dg.dg_CylSectors));
    D(bug("[PART] [%s] dg_Heads         : %d\n",__FUNCTION__ , ph->dg.dg_Heads));
    D(bug("[PART] [%s] dg_TrackSectors  : %d\n",__FUNCTION__ , ph->dg.dg_TrackSectors));

    /* Set start/end cylinder in DosEnvec */
    ph->de.de_LowCyl    = first_sector / cylsecs;
    ph->de.de_HighCyl   = ph->de.de_LowCyl + ph->dg.dg_Cylinders - 1;

    D(bug("\n[PART] [%s] Drive Geometry transfer to ph->de (DosEnvec):\n",__FUNCTION__ ));
    D(bug("[PART] [%s] de_LowCyl         : %d\n",__FUNCTION__ , ph->de.de_LowCyl));
    D(bug("[PART] [%s] de_HighCyl        : %d\n\n",__FUNCTION__ , ph->de.de_HighCyl));

    /* Fix up DosEnvec size if necessary */
    if (ph->de.de_TableSize < DE_BUFMEMTYPE)
        ph->de.de_TableSize = DE_BUFMEMTYPE;
}

/* Set DOSType and some defaults according to it */
void setDosType(struct DosEnvec *de, ULONG type)
{
    D(bug("[PART] setDosType: 0x%8lx\n", type ));
   
    de->de_TableSize      = DE_DOSTYPE;
    de->de_SectorPerBlock = 1;
    de->de_DosType        = type;
    /*
     * These two are actually device-dependent. However there's no way
     * to obtain them, so just fill in defaults.
     */
    de->de_MaxTransfer    = 0x00200000;
    de->de_Mask           = 0x7ffffffe;

    /* Some more filesystem-specific defaults */
    switch (type)
    {
    case ID_DOS_DISK:
    case ID_FFS_DISK:
    case ID_INTER_FFS_DISK:
    case ID_SFS_BE_DISK:
    	de->de_TableSize      = DE_BOOTBLOCKS;
    	de->de_Reserved       = 2;
    	de->de_BootBlocks     = 2;
    }
}
