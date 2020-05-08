/*
 * Copyright (C) 2016, Jason S. McMullan <jason.mcmullan@gmail.com>
 * All rights reserved.
 *
 * Licensed under the MIT License:
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>     // memset
#include <stdarg.h>

#include <exec/resident.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/alerts.h>
#include <exec/tasks.h>
#include <exec/io.h>

#include <libraries/expansion.h>

#include <devices/newstyle.h>
#include <devices/trackdisk.h>
#include <devices/timer.h>
#include <devices/scsidisk.h>

#include <aros/symbolsets.h>

#include <dos/filehandler.h>

#include <proto/exec.h>
#include <proto/disk.h>
#include <proto/expansion.h>

#include "common.h"

#include <saga/sd.h>

#include LC_LIBDEFS_FILE

#define SAGASD_HEADS    16
#define SAGASD_SECTORS  64
#define SAGASD_RETRY    5      /* By default, retry up to N times */

#undef DEBUG
#define DEBUG 0

#if DEBUG
#define bug(x,args...)   kprintf(x ,##args)
#define debug(x,args...) bug("%s:%ld " x "\n", __func__, (unsigned long)__LINE__ ,##args)
#else
#define bug(x,args...)   do { } while (0)
#define debug(x,args...) do { } while (0)
#endif

static VOID SAGASD_log(struct sdcmd *sd, int level, const char *format, ...)
{
    va_list args;

    if (level > DEBUG)
        return;

    va_start(args, format);
    vkprintf(format, args);
    kprintf("\n");
    va_end(args);
}

/* Execute the SD read or write command, return IOERR_* or TDERR_*
 */
static LONG SAGASD_ReadWrite(struct IORequest *io, UQUAD off64, BOOL is_write)
{
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct IOExtTD *iotd = (struct IOExtTD *)io;
    struct SAGASDUnit *sdu = (struct SAGASDUnit *)io->io_Unit;
    APTR data = iotd->iotd_Req.io_Data;
    ULONG len = iotd->iotd_Req.io_Length;
    ULONG block, block_size, bmask;
    UBYTE sderr;

    debug("%s: Flags: $%lx, Command: $%04lx, Offset: $%lx%08lx Length: %5ld, Data: $%08lx",
            is_write ? "write" : "read",
            io->io_Flags, io->io_Command,
            (ULONG)(off64 >> 32), (ULONG)off64, len, data);

    block_size = sdu->sdu_SDCmd.info.block_size;
    bmask = block_size - 1;

    /* Read/Write is not permitted if the unit is not Valid */
    if (!sdu->sdu_Valid)
        return TDERR_BadDriveType;

    if (sdu->sdu_ReadOnly)
        return TDERR_WriteProt;

    if ((off64 & bmask) || bmask == 0 || data == NULL)
        return IOERR_BADADDRESS;

    if ((len & bmask) || len == 0) {
        debug("IO %p Fault, io_Flags = %d, io_Command = %d, IOERR_BADLENGTH (len=0x%x, bmask=0x%x)", io, io->io_Flags, io->io_Command, len, bmask);
        return IOERR_BADLENGTH;
    }

    /* Make in units of sector size */
    len /= block_size;
    block = off64 / block_size;

    /* Nothing to do... */
    if (len == 0) {
        iostd->io_Actual = 0;
        return 0;
    }

    debug("%s: block=%ld, blocks=%ld", is_write ? "Write" : "Read", block, len);

    /* Do the IO */
    if (is_write) {
        sderr = sdcmd_write_blocks(&sdu->sdu_SDCmd, block, data, len);
    } else {
        sderr = sdcmd_read_blocks(&sdu->sdu_SDCmd, block, data, len);
    }

    debug("sderr=$%02x", sderr);

    if (sderr) {
        iostd->io_Actual = 0;

        /* Decode sderr into IORequest io_Errors */
        if (sderr & SDERRF_TIMEOUT)
            return TDERR_DiskChanged;
        if (sderr & SDERRF_PARAM)
            return TDERR_SeekError;
        if (sderr & SDERRF_ADDRESS)
            return TDERR_SeekError;
        if (sderr & (SDERRF_ERASESEQ | SDERRF_ERASERES))
            return TDERR_BadSecPreamble;
        if (sderr & SDERRF_CRC)
            return TDERR_BadSecSum;
        if (sderr & SDERRF_ILLEGAL)
            return TDERR_TooFewSecs;
        if (sderr & SDERRF_IDLE)
            return TDERR_PostReset;
    }
    
    iostd->io_Actual = block_size * len;
    return 0;
}

