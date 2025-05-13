/*
 * fat-handler - FAT12/16/32 filesystem handler
 *
 * Copyright � 2007-2020 The AROS Development Team
 * Copyright � 2006 Marek Szyprowski
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 *
 * $Id$
 */

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/timer.h>

#include <exec/types.h>

#include <devices/inputevent.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/trackdisk.h>

#include <clib/alib_protos.h>

#include <clib/macros.h>

#include <ctype.h>

#include "fat_fs.h"
#include "fat_protos.h"

#define DEBUG DEBUG_MISC
#include "debug.h"

static const UBYTE default_oem_name[] = "MSWIN4.1";
static const UBYTE default_filsystype[] = "FAT16   ";

static const ULONG fat16_cluster_thresholds[] =
{
    8400,
    32680,
    262144,
    524288,
    1048576,
    0xFFFFFFFF
};

static const ULONG fat32_cluster_thresholds[] =
{
    16777216,
    33554432,
    67108864,
    0xFFFFFFFF
};

/* 01-01-1981 */
static const struct DateStamp unset_date_limit =
{
    1096,
    0,
    0
};

#ifndef ID_BUSY
#define ID_BUSY 0x42555359
#endif

static LONG GetVolumeIdentity(struct FSSuper *sb, struct VolumeIdentity *volume);
static void FreeFATSuper(struct FSSuper *s);

