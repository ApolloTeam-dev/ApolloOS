/*
    Copyright � 2004-2020, The AROS Development Team. All rights reserved
    $Id$

    Desc:
    Lang: English
*/

#include <aros/debug.h>

#include <proto/exec.h>

/* We want all other bases obtained from our base */
#define __NOLIBBASE__

#include <proto/timer.h>
#include <proto/bootloader.h>
#include <proto/expansion.h>
#include <proto/oop.h>

#include <aros/bootloader.h>
#include <aros/symbolsets.h>
#include <exec/exec.h>
#include <exec/resident.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <hidd/hidd.h>
#include <hidd/bus.h>
#include <hidd/storage.h>
#include <utility/utility.h>
#include <libraries/expansion.h>
#include <libraries/configvars.h>
#include <dos/bptr.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <string.h>

#include "ata.h"
#include "timer.h"

#include LC_LIBDEFS_FILE

/* Add a bootnode using expansion.library */
BOOL ata_RegisterVolume(ULONG StartCyl, ULONG EndCyl, struct ata_Unit *unit)
{
    struct ExpansionBase *ExpansionBase;
    struct DeviceNode *devnode;
    TEXT dosdevname[7] = "DHROM0";
    const ULONG IdDOS = AROS_MAKE_ID('D','O','S','\000');
    const ULONG IdCDVD = AROS_MAKE_ID('C','D','F','S');         // Opticals are all using CDFS

    ExpansionBase = (struct ExpansionBase *)TaggedOpenLibrary(TAGGEDOPEN_EXPANSION);
    if (ExpansionBase)
    {
        IPTR pp[24];

        /* This should be dealt with using some sort of volume manager or such. */
        switch (unit->au_DevType)
        {
            case DG_DIRECT_ACCESS:
                break;
            case DG_CDROM:
                dosdevname[0] = 'C';
                dosdevname[1] = 'D';
                break;
            default:
                DATAINIT(bug("[ATA>>]:-ata_RegisterVolume called on unknown devicetype\n"));
        }

        if (unit->au_UnitNum < 10) dosdevname[5] += unit->au_UnitNum % 10;
        else dosdevname[5] = 'A' - 10 + unit->au_UnitNum;
    
        pp[0] 		            = (IPTR)dosdevname;
        pp[1]		            = (IPTR)MOD_NAME_STRING;
        pp[2]		            = unit->au_UnitNum;
        pp[DE_TABLESIZE    + 4] = DE_BOOTBLOCKS;

        pp[DE_SIZEBLOCK    + 4] = 1 << (unit->au_SectorShift - 2);          // LONG (32-bit) per Sector = (#Bytes/Sector)/4 = SectorSift-2 
        pp[DE_NUMHEADS     + 4] = unit->au_Heads;
        pp[DE_SECSPERBLOCK + 4] = 1;
        pp[DE_BLKSPERTRACK + 4] = unit->au_Sectors;
        pp[DE_RESERVEDBLKS + 4] = 2;
        pp[DE_LOWCYL       + 4] = StartCyl;
        pp[DE_HIGHCYL      + 4] = EndCyl;
        pp[DE_NUMBUFFERS   + 4] = 10;
        pp[DE_BUFMEMTYPE   + 4] = MEMF_PUBLIC | MEMF_31BIT;
        pp[DE_MAXTRANSFER  + 4] = 0x1FE00;
        pp[DE_MASK         + 4] = 0x7FFFFFFE;
        pp[DE_BOOTPRI      + 4] = ((unit->au_DevType == DG_DIRECT_ACCESS) ? 0 : 10);
        pp[DE_DOSTYPE      + 4] = ((unit->au_DevType == DG_DIRECT_ACCESS) ? IdDOS : IdCDVD);
        pp[DE_CONTROL      + 4] = 0;
        pp[DE_BOOTBLOCKS   + 4] = 2;
    
        devnode = MakeDosNode(pp);

        if (devnode)
        {
            DATAINIT(bug("[ATA>>]:-ata_RegisterVolume=%b, DosType=0x%08lx | LowCyl=%d | HighCyl=%d | Heads=%d | Blocksize=%d | BlockPerTrack=%d .. ",
                devnode->dn_Name, pp[DE_DOSTYPE + 4], StartCyl, EndCyl, unit->au_Heads, 1 << unit->au_SectorShift, unit->au_Sectors));

            if (unit->au_DevType == DG_DIRECT_ACCESS) // && ((unit->au_Flags & AF_Removable) == 0))
            {
                AddBootNode(pp[DE_BOOTPRI + 4], ADNF_STARTPROC, devnode, NULL);
                DATAINIT(bug("BootNode (Direct Access Medium)\n"));
            } else {
                DATAINIT(bug("DosNode (CD/DVD or other Non-Direct Access Medium)\n"));
            }

            return TRUE;
        }
        CloseLibrary((struct Library *)ExpansionBase);
    }
    return FALSE;
}