static LONG SAGASD_PerformSCSI(struct IORequest *io)
{
    struct SAGASDBase *sd = (struct SAGASDBase *)io->io_Device;
    //struct Library *SysBase = sd->sd_ExecBase;
    struct SAGASDUnit *sdu = (struct SAGASDUnit *)io->io_Unit;
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct SCSICmd *scsi = iostd->io_Data;
    struct sdcmd *sdc = &sdu->sdu_SDCmd;
    UBYTE *data = (UBYTE *)scsi->scsi_Data;
    ULONG i, block, blocks;
    LONG err;
    UBYTE r1;

    debug("len=%ld, cmd = %02lx %02lx %02lx ... (%ld)",
            iostd->io_Length, scsi->scsi_Command[0],
            scsi->scsi_Command[1], scsi->scsi_Command[2],
            scsi->scsi_CmdLength);
    if (iostd->io_Length < sizeof(*scsi)) {
        // RDPrep sends a bad io_Length sometimes
        debug("====== BAD PROGRAM: iostd->io_Length < sizeof(struct SCSICmd)");
        //return IOERR_BADLENGTH;
    }

    if (scsi->scsi_CmdLength < 6)
        return IOERR_BADLENGTH;

    if (scsi->scsi_Command == NULL)
        return IOERR_BADADDRESS;

    scsi->scsi_Actual = 0;

    switch (scsi->scsi_Command[0]) {
    case 0x00:      // TEST_UNIT_READY
        err = 0;
        break;
    case 0x12:      // INQUIRY
        for (i = 0; i < scsi->scsi_Length; i++) {
            UBYTE val;

            switch (i) {
            case 0: // direct-access device
                    val = ((sdu->sdu_Enabled ? 0 : 1) << 5) | 0;
                    break;
            case 1: // RMB = 1
                    val = (1 << 7);
                    break;
            case 2: // VERSION = 0
                    val = 0;
                    break;
            case 3: // NORMACA=0, HISUP = 0, RESPONSE_DATA_FORMAT = 2
                    val = (0 << 5) | (0 << 4) | 2;
                    break;
            case 4: // ADDITIONAL_LENGTH = 44 - 4
                    val = 44 - 4;
                    break;
            default:
                    if (i >= 8 && i < 16)
                        val = "Vampire "[i - 8];
                    else if (i >= 16 && i < 32)
                        val = "SAGA-SD        "[i - 16];
                    else if (i >= 32 && i < 36)
                        val = ((UBYTE *)(((struct Library *)sd)->lib_IdString))[i-32];
                    else if (i >= 36 && i < 44) {
                        val = sdc->info.cid[7 + (i-36)/2];
                        if ((i & 1) == 0)
                            val >>= 4;
                        val = "0123456789ABCDEF"[val & 0xf];
                    } else
                        val = 0;
                    break;
            }

            data[i] = val;
        }

        scsi->scsi_Actual = i;
        err = 0;
        break;
    case 0x08: // READ (6)
        block = scsi->scsi_Command[1] & 0x1f;
        block = (block << 8) | scsi->scsi_Command[2];
        block = (block << 8) | scsi->scsi_Command[3];
        blocks = scsi->scsi_Command[4];
        debug("READ (6) %ld @%ld => $%lx (%ld)",
                blocks, block, data, scsi->scsi_Length);
        if (block + blocks > sdc->info.blocks) {
            err = IOERR_BADADDRESS;
            break;
        }
        if (scsi->scsi_Length < blocks * sdc->info.block_size) {
            debug("Len (%ld) too small (%ld)", scsi->scsi_Length, blocks * sdc->info.block_size);
            err = IOERR_BADLENGTH;
            break;
        }
        if (data == NULL) {
            err = IOERR_BADADDRESS;
            break;
        }
        r1 = sdcmd_read_blocks(sdc, block, data, blocks);
        if (r1) {
            err = HFERR_BadStatus;
            break;
        }

        scsi->scsi_Actual = scsi->scsi_Length;
        err = 0;
        break;
    case 0x0a: // WRITE (6)
        block = scsi->scsi_Command[1] & 0x1f;
        block = (block << 8) | scsi->scsi_Command[2];
        block = (block << 8) | scsi->scsi_Command[3];
        blocks = scsi->scsi_Command[4];
        debug("WRITE (6) %ld @%ld => $%lx (%ld)",
                blocks, block, data, scsi->scsi_Length);
        if (block + blocks > sdc->info.blocks) {
            err = IOERR_BADADDRESS;
            break;
        }
        if (scsi->scsi_Length < blocks * sdc->info.block_size) {
            debug("Len (%ld) too small (%ld)", scsi->scsi_Length, blocks * sdc->info.block_size);
            err = IOERR_BADLENGTH;
            break;
        }
        if (data == NULL) {
            err = IOERR_BADADDRESS;
            break;
        }
        r1 = sdcmd_write_blocks(sdc, block, data, blocks);
        if (r1) {
            err = HFERR_BadStatus;
            break;
        }

        scsi->scsi_Actual = scsi->scsi_Length;
        err = 0;
        break;
    case 0x25: // READ CAPACITY (10)
        if (scsi->scsi_CmdLength < 10) {
            err = HFERR_BadStatus;
            break;
        }

        block = scsi->scsi_Command[2];
        block = (block << 8) | scsi->scsi_Command[3];
        block = (block << 8) | scsi->scsi_Command[4];
        block = (block << 8) | scsi->scsi_Command[5];

        if ((scsi->scsi_Command[8] & 1) || block != 0) {
            // PMI Not supported
            err = HFERR_BadStatus;
            break;
        }

        if (scsi->scsi_Length < 8) {
            err = IOERR_BADLENGTH;
            break;
        }

        for (i = 0; i < 4; i++)
            data[0 + i] = ((sdc->info.blocks - 1) >> (24 - i*8)) & 0xff;
        for (i = 0; i < 4; i++)
            data[4 + i] = (sdc->info.block_size >> (24 - i*8)) & 0xff;

        scsi->scsi_Actual = 8;
        err = 0;
        break;
    case 0x1a: // MODE SENSE (6)
        data[0] = 3 + 8 + 0x16;
        data[1] = 0; // MEDIUM TYPE
        data[2] = 0;
        data[3] = 8;
        if (sdc->info.blocks > (1 << 24))
            blocks = 0xffffff;
        else
            blocks = sdc->info.blocks;
        data[4] = (blocks >> 16) & 0xff;
        data[5] = (blocks >>  8) & 0xff;
        data[6] = (blocks >>  0) & 0xff;
        data[7] = 0;
        data[8] = 0;
        data[9] = 0;
        data[10] = (sdc->info.block_size >> 8) & 0xff;
        data[11] = (sdc->info.block_size >> 0) & 0xff;
        switch (((UWORD)scsi->scsi_Command[2] << 8) | scsi->scsi_Command[3]) {
        case 0x0300: // Format Device Mode
            for (i = 0; i < scsi->scsi_Length - 12; i++) {
                UBYTE val;

                switch (i) {
                case 0: // PAGE CODE
                    val = 0x03;
                    break;
                case 1: // PAGE LENGTH
                    val = 0x16;
                    break;
                case 2: // TRACKS PER ZONE 15..8
                    val = (SAGASD_HEADS >> 8) & 0xff;
                    break;
                case 3: // TRACKS PER ZONE 7..0
                    val = (SAGASD_HEADS >> 0) & 0xff;
                    break;
                case 10: // SECTORS PER TRACK 15..8
                    val = (SAGASD_SECTORS >> 8) & 0xff;
                    break;
                case 11: // SECTORS PER TRACK 7..0
                    val = (SAGASD_SECTORS >> 0) & 0xff;
                    break;
                case 12: // DATA BYTES PER PHYSICAL SECTOR 15..8
                    val = (sdc->info.block_size >> 8) & 0xff;
                    break;
                case 13: // DATA BYTES PER PHYSICAL SECTOR 7..0
                    val = (sdc->info.block_size >> 0) & 0xff;
                    break;
                case 20: // HSEC = 1, RMB = 1
                    val = (1 << 6) | (1 << 5);
                    break;
                default:
                    val = 0;
                    break;
                }

                debug("data[%2ld] = $%02lx", 12 + i, val);
                data[12 + i] = val;
            }

            scsi->scsi_Actual = data[0] + 1;
            err = 0;
            break;
        case 0x0400: // Rigid Drive Geometry
            for (i = 0; i < scsi->scsi_Length - 12; i++) {
                UBYTE val;

                switch (i) {
                case 0: // PAGE CODE
                    val = 0x04;
                    break;
                case 1: // PAGE LENGTH
                    val = 0x16;
                    break;
                case 2: // CYLINDERS 23..16
                    val = ((sdc->info.blocks / SAGASD_HEADS / SAGASD_SECTORS) >> 16) & 0xff;
                    break;
                case 3: // CYLINDERS 15..8
                    val = ((sdc->info.blocks / SAGASD_HEADS / SAGASD_SECTORS) >> 8) & 0xff;
                    break;
                case 4: //  CYLINDERS 7..0
                    val = ((sdc->info.blocks / SAGASD_HEADS / SAGASD_SECTORS) >> 0) & 0xff;
                    break;
                case 5: // HEADS
                    val = SAGASD_HEADS;
                    break;
                default:
                    val = 0;
                    break;
                }

                data[12 + i] = val;
                debug("data[%2ld] = $%02lx", 12 + i, val);
            }

            scsi->scsi_Actual = data[0] + 1;
            err = 0;
            break;
        default:
            debug("MODE SENSE: Unknown Page $%02lx.$%02lx",
                    scsi->scsi_Command[2], scsi->scsi_Command[3]);
            err = HFERR_BadStatus;
            break;
        }
        break;
    default:
        debug("Unknown SCSI command %d (%d)\n", scsi->scsi_Command[0], scsi->scsi_CmdLength);
        err = IOERR_NOCMD;
        break;
    }

    if (err == 0)
        iostd->io_Actual = sizeof(*scsi);
    else
        iostd->io_Actual = 0;

    return err;
}

