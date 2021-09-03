/*
    Copyright (C) 2013-2020, The AROS Development Team. All rights reserved.

    Desc: A600/A1200/A4000 ATA HIDD hardware detection routine
*/

#include <aros/debug.h>
#include <proto/exec.h>

#include <aros/asmcall.h>
#include <aros/symbolsets.h>
#include <asm/io.h>
#include <exec/lists.h>
#include <exec/rawfmt.h>
#include <hidd/hidd.h>
#include <hidd/storage.h>
#include <hidd/ata.h>
#include <oop/oop.h>
#include <proto/oop.h>

#include <hardware/custom.h>
#include <graphics/gfxbase.h>
#include <hardware/ata.h>

#include <string.h>

#include "bus_class.h"
#include "interface_pio.h"

//#define ENABLE_ATAPOWERFLYER



static UBYTE *getport(struct ata_ProbedBus *ddata)
{
    volatile struct Custom *custom = (struct Custom*)0xdff000;
    UBYTE id, status1, status2;
    volatile UBYTE *port, *altport;
    struct GfxBase *gfx;

    port = NULL;
    gfx = (struct GfxBase*)TaggedOpenLibrary(TAGGEDOPEN_GRAPHICS);
    Disable();
    port = (UBYTE*)GAYLE_BASE_1200;
    ddata->gayleirqbase = (UBYTE*)GAYLE_IRQ_1200;
    Enable();
    CloseLibrary((struct Library*)gfx);

    D(bug("[ATA:Gayle] GayleID : %02x\n", id);)

    ddata->port = (UBYTE*)port;

    D(bug("[ATA:Gayle] Possible Gayle IDE port @ %08x\n", (ULONG)port & ~3);)
    altport = port + 0x1010;
    
    ddata->altport = (UBYTE*)altport;

RETRY:

    Disable();
    port[ata_DevHead * 4] = ATAF_ERROR;
    /* If nothing connected, we get back what we wrote, ATAF_ERROR set */
    status1 = port[ata_Status * 4];
    port[ata_DevHead * 4] = ATAF_DATAREQ;
    status2 = port[ata_Status * 4];
    port[ata_DevHead * 4] = 0;
    Enable();

       D(bug("[ATA:Gayle] Status=%02x,%02x\n", status1, status2);)
       // BUSY and DRDY both active or ERROR/DATAREQ = no drive(s) = do not install driver
       if (   (((status1 | status2) & (ATAF_BUSY | ATAF_DRDY)) == (ATAF_BUSY | ATAF_DRDY))
           || ((status1 | status2) & (ATAF_ERROR | ATAF_DATAREQ)))
       {
//	   goto RETRY;
           
           //D(bug("[ATA:Gayle] No Devices detected\n");)
           //return NULL;
       }
 
    /* we may have connected drives */
    return (UBYTE*)port;
}

static int gayle_bus_Scan(struct ataBase *base)
{
    struct ata_ProbedBus *probedbus;
    OOP_Class *busClass = base->GayleBusClass;
    struct TagItem ata_tags[] =
    {
            {aHidd_Name         , (IPTR)"ata_gayle.hidd"        },
            {aHidd_HardwareName , 0                             },
#define ATA_TAG_HARDWARENAME 1
            {TAG_DONE           , 0                             }
    };

    probedbus = AllocVec(sizeof(struct ata_ProbedBus), MEMF_ANY | MEMF_CLEAR);
    if (probedbus && getport(probedbus)) {
        OOP_Object *ata;
            if (probedbus->doubler == 0)
                ata_tags[ATA_TAG_HARDWARENAME].ti_Data = (IPTR)"Amiga(tm) Gayle IDE Controller";
            else
                ata_tags[ATA_TAG_HARDWARENAME].ti_Data = (IPTR)"Amiga(tm) Gayle IDE Controller + Port Doubler";
        
        ata = HW_AddDriver(base->storageRoot, base->ataClass, ata_tags);
        if (ata) {
            struct TagItem attrs[] =
            {
                {aHidd_Name                     , (IPTR)"ata_gayle.hidd"        },
                {aHidd_HardwareName             , 0                             },
 #define BUS_TAG_HARDWARENAME 1
                {aHidd_DriverData               , (IPTR)probedbus               },
                {aHidd_ATABus_PIODataSize       , sizeof(struct pio_data)       },
                {aHidd_ATABus_BusVectors        , (IPTR)bus_FuncTable           },
                {aHidd_ATABus_PIOVectors        , (IPTR)pio_FuncTable           },
                {aHidd_Bus_KeepEmpty            , FALSE                         },
                {TAG_DONE                       , 0                             }
            };
            OOP_Object *bus;

            /*
             * We use this field as ownership indicator.
             * The trick is that HW_AddDriver() fails if either object creation fails
             * or subsystem-side setup fails. In the latter case our object will be
             * disposed.
             * We need to know whether OOP_DisposeObject() or we should deallocate
             * this structure on failure.
             */
            probedbus->atapb_Node.ln_Succ = NULL;

            /*
             * Check if we have a FastATA adaptor
             */
                attrs[BUS_TAG_HARDWARENAME].ti_Data = (IPTR)"Gayle IDE Channel";
            

            bus = HIDD_StorageController_AddBus(ata, busClass, attrs);
            if (bus)
                return TRUE;
            D(bug("[ATA:Gayle] Failed to create object for device IO: %x:%x IRQ: %x\n",
                probedbus->port, probedbus->altport, probedbus->gayleirqbase);)

            /*
             * Free the structure only upon object creation failure!
             * In case of success it becomes owned by the driver object!
             */
            if (!probedbus->atapb_Node.ln_Succ)
                 FreeVec(probedbus);
            return TRUE;
        }
    }
    FreeVec(probedbus);

    return TRUE;
}

ADD2INITLIB(gayle_bus_Scan, 30)
