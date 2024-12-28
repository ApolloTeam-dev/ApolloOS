/*
    Copyright © 2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc: A600/A1200/A4000 ATA HIDD PIO interface functions
    Lang: English
*/
#include <aros/debug.h>

#define DIO(x)
#define DDATA(x)

#include "interface_pio.h"

static void ata_out(struct pio_data *data, UBYTE val, UWORD offset)
{
    volatile UBYTE *addr;
    addr = data->port;
    addr[offset * 4] = val;
}

static UBYTE ata_in(struct pio_data *data, UWORD offset)
{
    volatile UBYTE *addr;
    UBYTE v;

    addr = data->port;
    v = addr[offset * 4];
    return v;
}


static void ata_outsl(struct pio_data *data, APTR address, ULONG count)
{
    volatile ULONG *addr = (ULONG*)data->dataport;

    if(data->a500)
    {
    asm volatile(
"1:     move.l (%[address])+,(0xda2000)  \n"
"       move.l (%[address])+,(0xda2000)  \n"
"       subq.l #1,%[count]              \n"
"       bnes 1b                         \n"
        ::[count]"d"(count >> 3),[address]"a"(address),[port]"a"(addr));
    }
    else
    {
    asm volatile(
"1:     move.l (%[address])+,(0xdd2000)  \n"
"       move.l (%[address])+,(0xdd2000)  \n"
"       subq.l #1,%[count]              \n"
"       bnes 1b                         \n"
        ::[count]"d"(count >> 3),[address]"a"(address),[port]"a"(addr));
    }
    
}


static void ata_insl(struct pio_data *data, APTR address, ULONG count)
{
    volatile ULONG *addr = (ULONG*)data->dataport;

    if(data->a500)
    {
    asm volatile(
"       bra 2f                          \n"
"1:                                     \n"
"       move16 0x00da6000,(%[address])+ \n"
"       move16 0x00da6000,(%[address])+ \n"
"2:     dbra   %[count],1b              \n"
        ::[count]"d"(count >> 5),[address]"a"(address));
    }
    else
    {
    asm volatile(
"       bra 2f                          \n"
"1:                                     \n"
"       move16 0x00dd6000,(%[address])+ \n"
"       move16 0x00dd6000,(%[address])+ \n"
"2:     dbra   %[count],1b              \n"
        ::[count]"d"(count >> 5),[address]"a"(address));
    }
}

const APTR bus_FuncTable[] =
{
    ata_out,
    ata_in,
    (APTR *)-1
};

const APTR pio_FuncTable[] =
{
    ata_outsl,
    ata_insl,
    ata_outsl,
    ata_insl,
    (APTR *)-1
};