#if defined(__OOP_NOATTRBASES__)
/* Keep order the same as order of IDs in struct ataBase! */
static CONST_STRPTR const attrBaseIDs[] =
{
    IID_Hidd_ATAUnit,
    IID_HW,
    IID_Hidd_Bus,
    IID_Hidd_ATABus,
    IID_Hidd_StorageUnit,
    NULL
};
#endif

#if defined(__OOP_NOMETHODBASES__)
static CONST_STRPTR const methBaseIDs[] =
{
    IID_HW,
    IID_Hidd_ATABus,
    IID_Hidd_StorageController,
    NULL
};
#endif

static int ATA_init(struct ataBase *ATABase)
{
    struct BootLoaderBase	*BootLoaderBase;

    DATAINIT(bug("[ATA--] %s: ata.device Initialization\n", __func__));

    /* Prepare the list of detected controllers */
    NEWLIST(&ATABase->ata_Controllers);

    /* Set default ata.device config options */
    ATABase->ata_32bit   = FALSE;
    ATABase->ata_NoMulti = FALSE;
    ATABase->ata_NoDMA   = FALSE;
    ATABase->ata_Poll    = FALSE;

    /*
     * start initialization: 
     * obtain kernel parameters
     */
    BootLoaderBase = OpenResource("bootloader.resource");
    DATAINIT(bug("[ATA--] %s: BootloaderBase = %p\n", __func__, BootLoaderBase));
    if (BootLoaderBase != NULL)
    {
        struct List *list;
        struct Node *node;

        list = (struct List *)GetBootInfo(BL_Args);
        if (list)
        {
            ForeachNode(list, node)
            {
                if (strncmp(node->ln_Name, "ATA=", 4) == 0)
                {
                    const char *CmdLine = &node->ln_Name[4];

                    if (strstr(CmdLine, "disable"))
                    {
                        DATAINIT(bug("[ATA  ] %s: Disabling ATA support\n", __func__));
                        return FALSE;
                    }
                    if (strstr(CmdLine, "32bit"))
                    {
                        DATAINIT(bug("[ATA  ] %s: Using 32-bit IO transfers\n", __func__));
                        ATABase->ata_32bit = TRUE;
                    }
                    if (strstr(CmdLine, "nomulti"))
                    {
                        DATAINIT(bug("[ATA  ] %s: Disabled multisector transfers\n", __func__));
                        ATABase->ata_NoMulti = TRUE;
                    }
                    if (strstr(CmdLine, "nodma"))
                    {
                        DATAINIT(bug("[ATA  ] %s: Disabled DMA transfers\n", __func__));
                        ATABase->ata_NoDMA = TRUE;
                    }
                    if (strstr(CmdLine, "poll"))
                    {
                        DATAINIT(bug("[ATA  ] %s: Using polling to detect end of busy state\n", __func__));
                        ATABase->ata_Poll = TRUE;
                    }
                }
            }
        }
    }

    ATABase->ata_UtilityBase = TaggedOpenLibrary(TAGGEDOPEN_UTILITY);
    if (!ATABase->ata_UtilityBase)
    {
        bug("[ATA--] %s: Failed to open utility.library v36\n", __func__);
        return FALSE;
    }
    /*
     * I've decided to use memory pools again. Alloc everything needed from 
     * a pool, so that we avoid memory fragmentation.
     */
    ATABase->ata_MemPool = CreatePool(MEMF_CLEAR | MEMF_PUBLIC | MEMF_SEM_PROTECTED , 8192, 4096);
    if (ATABase->ata_MemPool == NULL)
    {
        bug("[ATA--] %s: Failed to Allocate MemPool!\n", __func__);
        return FALSE;
    }

    DATAINIT(bug("[ATA--] %s: MemPool @ %p\n", __func__, ATABase->ata_MemPool));

    /*
     * ata drive cache memory allocation
     */
    ATABase->ata_CacheData = AllocMem(CACHE_SIZE*512, MEMF_CLEAR | MEMF_PUBLIC);
    if (ATABase->ata_CacheData == NULL)
    {
        bug("[ATA--] %s: Failed to Allocate CacheData!\n", __func__);
        return FALSE;
    }

    DATAINIT(bug("[ATA--] %s: CacheData @ %p\n", __func__, ATABase->ata_CacheData));

    ATABase->ata_CacheTags = AllocMem(CACHE_SIZE*8, MEMF_CLEAR | MEMF_PUBLIC);
    if (ATABase->ata_CacheTags == NULL)
    {
        bug("[ATA--] %s: Failed to Allocate CacheTags!\n", __func__);
        return FALSE;
    }
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        ATABase->ata_CacheTags[i] = 0xfffffffffffffffful;
    }

    DATAINIT(bug("[ATA--] %s: CacheTags @ %p\n", __func__, ATABase->ata_CacheTags));