LONG ReadFATSuper(struct FSSuper *sb)
{
    D(bug("----------------------------------------------------------------\n"));
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct Globals *glob = sb->glob;
    struct DosEnvec *de = BADDR(glob->fssm->fssm_Environ);
    LONG err = 0, td_err;
    ULONG bsize = de->de_SizeBlock * 4, total_sectors, id;
    struct FATBootSector *boot;
    struct FATEBPB *ebpb;
    struct FATFSInfo *fsinfo;
    BOOL invalid;
    ULONG end;
    LONG i;
    struct DirHandle dh;
    struct DirEntry dir_entry;
    APTR block_ref;
    UBYTE *fat_block;
    invalid = FALSE;

    D(bug("[FAT] [%s] Reading boot sector\n",__FUNCTION__ ));
    D(bug("[FAT] [%s] Blockspertrack %ld\n",__FUNCTION__ , de->de_BlocksPerTrack));
    D(bug("[FAT] [%s] Surfaces %ld\n", __FUNCTION__ , de->de_Surfaces));
    D(bug("[FAT] [%s] Lowcyl %ld\n", __FUNCTION__ , de->de_LowCyl));
    D(bug("[FAT] [%s] Highcyl %ld\n", __FUNCTION__ , de->de_HighCyl));

    boot = AllocMem(bsize, MEMF_ANY);
    if (!boot)
        return ERROR_NO_FREE_STORE;

    sb->first_device_sector = de->de_BlocksPerTrack * de->de_Surfaces * de->de_LowCyl;
    D(bug("\n[FAT] [%s] TRYING Boot sector at sector %ld\n", __FUNCTION__ , sb->first_device_sector));

    /* Get a preliminary total-sectors value so we don't risk going outside partition limits */
    sb->total_sectors = de->de_BlocksPerTrack * de->de_Surfaces * (de->de_HighCyl + 1) - sb->first_device_sector;
    D(bug("[FAT] [%s] Calculated Total Sectors %ld\n", __FUNCTION__ , sb->total_sectors));

    /*
     * Read the boot sector. We go direct because we don't have a cache yet,
     * and can't create one until we know the sector size, which is held in
     * the boot sector. In practice it doesn't matter - we're going to use
     * this once and once only.
     */
    if ((td_err = AccessDisk(FALSE, sb->first_device_sector, 1, bsize, (UBYTE *) boot, glob)) != 0)
    {
        D(bug("[FAT] [%s] Couldn't read boot block (%ld)\n", __FUNCTION__ , td_err));
        FreeMem(boot, bsize);
        return ERROR_UNKNOWN;
    }

    D(bug("[FAT] [%s] Boot sector:\n",__FUNCTION__ ));

    sb->sectorsize = AROS_LE2WORD(boot->bpb_bytes_per_sect);
    sb->sectorsize_bits = log2(sb->sectorsize);
    D(bug("[FAT] [%s] SectorSize = %ld\n", __FUNCTION__ , sb->sectorsize));
    D(bug("[FAT] [%s] SectorSize Bits = %ld\n", __FUNCTION__ , sb->sectorsize_bits));

    sb->cluster_sectors = boot->bpb_sect_per_clust;
    sb->clustersize = sb->sectorsize * boot->bpb_sect_per_clust;
    sb->clustersize_bits = log2(sb->clustersize);
    sb->cluster_sectors_bits = sb->clustersize_bits - sb->sectorsize_bits;

    D(bug("[FAT] [%s] SectorsPerCluster = %ld\n",__FUNCTION__ , (ULONG) boot->bpb_sect_per_clust));
    D(bug("[FAT] [%s] ClusterSize = %ld\n", __FUNCTION__ , sb->clustersize));
    D(bug("[FAT] [%s] ClusterSize Bits = %ld\n", __FUNCTION__ , sb->clustersize_bits));
    D(bug("[FAT] [%s] Cluster Sectors Bits = %ld\n", __FUNCTION__ , sb->cluster_sectors_bits));

    sb->first_fat_sector = AROS_LE2WORD(boot->bpb_rsvd_sect_count);
    D(bug("[FAT] [%s] First FAT Sector = %ld\n", __FUNCTION__ , sb->first_fat_sector));

    sb->fat_count = boot->bpb_num_fats;
    D(bug("[FAT] [%s] Number of FATs = %d\n", __FUNCTION__ , sb->fat_count));

    if (boot->bpb_fat_size_16 != 0)
    {
        sb->fat_size = AROS_LE2WORD(boot->bpb_fat_size_16);
        D(bug("[FAT] [%s] FAT16 Size = %ld\n", __FUNCTION__ , sb->fat_size));
    } else {
        sb->fat_size = AROS_LE2LONG(boot->ebpbs.ebpb32.bpb_fat_size_32);
        D(bug("[FAT] [%s] FAT32 Size = %ld\n", __FUNCTION__ , sb->fat_size));
    }

    if (boot->bpb_total_sectors_16 != 0)
    {
        total_sectors = AROS_LE2WORD(boot->bpb_total_sectors_16);
        D(bug("[FAT] [%s] Total Sectors (FAT16) = %ld\n", __FUNCTION__ , total_sectors));
    } else {
        total_sectors = AROS_LE2LONG(boot->bpb_total_sectors_32);
        D(bug("[FAT] [%s] Total Sectors (FAT32) = %ld\n", __FUNCTION__ , total_sectors));
    }
    
    /* Check that the boot block's sector count is the same as the
     * partition's sector count. This stops a resized partition being
     * mounted before reformatting */
    
    if ((total_sectors +4096) < sb->total_sectors)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] #Boot-Sectors (%ld) < Partition-Sectors (%ld)\n", __FUNCTION__ , total_sectors + 4096, sb->total_sectors));
    }

    sb->rootdir_sectors = ((AROS_LE2WORD(boot->bpb_root_entries_count)
        * sizeof(struct FATDirEntry)) + (sb->sectorsize - 1))
        >> sb->sectorsize_bits;
    D(bug("[FAT] [%s] RootDir Sectors = %ld\n", __FUNCTION__ , sb->rootdir_sectors));

    sb->data_sectors = sb->total_sectors - (sb->first_fat_sector
        + (sb->fat_count * sb->fat_size) + sb->rootdir_sectors);
    D(bug("[FAT] [%s] Data Sectors = %ld\n", __FUNCTION__ , sb->data_sectors));

    sb->clusters_count = sb->data_sectors >> sb->cluster_sectors_bits;
    D(bug("[FAT] [%s] Clusters Count = %ld\n", __FUNCTION__ , sb->clusters_count));

    sb->first_rootdir_sector =
        sb->first_fat_sector + (sb->fat_count * sb->fat_size);
    D(bug("[FAT] [%s] First RootDir Sector = %ld\n", __FUNCTION__ , sb->first_rootdir_sector));

    sb->first_data_sector =
        sb->first_fat_sector + (sb->fat_count * sb->fat_size)
        + sb->rootdir_sectors;
    D(bug("[FAT] [%s] First Data Sector = %ld\n", __FUNCTION__ , sb->first_data_sector));

    /* Check if disk is in fact a FAT filesystem */

    /* Valid sector size: 512, 1024, 2048, 4096 */
    if (sb->sectorsize != 512 && sb->sectorsize != 1024
        && sb->sectorsize != 2048 && sb->sectorsize != 4096)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid FAT Sectorsize: %ld\n", __FUNCTION__ , sb->sectorsize));
    }

    /* Valid bpb_sect_per_clust: 1, 2, 4, 8, 16, 32, 64, 128 */
    if ((boot->bpb_sect_per_clust & (boot->bpb_sect_per_clust - 1)) != 0
        || boot->bpb_sect_per_clust == 0 || boot->bpb_sect_per_clust > 128)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid FAT Sectorsize: %ld\n", __FUNCTION__ , sb->sectorsize));
    }

    /* Valid cluster size: 512, 1024, 2048, 4096, 8192, 16k, 32k, 64k */
    if (sb->clustersize > 64 * 1024)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid FAT Clustersize: %ld\n", __FUNCTION__ , sb->clustersize));
    }

    if (sb->first_fat_sector == 0)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid first FAT Sector (0) %ld\n", __FUNCTION__ , sb->first_fat_sector));
    }

    if (sb->fat_count == 0)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid FAT Count: %ld\n", __FUNCTION__ , sb->fat_count));
    }

    if (boot->bpb_media < 0xF0)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid Media (boot->bpb_Media < 0xF0\n", __FUNCTION__));
    }

    /* FAT "signature" */
    if (boot->bpb_signature[0] != 0x55 || boot->bpb_signature[1] != 0xaa)
    {
        invalid = TRUE;
        D(bug("[FAT] [%s] Invalid FAT Signature (boot->bpb_signature\n", __FUNCTION__ ));
    }

    if (invalid)
    {
        D(bug("[FAT] [%s] Invalid FAT Boot Sector\n",__FUNCTION__ ));
        
        FreeMem(boot, bsize);
        return ERROR_NOT_A_DOS_DISK;
    }

    end = 0xFFFFFFFF / sb->sectorsize;
    if ((sb->first_device_sector + sb->total_sectors - 1 > end)
        && (glob->readcmd == CMD_READ))
    {
        D(bug("[FAT] [%s] Device is too large\n",__FUNCTION__ ));
        ErrorMessage("Your device driver does not support 64-bit\n"
            "disk addressing, but it is needed to access\n"
            "the volume in device %s.\n\n"
            "In order to prevent data damage, access to\n"
            "this volume was blocked. Please upgrade\n"
            "your device driver.", "OK",
            (IPTR)(AROS_BSTR_ADDR(glob->devnode->dol_Name)));
        FreeMem(boot, bsize);
        return ERROR_UNKNOWN;
    }

    sb->cache = Cache_CreateCache(glob, 64, 64, sb->sectorsize, SysBase,
        DOSBase);
    if (sb->cache == NULL)
    {
        err = IoErr();
        FreeMem(boot, bsize);
        return err;
    }

    if (sb->clusters_count < 4085)
    {
        D(bug("[FAT] [%s] FAT12 filesystem detected\n",__FUNCTION__ ));
        sb->type = 12;
        sb->eoc_mark = 0x0FFF;
        sb->func_get_fat_entry = GetFat12Entry;
        sb->func_set_fat_entry = SetFat12Entry;
    }
    else if (sb->clusters_count < 65525)
    {
        D(bug("[FAT] [%s] FAT16 filesystem detected\n",__FUNCTION__ ));
        sb->type = 16;
        sb->eoc_mark = 0xFFFF;
        sb->func_get_fat_entry = GetFat16Entry;
        sb->func_set_fat_entry = SetFat16Entry;
    }
    else
    {
        D(bug("[FAT] [%s] FAT32 filesystem detected\n",__FUNCTION__ ));
        sb->type = 32;
        sb->eoc_mark = 0x0FFFFFFF;
        sb->func_get_fat_entry = GetFat32Entry;
        sb->func_set_fat_entry = SetFat32Entry;
    }

    /* Set up the FAT cache and load the first blocks */
    sb->fat_cachesize = 4096;
    sb->fat_cachesize_bits = log2(sb->fat_cachesize);
    sb->fat_cache_block = 0xffffffff;

    sb->fat_blocks_count =
        MIN(sb->fat_size, sb->fat_cachesize >> sb->sectorsize_bits);
    sb->fat_blocks = AllocVecPooled(glob->mempool, sizeof(APTR) * sb->fat_blocks_count);
    sb->fat_buffers = AllocVecPooled(glob->mempool, sizeof(APTR) * sb->fat_blocks_count);
    if (sb->fat_blocks == NULL || sb->fat_buffers == NULL)
    {
        err = ERROR_NO_FREE_STORE;
        FreeVecPooled(glob->mempool, sb->fat_blocks);
        FreeVecPooled(glob->mempool, sb->fat_buffers);
        FreeMem(boot, bsize);
        return err;
    }
    glob->sb = sb;

    if (sb->type != 32)
    {
        /* Set up volume ID */
        sb->volume_id = AROS_LE2LONG(boot->ebpbs.ebpb.bs_volid);

        /* Location of root directory */
        sb->rootdir_cluster = 0;
        sb->rootdir_sector = sb->first_rootdir_sector;
        ebpb = &boot->ebpbs.ebpb;
    }
    else
    {
        /* Set up volume ID */
        sb->volume_id = AROS_LE2LONG(boot->ebpbs.ebpb32.ebpb.bs_volid);

        /* Location of root directory */
        sb->rootdir_cluster =
            AROS_LE2LONG(boot->ebpbs.ebpb32.bpb_root_cluster);
        sb->rootdir_sector = 0;
        ebpb = &boot->ebpbs.ebpb32.ebpb;
    }

    D(bug("[FAT] [%s] Rootdir at cluster %ld sector %ld\n", __FUNCTION__ , sb->rootdir_cluster, sb->rootdir_sector));

    /* Initialise the root directory if this is a newly formatted volume */
    if (glob->formatting)
    {
        D(bug("[FAT] [%s] Initialise the CACHE root directory if this is a newly formatted volume\n", __FUNCTION__ ));

        /* Clear all FAT sectors */
        for (i = 0; err == 0 && i < sb->fat_size * 2; i++)
        {
            block_ref = Cache_GetBlock(sb->cache, sb->first_device_sector + sb->first_fat_sector + i, &fat_block);
            if (block_ref != NULL)
            {
                /* FIXME: Handle IO errors on cache read! */
                SetMem(fat_block, 0, bsize);
                if (i == 0)
                {
                    /* The first two entries are special */
                    if (sb->type == 32)
                        *(UQUAD *) fat_block = AROS_QUAD2LE(0x0FFFFFFF0FFFFFF8);
                    else if (sb->type == 16)
                        *(ULONG *) fat_block = AROS_LONG2LE(0xFFFFFFF8);
                    else
                        *(ULONG *) fat_block = AROS_LONG2LE(0x00FFFFF8);
                }
                Cache_MarkBlockDirty(sb->cache, block_ref);
                Cache_FreeBlock(sb->cache, block_ref);
            }
            else
                err = IoErr();
        }

        D(bug("[FAT] [%s] Finished Cleaning all CACHE FAT sectors - Result = %u\n", __FUNCTION__, err ));

        if (err == 0)
        {
            /* Allocate first cluster of the root directory */
            if (sb->type == 32)
                AllocCluster(sb, sb->rootdir_cluster);

            /* Get a handle on the root directory */
            InitDirHandle(sb, 0, &dh, FALSE, glob);

            D(bug("[FAT] [%s] Finished allocating first cluster and obtaining handle on root\n", __FUNCTION__ ));
        }

        /* Clear all entries in the root directory */
        for (i = 0; err == 0 && GetDirEntry(&dh, i, &dir_entry, glob) == 0; i++)
        {
            SetMem(&dir_entry.e.entry, 0, sizeof(struct FATDirEntry));
            err = UpdateDirEntry(&dir_entry, glob);
        }

        D(bug("[FAT] [%s] Finished Clear all entries in the root directory\n", __FUNCTION__ ));

        if (err == 0)
        {
            err = SetVolumeName(sb, ebpb->bs_vollab, FAT_MAX_SHORT_NAME);

            ReleaseDirHandle(&dh, glob);
            glob->formatting = FALSE;
            D(bug("[FAT] [%s] Root dir created.\n",__FUNCTION__ ));
        }

        if (err == 0)
        {
            /* Check everything is really written to disk before we proceed */
            if (!Cache_Flush(sb->cache))
                err = IoErr();
            D(bug("[FAT] [%s] SUCCESS - Cache Flushed\n", __FUNCTION__ ));    
        }
    }

    if (GetVolumeIdentity(sb, &(sb->volume)) != 0)
    {
        LONG i;
        UBYTE *uu = (void *)&sb->volume_id;

        /* No volume name entry, so construct name from serial number */
        for (i = 1; i < 10;)
        {
            int d;

            if (i == 5)
                sb->volume.name[i++] = '-';

            d = (*uu) & 0x0f;
            sb->volume.name[i++] = (d < 10) ? '0' + d : 'A' - 10 + d;
            d = ((*uu) & 0xf0) >> 4;
            sb->volume.name[i++] = (d < 10) ? '0' + d : 'A' - 10 + d;

            uu++;
        }

        sb->volume.name[i] = '\0';
        sb->volume.name[0] = 9;
    }

    /* Many FAT volumes do not have a creation date set, with the result
     * that two volumes with the same name are likely to be indistinguishable
     * on the DOS list. To work around this problem, we set the ds_Tick field
     * of such volumes' dol_VolumeDate timestamp to a pseudo-random value based
     * on the serial number. Since there are 3000 ticks in a minute, we use an
     * 11-bit hash value in the range 0 to 2047.
     */
    if (CompareDates(&sb->volume.create_time, &unset_date_limit) > 0)
    {
        id = sb->volume_id;
        sb->volume.create_time.ds_Days = 0;
        sb->volume.create_time.ds_Minute = 0;
        sb->volume.create_time.ds_Tick = (id >> 22 ^ id >> 11 ^ id) & 0x7FF;
        D(bug("[FAT] [%s] Set hash time to %ld ticks\n", __FUNCTION__ , sb->volume.create_time.ds_Tick));
    }

    /* Get initial number of free clusters */
    sb->free_clusters = -1;
    sb->next_cluster = -1;
    if (sb->type == 32)
    {
        sb->fsinfo_block = Cache_GetBlock(sb->cache, sb->first_device_sector + AROS_LE2WORD(boot->ebpbs.ebpb32.bpb_fs_info), (UBYTE **) &fsinfo);
        if (sb->fsinfo_block != NULL)
        {
            if (fsinfo->lead_sig == AROS_LONG2LE(FSI_LEAD_SIG) && fsinfo->struct_sig == AROS_LONG2LE(FSI_STRUCT_SIG) && fsinfo->trail_sig == AROS_LONG2LE(FSI_TRAIL_SIG))
            {
                sb->free_clusters = AROS_LE2LONG(fsinfo->free_count);
                sb->next_cluster = AROS_LE2LONG(fsinfo->next_free);
                D(bug("[FAT] [%s] Valid FATFSInfo block found\n",__FUNCTION__ ));
                sb->fsinfo_buffer = fsinfo;
            } else {
                Cache_FreeBlock(sb->cache, sb->fsinfo_block);
            }
        }
    }
    if (sb->free_clusters == -1) CountFreeClusters(sb);
    if (sb->next_cluster == -1) sb->next_cluster = 2;

    

    FreeMem(boot, bsize);
    if (err != 0)
        FreeFATSuper(sb);
    else
    {
        D(bug("[FAT] [%s] FAT Filesystem successfully detected.\n",__FUNCTION__ ));
        D(bug("[FAT] [%s] Free Clusters = %ld\n", __FUNCTION__ , sb->free_clusters));
        D(bug("[FAT] [%s] Next Free Cluster = %ld\n", __FUNCTION__ , sb->next_cluster));
    }

    return err;
}

