#include <aros/debug.h>

#include <proto/exec.h>

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/resident.h>
#include <utility/utility.h>
#include <oop/oop.h>

#include <devices/timer.h>

#include "ata.h"
#include "ata_bus.h"
#include "timer.h"

// use #define DERROR(x) x for error output
#define DERROR(x) x

// add #define DINIT(x) x for output on Initialization routines
#define DINIT(x) x

// add #define DD(x) x for regular level debug output
#define DD(x) 

// add #define DDD(x) x for output on low level routines
#define DDD(x)

// add #define DATAPI(x) x for output on Atapi routines
#define DATAPI(x) 

#define VREG_BOARD_Unknown  0x00 /* Unknown                         */
#define VREG_BOARD_V600     0x01 /* Vampire V2 V600(+),   for A600  */
#define VREG_BOARD_V500     0x02 /* Vampire V2 V500(+),   for A500  */
#define VREG_BOARD_V4FB     0x03 /* Apollo V4 FireBird,   for A500  */
#define VREG_BOARD_V4ID     0x04 /* Apollo V4 IceDrake,   for A1200 */
#define VREG_BOARD_V4SA     0x05 /* Apollo V4 Standalone            */
#define VREG_BOARD_V1200    0x06 /* Vampire V2 V1200,     for A1200 */
#define VREG_BOARD_V4MC     0x07 /* Apollo V4 Manticore,  for A600  */
#define VREG_BOARD_Future   0x08 /* Unknow                          */

static BYTE ata_Identify(struct ata_Unit *unit);
static BYTE ata_ReadSector32(struct ata_Unit *, ULONG, ULONG, APTR, ULONG *);
static BYTE ata_ReadSector64(struct ata_Unit *, UQUAD, ULONG, APTR, ULONG *);
static BYTE ata_ReadMultiple32(struct ata_Unit *, ULONG, ULONG, APTR, ULONG *);
static BYTE ata_ReadMultiple64(struct ata_Unit *, UQUAD, ULONG, APTR, ULONG *);
static BYTE ata_WriteSector32(struct ata_Unit *, ULONG, ULONG, APTR, ULONG *);
static BYTE ata_WriteSector64(struct ata_Unit *, UQUAD, ULONG, APTR, ULONG *);
static BYTE ata_WriteMultiple32(struct ata_Unit *, ULONG, ULONG, APTR, ULONG *);
static BYTE ata_WriteMultiple64(struct ata_Unit *, UQUAD, ULONG, APTR, ULONG *);
static void ata_ResetBus(struct ata_Bus *bus);
static BYTE ata_Eject(struct ata_Unit *);
static BOOL ata_WaitBusyTO(struct ata_Unit *unit, UWORD tout, BOOL irq, BOOL fake_irq, UBYTE *stout);

static BYTE atapi_SendPacket(struct ata_Unit *unit, APTR packet, APTR data, LONG datalen, BOOL *dma, BOOL write);
static BYTE atapi_DirectSCSI(struct ata_Unit *unit, struct SCSICmd *cmd);
static BYTE atapi_Read(struct ata_Unit *, ULONG, ULONG, APTR, ULONG *);
static BYTE atapi_Write(struct ata_Unit *, ULONG, ULONG, APTR, ULONG *);
static BYTE atapi_Eject(struct ata_Unit *);
static void atapi_RequestSense(struct ata_Unit* unit, UBYTE* sense, ULONG senselen);
static BYTE atapi_EndCmd(struct ata_Unit *unit);

static void common_SetBestXferMode(struct ata_Unit* unit);

#define DEVHEAD_VAL 0xe0

static void ata_strcpy(const UBYTE *str1, UBYTE *str2, ULONG size)
{
    register int i = size;

    while (size--)
        str2[size ^ 1] = str1[size];

    while (i > 0 && str2[--i] <= ' ')
        str2[i] = '\0';
}

static inline struct ata_Unit* ata_GetSelectedUnit(struct ata_Bus* bus)
{
    return bus->ab_SelectedUnit;
}

static inline UBYTE ata_ReadStatus(struct ata_Bus *bus)
{
    return PIO_In(bus, ata_Status);
}

static inline UBYTE ata_ReadAltStatus(struct ata_Bus *bus)
{
    return PIO_InAlt(bus, ata_AltStatus);
}

static inline BOOL ata_SelectUnit(struct ata_Unit* unit)
{
    struct ata_Bus *bus = unit->au_Bus;

    if (unit == bus->ab_SelectedUnit) return TRUE;

    DD(bug("ata_SelectUnit: CHANGE SELECTUNIT = %01d\n", unit->au_UnitNum);)

    PIO_Out(bus, unit->au_DevMask, ata_DevHead);

    do
    {
        ata_WaitNano(400, bus->ab_Base);
    }
    while (0 != (ATAF_BUSY & ata_ReadStatus(bus)));

    bus->ab_SelectedUnit = unit;

    return TRUE;
}

/*static void ata_IRQSignalTask(struct ata_Bus *bus)
{
    bus->ab_IntCnt++;
    Signal(bus->ab_Task, 1UL << bus->ab_SleepySignal);
}*/

static void ata_IRQSetHandler(struct ata_Unit *unit, void (*handler)(struct ata_Unit*, UBYTE), APTR piomem, ULONG blklen, ULONG piolen)
{
    unit->au_cmd_data = piomem;
    unit->au_cmd_length = (piolen < blklen) ? piolen : blklen;
    unit->au_cmd_total = piolen;
}

static void ata_IRQNoData(struct ata_Unit *unit, UBYTE status)
{
    ata_IRQSetHandler(unit, NULL, NULL, 0, 0);
    //ata_IRQSignalTask(unit->au_Bus);
}

static void ata_IRQPIORead(struct ata_Unit *unit, UBYTE status)
{
    ULONG   count;
    APTR    address;
    ULONG   retrycount; 

AGAIN:
    retrycount=100000000;
WAITBUSY: 
    status = PIO_In(unit->au_Bus, ata_Status);

    if (retrycount-- == 0)
    {
        unit->au_cmd_error = HFERR_BadStatus;
        return;
    }
    if (status & ATAF_BUSY) goto WAITBUSY;
    
    if (status & ATAF_DATAREQ)
    {
        Unit_InS(unit, unit->au_cmd_data, unit->au_cmd_length);

        unit->au_cmd_data += unit->au_cmd_length;
        unit->au_cmd_total -= unit->au_cmd_length;
        if (unit->au_cmd_total != 0)
        {
            if (unit->au_cmd_length > unit->au_cmd_total) unit->au_cmd_length = unit->au_cmd_total;
            goto AGAIN;
        }
    } else {
        unit->au_cmd_error = HFERR_BadStatus;
    }
    ata_IRQNoData(unit, status);
}


static void ata_IRQPIOWrite(struct ata_Unit *unit, UBYTE status)
{
    ULONG   count;
    APTR    address;
    ULONG   retrycount;

AGAINW:
    retrycount=100000000;
WAITBUSYW:
    status = PIO_In(unit->au_Bus, ata_Status);

    retrycount--;
    if (retrycount-- == 0)
    {
        unit->au_cmd_error = HFERR_BadStatus;
        return;
    }
    if (status & ATAF_BUSY) goto WAITBUSYW;

    if (status & ATAF_DATAREQ)
    {
        Unit_OutS(unit, unit->au_cmd_data, unit->au_cmd_length);

        unit->au_cmd_data += unit->au_cmd_length;
        unit->au_cmd_total -= unit->au_cmd_length;
        if (unit->au_cmd_length > unit->au_cmd_total) unit->au_cmd_length = unit->au_cmd_total;
        if (unit->au_cmd_total != 0) goto AGAINW;
    } else {
        if (unit->au_cmd_total != 0) unit->au_cmd_error = HFERR_BadStatus;
    }
    ata_IRQNoData(unit, status);
}


