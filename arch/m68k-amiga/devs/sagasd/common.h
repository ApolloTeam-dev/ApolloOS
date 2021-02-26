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

#ifndef COMMON_H
#define COMMON_H

#include <proto/arossupport.h>

static inline UBYTE Read8(IPTR addr)
{
    return *(volatile UBYTE *)addr;
}

static inline VOID Write8(IPTR addr, UBYTE value)
{
    if (!SIMULATE) {
        *(volatile UBYTE *)addr = value;
    }
}

static inline UWORD Read16(IPTR addr)
{
    return *(volatile UWORD *)addr;
}

static inline VOID Write16(IPTR addr, UWORD value)
{
    if (!SIMULATE) {
        *(volatile UWORD *)addr = value;
    }
}

static inline ULONG Read32(IPTR addr)
{
    return *(volatile ULONG *)addr;
}

static inline VOID Write32(IPTR addr, ULONG value)
{
    if (!SIMULATE) {
        *(volatile ULONG *)addr = value;
    }
}

#endif /* COMMON_H */
/* vim: set shiftwidth=4 expandtab:  */