static LONG GetVolumeIdentity(struct FSSuper *sb,
    struct VolumeIdentity *volume)
{
    D(bug("----------------------------------------------------------------\n"));
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct Globals *glob = sb->glob;
    struct DirHandle dh;
    struct DirEntry de;
    LONG err;
    int i;

    D(bug("[FAT] [%s] Searching root directory for volume name\n",__FUNCTION__ ));

    /* Search the directory for the volume ID entry. It would've been nice to
     * just use GetNextDirEntry but I didn't want a flag or something to tell
     * it not to skip the volume name */
    InitDirHandle(sb, sb->rootdir_cluster, &dh, FALSE, glob);

    while ((err = GetDirEntry(&dh, dh.cur_index + 1, &de, glob)) == 0)
    {

        /* Match the volume ID entry */
        if ((de.e.entry.attr & ATTR_VOLUME_ID_MASK) == ATTR_VOLUME_ID
            && de.e.entry.name[0] != 0xe5)
        {
            D(bug("[FAT] [%s] Found volume id entry %ld\n", __FUNCTION__ , dh.cur_index));

            /* Copy the name in. 'volume->name' is a BSTR */
            volume->name[1] = de.e.entry.name[0];
            for (i = 1; i < FAT_MAX_SHORT_NAME; i++)
            {
                volume->name[i + 1] = de.e.entry.name[i];
            }

            for (i = 10; volume->name[i + 1] == ' '; i--);
            volume->name[i + 2] = '\0';
            volume->name[0] = strlen(&(volume->name[1]));

            /* Get the volume creation date too */
            ConvertFATDate(de.e.entry.create_date, de.e.entry.create_time,
                &volume->create_time, glob);

            D(bug("[FAT] [%s] Volume name is '%s'\n", __FUNCTION__ , &(volume->name[1])));

            break;
        }

        /* Bail out if we hit the end of the dir */
        if (de.e.entry.name[0] == 0x00)
        {
            D(bug("[FAT] [%s] Found end-of-directory marker, volume name entry not found\n",__FUNCTION__ ));
            err = ERROR_OBJECT_NOT_FOUND;
            break;
        }
    }