static void ata_IRQPIOReadAtapi(struct ata_Unit *unit, UBYTE status)
{
    struct ata_Bus *bus = unit->au_Bus;
    ULONG size = 0;
    LONG remainder = 0;
    ULONG retry_busy, retry_datareq;

AGAIN:
    retry_busy = 200000;
    retry_datareq = 500;

WAITBUSY: 
    status = PIO_In(unit->au_Bus, ata_Status);
    ata_WaitNano(400, bus->ab_Base);

    retry_busy--;
    if (retry_busy == 0)
    {
        ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
        unit->au_cmd_error = HFERR_BadStatus;
        DERROR(bug("[ATAPI] ata_IRQPIOReadAtapi: ERROR = RetryCount (200000) reached ZERO on ATAF_BUSY clear\n"));
        return;
    }
    
    DATAPI(bug("r"));

    if ((status & ATAF_BUSY) != 0) // Wait for BSY to CLEAR
    {
        goto WAITBUSY;
    }

    DATAPI(bug("\n[ATAPI] ata_IRQPIOReadAtapi - Status: %lx\n", status));

WAITDATAREQ:
    status = PIO_In(unit->au_Bus, ata_Status);
    ata_WaitNano(400, bus->ab_Base);

    retry_datareq--;
    if (retry_datareq == 0)
    {
        ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
        unit->au_cmd_error = HFERR_BadStatus;
        DERROR(bug("[ATAPI] ata_IRQPIOReadAtapi: ERROR = RetryCount (500) reached ZERO on ATAF_DATAREQ SET\n"));
        return;
    }

    DATAPI(bug("R"));
    if ((status & ATAF_DATAREQ) != ATAF_DATAREQ) // Wait for DRQ to SET
    {
        goto WAITDATAREQ;
    }

    size = PIO_In(bus, atapi_ByteCntH) << 8 | PIO_In(bus, atapi_ByteCntL);
    
    DATAPI(bug("\n[ATAPI] ata_IRQPIOReadAtapi: data available for read (%ld bytes, max: %ld bytes)\n", size, unit->au_cmd_total));

    if (size > unit->au_cmd_total)
    {
        DERROR(bug("[ATAPI] ata_IRQPIOReadAtapi: CRITICAL! MORE DATA OFFERED THAN STORAGE CAN TAKE: %ld bytes vs %ld bytes left!\n", size, unit->au_cmd_total));
        size = unit->au_cmd_total;
    }

    if (size > 0) Unit_InS(unit, unit->au_cmd_data, size);

    unit->au_cmd_data = &((UBYTE*)unit->au_cmd_data)[size];
    unit->au_cmd_total -= size;

    DATAPI(bug("[ATAPI] ata_IRQPIOReadAtapi: %lu bytes read, %lu bytes remaining.\n", size, unit->au_cmd_total));

    if (unit->au_cmd_total > 0)
    {
        goto AGAIN;
    } else {
        ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
    }

    DERROR(status = PIO_In(unit->au_Bus, ata_Status));
    DERROR(if (status & ATAF_ERROR) bug("\n[ATAPI] *** ERROR: ata_IRQPIOReadAtapi - ATAF_ERROR"));

    retry_busy = 0;
    do
    {
        status = PIO_In(bus, atapi_Status);
        ata_WaitNano(400, bus->ab_Base);
        retry_busy++;
        if (retry_busy == 1000000)
        {
            DERROR(bug("[ATAPI] *** ERROR: ata_IRQPIOReadAtapi - ATAF_BUSY not cleared!\n"));
            break;
        }
    } while ((status & ATAF_BUSY) != 0); // Wait for BSY to CLEAR
}

static void ata_IRQPIOWriteAtapi(struct ata_Unit *unit, UBYTE status)
{
    struct ata_Bus *bus = unit->au_Bus;
    ULONG size = 0;
    ULONG retry_busy, retry_datareq; 
    
AGAINW:
    retry_busy = 200000;
    retry_datareq = 500;

WAITBUSYW: 
    status = PIO_In(unit->au_Bus, ata_Status);
    ata_WaitNano(400, bus->ab_Base);

    retry_busy--;
    if (retry_busy == 0)
    {
        ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
        unit->au_cmd_error = HFERR_BadStatus;
        DERROR(bug("[ATAPI] ata_IRQPIOWriteAtapi: ERROR = RetryCount (200000) reached ZERO on ATAF_BUSY clear\n"));
        return;
    }

    DATAPI(bug("w"));
    if ((status & ATAF_BUSY) != 0) // Wait for BSY to CLEAR
    {
        goto WAITBUSYW;
    }

    DATAPI(bug("\n[ATAPI] ata_IRQPIOWriteAtapi - Status: %lx\n", status));

WAITDATAREQW:
    status = PIO_In(unit->au_Bus, ata_Status);
    ata_WaitNano(400, bus->ab_Base);

    retry_datareq--;
    if (retry_datareq == 0)
    {
        ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
        unit->au_cmd_error = HFERR_BadStatus;
        DERROR(bug("\n[ATAPI] *** ERROR: ata_IRQPIOWriteAtapi: ERROR = RetryCount (500) reached ZERO on ATAF_DATAREQ SET\n"));
        return;
    }

    DATAPI(bug("W"));
    if ((status & ATAF_DATAREQ) != ATAF_DATAREQ) // Wait for DRQ to SET
    {
        goto WAITDATAREQW;
    }

    size = PIO_In(bus, atapi_ByteCntH) << 8 | PIO_In(bus, atapi_ByteCntL);

    DATAPI(bug("\n[ATAPI] ata_IRQPIOWriteAtapi: data requested for write (%ld bytes, max: %ld bytes)\n", size, unit->au_cmd_total));

    if (size > unit->au_cmd_total)
    {
        DERROR(bug("[ATAPI] ata_IRQPIOWriteAtapi: CRITICAL! MORE DATA REQUESTED THAN STORAGE CAN GIVE: %ld bytes vs %ld bytes left!\n", size, unit->au_cmd_total));
        size = unit->au_cmd_total;
    }

    Unit_OutS(unit, unit->au_cmd_data, size);

    unit->au_cmd_data = &((UBYTE*)unit->au_cmd_data)[size];
    unit->au_cmd_total -= size;

    DATAPI(bug("[ATAPI] ata_IRQPIOWriteAtapi: %lu bytes written, %lu bytes remaining..\n", size,  unit->au_cmd_total));

    if (unit->au_cmd_total > 0)
    {
        goto AGAINW;
    } else {
        ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
    }

    DERROR(status = PIO_In(unit->au_Bus, ata_Status));
    DERROR(if (status & ATAF_ERROR) bug("\n[ATAPI] *** ERROR: ata_IRQPIOWriteAtapi - ATAF_ERROR"));

    retry_busy = 0;
    do
    {
        status = PIO_In(bus, atapi_Status);
        ata_WaitNano(400, bus->ab_Base);
        retry_busy++;
        if (retry_busy == 1000000)
        {
            DERROR(bug("[ATAPI] *** ERROR: ata_IRQPIOWriteAtapi - ATAF_BUSY not cleared!\n"));
            break;
        }
    } while ((status & ATAF_BUSY) != 0); // Wait for BSY to CLEAR
}


static BOOL ata_WaitBusyTO(struct ata_Unit *unit, UWORD tout, BOOL irq, BOOL fake_irq, UBYTE *stout)
{
    struct ata_Bus *bus = unit->au_Bus;
    UBYTE status = 0xff;
    ULONG step = 0;
    BOOL res = TRUE;

    status = PIO_InAlt(bus, ata_AltStatus);
    while (status & ATAF_BUSY)
    {
        ++step;

        if ((step & 15) == 0)
        {
            if (step >> 4 > tout * 1000)
            {
                DERROR(bug("[ATA:%02ld] Device still busy after timeout. Aborting\n", unit->au_UnitNum));
                res = FALSE;
                break;
            }
            ata_WaitTO(unit->au_Bus->ab_Timer, 0, 1000, 0);
        }
        status = PIO_InAlt(bus, ata_AltStatus);
    }

    status = PIO_In(bus, ata_Status);

    DATAPI(bug("[ATA:%02ld] WaitBusy status: %lx / %ld\n", unit->au_UnitNum, status, res));

    SetSignal(0, 1 << bus->ab_SleepySignal);

    if (stout) *stout = status;
    return res;
}