#define CMD_NAME(x) if (cmd == x) return #x

static inline const char *cmd_name(int cmd)
{
    CMD_NAME(CMD_READ);
    CMD_NAME(CMD_WRITE);
    CMD_NAME(CMD_UPDATE);
    CMD_NAME(CMD_CLEAR);
    CMD_NAME(TD_ADDCHANGEINT);
    CMD_NAME(TD_CHANGENUM);
    CMD_NAME(TD_CHANGESTATE);
    CMD_NAME(TD_EJECT);
    CMD_NAME(TD_FORMAT);
    CMD_NAME(TD_GETDRIVETYPE);
    CMD_NAME(TD_GETGEOMETRY);
    CMD_NAME(TD_MOTOR);
    CMD_NAME(TD_PROTSTATUS);
    CMD_NAME(TD_READ64);
    CMD_NAME(TD_REMCHANGEINT);
    CMD_NAME(TD_WRITE64);
    CMD_NAME(NSCMD_DEVICEQUERY);
    CMD_NAME(NSCMD_TD_READ64);
    CMD_NAME(NSCMD_TD_WRITE64);
    CMD_NAME(HD_SCSICMD);

    return "Unknown";
}

/*
 *  Try to do IO commands. All commands which require talking with ahci devices
 *  will be handled slow, that is they will be passed to bus task which will
 *  execute them as soon as hardware will be free.
 */