    ReleaseDirHandle(&dh, glob);
    return err;
}

LONG FormatFATVolume(const UBYTE *name, UWORD len, struct Globals *glob)
{
    D(bug("----------------------------------------------------------------\n"));
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct DosEnvec *de = BADDR(glob->fssm->fssm_Environ);
    LONG td_err;
    ULONG bsize = de->de_SizeBlock * 4;
    struct FATBootSector *boot;
    struct FATEBPB *ebpb;
    struct FATFSInfo *fsinfo;
    UWORD type, i, root_entries_count;
    struct EClockVal eclock;
    ULONG sectors_per_cluster = 0, sector_count, first_fat_sector, fat_size, root_dir_sectors, first_device_sector, temp1, temp2;

    D(bug("[FAT] [%s] Blockspertrack %ld\n",__FUNCTION__ , de->de_BlocksPerTrack));
    D(bug("[FAT] [%s] Surfaces %ld\n", __FUNCTION__ , de->de_Surfaces));
    D(bug("[FAT] [%s] Lowcyl %ld\n", __FUNCTION__ , de->de_LowCyl));
    D(bug("[FAT] [%s] Highcyl %ld\n", __FUNCTION__ , de->de_HighCyl));

    /* Decide on FAT type based on number of sectors */
    sector_count = (de->de_HighCyl - de->de_LowCyl + 1) * de->de_Surfaces * de->de_BlocksPerTrack;
    if (sector_count < 4085)
        type = 12;
    else if (sector_count < 1024 * 1024)
        type = 16;
    else
        type = 32;


    D(bug("[FAT] [%s] FAT version chosen based on total sector count = %u | FAT%2d\n",__FUNCTION__, sector_count, type ));
    D(bug("[FAT] [%s] Writing boot sector\n",__FUNCTION__ ));

    /* Decide on cluster size and root dir entries */
    first_fat_sector = 1;
    if (type == 12)
    {
        if (sector_count == 1440)
        {
            sectors_per_cluster = 2;
            root_entries_count = 112;
        }
        else if (sector_count == 2880)
        {
            sectors_per_cluster = 1;
            root_entries_count = 224;
        }
        else if (sector_count == 5760)
        {
            sectors_per_cluster = 2;
            root_entries_count = 240;
        }
        else
        {
            /* We only support some common 3.5" floppy formats */
            return ERROR_NOT_IMPLEMENTED;
        }
    }
    else if (type == 16)
    {
        for (i = 0; fat16_cluster_thresholds[i] < sector_count; i++);
        sectors_per_cluster = 1 << i;
        root_entries_count = 512;
    }
    else
    {
        for (i = 0; fat32_cluster_thresholds[i] < sector_count; i++);
        sectors_per_cluster = 8 << i;
        root_entries_count = 0;
        first_fat_sector = 32;
    }

    D(bug("[FAT] [%s] First FAT Sector = %ld\n", __FUNCTION__ , first_fat_sector));

    /* Determine FAT size */
    root_dir_sectors = (root_entries_count * 32 + (bsize - 1)) / bsize;
    temp1 = sector_count - (first_fat_sector + root_dir_sectors);
    temp2 = 256 * sectors_per_cluster + 4;
    if (type == 32)
        temp2 /= 2;
    fat_size = (temp1 + temp2 - 1) / temp2;

    boot = AllocMem(bsize, MEMF_CLEAR);
    if (!boot)
        return ERROR_NO_FREE_STORE;

    /* Install x86 infinite loop boot code to keep major OSes happy */
    boot->bs_jmp_boot[0] = 0xEB;
    boot->bs_jmp_boot[1] = 0xFE;
    boot->bs_jmp_boot[2] = 0x90;

    CopyMem(default_oem_name, boot->bs_oem_name, 8);

    boot->bpb_bytes_per_sect = AROS_WORD2LE(bsize);
    boot->bpb_sect_per_clust = sectors_per_cluster;

    boot->bpb_rsvd_sect_count = AROS_WORD2LE(first_fat_sector);

    boot->bpb_num_fats = 2;

    boot->bpb_root_entries_count = AROS_WORD2LE(root_entries_count);

    if (sector_count < 0x10000 && type != 32)
        boot->bpb_total_sectors_16 = AROS_WORD2LE(sector_count);
    else
        boot->bpb_total_sectors_32 = AROS_LONG2LE(sector_count);

    boot->bpb_media = 0xF8;

    boot->bpb_sect_per_track = AROS_WORD2LE(de->de_BlocksPerTrack);
    boot->bpb_num_heads = AROS_WORD2LE(de->de_Surfaces);
    boot->bpb_hidden_sect = AROS_LONG2LE(de->de_Reserved);

    if (type == 32)
    {
        boot->ebpbs.ebpb32.bpb_fat_size_32 = AROS_LONG2LE(fat_size);
        boot->ebpbs.ebpb32.bpb_root_cluster = AROS_LONG2LE(2);
        boot->ebpbs.ebpb32.bpb_fs_info = AROS_WORD2LE(1);
        boot->ebpbs.ebpb32.bpb_back_bootsec = AROS_WORD2LE(6);
        ebpb = &boot->ebpbs.ebpb32.ebpb;
    }
    else
    {
        boot->bpb_fat_size_16 = AROS_WORD2LE(fat_size);
        ebpb = &boot->ebpbs.ebpb;
    }

    ebpb->bs_drvnum = 0x80;
    ebpb->bs_bootsig = 0x29;

    /* Generate a pseudo-random serial number. Not the original algorithm, but it shouldn't matter */
    ReadEClock(&eclock);
    ebpb->bs_volid = FastRand(eclock.ev_lo ^ eclock.ev_hi);

    /* Copy volume name in */
    for (i = 0; i < FAT_MAX_SHORT_NAME; i++)
        if (i < len)
            ebpb->bs_vollab[i] = toupper(name[i]);
        else
            ebpb->bs_vollab[i] = ' ';

    CopyMem(default_filsystype, ebpb->bs_filsystype, 8);
    if (type != 16)
    {
        if (type == 32)
            ebpb->bs_filsystype[3] = '3';
        ebpb->bs_filsystype[4] = '2';
    }

    boot->bpb_signature[0] = 0x55;
    boot->bpb_signature[1] = 0xaa;

    /* Write the boot sector */
    first_device_sector = de->de_BlocksPerTrack * de->de_Surfaces * de->de_LowCyl;

    D(bug("[FAT] [%s] Boot sector at sector %ld\n", __FUNCTION__ , first_device_sector));
    D(bug("[FAT] [%s] Calculated Total Sectors %ld\n", __FUNCTION__ , sector_count));
    D(bug("[FAT] [%s] SectorSize = %ld\n", __FUNCTION__ , bsize));
    D(bug("[FAT] [%s] SectorsPerCluster = %ld\n",__FUNCTION__ , sectors_per_cluster));
    D(bug("[FAT] [%s] First FAT Sector = %ld\n", __FUNCTION__ , first_fat_sector));
    D(bug("[FAT] [%s] FAT Size = %ld\n", __FUNCTION__ , fat_size));
    D(bug("[FAT] [%s] Total Sectors = %ld\n", __FUNCTION__ , sector_count));

    if ((td_err = AccessDisk(TRUE, first_device_sector, 1, bsize, (UBYTE *) boot, glob)) != 0)
    {
        D(bug("[FAT] [%s] Couldn't write boot block (%ld)\n", __FUNCTION__ , td_err));
        FreeMem(boot, bsize);
        return ERROR_UNKNOWN;
    }

    /* Write back-up boot sector and FS info sector */
    if (type == 32)
    {
        if ((td_err = AccessDisk(TRUE, first_device_sector + 6, 1, bsize,
            (UBYTE *) boot, glob)) != 0)
        {
            D(bug("[FAT] [%s] Couldn't write back-up boot block (%ld)\n", __FUNCTION__ , td_err));
            FreeMem(boot, bsize);
            return ERROR_UNKNOWN;
        }

        fsinfo = (APTR) boot;
        SetMem(fsinfo, 0, bsize);

        fsinfo->lead_sig = AROS_LONG2LE(FSI_LEAD_SIG);
        fsinfo->struct_sig = AROS_LONG2LE(FSI_STRUCT_SIG);
        fsinfo->trail_sig = AROS_LONG2LE(FSI_TRAIL_SIG);
        fsinfo->free_count = AROS_LONG2LE(0xFFFFFFFF);
        fsinfo->next_free = AROS_LONG2LE(0xFFFFFFFF);

        if ((td_err = AccessDisk(TRUE, first_device_sector + 1, 1, bsize,
            (UBYTE *) fsinfo, glob)) != 0)
        {
            D(bug("[FAT] [%s] Couldn't write back-up boot block (%ld)\n", __FUNCTION__ , td_err));
            FreeMem(boot, bsize);
            return ERROR_UNKNOWN;
        }
    }

    FreeMem(boot, bsize);

    glob->formatting = TRUE;

    return 0;
}