static BYTE ata_exec_cmd(struct ata_Unit* unit, ata_CommandBlock *block)
{
    struct ata_Bus *bus = unit->au_Bus;
    BYTE err = 0;
    APTR mem = block->buffer;
    UBYTE status;

    BOOL fake_irq = block->command == ATA_IDENTIFY_DEVICE || block->command == ATA_IDENTIFY_ATAPI;
    UWORD timeout = fake_irq? 1 : TIMEOUT;

    if (FALSE == ata_SelectUnit(unit)) return IOERR_UNITBUSY;

    switch (block->type)
    {
        case CT_CHS:
        case CT_LBA28:
            if (block->sectors > 256)
            {
                DERROR(bug("[ATA:%02ld] ata_exec_cmd: ERROR: Transfer length (%ld) exceeds 256 sectors. Aborting.\n", unit->au_UnitNum, block->sectors));
                return IOERR_BADLENGTH;
            }
        case CT_LBA48:
            if (block->sectors > 65536)
            {
                DERROR(bug("[ATA:%02ld] ata_exec_cmd: ERROR: Transfer length (%ld) exceeds 65536 sectors. Aborting.\n", unit->au_UnitNum, block->sectors));
                return IOERR_BADLENGTH;
            }
            if (block->secmul == 0)
            {
                DERROR(bug("[ATA:%02ld] ata_exec_cmd: ERROR: Invalid transfer multiplier. Should be at least set to 1 (correcting)\n", unit->au_UnitNum));
                block->secmul = 1;
            }
           break;

        case CT_NoBlock:
            break;

        default:
            DERROR(bug("[ATA:%02ld] ata_exec_cmd: ERROR: Invalid command type %lx. Aborting.\n", unit->au_UnitNum, block->type));
            return IOERR_NOCMD;
    }

    block->actual = 0;
    DDD(bug("[ATA:%02ld] ata_exec_cmd: Executing command %02lx\n", unit->au_UnitNum, block->command));

    if (block->feature != 0) PIO_Out(bus, block->feature, ata_Feature);

    ULONG piolen = block->length;
    ULONG blklen = block->secmul << unit->au_SectorShift;

    switch (block->type)
    {
        case CT_CHS:
            DDD(bug("[ATA:%02ld] ata_exec_cmd: Command uses CHS addressing (OLD)\n", unit->au_UnitNum));
            {
                ULONG cyl, head, sector;
                ULONG tmp = unit->au_Heads * unit->au_Sectors;
                cyl = block->blk / tmp;
                head = (block->blk % tmp) / unit->au_Sectors;
                sector = (block->blk % unit->au_Sectors) + 1;

                PIO_Out(bus, ((head) & 0x0f) | unit->au_DevMask, ata_DevHead);
                PIO_Out(bus, sector, ata_Sector);
                PIO_Out(bus, cyl & 0xff, ata_CylinderLow);
                PIO_Out(bus, (cyl >> 8) & 0xff, ata_CylinderHigh);
                PIO_Out(bus, block->sectors, ata_Count);
            }
            break;
        case CT_LBA28:
            DDD(bug("[ATA:%02ld] ata_exec_cmd: Command uses 28bit LBA addressing (OLD)\n", unit->au_UnitNum));

            PIO_Out(bus, ((block->blk >> 24) & 0x0f) | DEVHEAD_VAL | unit->au_DevMask, ata_DevHead);
            PIO_Out(bus, block->blk >> 16, ata_LBAHigh);
            PIO_Out(bus, block->blk >> 8, ata_LBAMid);
            PIO_Out(bus, block->blk, ata_LBALow);
            PIO_Out(bus, block->sectors, ata_Count);
            break;

        case CT_LBA48:
            DDD(bug("[ATA:%02ld] ata_exec_cmd: Command uses 48bit LBA addressing (NEW)\n", unit->au_UnitNum));

            PIO_Out(bus, DEVHEAD_VAL | unit->au_DevMask, ata_DevHead);
            PIO_Out(bus, block->blk >> 40, ata_LBAHigh);
            PIO_Out(bus, block->blk >> 32, ata_LBAMid);
            PIO_Out(bus, block->blk >> 24, ata_LBALow);

            PIO_Out(bus, block->blk >> 16, ata_LBAHigh);
            PIO_Out(bus, block->blk >> 8, ata_LBAMid);
            PIO_Out(bus, block->blk, ata_LBALow);

            PIO_Out(bus, block->sectors >> 8, ata_Count);
            PIO_Out(bus, block->sectors, ata_Count);
            break;

        case CT_NoBlock:
            DDD(bug("[ATA:%02ld] ata_exec_cmd: Command does not address any block\n", unit->au_UnitNum));
            break;
    }

    switch (block->method)
    {
        case CM_PIOWrite:
            PIO_OutAlt(bus, ATACTLF_INT_DISABLE, ata_AltControl);
            PIO_Out(bus, block->command, ata_Command);
            unit->au_cmd_error = 0;
            unit->au_cmd_data = mem;
            piolen = block->length;
            blklen = block->secmul << unit->au_SectorShift;
            unit->au_cmd_length = (piolen < blklen) ? piolen : blklen;
            unit->au_cmd_total = piolen;
            ata_IRQPIOWrite(unit, ATAF_BUSY);
            break;

        case CM_PIORead:
            PIO_OutAlt(bus, ATACTLF_INT_DISABLE, ata_AltControl);
            PIO_Out(bus, block->command, ata_Command);
            unit->au_cmd_error = 0;
            unit->au_cmd_data = mem;
            piolen = block->length;
            blklen = block->secmul << unit->au_SectorShift;
            unit->au_cmd_length = (piolen < blklen) ? piolen : blklen;
            unit->au_cmd_total = piolen;
            ata_IRQPIORead(unit, ATAF_BUSY);
            break;
    
        case CM_NoData:
            ata_IRQSetHandler(unit, &ata_IRQNoData, NULL, 0, 0);
            break;

        default:
            return IOERR_NOCMD;
            break;
    };

    if (FALSE == ata_WaitBusyTO(unit, timeout, TRUE, fake_irq, &status))
    {
        DERROR(bug("[ATA%02ld] ata_exec_cmd: Device is late - no response\n", unit->au_UnitNum));
        err = IOERR_UNITBUSY;
    } else {
        err = unit->au_cmd_error;
    }

    DDD(bug("[ATA:%02ld] ata_exec_cmd: Command done -> return code %ld\n", unit->au_UnitNum, err));

    return err;
}


static BYTE atapi_SendPacket(struct ata_Unit *unit, APTR packet, APTR data, LONG datalen, BOOL *dma, BOOL write)
{
    struct ata_Bus *bus = unit->au_Bus;
    *dma = FALSE;
    LONG err = 0;
    UBYTE status;

    ULONG retry_busy = 200000;
    ULONG retry_datareq = 500;

    UBYTE cmd[12] = {0};
    register int t=5,l=0;

    if (((UBYTE*)packet)[0] > 0x1f) t+= 4;
    if (((UBYTE*)packet)[0] > 0x5f) t+= 2;

    while (l<=t)
    {
        cmd[l] = ((UBYTE*)packet)[l];
        ++l;
    }
    
    DD({
        bug("[ATA:%02lx] atapi_SendPacket - Sending %s ATA packet: ", unit->au_UnitNum, (*dma) ? "DMA" : "PIO");
        l=0;
        while (l<12)
        {
            bug("%02lx ", ((UBYTE*)cmd)[l]);
            ++l;
        }
        bug("\n");

        DERROR(if (datalen & 1) bug("[ATAPI] atapi_SendPacket - ERROR: DATA LENGTH NOT EVEN! Rounding Up! (%ld bytes requested)\n", datalen));
    });

    //datalen = (datalen+1)&~1;

    if (FALSE == ata_SelectUnit(unit))
    {
        DERROR(bug("[ATAPI] atapi_SendPacket - WaitBusy failed at first check\n"));
        return IOERR_UNITBUSY;
    }

    do
    {
        status = PIO_In(unit->au_Bus, ata_Status);
        ata_WaitNano(400, bus->ab_Base);
        DD(bug("B"));

        retry_busy--;
        if (retry_busy == 0)
        {
            DERROR(bug("\n[ATAPI] *** ERROR: ATAF_BUSY | ATAF_DATAREQ not cleared!"));
            break;
        }
    } while ((status & (ATAF_BUSY | ATAF_DATAREQ)) != 0);  // Wait for BSY and DRQ to CLEAR

    DD(bug("\n[ATAPI] atapi_SendPacket - Data Lenght = %u\n", datalen));
    PIO_Out(bus, (datalen & 0xff), atapi_ByteCntL);
    PIO_Out(bus, (datalen >> 8) & 0xff, atapi_ByteCntH);
    
    DD(status = PIO_In(bus, ata_Status));
    DD(bug("[ATAPI] atapi_SendPacket - Status after atapi_ByteCntL/H: %lx\n", status)); 

    PIO_OutAlt(bus, ATACTLF_INT_DISABLE, ata_AltControl);
    PIO_Out(bus, ATA_PACKET, atapi_Command);

    do
    {
        status = PIO_In(unit->au_Bus, ata_Status);
        ata_WaitNano(400, bus->ab_Base);
        DD(bug("D"));
        retry_datareq--;
        if (retry_datareq == 0)
        {
            DERROR(bug("\n[ATAPI] *** ERROR: ATAF_DATAREQ not set!"));
            break;
        }         
    } while ((status & ATAF_DATAREQ) != ATAF_DATAREQ);  // Wait for DRQ to SET

    if (datalen == 0) ata_IRQSetHandler(unit, &ata_IRQNoData, 0, 0, 0);
    else if (write)   ata_IRQSetHandler(unit, &ata_IRQPIOWriteAtapi, data, 0, datalen);
    else              ata_IRQSetHandler(unit, &ata_IRQPIOReadAtapi, data, 0, datalen);

    Unit_OutS(unit, cmd, 12);
    ata_WaitNano(400, bus->ab_Base);

    status = PIO_In(bus, ata_Status);
    DD(bug("\n[ATAPI] atapi_SendPacket - Status after atapi_Command: %lx\n", status));  

    if (status & ATAF_ERROR) // Check for BUS Error
    {
        DERROR(bug("[ATAPI] atapi_SendPacket - BUS Error (Bad Status)\n"));
        return HFERR_BadStatus;
    } 

    if (datalen > 0)
    {
        if (write)
        {
            DD(bug("[ATAPI] atapi_SendPacket - Start ata_IRQPIOWriteAtapi\n"));
            ata_IRQPIOWriteAtapi(unit, ATAF_BUSY);
        } else {
            DD(bug("[ATAPI] atapi_SendPacket - Start ata_IRQPIOReadAtapi\n"));           
            ata_IRQPIOReadAtapi(unit, ATAF_BUSY);
        }
    }

    err = atapi_EndCmd(unit);

    return err;
}