static LONG SAGASD_PerformIO(struct IORequest *io)
{
    const UWORD NSDSupported[] = {
        CMD_READ,
        CMD_WRITE,
        CMD_UPDATE,
        CMD_CLEAR,
        TD_ADDCHANGEINT,
        TD_CHANGENUM,
        TD_CHANGESTATE,
        TD_EJECT,
        TD_FORMAT,
        TD_GETDRIVETYPE,
        TD_GETGEOMETRY,
        TD_MOTOR,
        TD_PROTSTATUS,
        TD_READ64,
        TD_REMCHANGEINT,
        TD_WRITE64,
        NSCMD_DEVICEQUERY,
        NSCMD_TD_READ64,
        NSCMD_TD_WRITE64,
        HD_SCSICMD,
        0
    };
    struct SAGASDBase *sd = (struct SAGASDBase *)io->io_Device;
    struct Library *SysBase = sd->sd_ExecBase;
    struct SAGASDUnit *sdu = (struct SAGASDUnit *)io->io_Unit;
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct IOExtTD *iotd = (struct IOExtTD *)io;
    APTR data = iotd->iotd_Req.io_Data;
    ULONG len = iotd->iotd_Req.io_Length;
    UQUAD off64;
    struct DriveGeometry *geom;
    struct NSDeviceQueryResult *nsqr;
    LONG err = IOERR_NOCMD;

    debug("");

    if (io->io_Error == IOERR_ABORTED)
        return io->io_Error;

    debug("IO %p Start, io_Flags = %d, io_Command = %d (%s)", io, io->io_Flags, io->io_Command, cmd_name(io->io_Command));

    switch (io->io_Command) {
    case CMD_CLEAR:     /* Invalidate read buffer */
        iostd->io_Actual = 0;
        err = 0;
        break;
    case CMD_UPDATE:    /* Flush write buffer */
        iostd->io_Actual = 0;
        err = 0;
        break;
    case NSCMD_DEVICEQUERY:
        if (len < sizeof(*nsqr)) {
            err = IOERR_BADLENGTH;
            break;
        }

        nsqr = data;
        nsqr->DevQueryFormat = 0;
        nsqr->SizeAvailable  = sizeof(struct NSDeviceQueryResult);
        nsqr->DeviceType     = NSDEVTYPE_TRACKDISK;
        nsqr->DeviceSubType  = 0;
        nsqr->SupportedCommands = (UWORD *)NSDSupported;
        iostd->io_Actual = sizeof(*nsqr);
        err = 0;
        break;
    case TD_PROTSTATUS:
        iostd->io_Actual = sdu->sdu_ReadOnly ? 1 : 0;
        err = 0;
        break;
    case TD_CHANGENUM:
        iostd->io_Actual = sdu->sdu_ChangeNum;
        err = 0;
        break;
    case TD_CHANGESTATE:
        Forbid();
        iostd->io_Actual = sdu->sdu_Present ? 0 : 1;
        Permit();
        err = 0;
        break;
    case TD_EJECT:
        // Eject removable media
        // We mark is as invalid, then wait for Present to toggle.
        Forbid();
        sdu->sdu_Valid = FALSE;
        Permit();
        err = 0;
        break;
    case TD_GETDRIVETYPE:
        iostd->io_Actual = DRIVE_NEWSTYLE;
        err = 0;
        break;
    case TD_GETGEOMETRY:
        if (len < sizeof(*geom)) {
            err = IOERR_BADLENGTH;
            break;
        }

        geom = data;
        memset(geom, 0, len);
        geom->dg_SectorSize   = sdu->sdu_SDCmd.info.block_size;
        geom->dg_TotalSectors = sdu->sdu_SDCmd.info.blocks;
        geom->dg_Cylinders    = sdu->sdu_SDCmd.info.blocks / (SAGASD_HEADS * SAGASD_SECTORS);
        geom->dg_CylSectors   = SAGASD_HEADS * SAGASD_SECTORS;
        geom->dg_Heads        = SAGASD_HEADS;
        geom->dg_TrackSectors = SAGASD_SECTORS;
        geom->dg_BufMemType   = MEMF_PUBLIC;
        geom->dg_DeviceType   = DG_DIRECT_ACCESS;
        geom->dg_Flags        = DGF_REMOVABLE;
        iostd->io_Actual = sizeof(*geom);
        err = 0;
        break;
    case TD_FORMAT:
        off64  = iotd->iotd_Req.io_Offset;
        err = SAGASD_ReadWrite(io, off64, TRUE);
        break;
    case TD_MOTOR:
        // FIXME: Tie in with power management
        iostd->io_Actual = sdu->sdu_Motor;
        sdu->sdu_Motor = iostd->io_Length ? 1 : 0;
        err = 0;
        break;
    case CMD_WRITE:
        off64  = iotd->iotd_Req.io_Offset;
        err = SAGASD_ReadWrite(io, off64, TRUE);
        break;
    case TD_WRITE64:
    case NSCMD_TD_WRITE64:
        off64  = iotd->iotd_Req.io_Offset;
        off64 |= ((UQUAD)iotd->iotd_Req.io_Actual)<<32;
        err = SAGASD_ReadWrite(io, off64, TRUE);
        break;
    case CMD_READ:
        off64  = iotd->iotd_Req.io_Offset;
        err = SAGASD_ReadWrite(io, off64, FALSE);
        break;
    case TD_READ64:
    case NSCMD_TD_READ64:
        off64  = iotd->iotd_Req.io_Offset;
        off64 |= ((UQUAD)iotd->iotd_Req.io_Actual)<<32;
        err = SAGASD_ReadWrite(io, off64, FALSE);
        break;
    case HD_SCSICMD:
        err = SAGASD_PerformSCSI(io);
        break;
    default:
        debug("Unknown IO command: %d", io->io_Command);
        err = IOERR_NOCMD;
        break;
    }

    debug("io_Actual = %d", iostd->io_Actual);
    debug("io_Error = %d", err);
    return err;
}