LONG SetVolumeName(struct FSSuper *sb, UBYTE *name, UWORD len)
{
    D(bug("----------------------------------------------------------------\n"));
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct Globals *glob = sb->glob;
    struct DirHandle dh;
    struct DirEntry de;
    LONG err, td_err;
    int i;
    struct DosEnvec *dosenv = BADDR(glob->fssm->fssm_Environ);
    ULONG bsize = dosenv->de_SizeBlock * 4;
    struct FATBootSector *boot;

    /* Truncate name if necessary */
    if (len > FAT_MAX_SHORT_NAME)
        len = FAT_MAX_SHORT_NAME;

    /* Read boot block */
    boot = AllocMem(bsize, MEMF_ANY);
    if (!boot)
        return ERROR_NO_FREE_STORE;

    if ((td_err = AccessDisk(FALSE, sb->first_device_sector, 1, bsize, (UBYTE *) boot, glob)) != 0)
    {
        D(bug("[FAT] [%s] Couldn't read boot block (%ld)\n", __FUNCTION__ , td_err));
        FreeMem(boot, bsize);
        return ERROR_UNKNOWN;
    }

    D(bug("[FAT] [%s] Searching root directory for volume name\n", __FUNCTION__ ));

    /* Search the directory for the volume ID entry. It would've been nice to
     * just use GetNextDirEntry but I didn't want a flag or something to tell
     * it not to skip the volume name */
    InitDirHandle(sb, 0, &dh, FALSE, glob);

    while ((err = GetDirEntry(&dh, dh.cur_index + 1, &de, glob)) == 0)
    {

        /* Match the volume ID entry */
        if ((de.e.entry.attr & ATTR_VOLUME_ID_MASK) == ATTR_VOLUME_ID
            && de.e.entry.name[0] != 0xe5)
        {
            D(bug("[FAT] [%s] Found volume id entry %ld\n", __FUNCTION__ , dh.cur_index));
            err = 0;
            break;
        }

        /* Bail out if we hit the end of the dir */
        if (de.e.entry.name[0] == 0x00)
        {
            D(bug("[FAT] [%s] Found end-of-directory marker, volume name entry not found\n",__FUNCTION__ ));
            err = ERROR_OBJECT_NOT_FOUND;
            break;
        }
    }

    /* Create a new volume ID entry if there wasn't one */
    if (err != 0)
    {
        err = AllocDirEntry(&dh, 0, &de, glob);
        if (err == 0)
            FillDirEntry(&de, ATTR_VOLUME_ID, 0, glob);
    }

    /* Copy the name in */
    if (err == 0)
    {
        for (i = 0; i < FAT_MAX_SHORT_NAME; i++)
            if (i < len)
                de.e.entry.name[i] = toupper(name[i]);
            else
                de.e.entry.name[i] = ' ';

        if ((err = UpdateDirEntry(&de, glob)) != 0)
        {
            D(bug("[FAT] [%s] Couldn't change volume name\n",__FUNCTION__ ));
            return err;
        }
    }

    /* Copy name to boot block as well, and save */
    if (sb->type == 32)
        CopyMem(de.e.entry.name, boot->ebpbs.ebpb32.ebpb.bs_vollab,
            FAT_MAX_SHORT_NAME);
    else
        CopyMem(de.e.entry.name, boot->ebpbs.ebpb.bs_vollab,
            FAT_MAX_SHORT_NAME);

    if ((td_err = AccessDisk(TRUE, sb->first_device_sector, 1, bsize,
        (UBYTE *) boot, glob)) != 0)
        D(bug("[FAT] [%s] Couldn't write boot block (%ld)\n", __FUNCTION__ , td_err));
    FreeMem(boot, bsize);

    /* Update name in SB */
    sb->volume.name[0] = len;
    sb->volume.name[1] = name[0];
    for (i = 1; i < len; i++)
        sb->volume.name[i + 1] = name[i];
    sb->volume.name[len + 1] = '\0';

    D(bug("[FAT] [%s] New volume name is '%s'\n", __FUNCTION__ , &(sb->volume.name[1])));

    ReleaseDirHandle(&dh, glob);
    return err;
}

static void FreeFATSuper(struct FSSuper *sb)
{
    D(bug("----------------------------------------------------------------\n"));
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct Globals *glob = sb->glob;
    D(bug("[FAT] [%s] Removing Super Block from memory\n",__FUNCTION__ ));
    Cache_DestroyCache(sb->cache);
    D(bug("[FAT] [%s] Removing FAT Buffers\n",__FUNCTION__ ));
    FreeVecPooled(glob->mempool, sb->fat_buffers);
    sb->fat_buffers = NULL;
    D(bug("[FAT] [%s] Removing FAT Blocks\n",__FUNCTION__ ));
    FreeVecPooled(glob->mempool, sb->fat_blocks);
    sb->fat_blocks = NULL;
    D(bug("[FAT] [%s] Done clearing memory\n",__FUNCTION__ ));
}