static BYTE atapi_DirectSCSI(struct ata_Unit *unit, struct SCSICmd *cmd)
{
    BYTE err = 0;
    BOOL dma = FALSE;
    
    cmd->scsi_Actual = 0;

    DD(bug("[ATA:%02lx] atapi_DirectSCSI: Sending ATAPI packet\n", unit->au_UnitNum));

    err = atapi_SendPacket(unit, cmd->scsi_Command, cmd->scsi_Data, cmd->scsi_Length, &dma, (cmd->scsi_Flags & SCSIF_READ) == 0);

    DD(bug("[ATA:%02lx] atapi_DirectSCSI: SCSI Flags: %02lx / Error: %ld\n", unit->au_UnitNum, cmd->scsi_Flags, err));

     if ((err != 0) && (cmd->scsi_Flags & SCSIF_AUTOSENSE))
    {
        DERROR(bug("[DSCSI] atapi_DirectSCSI: Calling atapi_RequestSense\n"));
        atapi_RequestSense(unit, cmd->scsi_SenseData, cmd->scsi_SenseLength);
    }

    return err;
}

/*
 * chops the large transfers into set of smaller transfers
 * specifically useful when requested transfer size is >256 sectors for 28bit commands
 */
static BYTE ata_exec_blk(struct ata_Unit *unit, ata_CommandBlock *blk)
{
    BYTE err = 0;
    ULONG part;
    ULONG max=256;
    ULONG count=blk->sectors;
    APTR buffer = blk->buffer;
    IPTR bounce_buffer_length = 0;

    if (blk->type == CT_LBA48) max <<= 8;

    DDD(bug("[ATA:%02ld] ata_exec_blk: Accessing %ld sectors starting from %x%08x\n", unit->au_UnitNum, count, (ULONG)(blk->blk >> 32), (ULONG)blk->blk));

    while ((count > 0) && (err == 0))
    {
        part = (count > max) ? max : count;
        blk->sectors = part;
        blk->length  = part << unit->au_SectorShift;

        DDD(bug("[ATA:%02ld] Transfer of %ld sectors from %x%08x\n", unit->au_UnitNum, part, (ULONG)(blk->blk >> 32), (ULONG)blk->blk));

        err = ata_exec_cmd(unit, blk);
        DDD(bug("[ATA:%02ld] ata_exec_blk: ata_exec_cmd returned %lx\n", unit->au_UnitNum, err));
        blk->buffer  = (APTR)((IPTR)blk->buffer + (part << unit->au_SectorShift));
        
        blk->blk    += part;
        count -= part;
    }

    return err;
}


void ata_init_unit(struct ata_Bus *bus, struct ata_Unit *unit, UBYTE u)
{
    struct ataBase *ATABase = bus->ab_Base;
    OOP_Object *obj = OOP_OBJECT(ATABase->busClass, bus);

    UWORD ApolloBoardID     = *(volatile UWORD *)0xdff3fc;  
    ApolloBoardID = ApolloBoardID >> 8; 

    unit->au_Bus        = bus;
    unit->pioInterface  = bus->pioInterface;
    unit->au_UnitNum    = bus->ab_BusNum << 1 | u;
    unit->au_DevMask    = 0xa0 | (u << 4);

    BOOL atapi = unit->au_Bus->ab_Dev[unit->au_UnitNum & 1] & 0x80;

    if (atapi)
    {
        unit->au_UseModes &= ~AF_XFER_PIO32;
        unit->au_ins       = bus->pioVectors->ata_insw;
        unit->au_outs      = bus->pioVectors->ata_outsw;
        DINIT(bug("[ATA:%02u] ata_init_unit: BUS = %u | UNIT = %d | PIO = ATAPI (16-Bit)\n", unit->au_UnitNum, bus->ab_BusNum, u);)
    } else {
        unit->au_UseModes |= AF_XFER_PIO32;
        unit->au_ins       = bus->pioVectors->ata_insl;
        unit->au_outs      = bus->pioVectors->ata_outsl;
        DINIT(bug("[ATA:%02u] ata_init_unit: BUS = %u | UNIT = %d | PIO = ATA (32-Bit)\n", unit->au_UnitNum, bus->ab_BusNum, u);)
    }
}

BOOL ata_setup_unit(struct ata_Bus *bus, struct ata_Unit *unit)
{
    UBYTE u;

    DINIT(bug("[ATA:%02ld] ata_setup_unit\n", unit->au_UnitNum);)
    ata_SelectUnit(unit);

    if (FALSE == ata_WaitBusyTO(unit, 1, FALSE, FALSE, NULL))
    {
        DERROR(bug("[ATA:%02ld] ata_setup_unit: ERROR: Drive not ready for use. Keeping functions stubbed\n", unit->au_UnitNum));
        return FALSE;
    }

    u = unit->au_UnitNum & 1;
    switch (bus->ab_Dev[u])
    {
        case DEV_SATAPI:
        case DEV_ATAPI:
        case DEV_SATA:
        case DEV_ATA:
            unit->au_Identify = ata_Identify;
            break;

        default:
        DINIT(bug("[ATA:%02ld] ata_setup_unit: Unsupported device %lx. All functions will remain stubbed.\n", unit->au_UnitNum, bus->ab_Dev[u]);)
            return FALSE;
    }

    if (unit->au_Identify(unit) != 0)
    {
        return FALSE;
    }

    return TRUE;
}


static void common_SetXferMode(struct ata_Unit* unit, ata_XferMode mode)
{
    struct ata_Bus *bus = unit->au_Bus;
    BOOL dma = FALSE;
/*
 * We can't set drive modes unless we also set the controller's timing registers
 * FIXME:   Implement aoHodd_ATABus_CanSetXferMode and moHidd_ATABus_SetXferMode
            support.
 * CHECKME: Current code lives with what machine's firmware has set for us. Looks
 *          like all firmwares set up the best DMA mode. But what if the firmware
 *          didn't set it up for some reason (the add-on controller which has been
 *          ignored by it
 *          for example) ? Shouldn't we check unit->au_UseModes here ? 
 */

 DINIT(bug("[ATA:%02ld] common_SetXferMode: Trying to set mode %d\n", unit->au_UnitNum, mode));

    /*
     * first, ONLY for ATA devices, set new commands
     */
    if (0 == (unit->au_XferModes & AF_XFER_PACKET))
    {
        if ((!unit->au_Bus->ab_Base->ata_NoMulti) && (unit->au_XferModes & AF_XFER_RWMULTI))
        {
            /* Multisector PIO */
            ata_IRQSetHandler(unit, ata_IRQNoData, NULL, 0, 0);
            PIO_Out(bus, unit->au_Drive->id_RWMultipleSize & 0xFF, ata_Count);
            PIO_Out(bus, ATA_SET_MULTIPLE, ata_Command);
            ata_WaitBusyTO(unit, -1, TRUE, FALSE, NULL);

            unit->au_UseModes |= AF_XFER_RWMULTI;
            unit->au_Read32    = ata_ReadMultiple32;
            unit->au_Write32   = ata_WriteMultiple32;
            if (unit->au_XferModes & AF_XFER_48BIT)
            {
                unit->au_UseModes |= AF_XFER_48BIT;
                unit->au_Read64    = ata_ReadMultiple64;
                unit->au_Write64   = ata_WriteMultiple64;
            }
        }
        else
        {
            /* 1-sector PIO */
            unit->au_Read32  = ata_ReadSector32;
            unit->au_Write32 = ata_WriteSector32;
            if (unit->au_XferModes & AF_XFER_48BIT)
            {
                unit->au_UseModes |= AF_XFER_48BIT;
                unit->au_Read64    = ata_ReadSector64;
                unit->au_Write64   = ata_WriteSector64;
            }
        }
    }

#if 0 // We can't set drive modes unless we also set the controller's timing registers
    if ((mode >= AB_XFER_PIO0) && (mode <= AB_XFER_PIO4))
    {
        type = 8 + (mode - AB_XFER_PIO0);
    }
    else if ((mode >= AB_XFER_MDMA0) && (mode <= AB_XFER_MDMA2))
    {
        type = 32 + (mode - AB_XFER_MDMA0);
        dma=TRUE;
    }
    else if ((mode >= AB_XFER_UDMA0) && (mode <= AB_XFER_UDMA6))
    {
        type = 64 + (mode - AB_XFER_UDMA0);
        dma=TRUE;
    }
    else
    {
        type = 0;
    }

    acb.sectors = type;
    if (0 != ata_exec_cmd(unit, &acb))
    {
        DINIT(bug("[ATA:%02ld] common_SetXferMode: ERROR: Failed to apply new xfer mode.\n", unit->au_UnitNum));
    }

    if (!HIDD_ATABus_SetXferMode(obj, mode))
    {
        /*
         * DMA mode setup failed.
         * FIXME: Should completely revert back to PIO protocol, or try lower mode.
         */
        dma = FALSE;
    }
#else
    if (mode >= AB_XFER_MDMA0)
        dma = TRUE;
#endif

    if (dma)
    {
        unit->au_Flags |= AF_DMA; /* This flag is used by ATAPI protocol */
    }
    else
    {
    unit->au_UseModes &= ~AF_XFER_DMA_MASK;
    unit->au_Flags    &= ~AF_DMA;
    }
}

