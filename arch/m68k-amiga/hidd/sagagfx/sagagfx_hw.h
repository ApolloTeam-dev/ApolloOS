#ifndef SAGAGFX_HW_H
#define SAGAGFX_HW_H

/*
    Copyright © 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAGAGfx Hidd class.
    Lang: English.
*/

#undef DEBUG
#define DEBUG 0

#include <aros/debug.h>
#include <exec/types.h>

/*
 **************************************************************
 **  DEFINES
 **************************************************************
*/

#define VREG_BOARD_Unknown      0x00      // Unknown
#define VREG_BOARD_V600         0x01      // V600
#define VREG_BOARD_V500         0x02      // V500
#define VREG_BOARD_V4           0x03      // V4
#define VREG_BOARD_V666         0x04      // V666
#define VREG_BOARD_V4SA         0x05      // V4SA
#define VREG_BOARD_V1200        0x06      // V1200
#define VREG_BOARD_V4000        0x07      // V4000
#define VREG_BOARD_VCD32        0x08      // VCD32
#define VREG_BOARD_Future       0x09      // Future

#define VREG_BOARD              0xDFF3FC  // [16-bits] BoardID [HIGH-Byte: MODEL, LOW-Byte: xFREQ]

/*
 **************************************************************
 **  INLINES
 **************************************************************
*/

static inline __attribute__((always_inline)) UWORD READ16(IPTR a)
{
    return (*(volatile UWORD*)a);
}

static inline __attribute__((always_inline)) ULONG READ32(IPTR a)
{
    return (*(volatile ULONG*)a);
}

static inline __attribute__((always_inline)) VOID WRITE16(IPTR a, UWORD b)
{
    D(bug("WRITE16(%p,%04x)\n", a, b));
    *(volatile UWORD *)(a) = (b);
}

static inline __attribute__((always_inline)) VOID WRITE32(IPTR a, ULONG b)
{
    D(bug("WRITE32(%p,%08x)\n", a, b));
    *(volatile ULONG *)(a) = (b);
}

/*
 **************************************************************
 **  END OF FILE
 **************************************************************
*/

#endif /* SAGAGFX_HW_H */