void FillDiskInfo(struct InfoData *id, struct Globals *glob)
{
    //D(bug("----------------------------------------------------------------\n"));
    //D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct DosEnvec *de = BADDR(glob->fssm->fssm_Environ);

    id->id_NumSoftErrors = 0;
    id->id_UnitNumber = glob->fssm->fssm_Unit;
    id->id_DiskState = ID_VALIDATED;

    if (glob->sb)
    {
        id->id_NumBlocks = glob->sb->total_sectors;
        id->id_NumBlocksUsed = glob->sb->total_sectors - (glob->sb->free_clusters << glob->sb->cluster_sectors_bits);
        id->id_BytesPerBlock = glob->sb->sectorsize;

        id->id_DiskType = ID_DOS_DISK;

        id->id_VolumeNode = MKBADDR(glob->sb->doslist);
        id->id_InUse = (IsListEmpty(&glob->sb->info->locks) && IsListEmpty(&glob->sb->info->notifies)) ? DOSFALSE : DOSTRUE;
    }

    else
    {
        id->id_NumBlocks = de->de_Surfaces * de->de_BlocksPerTrack
            * (de->de_HighCyl + 1 - de->de_LowCyl) / de->de_SectorPerBlock;
        id->id_NumBlocksUsed = id->id_NumBlocks;
        id->id_BytesPerBlock = de->de_SizeBlock << 2;

        id->id_DiskState = ID_VALIDATED;

        if (glob->disk_inhibited != 0)
            id->id_DiskType = ID_BUSY;
        else if (glob->disk_inserted)
            id->id_DiskType = ID_NOT_REALLY_DOS;        //ID_UNREADABLE_DISK;
        else
            id->id_DiskType = ID_NO_DISK_PRESENT;

        id->id_VolumeNode = BNULL;
        id->id_InUse = DOSFALSE;
    }
}

static void SendVolumePacket(struct DosList *vol, ULONG action, struct Globals *glob)
{
    //D(bug("----------------------------------------------------------------\n"));
    D(bug("[FAT] [%s] ACTION_DISK_CHANGE\n",__FUNCTION__ ));

    struct DosPacket *dospacket;

    dospacket = AllocDosObject(DOS_STDPKT, TAG_DONE);
    dospacket->dp_Type = ACTION_DISK_CHANGE;
    dospacket->dp_Arg1 = ID_FAT_DISK;
    dospacket->dp_Arg2 = (IPTR) vol;
    dospacket->dp_Arg3 = action;
    dospacket->dp_Port = NULL;

    PutMsg(glob->ourport, dospacket->dp_Link);
}