static void common_SetBestXferMode(struct ata_Unit* unit)
{
    struct ata_Bus *bus = unit->au_Bus;
    struct ataBase *ATABase = bus->ab_Base;
    OOP_Object *obj = OOP_OBJECT(ATABase->busClass, bus);
    int iter;
    int max = AB_XFER_UDMA6;

    if ((!bus->dmaInterface)
        || (   !(unit->au_Drive->id_MWDMASupport & 0x0700)
            && !(unit->au_Drive->id_UDMASupport  & 0x7f00)))
    {
        /*
         * make sure you reduce scan search to pio here!
         * otherwise this and above function will fall into infinite loop
         */
        DINIT(bug("[ATA:%02ld] common_SetBestXferMode: DMA is disabled for this drive.\n", unit->au_UnitNum));
        max = AB_XFER_PIO4;
    }
    else if (!OOP_GET(obj, aHidd_ATABus_Use80Wire))
    {
        DINIT(bug("[ATA:%02ld] common_SetBestXferMode: An 80-wire cable has not been detected for this drive. Disabling modes above UDMA2.\n", unit->au_UnitNum));
        max = AB_XFER_UDMA2;
    }

    for (iter=max; iter>=AB_XFER_PIO0; --iter)
    {
        if (unit->au_XferModes & (1<<iter))
        {
            common_SetXferMode(unit, iter);
            return;
        }
    }
    DERROR(bug("[ATA:%02ld] common_SetBestXferMode: ERROR: device never reported any valid xfer modes. will continue at default\n", unit->au_UnitNum);)
    common_SetXferMode(unit, AB_XFER_PIO0);
}

static void common_DetectXferModes(struct ata_Unit* unit)
{
    int iter;

    DINIT(bug("[ATA:%02ld] common_DetectXferModes: Supports\n", unit->au_UnitNum));

    if (unit->au_Drive->id_Commands4 & (1 << 4))
    {
        DINIT(bug("[ATA:%02ld] common_DetectXferModes: - Packet interface\n", unit->au_UnitNum));
        unit->au_XferModes     |= AF_XFER_PACKET;
        unit->au_DirectSCSI     = atapi_DirectSCSI;
    }
    else if (unit->au_Drive->id_Commands5 & (1 << 10))
    {
        /* ATAPI devices do not use this bit. */
        DINIT(bug("[ATA:%02ld] common_DetectXferModes: - 48bit I/O\n", unit->au_UnitNum));
        unit->au_XferModes     |= AF_XFER_48BIT;
    }

    if ((unit->au_XferModes & AF_XFER_PACKET) || (unit->au_Drive->id_Capabilities & (1<< 9)))
    {
        DINIT(bug("[ATA:%02ld] common_DetectXferModes: - LBA Addressing\n", unit->au_UnitNum));
        unit->au_XferModes     |= AF_XFER_LBA;
        unit->au_UseModes      |= AF_XFER_LBA;
    }
    else
    {
        DINIT(bug("[ATA:%02ld] common_DetectXferModes: - DEVICE DOES NOT SUPPORT LBA ADDRESSING >> THIS IS A POTENTIAL PROBLEM <<\n", unit->au_UnitNum));
        unit->au_Flags |= AF_CHSOnly;
    }

    if (unit->au_Drive->id_RWMultipleSize & 0xff)
    {
        DINIT(bug("[ATA:%02ld] common_DetectXferModes: - R/W Multiple (%ld sectors per xfer)\n", unit->au_UnitNum, unit->au_Drive->id_RWMultipleSize & 0xff));
        unit->au_XferModes     |= AF_XFER_RWMULTI;
    }

    DINIT(bug("[ATA:%02ld] common_DetectXferModes: - PIO0 PIO1 PIO2 ", unit->au_UnitNum));

    unit->au_XferModes |= AF_XFER_PIO(0) | AF_XFER_PIO(1) | AF_XFER_PIO(2);
    if (unit->au_Drive->id_ConfigAvailable & (1 << 1))
    {
        for (iter = 0; iter < 2; iter++)
        {
            if (unit->au_Drive->id_PIOSupport & (1 << iter))
            {
                DINIT(bug("PIO%ld ", 3 + iter));
                unit->au_XferModes |= AF_XFER_PIO(3 + iter);
            }
        }
        DINIT(bug("\n"));
    }

    if ((unit->au_Drive->id_ConfigAvailable & (1 << 1)) &&
        (unit->au_Drive->id_Capabilities & (1<<8)))
    {
        DINIT(bug("[ATA:%02ld] common_DetectXferModes: DMA:\n", unit->au_UnitNum));
        if (unit->au_Drive->id_MWDMASupport & 0xff)
        {
            DINIT(bug("[ATA:%02ld] common_DetectXferModes: - ", unit->au_UnitNum));
            for (iter = 0; iter < 3; iter++)
            {
                if (unit->au_Drive->id_MWDMASupport & (1 << iter))
                {
                    unit->au_XferModes |= AF_XFER_MDMA(iter);
                    if (unit->au_Drive->id_MWDMASupport & (256 << iter))
                    {
                        unit->au_UseModes |= AF_XFER_MDMA(iter);
                        DINIT(bug("[MDMA%ld] ", iter));
                    }
                        DINIT(else bug("MDMA%ld ", iter);)
                }
            }
            DINIT(bug("\n"));
        }

        if (unit->au_Drive->id_UDMASupport & 0xff)
        {
            DINIT(bug("[ATA:%02ld] common_DetectXferModes: - ", unit->au_UnitNum));
            for (iter = 0; iter < 7; iter++)
            {
                if (unit->au_Drive->id_UDMASupport & (1 << iter))
                {
                    unit->au_XferModes |= AF_XFER_UDMA(iter);
                    if (unit->au_Drive->id_UDMASupport & (256 << iter))
                    {
                        unit->au_UseModes |= AF_XFER_UDMA(iter);
                        DINIT(bug("[UDMA%ld] ", iter));
                    }
                        DINIT(else bug("UDMA%ld ", iter);)
                }
            }
            DINIT(bug("\n"));
        }
    }
}

#define SWAP_LE_WORD(x) (x) = AROS_LE2WORD((x))
#define SWAP_LE_LONG(x) (x) = AROS_LE2LONG((x))
#define SWAP_LE_QUAD(x) (x) = AROS_LE2LONG((x) >> 32) | (((QUAD)(AROS_LE2LONG((x) & 0xffffffff))) << 32)

