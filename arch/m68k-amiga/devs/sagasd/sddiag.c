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

#include <aros/debug.h>
#undef DEBUG

#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/alib.h>

#include <aros/shcommands.h>

#include "sdcmd.h"
#include "common.h"

#include <saga/sd.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))
#endif

struct Library *SysBase, *DOSBase;

int DebugLevel;

static void setBases(struct ExecBase *pSysBase, struct DosLibrary *pDOSBase)
{
    SysBase = (struct Library *)pSysBase;
    DOSBase = (struct Library *)pDOSBase;
}

static VOID sdcmd_log(struct sdcmd *sd, int level, const char *format, ...)
{
    static char buffer[1024];
    const ULONG m68k_string_sprintf = 0x16c04e75;

    if (level > DebugLevel)
        return;

    RawDoFmt(format, &format+1, (VOID_FUNC)&m68k_string_sprintf, buffer);
    Printf("sdcmd: %s\n", buffer);
}

ULONG write_buffer[512 * 16 / sizeof(ULONG)];
ULONG read_buffer[512 * 16 / sizeof(ULONG)];

AROS_SH6H(SDDiag , 0.10b,                   "SAGA SD Diagnostic\n",
AROS_SHAH(ULONG *  ,IO= ,IOBASE,/K/N,  0 ,  "SD IO Base\n"),
AROS_SHAH(ULONG *  ,P= ,PATTERN,/K/N,  0 ,  "Pattern for WRITE testing\n"),
AROS_SHAH(BOOL     ,R= ,READ,/S,    FALSE, "Perform READ test to last 16 blocks\n"),
AROS_SHAH(BOOL     ,W= ,WRITE,/S,    FALSE, "Perform WRITE test to last 16 blocks\n"),
AROS_SHAH(ULONG *  ,   ,RETRY,/N, 0, "Number of CRC retries (5)\n"),
AROS_SHAH(ULONG *  ,D= ,DEBUG,/N, 0, "Debug level\n")
)
{
    AROS_SHCOMMAND_INIT

    ULONG iobase = SHArg(IOBASE) ? *SHArg(IOBASE) : SAGA_SD_BASE;
    ULONG pattern = SHArg(PATTERN) ? *SHArg(PATTERN) : 0x5af00000;
    BOOL readTest = SHArg(READ);
    BOOL writeTest = SHArg(WRITE);
    ULONG retry = SHArg(RETRY) ? *SHArg(RETRY) : 5;
    int i;
    ULONG test_block;
    UBYTE err;
    struct sdcmd sd = {};

    setBases(SysBase, DOSBase);

    DebugLevel = SHArg(DEBUG) ? *SHArg(DEBUG) : 0;
    Printf("DebugLevel: %ld\n", DebugLevel);

    sd.iobase = iobase;
    sd.retry.read = retry;
    sd.retry.write = retry;
    sd.func.log = sdcmd_log;

    err = sdcmd_detect(&sd);

    if (!err) {
        Printf("SD Card Detected on $0x%lx:\n", iobase);
        Printf("Block Size: %ld\n", sd.info.block_size);
        Printf("Blocks: %ld\n", sd.info.blocks);
        Printf("Capacity: ");
        if (sd.info.blocks < 1024*2) {
            Printf("%ldK\n", (sd.info.blocks + 1) / 2);
        } else if (sd.info.blocks < 1024*1024*2) {
            Printf("%ldM\n", (sd.info.blocks + 1024*2-1) / 1024 / 2);
        } else {
            Printf("%ldG\n", (sd.info.blocks + 1024*1024*2-1) / 1024 / 1024 / 2);
        }

        Printf("OCR: %08lx\n", sd.info.ocr);
        Printf("CID:");
        for (i = 0; i < 16; i++)
            Printf(" %02lx", sd.info.cid[i]);
        Printf("\n");
        Printf("CSD:");
        for (i = 0; i < 16; i++)
            Printf(" %02lx", sd.info.csd[i]);
        Printf("\n");
    } else {
        Printf("SD Card Detection Error: %lx\n", err);
        return RETURN_FAIL; 
    }

    test_block = sd.info.blocks - 16;

    if (writeTest) {
        for (i = 0; i < ARRAY_SIZE(write_buffer); i++)
            write_buffer[i] = pattern + i;

        Printf("WRITE Test: %ld bytes, at block %ld: ",
                512, test_block);

        err = sdcmd_write_block(&sd, test_block, (UBYTE *)&write_buffer[0]);
        if (err) {
            Printf("FAILED WRITE, err=0x%02lx\n", err);
            return RETURN_FAIL;
        }

        err = sdcmd_read_block(&sd, test_block, (UBYTE *)&read_buffer[0]);
        if (err) {
            Printf("FAILED READBACK, err=0x%02lx\n", err);
            return RETURN_FAIL;
        }
       
        err = 0;
        for (i = 0; i < 512/sizeof(ULONG); i++) {
            if (read_buffer[i] != write_buffer[i]) {
                if (!err) {
                    Printf("FAILED DATA:\n");
                }
                err++;
                Printf("$%04lx: $%08lx [expected $%08lx]\n", i*sizeof(ULONG),
                        read_buffer[i], write_buffer[i]);
            }
        }
        if (err)
            return RETURN_FAIL;
        Printf("PASSED\n");

        Printf("WRITE Test: %ld bytes, at block %ld: ",
                512*15, test_block);

        err = sdcmd_write_blocks(&sd, test_block+1, (UBYTE *)&write_buffer[512/sizeof(ULONG)], 15);
        if (err) {
            Printf("FAILED WRITE, err=0x%02lx\n", err);
            return RETURN_FAIL;
        }

        err = sdcmd_read_blocks(&sd, test_block+1, (UBYTE *)&read_buffer[512/sizeof(ULONG)], 15);
        if (err) {
            Printf("FAILED READBACK, err=0x%02lx\n", err);
            return RETURN_FAIL;
        }
       
        err = 0;
        for (i = 0; i < 15 * 512/sizeof(ULONG); i++) {
            if (read_buffer[512/sizeof(ULONG) + i] !=
                write_buffer[512/sizeof(ULONG) + i]) {
                if (!err) {
                    Printf("FAILED DATA:\n");
                }
                err++;
                Printf("$%04lx: $%08lx [expected $%08lx]\n", i*sizeof(ULONG),
                        read_buffer[512/sizeof(ULONG) + i], write_buffer[512/sizeof(ULONG) + i]);
            }
        }
        if (err)
            return RETURN_FAIL;

        Printf("PASSED\n");
    }

    if (readTest) {
        Printf("READ Test: %ld bytes, at block %ld: ",
                512, test_block);

        err = sdcmd_read_block(&sd, test_block, (UBYTE *)&read_buffer[0]);
        if (err) {
            Printf("FAILED READ, err=0x%02lx\n", err);
            return RETURN_FAIL;
        }
       
        Printf("PASSED\n");

        Printf("READ Test: %ld bytes, at block %ld: ",
                512*15, test_block);

        err = sdcmd_read_blocks(&sd, test_block+1, (UBYTE *)&read_buffer[512/sizeof(ULONG)], 15);
        if (err) {
            Printf("FAILED READ, err=0x%02lx\n", err);
            return RETURN_FAIL;
        }
       
        Printf("PASSED\n");
    }

    return RETURN_OK;

    AROS_SHCOMMAND_EXIT
}

/* vim: set shiftwidth=4 expandtab:  */