void DoDiskInsert(struct Globals *glob)
{
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct FSSuper *sb;
    ULONG err;
    struct DosList *dl;
    struct VolumeInfo *vol_info = NULL;
    struct GlobalLock *global_lock;
    struct ExtFileLock *ext_lock;
    struct MinNode *lock_node;
    APTR pool;
    struct NotifyNode *nn;
    struct DosList *newvol = NULL;

    struct DriveGeometry geometry;
    struct FileSysStartupMsg *fssm = glob->fssm;
    struct DosEnvec *de = BADDR(fssm->fssm_Environ);

    D(bug("\n[FAT] [%s] Current glob->fssm->fssm_Environ (DOSENVEC):\n",__FUNCTION__));
    D(bug("[FAT] [%s] de_SizeBlock      : %d\n",__FUNCTION__ , de->de_SizeBlock));
    D(bug("[FAT] [%s] de_Surfaces       : %d\n",__FUNCTION__ , de->de_Surfaces));
    D(bug("[FAT] [%s] de_BlocksPerTrack : %d\n",__FUNCTION__ , de->de_BlocksPerTrack));
    D(bug("[FAT] [%s] dg_LowCyl         : %d\n",__FUNCTION__ , de->de_LowCyl));
    D(bug("[FAT] [%s] dg_HighCyl        : %d\n\n",__FUNCTION__ , de->de_HighCyl));
    
    // To Support hot-swap for FAT32 we need to reset DosEnvec based on Inserted Disk properties
    // First we obtain SizeBlock, Surfaces and BlocksPerTrack directly from TD_GETGEOMETRY query to device driver
    glob->diskioreq->iotd_Req.io_Data = &geometry;
    glob->diskioreq->iotd_Req.io_Command = TD_GETGEOMETRY;
    glob->diskioreq->iotd_Req.io_Length = sizeof(struct DriveGeometry);
    DoIO((struct IORequest *)glob->diskioreq);

    D(bug("\n[FAT] [%s] Disk geometry received from Device Driver:\n",__FUNCTION__ ));
    D(bug("[FAT] [%s] dg_SectorSize    : %d\n",__FUNCTION__ , geometry.dg_SectorSize));
    D(bug("[FAT] [%s] dg_TotalSectors  : %d\n",__FUNCTION__ , geometry.dg_TotalSectors));
    D(bug("[FAT] [%s] dg_Cylinders     : %d\n",__FUNCTION__ , geometry.dg_Cylinders));
    D(bug("[FAT] [%s] dg_CylSectors    : %d\n",__FUNCTION__ , geometry.dg_CylSectors));
    D(bug("[FAT] [%s] dg_Heads         : %d\n",__FUNCTION__ , geometry.dg_Heads));
    D(bug("[FAT] [%s] dg_TrackSectors  : %d\n",__FUNCTION__ , geometry.dg_TrackSectors));
    
    de->de_SizeBlock        = geometry.dg_SectorSize >> 2;                                  // Sizeblock = #LONG p/Sector = SectorSize/4 = BitShift 2
    de->de_Surfaces         = geometry.dg_Heads;                                            // Surfaces = #Heads
    de->de_BlocksPerTrack   = geometry.dg_TrackSectors;                                     // Block p/Track = #Sectors

    // First we check if there is a disk inserted (Sector Size == 512) or no disk is inserted (Sector Size == 0)
    if (geometry.dg_SectorSize == 0)
    {
        D(bug("[FAT] [%s] No Disk Inserted - Device is ready for insertion of FAT Disk\n",__FUNCTION__ ));
        return;
    }

    // Then we check if disk inserted is a Amiga Partitioned RDB Disk (BYTE 0x200=512 = Sector#1/LONG#0x0 | ID=0x5244534b=RDSK). IN the case of a hybrid disk (RDB + MBR) we give priority to RDB
    // if RDB is found then we cannot do anything and we present user with a requester to inform that inserted disk is Amiga RDB and a reboot is required to perform DosBoot process to Mount RDB  
    
    UBYTE read_buffer[0x200];

    err = RawDiskSectorRead(1, geometry.dg_SectorSize, (UBYTE*)&read_buffer[0], glob);
    if (err)
    {
        D(bug("[FAT] [%s] Read Error = %d on BootSector #1 (RDB Check)\n",__FUNCTION__, err ));
        return;
    } else {
        ULONG RDB_ID = ((read_buffer[0]<<24) + (read_buffer[1]<<16) + (read_buffer[2]<<8) + read_buffer[3]);
        D(kprintf("[FAT] [%s] RDB = %4d|%08x\n", __FUNCTION__, 0x200, RDB_ID)); 
        if(RDB_ID == 0x5244534b)
        {
            glob->diskioreq->iotd_Req.io_Command = 0xffff;
            glob->diskioreq->iotd_Req.io_Length = 1;
            DoIO((struct IORequest *)glob->diskioreq);
            return;
        }
    }

    // Now we check if disk inserted is a PC Partitoned MBR Disk (BYTE 0x1fe=510 = Sector#0/WORD#0x1fe | ID=0x55aa=MBR)
    // if MBR is found then we read MBR-PART-1 to obtain start sector and total sector count for first partition (which gives us LowCyl and HighCyl) and from there we try to Mount
    err = RawDiskSectorRead(0, geometry.dg_SectorSize, (UBYTE*)&read_buffer[0], glob);
    if (err)
    {
        D(bug("[FAT] [%s] Read Error on BootSector #0 (MBR Check)\n",__FUNCTION__ ));
        return;
    }

    UWORD MBR_ID = (read_buffer[0x1fe]<<8) + read_buffer[0x1ff];
    D(kprintf("[FAT] [%s] MBR = %4d|%08x\n", __FUNCTION__, 0x1fe, MBR_ID));
    if(MBR_ID != 0x55aa)
    {
        D(bug("[FAT] [%s] No RDB or MBR Boot Record found\n",__FUNCTION__ ));
        return;
    }
    
    ULONG MDB_PART1_START = (read_buffer[0x1be + 0x8])+(read_buffer[0x1be + 0x9]<<8)+(read_buffer[0x1be + 0xa]<<16)+(read_buffer[0x1be + 0xb]<<24);
    ULONG MDB_PART1_TOTAL = (read_buffer[0x1be + 0xc])+(read_buffer[0x1be + 0xd]<<8)+(read_buffer[0x1be + 0xe]<<16)+(read_buffer[0x1be + 0xf]<<24);
    ULONG MDB_PART2_START = (read_buffer[0x1ce + 0x8])+(read_buffer[0x1ce + 0x9]<<8)+(read_buffer[0x1ce + 0xa]<<16)+(read_buffer[0x1ce + 0xb]<<24);
    ULONG MDB_PART2_TOTAL = (read_buffer[0x1ce + 0xc])+(read_buffer[0x1ce + 0xd]<<8)+(read_buffer[0x1ce + 0xe]<<16)+(read_buffer[0x1ce + 0xf]<<24);
    ULONG MDB_PART3_START = (read_buffer[0x1de + 0x8])+(read_buffer[0x1de + 0x9]<<8)+(read_buffer[0x1de + 0xa]<<16)+(read_buffer[0x1de + 0xb]<<24);
    ULONG MDB_PART3_TOTAL = (read_buffer[0x1de + 0xc])+(read_buffer[0x1de + 0xd]<<8)+(read_buffer[0x1de + 0xe]<<16)+(read_buffer[0x1de + 0xf]<<24);
    ULONG MDB_PART4_START = (read_buffer[0x1ee + 0x8])+(read_buffer[0x1ee + 0x9]<<8)+(read_buffer[0x1ee + 0xa]<<16)+(read_buffer[0x1ee + 0xb]<<24);
    ULONG MDB_PART4_TOTAL = (read_buffer[0x1ee + 0xc])+(read_buffer[0x1ee + 0xd]<<8)+(read_buffer[0x1ee + 0xe]<<16)+(read_buffer[0x1ee + 0xf]<<24);
                
    D(kprintf("\tPART#1-LBA-Start = %4d|%08x\n", 0x1be + 0x8, MDB_PART1_START));
    D(kprintf("\tPART#1-LBA-Total = %4d|%08x\n", 0x1be + 0xc, MDB_PART1_TOTAL));
    D(kprintf("\tPART#2-LBA-Start = %4d|%08x\n", 0x1ce + 0x8, MDB_PART2_START));
    D(kprintf("\tPART#2-LBA-Total = %4d|%08x\n", 0x1ce + 0xc, MDB_PART2_TOTAL));
    D(kprintf("\tPART#3-LBA-Start = %4d|%08x\n", 0x1de + 0x8, MDB_PART3_START));
    D(kprintf("\tPART#3-LBA-Total = %4d|%08x\n", 0x1de + 0xc, MDB_PART3_TOTAL));
    D(kprintf("\tPART#4-LBA-Start = %4d|%08x\n", 0x1ee + 0x8, MDB_PART4_START));
    D(kprintf("\tPART#4-LBA-Total = %4d|%08x\n\n", 0x1ee + 0xc, MDB_PART4_TOTAL));

    //Final step is to translate MDB_PART1_START LBA addresses to a valid LowCyl and with MDB_PART1_TOTAL also a valid HighCyl
    //LBA = Surfaces * BlocksPerTrack * Cylinders or Cylinders = LBA / Surfaces / BlocksPerTrack
    //In most cases the default values of Surfaces = 16 and BlocksPerTrack = 64 will work, but for smaller SD this may fail because Cylinder must be an integer and >0
    
    if ((MDB_PART1_START % de->de_BlocksPerTrack % de->de_Surfaces) == 0)
    {
        de->de_LowCyl           = MDB_PART1_START / de->de_BlocksPerTrack / de->de_Surfaces ;                      // Read ULONG from MBR-PART1 = 0x1BE + 0x8
        de->de_HighCyl          = MDB_PART1_TOTAL / de->de_BlocksPerTrack / de->de_Surfaces + de->de_LowCyl;      // Read ULONG from MBR-PART1 = 0x1BE + 0xC
    } else {
        de->de_Surfaces         = 1;
        de->de_BlocksPerTrack   = 1;
        de->de_LowCyl           = MDB_PART1_START;
        de->de_HighCyl          = MDB_PART1_TOTAL;
    }

    D(bug("\n[FAT] [%s] Drive Geometry transfer to glob->fssm->fssm_Environ:\n",__FUNCTION__ ));
    D(bug("[FAT] [%s] de_SizeBlock      : %d\n",__FUNCTION__ , de->de_SizeBlock));
    D(bug("[FAT] [%s] de_Surfaces       : %d\n",__FUNCTION__ , de->de_Surfaces));
    D(bug("[FAT] [%s] de_BlocksPerTrack : %d\n",__FUNCTION__ , de->de_BlocksPerTrack));
    D(bug("[FAT] [%s] de_LowCyl         : %d\n",__FUNCTION__ , de->de_LowCyl));
    D(bug("[FAT] [%s] de_HighCyl        : %d\n\n",__FUNCTION__ , de->de_HighCyl));

    if (glob->sb == NULL && (sb = AllocVecPooled(glob->mempool, sizeof(struct FSSuper))))
    {
        SetMem(sb, 0, sizeof(struct FSSuper));

        sb->glob = glob;
        err = ReadFATSuper(sb);

        D(bug("[FAT] [%s] ReadFatSuper Result = %u\n", __FUNCTION__, err));

        if (err == 0)
        {
            D(bug("[FAT] [%s] Scan volume list for a matching volume\n", __FUNCTION__)); // would be better to match by serial number)

            D(bug("[FAT] [%s] LockDosList\n", __FUNCTION__));
            dl = AttemptLockDosList(LDF_VOLUMES | LDF_WRITE);
            
            D(bug("[FAT] [%s] FindDosEntry\n", __FUNCTION__));
            dl = FindDosEntry(dl, sb->volume.name + 1, LDF_VOLUMES | LDF_WRITE);
            
            D(bug("[FAT] [%s] UnLockDosList\n", __FUNCTION__));
            UnLockDosList(LDF_VOLUMES | LDF_WRITE);

            if (dl != NULL && CompareDates(&dl->dol_misc.dol_volume.dol_VolumeDate, &sb->volume.create_time) != 0)
                dl = NULL;

            if (dl != NULL)
            {
                dl->dol_Task = glob->ourport;
                sb->doslist = dl;

                D(bug("[FAT] [%s] Found old volume.\n", __FUNCTION__ ));

                vol_info = BADDR(dl->dol_misc.dol_volume.dol_LockList);

#if 0    /* No point until we match volumes by serial number */
                /* Update name */
#ifdef AROS_FAST_BPTR
                /* ReadFATSuper() sets a null byte after the string, so this should be fine */
                CopyMem(sb->volume.name + 1, dl->dol_Name, sb->volume.name[0] + 1);
#else
                CopyMem(sb->volume.name, dl->dol_Name, sb->volume.name[0] + 2);
#endif
#endif

                /* Patch locks and notifications to match this handler
                 * instance */
                ForeachNode(&vol_info->locks, global_lock)
                {
                    ForeachNode(&global_lock->locks, lock_node)
                    {
                        ext_lock = LOCKFROMNODE(lock_node);
                        D(bug("[FAT] [%s] Patching adopted lock %p. old port = %p, new port = %p\n",
                            __FUNCTION__ , ext_lock, ext_lock->fl_Task, glob->ourport));
                        ext_lock->fl_Task = glob->ourport;
                        ext_lock->sb = sb;
                        ext_lock->ioh.sb = sb;
                    }
                }

                ForeachNode(&vol_info->root_lock.locks, lock_node)
                {
                    ext_lock = LOCKFROMNODE(lock_node);
                    D(bug("[FAT] [%s] Patching adopted ROOT lock %p. old port = %p, new port = %p\n",
                          __FUNCTION__ , ext_lock, ext_lock->fl_Task, glob->ourport));
                    ext_lock->fl_Task = glob->ourport;
                    ext_lock->sb = sb;
                    ext_lock->ioh.sb = sb;
                }

                ForeachNode(&vol_info->notifies, nn)
                    nn->nr->nr_Handler = glob->ourport;
            }
            else
            {
                D(bug("[FAT] [%s] Creating new volume.\n",__FUNCTION__ ));

                /* Create transferable core volume info */
                pool = CreatePool(MEMF_PUBLIC, DEF_POOL_SIZE, DEF_POOL_THRESHOLD);
                if (pool != NULL)
                {
                    vol_info = AllocVecPooled(pool, sizeof(struct VolumeInfo));
                    if (vol_info != NULL)
                    {
                        vol_info->mem_pool = pool;
                        vol_info->id = sb->volume_id;
                        NEWLIST(&vol_info->locks);
                        NEWLIST(&vol_info->notifies);

                        vol_info->root_lock.dir_cluster = FAT_ROOTDIR_MARK;
                        vol_info->root_lock.dir_entry = FAT_ROOTDIR_MARK;
                        vol_info->root_lock.access = SHARED_LOCK;
                        vol_info->root_lock.first_cluster = 0;
                        vol_info->root_lock.attr = ATTR_DIRECTORY;
                        vol_info->root_lock.size = 0;
                        CopyMem(sb->volume.name, vol_info->root_lock.name, sb->volume.name[0] + 1);
                        NEWLIST(&vol_info->root_lock.locks);
                    }

                    if ((newvol = AllocVecPooled(pool, sizeof(struct DosList))))
                    {
                        newvol->dol_Next = BNULL;
                        newvol->dol_Type = DLT_VOLUME;
                        newvol->dol_Task = glob->ourport;
                        newvol->dol_Lock = BNULL;

                        CopyMem(&sb->volume.create_time, &newvol->dol_misc.dol_volume.dol_VolumeDate, sizeof(struct DateStamp));

                        newvol->dol_misc.dol_volume.dol_LockList = MKBADDR(vol_info);

                        newvol->dol_misc.dol_volume.dol_DiskType =
                            (sb->type == 12) ? ID_FAT12_DISK :
                            (sb->type == 16) ? ID_FAT16_DISK :
                            (sb->type == 32) ? ID_FAT32_DISK :
                            ID_FAT12_DISK;

                        if ((newvol->dol_Name = MKBADDR(AllocVecPooled(pool, FAT_MAX_SHORT_NAME + 2))))
                        {
#ifdef AROS_FAST_BPTR
                            /* ReadFATSuper() sets a null byte after the
                             * string, so this should be fine */
                            CopyMem(sb->volume.name + 1, newvol->dol_Name,
                                sb->volume.name[0] + 1);
#else
                            CopyMem(sb->volume.name,
                                BADDR(newvol->dol_Name),
                                sb->volume.name[0] + 2);
#endif

                            sb->doslist = newvol;
                        }
                    }
                    if (vol_info == NULL || newvol == NULL)
                        DeletePool(pool);
                }
            }

            sb->info = vol_info;
            glob->last_num = -1;

            if (dl != NULL) SendEvent(IECLASS_DISKINSERTED, glob);
            else SendVolumePacket(newvol, ACTION_VOLUME_ADD, glob);
            D(bug("[FAT] [%s] Disk successfully initialised\n",__FUNCTION__ ));
            return;
        } 
        FreeVecPooled(glob->mempool, sb);
    }

    glob->diskioreq->iotd_Req.io_Command = TD_EJECT;
    glob->diskioreq->iotd_Req.io_Length = 1;
    DoIO((struct IORequest *)glob->diskioreq);

    return;
}