static void SAGASD_Detect(struct Library *SysBase, struct SAGASDUnit *sdu)
{
    BOOL present;

    /* Update sdu_Present, regardless */

    asm ( "tst.b 0xbfe001\r\n" );
    present = sdcmd_present(&sdu->sdu_SDCmd);
    if (present != sdu->sdu_Present) {
        if (present) {
            UBYTE sderr;

            /* Re-run the identify */
            sderr = sdcmd_detect(&sdu->sdu_SDCmd);

            Forbid();
            /* Make the drive present. */
            sdu->sdu_Present = TRUE;
            sdu->sdu_ChangeNum++;
            sdu->sdu_Valid = (sderr == 0) ? TRUE : FALSE;
            debug("========= sdu_Valid: %s", sdu->sdu_Valid ? "TRUE" : "FALSE");
            debug("========= Blocks: %ld", sdu->sdu_SDCmd.info.blocks);
            Permit();
        } else {
            Forbid();
            sdu->sdu_Present = FALSE;
            sdu->sdu_Valid = FALSE;
            Permit();
        }
    }
}

/* This low-priority task handles all the non-quick IO
 */
static void SAGASD_IOTask(struct Library *SysBase)
{
    struct Task *this = FindTask(NULL);
    struct SAGASDUnit *sdu = this->tc_UserData;
    struct MsgPort *mport = sdu->sdu_MsgPort;
    struct MsgPort *tport = NULL;
    struct timerequest *treq = NULL;
    ULONG sigset;
    struct Message status;

    debug("");

    status.mn_Length = 1;   // Failed?
    if ((status.mn_ReplyPort = CreateMsgPort())) {
        if ((tport = CreateMsgPort())) {
            if ((treq = (struct timerequest *)CreateIORequest(tport, sizeof(*treq)))) {
                if (0 == OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)treq, 0)) {
                    status.mn_Length = 0;   // Success!
                } else {
                    DeleteIORequest(treq);
                }
            } else {
                DeleteMsgPort(tport);
            }
        } else {
            DeleteMsgPort(status.mn_ReplyPort);
        }
    }

    debug("mport=%p", mport);
    sdu->sdu_MsgPort = status.mn_ReplyPort;

    /* Update status, for the boot node */
    SAGASD_Detect(SysBase, sdu);

    /* Send the 'I'm Ready' message */
    debug("sdu_MsgPort=%p", sdu->sdu_MsgPort);
    PutMsg(mport, &status);

    debug("ReplyPort=%p%s empty", status.mn_ReplyPort, IsListEmpty(&status.mn_ReplyPort->mp_MsgList) ? "" : " not");
    if (status.mn_ReplyPort) {
        WaitPort(status.mn_ReplyPort);
        GetMsg(status.mn_ReplyPort);
    }
    debug("");

    if (status.mn_Length) {
        /* There was an error... */
        DeleteMsgPort(mport);
    debug("");
        return;
    }

    debug("");
    mport = sdu->sdu_MsgPort;
       
    sigset = (1 << tport->mp_SigBit) | (1 << mport->mp_SigBit);

    for (;;) {
        struct IORequest *io;

        SAGASD_Detect(SysBase, sdu);

        io = (struct IORequest *)GetMsg(mport);
        if (!io) {
            ULONG sigs;

            /* Wait up to 100ms for a IO message. If none, then
             * recheck the SD DETECT pin.
             */
            treq->tr_node.io_Command = TR_ADDREQUEST;
            treq->tr_time.tv_secs = 0;
            treq->tr_time.tv_micro = 100000;
            SendIO((struct IORequest *)treq);

            /* Wait on either the MsgPort, or the timer */
            sigs = Wait(sigset);
            if (sigs & (1 << mport->mp_SigBit)) {
                /* Message port was signalled */
                io = (struct IORequest *)GetMsg(mport);
                /* Cancel the timer */
                AbortIO((struct IORequest *)treq);
            } else {
                /* Timeout was signalled */
                io = NULL;
            }

            /* Clean up the timer IO */
            WaitIO((struct IORequest *)treq);
        }

        /* If there was no io, continue on...
         */
        if (!io)
            continue;

        /* If io_Command == ~0, this indicates that we are killing
         * this task.
         */
        if (io->io_Command == ~0) {
            io->io_Error = 0;
            io->io_Message.mn_Node.ln_Type=NT_MESSAGE;
            ReplyMsg(&io->io_Message);
            break;
        }

        io->io_Error = SAGASD_PerformIO(io);
        io->io_Message.mn_Node.ln_Type=NT_MESSAGE;

        /* Need a reply now? */
        ReplyMsg(&io->io_Message);
    }

    /* Clean up */
    CloseDevice((struct IORequest *)treq);
    DeleteIORequest((struct IORequest *)treq);
    DeleteMsgPort(tport);
    DeleteMsgPort(mport);
}

