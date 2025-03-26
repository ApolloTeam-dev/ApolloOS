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

// use #define DERROR(x) x for error output
#define DERROR(x) x

// add #define DINIT(x) x for output on Initialization routines
#define DINIT(x) x

// add #define DD(x) x for regular level debug output
#define DD(x) 

// add #define DDD(x) x for output on low level routines
#define DDD(x)

#define VREG_BOARD_Unknown  0x00 /* Unknown                         */
#define VREG_BOARD_V600     0x01 /* Vampire V2 V600(+),   for A600  */
#define VREG_BOARD_V500     0x02 /* Vampire V2 V500(+),   for A500  */
#define VREG_BOARD_V4FB     0x03 /* Apollo V4 FireBird,   for A500  */
#define VREG_BOARD_V4ID     0x04 /* Apollo V4 IceDrake,   for A1200 */
#define VREG_BOARD_V4SA     0x05 /* Apollo V4 Standalone            */
#define VREG_BOARD_V1200    0x06 /* Vampire V2 V1200,     for A1200 */
#define VREG_BOARD_V4MC     0x07 /* Apollo V4 Manticore,  for A600  */
#define VREG_BOARD_Future   0x08 /* Unknow                          */

static UBYTE *getport(struct ata_ProbedBus *ddata, int buscounter)
{
    volatile struct Custom *custom = (struct Custom*)0xdff000;
    UBYTE status1, status2;
    volatile UBYTE *port, *altport;
    struct GfxBase *gfx;
    int retrynum = 0;

    struct GayleAdr {unsigned short a;};
    struct GayleAdr *ga     = (struct GayleAdr *)0xda0018;
    struct GayleAdr *gd     = (struct GayleAdr *)0xdd0018;
    struct GayleAdr *ca     = (struct GayleAdr *)0xda1010;
    struct GayleAdr *cd     = (struct GayleAdr *)0xdd1010;
    struct GayleAdr *cmda   = (struct GayleAdr *)0xda101C;
    struct GayleAdr *cmdd   = (struct GayleAdr *)0xdd101C;
    
    UWORD ApolloBoardID     = *(volatile UWORD *)0xdff3fc;  
    ApolloBoardID = ApolloBoardID >> 8;  

    DINIT(bug("[ATA:Probe] BusNumber = %01d | ApolloBoardID = %d\n", buscounter, ApolloBoardID);)

    port = NULL;
    gfx = (struct GfxBase*)TaggedOpenLibrary(TAGGEDOPEN_GRAPHICS);
    Disable();
    if(buscounter == 0)
    {
        // V4-ID and V4-MC have mainboard native IDE on $DA, V2-500, V2-600, V2-1200, V4-FB and V4-SA have Fast-IDE Gayle on $DA
        {
            port = (UBYTE*)GAYLE_BASE_DA;
            ddata->gayleirqbase = (UBYTE*)GAYLE_IRQ_DA;
            ddata->da = TRUE;
            if((ApolloBoardID == VREG_BOARD_V500)
             ||(ApolloBoardID == VREG_BOARD_V600)
             ||(ApolloBoardID == VREG_BOARD_V1200)
             ||(ApolloBoardID == VREG_BOARD_V4FB)
             ||(ApolloBoardID == VREG_BOARD_V4SA))
            {
                ddata->v4 = TRUE;
                DINIT(bug("[ATA:Probe] Port = GAYLE_BASE_DA (V4 Fast-IDE)\n");)
            } else {
                ddata->v4 = FALSE;
                DINIT(bug("[ATA:Probe] Port = GAYLE_BASE_DA (Native IDE)\n");)
            }
        } 
    } else {
        // V4-ID and V4-MC have Fast-IDE Gayle on $DD
        if((ApolloBoardID == VREG_BOARD_V4ID)
         ||(ApolloBoardID == VREG_BOARD_V4MC))
        {
            port = (UBYTE*)GAYLE_BASE_DD;
            ddata->gayleirqbase = (UBYTE*)GAYLE_IRQ_DD;
            ddata->da = FALSE;
            ddata->v4 = TRUE;
            DINIT(bug("[ATA:Probe] Port = GAYLE_BASE_DD (V4 Fast-IDE)\n");)
        }
    }
    Enable();
    CloseLibrary((struct Library*)gfx);

    if(port == NULL)
    {
        DERROR(bug("[ATA:Probe] No Gayle IDE port @ %08x\n", (ULONG)port & ~3);)
        return NULL;
    } 

    ddata->port = (UBYTE*)port;
    altport = port + 0x1010;
    ddata->altport = (UBYTE*)altport;

    Disable();
    port[ata_DevHead * 4] = ATAF_ERROR;
    status1 = port[ata_Status * 4];
    port[ata_DevHead * 4] = ATAF_DATAREQ;
    status2 = port[ata_Status * 4];
    port[ata_DevHead * 4] = 0;
    Enable();

    DINIT(bug("[ATA:Probe] Status=%02x,%02x\n", status1, status2);)

RETRY:

    if ((((status1 | status2) & (ATAF_BUSY | ATAF_DRDY)) == (ATAF_BUSY | ATAF_DRDY)) || ((status1 | status2) & (ATAF_ERROR | ATAF_DATAREQ)))
    {
        if(retrynum++ < 10) goto RETRY;
        
        DERROR(bug("[ATA:Probe] No Devices detected\n");)
        return NULL;
    }

    return (UBYTE*)port;
}

