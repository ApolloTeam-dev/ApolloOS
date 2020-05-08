/* Raw SD interface
 *
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

#include <string.h>     // For memset

#include <exec/types.h>

#include <saga/sd.h>

#include "sdcmd.h"

#include "common.h"

#define sdcmd_log(sd,level,fmt,args...) \
    do { \
        if (sd->func.log) \
            sd->func.log(sd, level, "%s:%ld " fmt, __func__, (ULONG)__LINE__ ,##args); \
    } while (0)

#define diag(fmt,args...)       sdcmd_log(sd, SDLOG_DIAG, fmt ,##args)
#define debug(fmt,args...)      sdcmd_log(sd, SDLOG_DEBUG, fmt ,##args)
#define info(fmt,args...)       sdcmd_log(sd, SDLOG_INFO,  fmt ,##args)
#define warn(fmt,args...)       sdcmd_log(sd, SDLOG_WARN, fmt ,##args)
#define error(fmt,args...)      sdcmd_log(sd, SDLOG_ERROR, fmt ,##args)

#define SDCMD_CLKDIV_SLOW       0xff
#define SDCMD_CLKDIV_FAST       0x01
#define SDCMD_CLKDIV_FASTER     0x00

static UBYTE crc7(UBYTE crc, UBYTE byte)
{
    int i;

    for (i = 0; i < 8; i++, byte <<= 1) {
        crc <<= 1;
        if ((byte ^ crc) & 0x80)
            crc ^= 0x09;
    }

    crc &= 0x7f;

    return crc;
}

#if 0
/* Compact CRC16 */
static UWORD crc16(UWORD crc, UBYTE byte)
{
    crc = ((UBYTE)(crc >> 8)) | ((UBYTE)crc << 8);
    crc ^= byte;
    crc ^= (UBYTE)(crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;

    return crc;
}
#else
/* Table based CRC16 */
static const UWORD crc16_ccitt_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static UWORD crc16(UWORD crc, UBYTE byte)
{
    return crc16_ccitt_table[(crc >> 8) ^ byte] ^ (crc << 8);
}
#endif

static VOID sdcmd_out(struct sdcmd *sd, UBYTE data)
{
    diag("SD_DATA <= $%02lx", data);

    Write8(sd->iobase + SAGA_SD_DATA, data);

    (void)Read8(sd->iobase + SAGA_SD_DATA);
}

static UBYTE sdcmd_in(struct sdcmd *sd)
{
    UBYTE val;

    Write8(sd->iobase + SAGA_SD_DATA, 0xff);
    val = Read8(sd->iobase + SAGA_SD_DATA);

    diag("SD_DATA => $%02lx", val);

    return val;
}

static UWORD sdcmd_ins(struct sdcmd *sd, UWORD crc, UBYTE *buff, size_t len)
{
    UBYTE val;
    ULONG dataio = sd->iobase + SAGA_SD_DATA;

    if (len == 0)
        return crc;

    /* Since the read of the SAGA_SD_DATA will stall until
     * filled by the SPI, we amortize that cost by computing
     * the CRC16 while waiting for the next fill.
     */
    Write8(dataio, 0xff);
    for (len--; len > 0; len--, buff++) {
        val = Read8(dataio + SAGA_SD_DATA);
        Write8(dataio + SAGA_SD_DATA, 0xff);
        crc = crc16(crc, val);
        *buff = val;
        if (DEBUG)
            diag("SD_DATA => $%02lx", val);
    }
    val = Read8(dataio + SAGA_SD_DATA);
    crc = crc16(crc, val);
    *buff = val;

    return crc;
}
            
BOOL sdcmd_present(struct sdcmd *sd)
{
    UWORD val;
    
    val = Read16(sd->iobase + SAGA_SD_STAT);

    diag("SD_STAT => $%04lx", val);

    return (val & SAGA_SD_STAT_NCD) ? FALSE : TRUE;
}

VOID sdcmd_select(struct sdcmd *sd, BOOL cs)
{
    UWORD val;

    val = cs ? 0 : SAGA_SD_CTL_NCS;

    diag("SD_CTL  => $%04lx", val);

    Write16(sd->iobase + SAGA_SD_CTL, val);
    sdcmd_out(sd, 0xff);

    /*  Wait for card ready */
    if (cs) {
        int i;
        for (i = 0; i < SDCMD_TIMEOUT; i++) {
            UBYTE r1 = sdcmd_in(sd);
            if (r1 == 0xff)
                break;
        }
    }
}

static VOID sdcmd_clkdiv(struct sdcmd *sd, UBYTE val)
{
    diag("SD_CLK  => $%04lx", val);

    Write16(sd->iobase + SAGA_SD_CLK, val);
}


void sdcmd_send(struct sdcmd *sd, UBYTE cmd, ULONG arg)
{
    int i;
    UBYTE crc;

    sdcmd_select(sd, TRUE);

    cmd = (cmd & 0x3f) | SDCMD_VALID;

    crc = crc7(0, cmd);

    sdcmd_out(sd, (cmd & 0x3f) | 0x40);

    for (i = 0; i < 4; i++, arg <<= 8) {
        UBYTE byte = (arg >> 24) & 0xff;
        crc = crc7(crc, byte);

        sdcmd_out(sd, byte);
    }

    sdcmd_out(sd, (crc << 1) | 1);
}

static UBYTE sdcmd_r1a(struct sdcmd *sd)
{
    UBYTE r1;
    int i;

    for (i = 0; i < SDCMD_TIMEOUT; i++) {
        r1 = sdcmd_in(sd);
        if (!(r1 & SDERRF_TIMEOUT))
            return r1;
    }

    return SDERRF_TIMEOUT;
}

#define R1_OK(x)        (((x) & ~SDERRF_IDLE) == 0)

UBYTE sdcmd_r1(struct sdcmd *sd)
{
    UBYTE r1;

    r1 = sdcmd_r1a(sd);
    sdcmd_select(sd, FALSE);

    return r1;
}

UBYTE sdcmd_r2(struct sdcmd *sd, UBYTE *r2)
{
    UBYTE r1;

    r1 = sdcmd_r1a(sd);
    if (R1_OK(r1))
        *r2 = sdcmd_in(sd);

    sdcmd_select(sd, FALSE);

    return r1;
}

UBYTE sdcmd_r3(struct sdcmd *sd, ULONG *ocr)
{
    UBYTE r1;
    ULONG r3 = 0;
    int i;

    r1 = sdcmd_r1a(sd);
    if (R1_OK(r1)) {
        for (i = 0; i < 4; i++) {
            r3 <<= 8;
            r3 |= sdcmd_in(sd);
        }
    }

    sdcmd_select(sd, FALSE);

    debug("r3=0x%08lx", r3);
    *ocr = r3;

    return r1;
}

UBYTE sdcmd_r7(struct sdcmd *sd, ULONG *ifcond)
{
    UBYTE r1;
    ULONG r7 = 0;
    int i;

    r1 = sdcmd_r1a(sd);
    if (R1_OK(r1)) {
        for (i = 0; i < 4; i++) {
            r7 <<= 8;
            r7 |= sdcmd_in(sd);
        }
    }

    sdcmd_select(sd, FALSE);

    debug("r7=0x%08lx", r7);
    *ifcond = r7;

    return r1;
}

UBYTE sdcmd_asend(struct sdcmd *sd, UBYTE acmd, ULONG arg)
{
    UBYTE r1;

    /* Next command is an app command.. */
    sdcmd_send(sd, SDCMD_APP_CMD, 0);

    r1 = sdcmd_r1a(sd);
    if (!R1_OK(r1)) {
        sdcmd_select(sd, FALSE);
        return r1;
    }

    sdcmd_send(sd, acmd, arg);

    return 0;
}

UBYTE sdcmd_read_packet(struct sdcmd *sd, UBYTE *buff, int len)
{
    UBYTE token = 0xfe;
    UBYTE byte;
    UWORD crc, tmp;
    int i;

    /* Wait for the Data Token */
    for (i = 0; i < SDCMD_TIMEOUT; i++) {
        byte = sdcmd_in(sd);
        if (byte == token)
            break;
    }

    if (i == SDCMD_TIMEOUT) {
        sdcmd_select(sd, FALSE);
        return SDERRF_TIMEOUT;
    }

    crc = sdcmd_ins(sd, 0, buff, len);

    /* Read the CRC16 */
    tmp = (UWORD)sdcmd_in(sd) << 8;
    tmp |= sdcmd_in(sd);

    if (tmp != crc)
        return SDERRF_CRC;

    return 0;
}


UBYTE sdcmd_stop_transmission(struct sdcmd *sd)
{
    LONG crc_retry = sd->retry.read;
    UBYTE r1, tmp;
    int i = 0;

    do {
        sdcmd_send(sd, SDCMD_STOP_TRANSMISSION, 0);

        /* Read the stuff byte */
        sdcmd_in(sd);

        /* Read response */
        r1 = sdcmd_r1a(sd);
        debug("r1=$%02lx", r1);
    } while ((r1 & SDERRF_CRC) && (crc_retry-- > 0));

    /* If it's a CRC error, after our retires, just die. */
    if (r1 & SDERRF_CRC)
        goto exit;

    /* Otherwise we ignore it... There is no circumstance
     * where it will have meaningful information,
     * and some cards put junk data in the R1
     * response.
     */
    r1 = 0;

    /* Wait until not busy */
    for (i = 0; i < SDCMD_TIMEOUT; i++) {
        tmp = sdcmd_in(sd);
        debug("tmp=$%02lx", tmp);
        if (tmp == 0xff)
           break;
    }

exit:
    sdcmd_select(sd, FALSE);

    return (i == SDCMD_TIMEOUT) ? SDERRF_TIMEOUT : r1;
}

UBYTE sdcmd_write_packet(struct sdcmd *sd, UBYTE token, CONST UBYTE *buff, int len)
{
    UBYTE r1, byte;
    UWORD crc;
    int i;

    /* Send a spacing byte */
    sdcmd_out(sd, 0xff);

    /* Start the data packet */
    sdcmd_out(sd, token);
    crc = 0;

    /* Send the block */
    for (i = 0; i < SDSIZ_BLOCK; i++, buff++) {
        byte = *buff;
        sdcmd_out(sd, byte);
        crc = crc16(crc, byte);
    }

    /* Send the CRC16, MSB first */
    sdcmd_out(sd, (crc >> 8) & 0xff);
    sdcmd_out(sd, (crc >> 0) & 0xff);

    /* Read the Data Response */
    byte = sdcmd_in(sd);
    if ((byte & SDDRS_VALID_MASK) != SDDRS_VALID) {
        /* Terminate the read early */
        sdcmd_stop_transmission(sd);
        return SDERRF_CRC;
    }

    r1 = ((byte & SDDRS_CODE_MASK) == SDDRS_CODE_ACCEPTED) ? 0 : SDERRF_CRC;
    debug("byte=$%02lx, r1=$%02lx", byte, r1);

    /* Wait for the idle pattern */
    /* Wait until not busy */
    for (i = 0; i < SDCMD_TIMEOUT; i++) {
        UBYTE tmp = sdcmd_in(sd);
        debug("ptmp = $%02lx", tmp);
        if (tmp == 0xff)
            break;
    }

    return (i == SDCMD_TIMEOUT) ? SDERRF_TIMEOUT : r1;
}

static ULONG bits(UBYTE *mask, int start, int len)
{
    ULONG ret = 0;
    int i;

    mask -= start / 8;

    start &= 7;

    for (i = 0; i < len; i++, start++) {
        if (start == 8) {
            mask--;
            start = 0;
        }

        ret |= ((*mask >> start) & 1) << i;
    }

    return ret;
}

/* If non-zero, filled in the total size in blocks of the device
 */
UBYTE sdcmd_detect(struct sdcmd *sd)
{
    struct sdcmd_info *info = &sd->info;
    UBYTE r1;
    UBYTE speed;
    ULONG r7;
    int i;

    memset(info, 0, sizeof(*info));

    /* First, check the DETECT bit */
    if (!sdcmd_present(sd))
        return ~0;

    /* Switch to slow speed mode */
    sdcmd_clkdiv(sd, SDCMD_CLKDIV_SLOW);

    /* Emit at least 74 clocks of idle */
    sdcmd_select(sd, TRUE);
    for (i = 0; i < 10; i++)
        sdcmd_out(sd, 0xff);
    sdcmd_select(sd, FALSE);

    /* Stuff two idle bytes while deasserted */
    sdcmd_out(sd, 0xff);
    sdcmd_out(sd, 0xff);

    /* Put into idle state */
    sdcmd_send(sd, SDCMD_GO_IDLE_STATE, 0);
    r1 = sdcmd_r1(sd);
    /* It's ok (and expected) that we are in IDLE state */
    r1 &= ~SDERRF_IDLE;
    if (r1)
        return r1;

    /* Do SHDC detection during idle */
    sdcmd_send(sd, SDCMD_SEND_IF_COND, 0x000001aa);
    r1 = sdcmd_r7(sd, &r7);
    /* It's ok (and expected) that we are in IDLE state */
    r1 &= ~SDERRF_IDLE;
    if (!r1) {
        /* Looks like a SDHC card? */
        if ((r7 & 0x000001ff) == 0x000001aa) {
            /* Set HCS (SDHC) mode */
            sdcmd_asend(sd, SDCMD_SD_SEND_OP_COND, SDOCRF_HCS);
            r1 = sdcmd_r1(sd);
            /* It's ok (and expected) that we are in IDLE state */
            r1 &= ~SDERRF_IDLE;
            if (r1)
                return r1;
        }
    }

    /* Wait for card to complete idle */
    for (i = 0; i < SDCMD_IDLE_RETRY; i++) {
        UBYTE err;
       
        /* Initiate SDC init process */
        err = sdcmd_asend(sd, SDCMD_SD_SEND_OP_COND, 0);
        if (err)
            return err;

        r1 = sdcmd_r1(sd);
        if (!(r1 & SDERRF_IDLE))
            break;
    }

    debug("r1=0x%lx", r1);
    if (r1)
        return r1;

    /* Enable CRC check mode */
    sdcmd_send(sd, SDCMD_CRC_ON_OFF, 1);
    r1 = sdcmd_r1(sd);
    if (r1) {
        /* Non-fatal if this failed */
        debug("r1=0x%lx", r1);
    }

    /* Check for voltage levels */
    sdcmd_send(sd, SDCMD_READ_OCR, 0);
    r1 = sdcmd_r3(sd, &info->ocr);
    debug("r1=0x%lx", r1);
    if (r1)
        return r1;

    /* Not in our voltage range */
    info("ocr=0x%08lx (vs 0x%08lx)", info->ocr, SDOCRF_MAX_3_3V | SDOCRF_MAX_3_4V);
    if (!(info->ocr & (SDOCRF_MAX_3_3V | SDOCRF_MAX_3_4V)))
        return SDERRF_IDLE;

    if (info) {
        ULONG c_size_mult, read_bl_len, c_size;
        UBYTE *csd = &info->csd[0];
        UBYTE *cid = &info->cid[0];

        /* Get the CSD data */
        sdcmd_send(sd, SDCMD_SEND_CSD, 0);
        r1 = sdcmd_r1a(sd);
        debug("r1=0x%lx", r1);
        if (r1)
            goto exit;

        r1 = sdcmd_read_packet(sd, csd, 16);
        debug("r1=0x%lx", r1);
        if (r1)
            goto exit;

        info("csd=%02lx%02lx%02lx%02lx-%02lx%02lx%02lx%02lx-%02lx%02lx%02lx%02lx-%02lx%02lx%02lx%02lx",
                csd[0], csd[1], csd[2], csd[3],
                csd[4], csd[5], csd[6], csd[7],
                csd[8], csd[9], csd[10], csd[11],
                csd[12], csd[13], csd[14], csd[15]);

        /* Get the CID data */
        sdcmd_send(sd, SDCMD_SEND_CID, 0);
        r1 = sdcmd_r1a(sd);
        debug("r1=%d", r1);
        if (r1)
            goto exit;

        r1 = sdcmd_read_packet(sd, cid, 16);
        sdcmd_select(sd, FALSE);

        debug("r1=0x%lx", r1);
        if (r1)
            return r1;
        info("cid=%02lx%02lx%02lx%02lx-%02lx%02lx%02lx%02lx-%02lx%02lx%02lx%02lx-%02lx%02lx%02lx%02lx",
                cid[0], cid[1], cid[2], cid[3],
                cid[4], cid[5], cid[6], cid[7],
                cid[8], cid[9], cid[10], cid[11],
                cid[12], cid[13], cid[14], cid[15]);

        info->block_size = SDSIZ_BLOCK;

        if (info->ocr & SDOCRF_HCS) {
            /* SDHC calculation */

            /* Bits 68:48 of the CSD */
            c_size = bits(&csd[15], 48, 20);

            info("SDHC: c_size=%ld", c_size);

            info->blocks = (c_size + 1) * 1024;

            info->addr_shift = 0;
        } else {
            /* SD calculation */
            /* Bits 49:47 of the CSD */
            c_size_mult = bits(&csd[15], 47, 3);
            /* Bits 83:80 of the CSD */
            read_bl_len = bits(&csd[15], 80, 4);
            /* Bits 73:62 of the CSD */
            c_size = bits(&csd[15], 62, 12);

            info("SD: c_size_mult=%ld, read_bl_len=%ld, c_size=%ld", c_size_mult, read_bl_len, c_size);

            info->blocks = (1UL << (c_size_mult + read_bl_len + 2 - 9)) * (c_size + 1);

            /* Set block size */
            sdcmd_send(sd, SDCMD_SET_BLOCKLEN, info->block_size);
            r1 = sdcmd_r1(sd);
            if (r1)
                return r1;

            info->addr_shift = 9;
        }
        info("blocks=%ld", info->blocks);
    }

    /* Default speed mode */
    speed = SDCMD_CLKDIV_FAST;

    /* Try setting the card into high speed mode.  It's possible
     * to check first, but just trying to set is enough?
     *
     * First nibble is Function Group 1 - Access mode / Bus Speed mode;
     * the only thing that applies to us in SPI mode.
     */
    sdcmd_send(sd, SDCMD_SWITCH_FUNCTION, 0x80fffff1);
    r1 = sdcmd_r1a(sd);
    debug("r1=0x%lx", r1);
    if (!r1) {
        UBYTE cmd6[512/8];
        ULONG f1_sel;

        r1 = sdcmd_read_packet(sd, cmd6, sizeof(cmd6));
        sdcmd_select(sd, FALSE);

        f1_sel = bits(&cmd6[63], 376, 4);

        /* Out of all of the above, just check f1_sel to see
         * if it is what we set it to.
         */
        if (f1_sel == 1)
            speed = SDCMD_CLKDIV_FASTER;
    }

    /* Switch to high speed mode */
    sdcmd_clkdiv(sd, speed);
    r1 = 0;

exit:
    sdcmd_select(sd, FALSE);
    return r1;
}

UBYTE sdcmd_read_block(struct sdcmd *sd, ULONG addr, UBYTE *buff)
{
    UBYTE r1;
    LONG crc_retry = sd->retry.read;

    do {
        info("read block=%ld", addr);

        /* Send the read block command */
        sdcmd_send(sd, SDCMD_READ_SINGLE_BLOCK, addr << sd->info.addr_shift);
        r1 = sdcmd_r1a(sd);
        if (!r1)
            r1 = sdcmd_read_packet(sd, buff, SDSIZ_BLOCK);

        sdcmd_select(sd, FALSE);

    } while ((crc_retry-- > 0) && (r1 & SDERRF_CRC));

    return r1;
}


UBYTE sdcmd_read_blocks(struct sdcmd *sd, ULONG addr, UBYTE *buff, int blocks)
{
    UBYTE r1;
    LONG crc_retry = sd->retry.read;

    if (blocks == 1)
        return sdcmd_read_block(sd, addr, buff);

    do {
        info("read block=%ld, blocks=%ld", addr, blocks);

        /* Send the read block command */
        sdcmd_send(sd, SDCMD_READ_MULTIPLE_BLOCK, addr << sd->info.addr_shift);
        r1 = sdcmd_r1a(sd);
        if (r1) {
            debug("r1=$%02lx", r1);
            sdcmd_select(sd, FALSE);
            continue;
        }

        for (; blocks > 0; addr++, blocks--, buff += SDSIZ_BLOCK) {
            r1 = sdcmd_read_packet(sd, buff, SDSIZ_BLOCK);
            if (r1) {
                debug("r1=$%02lx", r1);
                /* Terminate the read early */
                sdcmd_stop_transmission(sd);
                break;
            }

            /* Reset the retry counter if we read a block */
            crc_retry = sd->retry.read;
        }

        if (r1)
            continue;
              
        /* Terminate the read */
        r1 = sdcmd_stop_transmission(sd);
    } while ((r1 & SDERRF_CRC) && (crc_retry-- > 0));

    return r1;
}

UBYTE sdcmd_write_block(struct sdcmd *sd, ULONG addr, CONST UBYTE *buff)
{
    LONG crc_retry = sd->retry.write;
    UBYTE token = 0xfe;
    UBYTE r1;

    do {
        info("write block=%ld", addr);

        /* Send write block command */
        sdcmd_send(sd, SDCMD_WRITE_SINGLE_BLOCK, addr << sd->info.addr_shift);
        r1 = sdcmd_r1a(sd);
        if (!r1)
            r1 = sdcmd_write_packet(sd, token, buff, SDSIZ_BLOCK);

        sdcmd_select(sd, FALSE);
    } while ((r1 & SDERRF_CRC) && (crc_retry-- > 0));

    return r1;
}


UBYTE sdcmd_write_blocks(struct sdcmd *sd, ULONG addr, CONST UBYTE *buff, int blocks)
{
    LONG crc_retry = sd->retry.write;
    UBYTE token = 0xfc, stop_token = 0xfd;
    UBYTE tmp, r1;
    int i;

    if (blocks == 1)
        return sdcmd_write_block(sd, addr, buff);

    do {
        info("block=%ld, blocks=%ld", addr, blocks);

        /* Send write blocks command */
        sdcmd_send(sd, SDCMD_WRITE_MULTIPLE_BLOCK, addr << sd->info.addr_shift);
        r1 = sdcmd_r1a(sd);
        if (r1) {
            sdcmd_select(sd, FALSE);
            continue;
        }

        for (; blocks; addr++, blocks--, buff += SDSIZ_BLOCK) {
            r1 = sdcmd_write_packet(sd, token, buff, SDSIZ_BLOCK);
            debug("pr1=$%02lx", r1);
            if (r1)
                break;

            /* Reset the retry counter if we wrote a block */
            crc_retry = sd->retry.read;
        }

        sdcmd_select(sd, TRUE);

        /* Send stop token */
        sdcmd_out(sd, stop_token);

        /* Read 'stuff' byte */
        sdcmd_in(sd);

        /* Wait until not busy */
        for (i = 0; i < SDCMD_TIMEOUT; i++) {
            tmp = sdcmd_in(sd);
            debug("tmp=$%02lx", tmp);
            if (tmp == 0xff)
                break;
        }

        sdcmd_select(sd, FALSE);

        debug("i=%ld, r1=$%02lx", i, r1);
        if (i == SDCMD_TIMEOUT)
            r1 = SDERRF_TIMEOUT;

    } while ((r1 & SDERRF_CRC) && (crc_retry-- > 0));

    return r1;
}

/* vim: set shiftwidth=4 expandtab:  */