BOOL AttemptDestroyVolume(struct FSSuper *sb)
{
    D(bug("[FAT] [%s] Start\n",__FUNCTION__ ));

    struct Globals *glob = sb->glob;
    BOOL destroyed = FALSE;

    D(bug("[FAT] [%s] Attempting to destroy volume\n",__FUNCTION__ ));

    /* Check if the volume can be removed */
    if (IsListEmpty(&sb->info->locks) && IsListEmpty(&sb->info->notifies))
    {
        D(bug("[FAT] [%s] Removing volume completely\n", __FUNCTION__ ));

        if (sb == glob->sb)
            glob->sb = NULL;
        else
            Remove((struct Node *)sb);

        SendVolumePacket(sb->doslist, ACTION_VOLUME_REMOVE, glob);

        FreeFATSuper(sb);

        D(bug("[FAT] [%s] FreeFATSuper finished\n", __FUNCTION__ ));

        FreeVecPooled(glob->mempool, sb);

        D(bug("[FAT] [%s] SuperBlock Freed completely\n", __FUNCTION__ ));

        destroyed = TRUE;
    } else {
        D(bug("[FAT] [%s] Volume NOT REMOVED as it has Locks and/or Notifications pending\n",__FUNCTION__ ));
    }

    return destroyed;
}

void DoDiskRemove(struct Globals *glob)
{
    D(bug("[FAT] [%s] %s\n",__FUNCTION__, AROS_BSTR_ADDR(glob->sb->doslist->dol_Name) ));

    if (glob->sb)
    {
        struct FSSuper *sb = glob->sb;

        if (!AttemptDestroyVolume(sb))
        {
            sb->doslist->dol_Task = NULL;
            glob->sb = NULL;
            D(bug("[FAT] [%s] Moved in-mem SB block to spare, waiting for locks and notifications to be freed\n",__FUNCTION__ ));
            AddTail((struct List *)&glob->sblist, (struct Node *)sb);
            SendEvent(IECLASS_DISKREMOVED, glob);
        }
    }
}