static int gayle_bus_Scan(struct ataBase *base)
{
    struct ata_ProbedBus *probedbus;
    OOP_Class *busClass = base->GayleBusClass;

    DINIT(bug("[ATA:Probe] Starting Gayle Bus Scan\n");)

    for(int buscounter=0; buscounter<2; buscounter++)
    {
        struct TagItem ata_tags[] =
        {
                {aHidd_Name         , (IPTR)"ata_gayle.hidd"                    },
                {aHidd_HardwareName , (IPTR)"Amiga Gayle IDE Controller $DA"    },
                {TAG_DONE           , 0                                         }
        };

        probedbus = AllocVec(sizeof(struct ata_ProbedBus), MEMF_ANY | MEMF_CLEAR);
        if (probedbus && getport(probedbus, buscounter))
        {
            OOP_Object *ata;
            if(buscounter == 1)
            {
                ata_tags[1].ti_Data = (IPTR)"ata_gayle2.hidd";
                ata_tags[1].ti_Data = (IPTR)"Amiga Gayle IDE Controller $DD";
            }
        
            ata = HW_AddDriver(base->storageRoot, base->ataClass, ata_tags);
            DINIT(bug("[ATA:Probe] Added: %s\n", ata_tags[1].ti_Data);)

            if (ata) {
                struct TagItem attrs[] =
                {
                    {aHidd_Name                     , (IPTR)"ata_gayle.hidd"        },
                    {aHidd_HardwareName             , (IPTR)"Gayle IDE Bus $DA" },
                    {aHidd_DriverData               , (IPTR)probedbus               },
                    {aHidd_ATABus_PIODataSize       , sizeof(struct pio_data)       },
                    {aHidd_ATABus_BusVectors        , (IPTR)bus_FuncTable           },
                    {aHidd_ATABus_PIOVectors        , (IPTR)pio_FuncTable           },
                    {aHidd_Bus_KeepEmpty            , FALSE                         },
                    {TAG_DONE                       , 0                             }
                };

                if(buscounter == 1)
                {
                    attrs[0].ti_Data = (IPTR)"ata_gayle2.hidd";
                    attrs[1].ti_Data = (IPTR)"Gayle IDE Bus $DD";
                }

                OOP_Object *bus;

                probedbus->atapb_Node.ln_Succ = NULL;

                bus = HIDD_StorageController_AddBus(ata, busClass, attrs);

                if (bus)
                {
                    DINIT(bug("[ATA:Probe] Added: %s\n", attrs[1].ti_Data);)
                    return TRUE;
                }
                DERROR(bug("[ATA:Probe] Failed to create object for device IO: %x:%x IRQ: %x\n", probedbus->port, probedbus->altport, probedbus->gayleirqbase);)
                if (!probedbus->atapb_Node.ln_Succ) FreeVec(probedbus);
                return TRUE;
            }
        }
        FreeVec(probedbus);
    }
    return TRUE;
}

ADD2INITLIB(gayle_bus_Scan, 30)