AROS_LH1(void, BeginIO,
 AROS_LHA(struct IORequest *, io, A1),
    struct SAGASDBase *, SAGASDBase, 5, SAGASD)
{
    AROS_LIBFUNC_INIT

    debug("io_Command = %d, io_Flags = 0x%x", io->io_Command, io->io_Flags);

    struct Library *SysBase = SAGASDBase->sd_ExecBase;
    struct SAGASDUnit *sdu = (struct SAGASDUnit *)io->io_Unit;

    if (io->io_Flags & IOF_QUICK) {
        /* Commands that don't require any IO */
	switch(io->io_Command)
	{
            case NSCMD_DEVICEQUERY:
	    case TD_GETNUMTRACKS:
	    case TD_GETDRIVETYPE:
	    case TD_GETGEOMETRY:
	    case TD_REMCHANGEINT:
	    case TD_ADDCHANGEINT:
	    case TD_PROTSTATUS:
	    case TD_CHANGENUM:
	    io->io_Error = SAGASD_PerformIO(io);
            io->io_Message.mn_Node.ln_Type=NT_MESSAGE;
	    return;
	}
    }

    /* Not done quick */
    io->io_Flags &= ~IOF_QUICK;

    /* Forward to the unit's IO task */
    debug("Msg %p (reply %p) => MsgPort %p", &io->io_Message, io->io_Message.mn_ReplyPort, sdu->sdu_MsgPort);
    PutMsg(sdu->sdu_MsgPort, &io->io_Message);