static BYTE ata_Identify(struct ata_Unit *unit)
{
    BOOL atapi = unit->au_Bus->ab_Dev[unit->au_UnitNum & 1] & 0x80;
    BOOL supportLBA, supportLBA48;
    ata_CommandBlock acb = {atapi ? ATA_IDENTIFY_ATAPI : ATA_IDENTIFY_DEVICE,0,1,0,0,0,unit->au_Drive,sizeof(struct DriveIdent),0,CM_PIORead,CT_NoBlock};
    UWORD n = 0, *p, *limit;

    DINIT(bug("[ATA:%02ld] ata_Identify: Executing ATA_IDENTIFY_%s command\n", unit->au_UnitNum, atapi ? "ATAPI" : "DEVICE");)
    if (ata_exec_cmd(unit, &acb))
    {
        acb.command = atapi ? ATA_IDENTIFY_DEVICE : ATA_IDENTIFY_ATAPI;
        DINIT(bug("[ATA:%02ld] ata_Identify: Executing ATA_IDENTIFY_%s command instead\n", unit->au_UnitNum, atapi ? "DEVICE" : "ATAPI");)
        if (ata_exec_cmd(unit, &acb))
        {
            DINIT(bug("[ATA:%02ld] ata_Identify: Both command variants failed. Discarding drive.\n", unit->au_UnitNum);)
            return IOERR_OPENFAIL;
        }
        unit->au_Bus->ab_Dev[unit->au_UnitNum & 1] ^= 0x82;
        atapi = unit->au_Bus->ab_Dev[unit->au_UnitNum & 1] & 0x80;
        DINIT(bug("[ATA:%02ld] ata_Identify: Incorrect device signature detected. Switching device type to %lx.\n", unit->au_UnitNum, unit->au_Bus->ab_Dev[unit->au_UnitNum & 1]);)
    }

#if (AROS_BIG_ENDIAN != 0)
    SWAP_LE_WORD(unit->au_Drive->id_General);
    SWAP_LE_WORD(unit->au_Drive->id_OldCylinders);
    SWAP_LE_WORD(unit->au_Drive->id_SpecificConfig);
    SWAP_LE_WORD(unit->au_Drive->id_OldHeads);
    SWAP_LE_WORD(unit->au_Drive->id_OldSectors);
    SWAP_LE_WORD(unit->au_Drive->id_RWMultipleSize);
    SWAP_LE_WORD(unit->au_Drive->id_Capabilities);
    SWAP_LE_WORD(unit->au_Drive->id_OldCaps);
    SWAP_LE_WORD(unit->au_Drive->id_OldPIO);
    SWAP_LE_WORD(unit->au_Drive->id_ConfigAvailable);
    SWAP_LE_WORD(unit->au_Drive->id_OldLCylinders);
    SWAP_LE_WORD(unit->au_Drive->id_OldLHeads);
    SWAP_LE_WORD(unit->au_Drive->id_OldLSectors);
    SWAP_LE_WORD(unit->au_Drive->id_RWMultipleTrans);
    SWAP_LE_WORD(unit->au_Drive->id_DMADir);
    SWAP_LE_WORD(unit->au_Drive->id_MWDMASupport);
    SWAP_LE_WORD(unit->au_Drive->id_PIOSupport);
    SWAP_LE_WORD(unit->au_Drive->id_MWDMA_MinCycleTime);
    SWAP_LE_WORD(unit->au_Drive->id_MWDMA_DefCycleTime);
    SWAP_LE_WORD(unit->au_Drive->id_PIO_MinCycleTime);
    SWAP_LE_WORD(unit->au_Drive->id_PIO_MinCycleTimeIORDY);
    SWAP_LE_WORD(unit->au_Drive->id_QueueDepth);
    SWAP_LE_WORD(unit->au_Drive->id_ATAVersion);
    SWAP_LE_WORD(unit->au_Drive->id_ATARevision);
    SWAP_LE_WORD(unit->au_Drive->id_Commands1);
    SWAP_LE_WORD(unit->au_Drive->id_Commands2);
    SWAP_LE_WORD(unit->au_Drive->id_Commands3);
    SWAP_LE_WORD(unit->au_Drive->id_Commands4);
    SWAP_LE_WORD(unit->au_Drive->id_Commands5);
    SWAP_LE_WORD(unit->au_Drive->id_Commands6);
    SWAP_LE_WORD(unit->au_Drive->id_UDMASupport);
    SWAP_LE_WORD(unit->au_Drive->id_SecurityEraseTime);
    SWAP_LE_WORD(unit->au_Drive->id_ESecurityEraseTime);
    SWAP_LE_WORD(unit->au_Drive->id_CurrentAdvPowerMode);
    SWAP_LE_WORD(unit->au_Drive->id_MasterPwdRevision);
    SWAP_LE_WORD(unit->au_Drive->id_HWResetResult);
    SWAP_LE_WORD(unit->au_Drive->id_AcousticManagement);
    SWAP_LE_WORD(unit->au_Drive->id_StreamMinimunReqSize);
    SWAP_LE_WORD(unit->au_Drive->id_StreamingTimeDMA);
    SWAP_LE_WORD(unit->au_Drive->id_StreamingLatency);
    SWAP_LE_WORD(unit->au_Drive->id_StreamingTimePIO);
    SWAP_LE_WORD(unit->au_Drive->id_PhysSectorSize);
    SWAP_LE_WORD(unit->au_Drive->id_RemMediaStatusNotificationFeatures);
    SWAP_LE_WORD(unit->au_Drive->id_SecurityStatus);
    SWAP_LE_LONG(unit->au_Drive->id_WordsPerLogicalSector);
    SWAP_LE_LONG(unit->au_Drive->id_LBASectors);
    SWAP_LE_LONG(unit->au_Drive->id_StreamingGranularity);
    SWAP_LE_QUAD(unit->au_Drive->id_LBA48Sectors);
#endif

    if (atapi)
    {
        unit->au_SectorShift    = 11;
        unit->au_Read32         = atapi_Read;
        unit->au_Write32        = atapi_Write;
        unit->au_DirectSCSI     = atapi_DirectSCSI;
        unit->au_Eject          = atapi_Eject;
        unit->au_Flags         |= AF_DiscChanged;
        unit->au_DevType        = (unit->au_Drive->id_General >>8) & 0x1f;
        unit->au_XferModes      = AF_XFER_PACKET;
        unit->au_UseModes      |= AF_XFER_PACKET; /* OR because this field may already contain AF_XFER_PIO32 */
    } else {
        unit->au_SectorShift    = 9;
        unit->au_DevType        = DG_DIRECT_ACCESS;
        unit->au_Read32         = ata_ReadSector32;
        unit->au_Write32        = ata_WriteSector32;
        unit->au_Eject          = ata_Eject;
        unit->au_XferModes      = 0;
        unit->au_Flags         |= AF_DiscPresent | AF_DiscChanged;
    }

    ata_strcpy(unit->au_Drive->id_Model, unit->au_Model, 40);
    ata_strcpy(unit->au_Drive->id_SerialNumber, unit->au_SerialNumber, 20);
    ata_strcpy(unit->au_Drive->id_FirmwareRev, unit->au_FirmwareRev, 8);

    DINIT(bug("[ATA:%02ld] ata_Identify: Unit info: Model=%s | Serial=%s | Firmware=%s | DevType = %u\n",
        unit->au_UnitNum, unit->au_Model, unit->au_SerialNumber, unit->au_FirmwareRev, unit->au_DevType);)

    if (atapi)
    {
        DINIT(bug("[ATA:%02ld] ata_Identify: Device is removable.\n", unit->au_UnitNum);)
        unit->au_Flags |= AF_Removable;
        common_DetectXferModes(unit);
        common_SetBestXferMode(unit);
    }

    supportLBA = (unit->au_Drive->id_Capabilities & (1 << 9)) != 0;
    supportLBA48 = supportLBA && (unit->au_Drive->id_Commands5 & (1 << 10)) != 0;

    unit->au_Capacity = unit->au_Drive->id_LBASectors;
    if (supportLBA48) unit->au_Capacity48 = unit->au_Drive->id_LBA48Sectors;
    else unit->au_Capacity48 = unit->au_Capacity;

    if (atapi)
    {
        switch (unit->au_DevType)
        {
            case DG_CDROM:
            case DG_WORM:
            case DG_OPTICAL_DISK:
                unit->au_SectorShift    = 11;
                unit->au_Heads          = 1;
                unit->au_Sectors        = 75;
                unit->au_Cylinders      = 4440;
                break;

            case DG_DIRECT_ACCESS:
                unit->au_SectorShift = 9;
                if (!strcmp("LS-120", &unit->au_Model[0]))                  // CHANGE: HARDCODED SUCKS
                {
                    unit->au_Heads      = 2;
                    unit->au_Sectors    = 18;
                    unit->au_Cylinders  = 6848;
                }
                else if (!strcmp("ZIP 100 ", &unit->au_Model[8]))           // CHANGE: HARDCODED SUCKS
                {
                    unit->au_Heads      = 1;
                    unit->au_Sectors    = 64;
                    unit->au_Cylinders  = 3072;
                }
                break;
        }

        atapi_TestUnitOK(unit);
        atapi_TestUnitOK(unit);
        
    } else {
        /*
           For drive capacities > 8.3GB assume maximal possible layout.
           It really doesn't matter here, as BIOS will not handle them in
           CHS way anyway :)
           i guess this just solves that weirdo div-by-zero crash, if nothing
           else...
           */
        if (supportLBA && ((unit->au_Drive->id_LBA48Sectors > (63 * 255 * 1024)) || (unit->au_Drive->id_LBASectors > (63 * 255 * 1024))))
        {
            ULONG div = 1;
            /*
             * TODO: this shouldn't be casted down here.
             */
            ULONG sec = unit->au_Capacity48;

            if (sec < unit->au_Capacity48)
                sec = ~((ULONG)0);

            if (sec < unit->au_Capacity)
                sec = unit->au_Capacity;

            unit->au_Sectors = 63;
            sec /= 63;
            /*
             * keep dividing by 2
             */
            do
            {
                if (((sec >> 1) << 1) != sec)
                    break;
                if ((div << 1) > 255)
                    break;
                div <<= 1;
                sec >>= 1;
            } while (1);

            do
            {
                if (((sec / 3) * 3) != sec)
                    break;
                if ((div * 3) > 255)
                    break;
                div *= 3;
                sec /= 3;
            } while (1);

            unit->au_Cylinders  = sec;
            unit->au_Heads      = div;
        }
        else
        {
            unit->au_Cylinders  = unit->au_Drive->id_OldLCylinders;
            unit->au_Heads      = unit->au_Drive->id_OldLHeads;
            unit->au_Sectors    = unit->au_Drive->id_OldLSectors;
            if (!supportLBA) {
                unit->au_Capacity   = unit->au_Cylinders * unit->au_Heads * unit->au_Sectors;
                unit->au_Capacity48 = unit->au_Capacity;
            }
        }
    }

    DINIT(bug("[ATA:%02ld] ata_Identify: Cap28=%u | Cap48=%llu | CHS=%u/%u/%u\n", unit->au_UnitNum, unit->au_Capacity, unit->au_Capacity48, unit->au_Cylinders, unit->au_Heads, unit->au_Sectors);)

    return 0;
}

/*
 * ata read32 commands
 */
