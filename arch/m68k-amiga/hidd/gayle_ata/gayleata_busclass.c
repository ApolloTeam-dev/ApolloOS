/*
    Copyright © 2013-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: A600/A1200/A4000 ATA HIDD
    Lang: English
*/

#include <aros/debug.h>

#include <hardware/ata.h>
#include <hidd/bus.h>
#include <hidd/ata.h>
#include <hidd/pci.h>
#include <oop/oop.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/kernel.h>
#include <proto/oop.h>
#include <proto/utility.h>
#include <hardware/intbits.h>

#include "bus_class.h"
#include "interface_pio.h"

/* Class Methods */

OOP_Object *GAYLEATA__Root__New(OOP_Class *cl, OOP_Object *o, struct pRoot_New *msg)
{
    struct ataBase *base = cl->UserData;
    struct ata_ProbedBus *bus = (struct ata_ProbedBus *)GetTagData(aHidd_DriverData, 0, msg->attrList);

    bug("[ATA:Gayle] %s()\n", __func__);

    if (!bus)
        return NULL;

    o = (OOP_Object *)OOP_DoSuperMethod(cl, o, &msg->mID);
    if (o)
    {
        struct ATA_BusData *data = OOP_INST_DATA(cl, o);

        data->bus = bus;
        data->bus->atapb_Node.ln_Succ = (struct Node *)-1;
        data->gaylebase = data->bus->port;
        data->gayleirqbase = data->bus->gayleirqbase;
        data->da = data->bus->da;
        data->v4 = data->bus->v4;
    }
    bug("[ATA:Gayle] %s: Instance @ %p\n", __func__, o);
    return o;
}

void GAYLEATA__Root__Dispose(OOP_Class *cl, OOP_Object *o, OOP_Msg msg)
{
    struct ATA_BusData *data = OOP_INST_DATA(cl, o);

    bug("[ATA:Gayle] %s()\n", __func__);

    FreeVec(data->bus);

    OOP_DoSuperMethod(cl, o, msg);
}

void GAYLEATA__Root__Get(OOP_Class *cl, OOP_Object *o, struct pRoot_Get *msg)
{
    struct ataBase *base = cl->UserData;
    ULONG idx;

    bug("[ATA:Gayle] %s()\n", __func__);

    Hidd_ATABus_Switch(msg->attrID, idx)
    {
    case aoHidd_ATABus_Use80Wire:
        *msg->storage = FALSE;
        return;

    case aoHidd_ATABus_UseDMA:
        *msg->storage = FALSE;
        return;

    case aoHidd_ATABus_Use32Bit:
        *msg->storage = TRUE;
        return;
    }

    OOP_DoSuperMethod(cl, o, &msg->mID);
}

void GAYLEATA__Root__Set(OOP_Class *cl, OOP_Object *o, struct pRoot_Set *msg)
{
    struct ataBase *base = cl->UserData;
    struct ATA_BusData *data = OOP_INST_DATA(cl, o);
    struct TagItem *tstate = msg->attrList;
    struct TagItem *tag;

    bug("[ATA:Gayle] %s()\n", __func__);

    while ((tag = NextTagItem(&tstate)))
    {
        ULONG idx;

        Hidd_Bus_Switch(tag->ti_Tag, idx)
        {
        case aoHidd_Bus_IRQHandler:
            data->ata_HandleIRQ = (APTR)tag->ti_Data;
            break;

        case aoHidd_Bus_IRQData:
            data->irqData = (APTR)tag->ti_Data;
            break;
        }
    }

    OOP_DoSuperMethod(cl, o, &msg->mID);
}

APTR GAYLEATA__Hidd_ATABus__GetPIOInterface(OOP_Class *cl, OOP_Object *o, OOP_Msg msg)
{
    struct ATA_BusData *data = OOP_INST_DATA(cl, o);
    struct pio_data *pio;

    bug("[ATA:Gayle] %s()\n", __func__);

    pio = (struct pio_data *)OOP_DoSuperMethod(cl, o, msg);
    if (pio)
    {
        pio->port = data->bus->port;
        pio->altport  = data->bus->altport;
        pio->da = data->bus->da;
        pio->v4 = data->bus->v4;
        pio->dataport = (UBYTE*)(((ULONG)pio->port) + 0x2000);
    }

    return pio;
}

void GAYLEATA__Hidd_ATABus__Shutdown(OOP_Class *cl, OOP_Object *o, OOP_Msg msg)
{
    OOP_DoSuperMethod(cl, o, msg);
}
