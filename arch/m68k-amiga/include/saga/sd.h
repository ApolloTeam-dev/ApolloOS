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

#ifndef SAGA_SD_H
#define SAGA_SD_H

#define SAGA_SD_BASE            0xde0000        /* Base address */

#define SAGA_SD_DATA            0x0000          /* Read as UBYTE/Write as UBYTE */

#define SAGA_SD_CTL             0x0004          /* UWORD, Write only */
#define   SAGA_SD_CTL_NCS       (1 << 0)        /* nChipSelect */

#define SAGA_SD_STAT            0x0006          /* UWORD, Read only */
#define   SAGA_SD_STAT_NCD      (1 << 0)        /* nCardDetect */

#define SAGA_SD_CLK             0x000c
#define   SAGA_SD_CLK_DIV(x)    ((x) & 0xff)

#endif /* SAGA_SD_H */
/* vim: set shiftwidth=4 expandtab:  */