#if defined(__OOP_NOATTRBASES__)
    if (OOP_ObtainAttrBasesArray(&ATABase->unitAttrBase, attrBaseIDs))
    {
        bug("[ATA--] %s: Failed to obtain AttrBases!\n", __func__);
        return FALSE;
    }
    DATAINIT(
      bug("[ATA--] %s: HiddBusAB %x @ 0x%p\n", __func__, HiddBusAB, &HiddBusAB);
      bug("[ATA--] %s: HiddATABusAB %x @ 0x%p\n", __func__, HiddATABusAB, &HiddATABusAB);
    )
#endif

#if defined(__OOP_NOMETHODBASES__)
    if (OOP_ObtainMethodBasesArray(&ATABase->hwMethodBase, methBaseIDs))
    {
        bug("[ATA--] %s: Failed to obtain MethodBases!\n", __func__);
        bug("[ATA--] %s:     %s = %p\n", __func__, methBaseIDs[0], ATABase->hwMethodBase);
        bug("[ATA--] %s:     %s = %p\n", __func__, methBaseIDs[1], ATABase->busMethodBase);
        bug("[ATA--] %s:     %s = %p\n", __func__, methBaseIDs[2], ATABase->HiddSCMethodBase);
#if defined(__OOP_NOATTRBASES__)
         OOP_ReleaseAttrBasesArray(&ATABase->unitAttrBase, attrBaseIDs);
#endif
        return FALSE;
    }
#endif

    InitSemaphore(&ATABase->DetectionSem);

    DATAINIT(bug("[ATA  ] %s: Base ATA Hidd Class @ 0x%p\n", __func__, ATABase->ataClass));

    return TRUE;
}

static int ata_expunge(struct ataBase *ATABase)
{
    struct ata_Controller *ataNode, *tmpNode;
    ForeachNodeSafe (&ATABase->ata_Controllers, ataNode, tmpNode)
    {
        OOP_Object *storageRoot;
        /*
         * CLID_Hidd_Storage is a singletone, you can get it as many times as
         * you want. Here we save up some space in struct ataBase by
         * obtaining storageRoot object only when we need it. This happens
         * rarely, so small performance loss is OK here.
         */
        storageRoot = OOP_NewObject(NULL, CLID_Hidd_Storage, NULL);
        if (!storageRoot)
            storageRoot = OOP_NewObject(NULL, CLID_HW_Root, NULL);
        if (storageRoot && HW_RemoveDriver(storageRoot, ataNode->ac_Object))
        {
            Remove(&ataNode->ac_Node);
            /* Destroy our singletone */
            OOP_MethodID disp_msg = OOP_GetMethodID(IID_Root, moRoot_Dispose);

            DATAINIT(bug("[ATA  ] ata_expunge: destroying subystem object\n"));
            OOP_DoSuperMethod(ataNode->ac_Class, ataNode->ac_Object, &disp_msg);
            FreeMem(ataNode, sizeof(struct ata_Controller));
        }
        else
        {
            /* Our subsystem is in use, we have some bus driver(s) around. */
            DATAINIT(bug("[ATA  ] ata_expunge: ATA subsystem is in use\n"));
            return FALSE;
        }
    }

#if defined(__OOP_NOATTRBASES__)
    DATAINIT(bug("[ATA  ] ata_expunge: Releasing attribute bases\n"));
    OOP_ReleaseAttrBasesArray(&ATABase->unitAttrBase, attrBaseIDs);
#endif

    if (ATABase->ata_UtilityBase)
        CloseLibrary(ATABase->ata_UtilityBase);

    DATAINIT(bug("[ATA  ] ata_expunge: Exiting\n"));
    return TRUE;
}

static int open(struct ataBase *ATABase, struct IORequest *iorq, ULONG unitnum, ULONG flags)
{
    struct ata_Controller *ataNode;
    struct Hook searchHook =
    {
        .h_Entry = Hidd_ATABus_Open,
        .h_Data  = iorq
    };

    /* Assume it failed */
    iorq->io_Error  = IOERR_OPENFAIL;
    iorq->io_Device = &ATABase->ata_Device;
    iorq->io_Unit   = (APTR)(IPTR)-1;

    /* Try to find the unit */
    ForeachNode (&ATABase->ata_Controllers, ataNode)
    {
        HIDD_StorageController_EnumBuses(ataNode->ac_Object, &searchHook, (APTR)(IPTR)unitnum);
    }
    DATAINIT(bug("[ATA%02d] Open result: %d\n", unitnum, iorq->io_Error));

    /* If found, io_Error will be reset to zero */
    return iorq->io_Error ? FALSE : TRUE;
}

/* Close given device */
static int close
(
    LIBBASETYPEPTR LIBBASE,
    struct IORequest *iorq
)
{
    struct ata_Unit *unit = (struct ata_Unit *)iorq->io_Unit;

    /* First of all make the important fields of struct IORequest invalid! */
    iorq->io_Unit = (struct Unit *)~0;
    
    /* Decrease use counters of unit */
    unit->au_Unit.unit_OpenCnt--;

    return TRUE;
}

ADD2INITLIB(ATA_init, 0)
ADD2EXPUNGELIB(ata_expunge, 0)
ADD2OPENDEV(open, 0)
ADD2CLOSEDEV(close, 0)