    AROS_LIBFUNC_EXIT
}

AROS_LH1(LONG, AbortIO,
    AROS_LHA(struct IORequest *, io, A1),
    struct SAGASDBase *, SAGASDBase, 6, SAGASD)
{
    AROS_LIBFUNC_INIT

    struct Library *SysBase = SAGASDBase->sd_ExecBase;

    debug("");

    Forbid();
    io->io_Error = IOERR_ABORTED;
    Permit();

    return 0;

    AROS_LIBFUNC_EXIT
}


static void SAGASD_BootNode(
        struct SAGASDBase *SAGASDBase,
        struct Library *ExpansionBase,
        ULONG unit)
{
    struct SAGASDUnit *sdu = &SAGASDBase->sd_Unit[unit];
    TEXT dosdevname[4] = "SD0";
    IPTR pp[4 + DE_BOOTBLOCKS + 1] = {};
    struct DeviceNode *devnode;

    debug("");

    dosdevname[2] += unit;
    debug("Adding bootnode %s %d x %d", dosdevname,sdu->sdu_SDCmd.info.blocks, sdu->sdu_SDCmd.info.block_size);

    pp[0] = (IPTR)dosdevname;
    pp[1] = (IPTR)"sagasd.device";
    pp[2] = unit;
    pp[3] = 0;
    pp[DE_TABLESIZE + 4] = DE_BOOTBLOCKS;
    pp[DE_SIZEBLOCK + 4] = sdu->sdu_SDCmd.info.block_size >> 2;
    pp[DE_NUMHEADS + 4] = SAGASD_HEADS;
    pp[DE_SECSPERBLOCK + 4] = 1;
    pp[DE_BLKSPERTRACK + 4] = SAGASD_SECTORS;
    pp[DE_RESERVEDBLKS + 4] = 2;
    pp[DE_LOWCYL + 4] = 0;
    pp[DE_HIGHCYL + 4] = sdu->sdu_SDCmd.info.blocks / (SAGASD_HEADS * SAGASD_SECTORS);
    pp[DE_NUMBUFFERS + 4] = 1;
    pp[DE_BUFMEMTYPE + 4] = MEMF_PUBLIC;
    pp[DE_MAXTRANSFER + 4] = 0x00200000;
    pp[DE_MASK + 4] = 0xFFFFFFFE;
    pp[DE_BOOTPRI + 4] = 5 - (unit * 10);
    pp[DE_DOSTYPE + 4] = 0x444f5303;
    pp[DE_BOOTBLOCKS + 4] = 2;
    devnode = MakeDosNode(pp);

    if (devnode)
   	AddBootNode(pp[DE_BOOTPRI + 4], 0 & ADNF_STARTPROC, devnode, NULL);
}

#define PUSH(task, type, value) do {\
    struct Task *_task = task; \
    type _val = value; \
    _task->tc_SPReg -= sizeof(_val); \
    CopyMem(&_val, (APTR)_task->tc_SPReg, sizeof(_val)); \
} while (0)

