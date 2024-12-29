/*
    Copyright © 2013-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: A600/A1200/A4000 ATA HIDD hardware detection routine
    Lang: English
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



static UBYTE *getport(struct ata_ProbedBus *ddata, int a500)
{
    volatile struct Custom *custom = (struct Custom*)0xdff000;
    UBYTE id, status1, status2;
    volatile UBYTE *port, *altport;
    struct GfxBase *gfx;
    int retrynum = 0;

    struct GayleAdr {unsigned short a;};
    struct GayleAdr *ga = (struct GayleAdr *)0xda0018;
    struct GayleAdr *gd = (struct GayleAdr *)0xdd0018;
    struct GayleAdr *ca = (struct GayleAdr *)0xda1010;
    struct GayleAdr *cd = (struct GayleAdr *)0xdd1010;
    struct GayleAdr *cmda = (struct GayleAdr *)0xda101C;
    struct GayleAdr *cmdd = (struct GayleAdr *)0xdd101C;
    struct Board {UWORD type;};
    struct Board *MyBoard = (struct Board *)0xDFF3FC;
    /* Is the following correct or can we do it more intelligent? */
    if(a500 > 0)
    {
        // A500 only has one controller
        if((MyBoard->type == 0x02)||(MyBoard->type == 0x03))
          return 0;
        ga->a = 0x0;
        cmda->a = 0x10;
        ca->a = 0x10;
        if(ca->a != 0x10) return 0; // no hardware?
        ca->a = 0x34;
        if(ca->a != 0x34) return 0; // no hardware?
    	if((ga->a & 0xFF) == 0xFF) return 0;
    }
    else
    {
         gd->a = 0x0;
         cd->a = 0x10;
         cmdd->a = 0x10;
         if(cd->a != 0x10) return 0; // no hardware?
         cd->a = 0x34;
         if(cd->a != 0x34) return 0; // no hardware?
   	     if((gd->a & 0xFF ) == 0xFF) return 0;
    }
    
    port = NULL;
    gfx = (struct GfxBase*)TaggedOpenLibrary(TAGGEDOPEN_GRAPHICS);
    Disable();
    if(a500 > 0)
    {
        port = (UBYTE*)GAYLE_BASE_500;
        ddata->gayleirqbase = (UBYTE*)GAYLE_IRQ_500;
        ddata->a500 = TRUE;
    }
    else
    {
        port = (UBYTE*)GAYLE_BASE_1200;
        ddata->gayleirqbase = (UBYTE*)GAYLE_IRQ_1200;
        ddata->a500 = FALSE;
    }  
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
           retrynum++;
           if(retrynum == 10) return NULL;
	       goto RETRY;
           
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

    for(int i=0; i<2; i++)
    {
    
        struct TagItem ata_tags[] =
        {
                {aHidd_Name         , (IPTR)"ata_gayle.hidd"        },
                {aHidd_HardwareName , 0                             },
#define ATA_TAG_HARDWARENAME 1
                {TAG_DONE           , 0                             }
        };

        probedbus = AllocVec(sizeof(struct ata_ProbedBus), MEMF_ANY | MEMF_CLEAR);
        if (probedbus && getport(probedbus, i)) {
            OOP_Object *ata;
            if(i == 0)
            {
                if (probedbus->doubler == 0)
                    ata_tags[ATA_TAG_HARDWARENAME].ti_Data = (IPTR)"Amiga(tm) Gayle IDE Controller";
                else
                    ata_tags[ATA_TAG_HARDWARENAME].ti_Data = (IPTR)"Amiga(tm) Gayle IDE Controller + Port Doubler";
            }
            else
            {
                if (probedbus->doubler == 0)
                    ata_tags[ATA_TAG_HARDWARENAME].ti_Data = (IPTR)"Amiga(tm) Gayle IDE Controller 2";
                else
                    ata_tags[ATA_TAG_HARDWARENAME].ti_Data = (IPTR)"Amiga(tm) Gayle IDE Controller 2 + Port Doubler";
            }
        
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
                if(i == 1)
                    attrs[0].ti_Data = (IPTR)"ata_gayle2.hidd";
                
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
                if(i == 0)    
                     attrs[BUS_TAG_HARDWARENAME].ti_Data = (IPTR)"Gayle IDE Channel";
                else attrs[BUS_TAG_HARDWARENAME].ti_Data = (IPTR)"Gayle IDE Channel 2";
            

                bus = HIDD_StorageController_AddBus(ata, busClass, attrs);

                // Do not exit, we could have more controllers
                // if (bus)
                //    return TRUE;
               
                D(bug("[ATA:Gayle] Failed to create object for device IO: %x:%x IRQ: %x\n",
                    probedbus->port, probedbus->altport, probedbus->gayleirqbase);)

                /*
                 * Free the structure only upon object creation failure!
                 * In case of success it becomes owned by the driver object!
                 * Edit: Can we really free it yet? We are collecting controllers. Please enable the two lines if sure.
                 */
            //    if (!probedbus->atapb_Node.ln_Succ)
            //         FreeVec(probedbus);
            //    return TRUE;
            }
        }
        FreeVec(probedbus);
    }

    return TRUE;
}

ADD2INITLIB(gayle_bus_Scan, 30)