static BYTE ata_ReadSector32(struct ata_Unit *unit, ULONG block, ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_READ,
        0,
        1,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIORead,
        (unit->au_Flags & AF_CHSOnly) ? CT_CHS : CT_LBA28,
    };
    BYTE err;

    DDD(bug("\n[ATA:%02ld] ata_ReadSector32()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}

static BYTE ata_ReadMultiple32(struct ata_Unit *unit, ULONG block, ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_READ_MULTIPLE,
        0,
        unit->au_Drive->id_RWMultipleSize & 0xff,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIORead,
        (unit->au_Flags & AF_CHSOnly) ? CT_CHS : CT_LBA28,
    };
    BYTE err;

    DDD(bug("[ATA:%02ld] ata_ReadMultiple32()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}


/*
 * ata read64 commands
 */
static BYTE ata_ReadSector64(struct ata_Unit *unit, UQUAD block,
    ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_READ64,
        0,
        1,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIORead,
        CT_LBA48
    };
    BYTE err = 0;

    DDD(bug("[ATA:%02ld] ata_ReadSector64()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}

static BYTE ata_ReadMultiple64(struct ata_Unit *unit, UQUAD block,
    ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_READ_MULTIPLE64,
        0,
        unit->au_Drive->id_RWMultipleSize & 0xff,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIORead,
        CT_LBA48
    };
    BYTE err;

    DDD(bug("[ATA:%02ld] ata_ReadMultiple64()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}


/*
 * ata write32 commands
 */
static BYTE ata_WriteSector32(struct ata_Unit *unit, ULONG block,
    ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_WRITE,
        0,
        1,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIOWrite,
        (unit->au_Flags & AF_CHSOnly) ? CT_CHS : CT_LBA28,
    };
    BYTE err;

    DDD(bug("\n[ATA:%02ld] ata_WriteSector32()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb))) return err;

    *act = count << unit->au_SectorShift;
    return 0;
}

static BYTE ata_WriteMultiple32(struct ata_Unit *unit, ULONG block,
    ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_WRITE_MULTIPLE,
        0,
        unit->au_Drive->id_RWMultipleSize & 0xff,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIOWrite,
        (unit->au_Flags & AF_CHSOnly) ? CT_CHS : CT_LBA28,
    };
    BYTE err;

    DDD(bug("[ATA:%02ld] ata_WriteMultiple32()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}


/*
 * ata write64 commands
 */
static BYTE ata_WriteSector64(struct ata_Unit *unit, UQUAD block,
    ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_WRITE64,
        0,
        1,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIOWrite,
        CT_LBA48
    };
    BYTE err;

    DDD(bug("[ATA:%02ld] ata_WriteSector64()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}

static BYTE ata_WriteMultiple64(struct ata_Unit *unit, UQUAD block,
    ULONG count, APTR buffer, ULONG *act)
{
    ata_CommandBlock acb =
    {
        ATA_WRITE_MULTIPLE64,
        0,
        unit->au_Drive->id_RWMultipleSize & 0xff,
        0,
        block,
        count,
        buffer,
        count << unit->au_SectorShift,
        0,
        CM_PIOWrite,
        CT_LBA48
    };
    BYTE err;

    DDD(bug("[ATA:%02ld] ata_WriteMultiple64()\n", unit->au_UnitNum));

    *act = 0;
    if (0 != (err = ata_exec_blk(unit, &acb)))
        return err;

    *act = count << unit->au_SectorShift;
    return 0;
}


/*
 * ata miscellaneous commands
 */
static BYTE ata_Eject(struct ata_Unit *unit)
{
    ata_CommandBlock acb =
    {
        ATA_MEDIA_EJECT,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        CM_NoData,
        CT_NoBlock
    };

    DD(bug("[ATA:%02ld] ata_Eject()\n", unit->au_UnitNum));

    return ata_exec_cmd(unit, &acb);
}

/*
 * atapi commands
 */


int atapi_TestUnitOK(struct ata_Unit *unit)
{
    struct SCSICmd sc   = {0}; 
    
    UBYTE cmd[10]       = {0x25,0,0,0,0,0,0,0,0,0};

    struct
    {
        ULONG logicalsectors;
        ULONG blocksize;
    } capacity;

    BYTE result;

    sc.scsi_CmdLength   = sizeof(cmd);
    sc.scsi_Command     = (UBYTE*)&cmd;
    sc.scsi_CmdActual   = 0;
    sc.scsi_Flags       = SCSIF_READ;
    sc.scsi_Data        = (UWORD*)&capacity;
    sc.scsi_Length      = sizeof(capacity);
    sc.scsi_SenseData   = 0;

    DINIT(bug("\n[ATA:%02ld] atapi_TestUnitOK - Sending Read Capacity ATAPI Command\n", unit->au_UnitNum));
        
    result = unit->au_DirectSCSI(unit, &sc);

    if( result == 0 )                                       // Check for Error
    {
        if ( (unit->au_Flags & AF_DiscPresent) == 0 )       // TRUE -> Check if AF_DiscPresent is Cleared 
        {
            unit->au_Flags |= AF_DiscPresent;                   // TRUE -> Set AF_DiscPresent
            unit->au_Flags |= AF_DiscChanged;                   // TRUE -> Set AF_DiscChanged
        }
    } else {
        unit->au_Flags &= ~AF_DiscPresent;                  // FALSE -> clear AF_DiscPresent
    }

    DINIT(bug("[ATA:%02ld] atapi_TestUnitOK: | Result = %d | Media = %s | Change = %s\n", unit->au_UnitNum, result, unit->au_Flags & AF_DiscPresent ? "YES" : "NO", unit->au_Flags & AF_DiscChanged ? "YES" : "NO"));
   
    unit->au_Capacity   = capacity.logicalsectors;

    #define HEADS_PER_CYLINDER  16
    #define SECTORS_PER_TRACK   63

    ULONG translated_cylinders  = capacity.logicalsectors / (HEADS_PER_CYLINDER * SECTORS_PER_TRACK);

    unit->au_Cylinders  = (ULONG)translated_cylinders;
    unit->au_Heads      = (UBYTE)HEADS_PER_CYLINDER;
    unit->au_Sectors    = (UBYTE)SECTORS_PER_TRACK;

    DINIT(bug("[ATA:%02ld] atapi_TestUnitOK: LBA = %u | Block = %u | Cylinders = %u | Heads = %u | Sectors = %u\n",
        unit->au_UnitNum, capacity.logicalsectors, capacity.blocksize, unit->au_Cylinders, unit->au_Heads, unit->au_Sectors));
}

static BYTE atapi_Read(struct ata_Unit *unit, ULONG block, ULONG count, APTR buffer, ULONG *act)
{
    UBYTE cmd[] = {
       SCSI_READ10, 0, block>>24, block>>16, block>>8, block, 0, count>>8, count, 0
    };
    struct SCSICmd sc = {
       0
    };

    DD(bug("\n[ATA:%02ld] atapi_Read()\n", unit->au_UnitNum));

    sc.scsi_Command = (void*) &cmd;
    sc.scsi_CmdLength = sizeof(cmd);
    sc.scsi_Data = buffer;
    sc.scsi_Length = count << unit->au_SectorShift;
    sc.scsi_Flags = SCSIF_READ;

    return unit->au_DirectSCSI(unit, &sc);
}

static BYTE atapi_Write(struct ata_Unit *unit, ULONG block, ULONG count,
    APTR buffer, ULONG *act)
{
    UBYTE cmd[] = {
       SCSI_WRITE10, 0, block>>24, block>>16, block>>8, block, 0, count>>8, count, 0
    };
    struct SCSICmd sc = {
       0
    };

    DD(bug("\n[ATA:%02ld] atapi_Write()\n", unit->au_UnitNum));

    sc.scsi_Command = (void*) &cmd;
    sc.scsi_CmdLength = sizeof(cmd);
    sc.scsi_Data = buffer;
    sc.scsi_Length = count << unit->au_SectorShift;
    sc.scsi_Flags = SCSIF_WRITE;

    return unit->au_DirectSCSI(unit, &sc);
}

static BYTE atapi_Eject(struct ata_Unit *unit)
{
    struct atapi_StartStop cmd = {
        command: SCSI_STARTSTOP,
        immediate: 1,
        flags: ATAPI_SS_EJECT,
    };

    struct SCSICmd sc = {
       0
    };

    DD(bug("[ATA:%02ld] atapi_Eject()\n", unit->au_UnitNum));

    sc.scsi_Command = (void*) &cmd;
    sc.scsi_CmdLength = sizeof(cmd);
    sc.scsi_Flags = SCSIF_READ;

    return unit->au_DirectSCSI(unit, &sc);
}

static void atapi_RequestSense(struct ata_Unit* unit, UBYTE* sense, ULONG senselen)
{
    UBYTE cmd[] = {3, 0, 0, 0, senselen & 0xfe, 0};
    struct SCSICmd sc = {0};

    DD(bug("[ATA:%02ld] atapi_RequestSense()\n", unit->au_UnitNum));

    if ((senselen == 0) || (sense == 0)) return 0;
    
    sc.scsi_Data = (void*)sense;
    sc.scsi_Length = senselen & 0xfe;
    sc.scsi_Command = (void*)&cmd;
    sc.scsi_CmdLength = 6;
    sc.scsi_Flags = SCSIF_READ;

    unit->au_DirectSCSI(unit, &sc);

    DD(bug("[ATA:%02ld] atapi_RequestSense: SenseKey: %lx | ASC: %lx | ASCQ: %lx\n", unit->au_UnitNum, sense[2]&0xf, sense[12], sense[13]));
    return;

}

static ULONG ata_ReadSignature(struct ata_Bus *bus, int unit, BOOL *DiagExecuted)
{
    UBYTE status;
    UBYTE tmp1, tmp2;
    ULONG retrycount = 10000000;

    DINIT(bug("[ATA  ] ata_ReadSignature(%02ld)\n", unit));

    if (!*DiagExecuted)
    {
        DINIT(bug("[ATA  ] ata_ReadSignature: ATA_EXECUTE_DIAG\n"));
        PIO_Out(bus, ATA_EXECUTE_DIAG, ata_Command);
        ata_WaitNano(400, bus->ab_Base);
        *DiagExecuted = TRUE;
    }

    PIO_Out(bus, DEVHEAD_VAL | (unit << 4), ata_DevHead);

    do
    {
        status = PIO_In(bus, atapi_Status);
        retrycount--;
        ata_WaitNano(400, bus->ab_Base);
    }
    while ((retrycount > 0) && (status & ATAF_BUSY));
    
    DINIT( if (retrycount == 0) bug("[ATA  ] ata_ReadSignature: ERROR = ATAF_BUSY failed to clear within 10000000 tries\n") );
    
    tmp1 = PIO_In(bus, ata_LBAMid);
    tmp2 = PIO_In(bus, ata_LBAHigh);

    DINIT(bug("[ATA  ] ata_ReadSignature: LBAMid=%02lx | LBAHigh=%02lx | LBA=%04lx | Status=%02x\n", tmp1, tmp2, (tmp1 << 8) | tmp2, status));

    switch ((tmp1 << 8) | tmp2)
    {
        case 0x14eb:
            DINIT(bug("[ATA  ] ata_ReadSignature: Found signature for ATAPI device\n"));
            return DEV_ATAPI;

        case 0x3cc3:
            DINIT(bug("[ATA  ] ata_ReadSignature: Found signature for SATA device\n"));
            return DEV_SATA;

        case 0x6996:
            DINIT(bug("[ATA  ] ata_ReadSignature: Found signature for SATAPI device\n"));
            return DEV_SATAPI;

        default:
            if ( (ata_ReadStatus(bus) & 0xfe) == 0 )
            {
                DINIT(bug("[ATA  ] ata_ReadSignature: ERROR = NO Signature Found\n"));
                return DEV_NONE;
            } else {
                DINIT(bug("[ATA  ] ata_ReadSignature: Found signature for ATA device\n"));
                return DEV_ATA;
            }
    }
}

static void ata_ResetBus(struct ata_Bus *bus)
{
    struct ataBase *ATABase = bus->ab_Base;
    ULONG Counter;
    BOOL  DiagExecuted = FALSE;
    volatile UWORD *color0=0xDFF180;

    DINIT(bug("[ATA:ResetBus] Reset ATA Bus\n", bus->ab_BusNum);)

    PIO_OutAlt(bus, ATACTLF_RESET | ATACTLF_INT_DISABLE, ata_AltControl);
    for(Counter=100000; Counter ; Counter--)
    {
        *color0=0x0030;
        *color0=0x0000;
    }
    PIO_OutAlt(bus, ATACTLF_INT_DISABLE, ata_AltControl);
    ata_WaitTO(bus->ab_Timer, 0, 20000, 0);
    
    /*if ( bus->ab_Dev[0] != DEV_NONE )
    {
        PIO_Out(bus, DEVHEAD_VAL, ata_DevHead);
        ata_WaitTO(bus->ab_Timer, 0, 20000, 0);
        
        DINIT(bug("[ATA:ResetBus] Wait for Master to clear BSY\n", bus->ab_BusNum);)
        bus->ab_Dev[0] = DEV_NONE;
        for(Counter=20000000; Counter ; Counter--)
        {
            if ((ata_ReadStatus(bus) & ATAF_BUSY) == 0)
            {
                bus->ab_Dev[0] = DEV_UNKNOWN;
                break;
            } else {
                *color0=0x0090;
                *color0=0x0000;
            }
        }
        if (Counter==0) DERROR(bug("[ATA:ResetBus] ERROR: Master did NOT clear BSY\n", bus->ab_BusNum));
    }

    if ( bus->ab_Dev[1] != DEV_NONE )
    {
        PIO_Out(bus, DEVHEAD_VAL | (1 << 4), ata_DevHead);
        ata_WaitTO(bus->ab_Timer, 0, 20000, 0);
        
        DINIT(bug("[ATA:ResetBus] Wait for Slave to clear BSY\n", bus->ab_BusNum);)
        bus->ab_Dev[1] = DEV_NONE;
        for(Counter=20000000; Counter ; Counter--)
        {
            if ((ata_ReadStatus(bus) & ATAF_BUSY) == 0)
            {
                bus->ab_Dev[1] = DEV_UNKNOWN;
                break;
            } else {
                *color0=0x0090;
                *color0=0x0000;
            }
        }
        if (Counter==0) DERROR(bug("[ATA:ResetBus] ERROR: Slave did NOT clear BSY\n", bus->ab_BusNum));
    }*/

    if ( bus->ab_Dev[0] != DEV_NONE ) bus->ab_Dev[0] = ata_ReadSignature(bus, 0, &DiagExecuted);
    if ( bus->ab_Dev[1] != DEV_NONE ) bus->ab_Dev[1] = ata_ReadSignature(bus, 1, &DiagExecuted);
}

void ata_InitBus(struct ata_Bus *bus)
{
    struct ataBase *ATABase = bus->ab_Base;
    OOP_Object *obj = OOP_OBJECT(ATABase->busClass, bus);
    IPTR haveAltIO;
    UBYTE tmp1, tmp2;
    UWORD i;
    LONG try;
    volatile UWORD *color0=0xDFF180;

    /*
     * initialize timer for the sake of scanning
     */
    bus->ab_Timer = ata_OpenTimer(bus->ab_Base);

    OOP_GetAttr(obj, aHidd_ATABus_UseIOAlt, &haveAltIO);
    bus->haveAltIO = haveAltIO != 0;

    DINIT(bug("[ATA:InitBus] ata_InitBus(%p)\n", bus);)

    bus->ab_Dev[0] = DEV_NONE;
    bus->ab_Dev[1] = DEV_NONE;

    /* Check if device 0 and/or 1 is present on this bus. It may happen that
       a single drive answers for both device addresses, but the phantom
       drive will be filtered out later */
    for (i = 0; i < MAX_BUSUNITS; i++)
    {
        /* Select device and disable IRQs */
        PIO_Out(bus, DEVHEAD_VAL | (i << 4), ata_DevHead);
        ata_WaitTO(bus->ab_Timer, 0, 400, 0);
        PIO_OutAlt(bus, ATACTLF_INT_DISABLE, ata_AltControl);

        /* Write some pattern to registers. This is a variant of a more
           common technique, with the difference that we don't use the
           sector count register because some bad ATAPI drives disallow
           writing to it */
        PIO_Out(bus, 0x55, ata_LBALow);
        PIO_Out(bus, 0xaa, ata_LBAMid);
        PIO_Out(bus, 0xaa, ata_LBALow);
        PIO_Out(bus, 0x55, ata_LBAMid);
        PIO_Out(bus, 0x55, ata_LBALow);
        PIO_Out(bus, 0xaa, ata_LBAMid);

        tmp1 = PIO_In(bus, ata_LBALow);
        tmp2 = PIO_In(bus, ata_LBAMid);
        DINIT(bug("[ATA:%02d] ata_InitBus: Reply 0x%02X 0x%02X\n", i, tmp1, tmp2);)
 
        if ((tmp1 == 0x55) && (tmp2 == 0xaa))
        {
            bus->ab_Dev[i] = DEV_UNKNOWN;
            DINIT(bug("[ATA:%02d] ata_InitBus: Device type = 0x%02X\n", i, bus->ab_Dev[i]);)
        } else {
            DINIT(bug("[ATA:%02d] ata_InitBus: No Device Found\n", i);)
        }
    }
exit:
    ata_ResetBus(bus);
    ata_CloseTimer(bus->ab_Timer);
    DINIT(bug("[ATA:InitBus] ata_InitBus: Finished\n");)
}

/*
 * not really sure what this is meant to be - TO BE REPLACED
 */
static const ULONG ErrorMap[] = {
    CDERR_NotSpecified,
    CDERR_NoSecHdr,
    CDERR_NoDisk,
    CDERR_NoSecHdr,
    CDERR_NoSecHdr,
    CDERR_NOCMD,
    CDERR_NoDisk,
    CDERR_WriteProt,
    CDERR_NotSpecified,
    CDERR_NotSpecified,
    CDERR_NotSpecified,
    CDERR_ABORTED,
    CDERR_NotSpecified,
    CDERR_NotSpecified,
    CDERR_NoSecHdr,
    CDERR_NotSpecified,
};

static BYTE atapi_EndCmd(struct ata_Unit *unit)
{
    struct ata_Bus *bus = unit->au_Bus;
    UBYTE status, error;
    
    DD(bug("[ATA:%02ld] atapi_EndCmd: Command complete. Status: %lx | RETRY = %u\n\n", unit->au_UnitNum, status, retry_busy));

    if (!(status & ATAPIF_CHECK))
    {
        return 0;
    } else {
        error = PIO_In(bus, atapi_Error);
        DERROR(bug("[ATA:%02ld] atapi_EndCmd: ERROR code 0x%lx\n", unit->au_UnitNum, error));
        
        if (error != 0x60)
        {
            ULONG   sense_lenght = 18;
            UBYTE*  sense_data = AllocMem(sense_lenght, MEMF_ANY|MEMF_PUBLIC);
            
            atapi_RequestSense(unit, sense_data, sense_lenght);

            FreeMem(sense_data,sense_lenght);
        }
        
        return ErrorMap[error >> 4];
    }
}