static void SAGASD_InitUnit(struct SAGASDBase * SAGASDBase, int id)
{
    struct Library *SysBase = SAGASDBase->sd_ExecBase;
    struct SAGASDUnit *sdu = &SAGASDBase->sd_Unit[id];

    debug("");
    switch (id) {
    case 0:
        sdu->sdu_SDCmd.iobase  = SAGA_SD_BASE;
        sdu->sdu_Enabled = TRUE;
        break;
    default:
        sdu->sdu_Enabled = FALSE;
    }

    sdu->sdu_SDCmd.func.log = SAGASD_log;
    sdu->sdu_SDCmd.retry.read = SAGASD_RETRY;
    sdu->sdu_SDCmd.retry.write = SAGASD_RETRY;

    /* If the unit is present, create an IO task for it
     */
    if (sdu->sdu_Enabled) {
        struct Task *utask = &sdu->sdu_Task;
        struct MsgPort *initport;

        if ((initport = CreateMsgPort())) {
            struct Message *msg;

            strncpy(sdu->sdu_Name, "SDIO0", sizeof(sdu->sdu_Name));
            sdu->sdu_Name[4] += id;
            sdu->sdu_MsgPort = initport;

            /* Initialize the task */
            memset(utask, 0, sizeof(*utask));
            utask->tc_Node.ln_Pri = 1;
            utask->tc_Node.ln_Name = &sdu->sdu_Name[0];
            utask->tc_SPReg = utask->tc_SPUpper = &sdu->sdu_Stack[SDU_STACK_SIZE];
            utask->tc_SPLower = &sdu->sdu_Stack[0];

            /* Push arguments onto the stack */
            PUSH(utask, struct Library *, SysBase);

            NEWLIST(&utask->tc_MemEntry);
            utask->tc_UserData = sdu;

            AddTask(utask, SAGASD_IOTask, NULL);

            WaitPort(initport);
            msg = GetMsg(initport);
            debug("StartMsg=%p (%ld)", msg, msg->mn_Length);
            sdu->sdu_Enabled = (msg->mn_Length == 0) ? TRUE : FALSE;
            debug("  ReplyPort=%p", msg->mn_ReplyPort);
            ReplyMsg(msg);

            DeleteMsgPort(initport);
        } else {
            sdu->sdu_Enabled = FALSE;
        }
    }

    debug("unit=%d enabled=%d", id, SAGASDBase->sd_Unit[id].sdu_Enabled ? 1 : 0);
}

static int GM_UNIQUENAME(init)(struct SAGASDBase * SAGASDBase)
{
    struct Library *SysBase = SAGASDBase->sd_ExecBase;
    struct Library *ExpansionBase;
    ULONG i;

    debug("");
    UWORD *SAGA_GFX_VIDMODE = (UWORD*) 0xDFF1F4;
    asm ( "tst.b 0xbfe001\r\n" );    // Wait a moment, then...

    ExpansionBase = TaggedOpenLibrary(TAGGEDOPEN_EXPANSION);
    if (!ExpansionBase)
  	Alert(AT_DeadEnd | AO_TrackDiskDev | AG_OpenLib);

    for (i = 0; i < SAGASD_UNITS; i++)
	SAGASD_InitUnit(SAGASDBase, i);

    /* Only add bootnode if recalibration succeeded */
    for (i = 0; i < SAGASD_UNITS; i++) {
	if (SAGASDBase->sd_Unit[i].sdu_Valid)
	    SAGASD_BootNode(SAGASDBase, ExpansionBase, i);
    }

    CloseLibrary((struct Library *)ExpansionBase);

    return TRUE;
}

static int GM_UNIQUENAME(expunge)(struct SAGASDBase * SAGASDBase)
{
    struct Library *SysBase = SAGASDBase->sd_ExecBase;
    struct IORequest io = {};
    int i;

    debug("");

    for (i = 0; i < SAGASD_UNITS; i++) {
        io.io_Device = &SAGASDBase->sd_Device;
        io.io_Unit = &SAGASDBase->sd_Unit[i].sdu_Unit;
        io.io_Flags = 0;
        io.io_Command = ~0;

        /* Signal the unit task to die */
        DoIO(&io);
    }

    return TRUE;
}

static int GM_UNIQUENAME(open)(struct SAGASDBase * SAGASDBase,
                               struct IOExtTD *iotd, ULONG unitnum,
                               ULONG flags)
{
    iotd->iotd_Req.io_Error = IOERR_OPENFAIL;

    /* Is the requested unitNumber valid? */
    if (unitnum < SAGASD_UNITS) {
        struct SAGASDUnit *sdu;

        iotd->iotd_Req.io_Device = (struct Device *)SAGASDBase;

        /* Get SDU structure */
        sdu = &SAGASDBase->sd_Unit[unitnum];
	if (sdu->sdu_Enabled) {
    	    iotd->iotd_Req.io_Unit = &sdu->sdu_Unit;
    	    sdu->sdu_Unit.unit_OpenCnt++;
    	    iotd->iotd_Req.io_Error = 0;
	}

        debug("Open=%d", unitnum, iotd->iotd_Req.io_Error);
    }
  
    return iotd->iotd_Req.io_Error == 0;

}

static int GM_UNIQUENAME(close)(struct SAGASDBase *SAGASDBase,
                                struct IOExtTD *iotd)
{
    iotd->iotd_Req.io_Unit->unit_OpenCnt --;

    return TRUE;
}

ADD2INITLIB(GM_UNIQUENAME(init), 0)
ADD2EXPUNGELIB(GM_UNIQUENAME(expunge), 0)
ADD2OPENDEV(GM_UNIQUENAME(open), 0)
ADD2CLOSEDEV(GM_UNIQUENAME(close), 0)
/* vim: set shiftwidth=4 expandtab:  */
