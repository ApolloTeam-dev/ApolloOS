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
#define VREG_BOARD_V2_V600      0x01      // V2-V600 (legacy)
#define VREG_BOARD_V2_V500      0x02      // V2-V500 (legacy)
#define VREG_BOARD_V4_V500      0x03      // V4-V500 = FireBird
#define VREG_BOARD_V4_V1200     0x04      // V4-V1200 = Icedrake
#define VREG_BOARD_V4_SA        0x05      // V4-SA = StandAlone
#define VREG_BOARD_V2_V1200     0x06      // V2-V1200 (legacy)
#define VREG_BOARD_V4_V600      0x07      // V4-V600 = MantiCore
#define VREG_BOARD_Future_1     0x08      // Future
#define VREG_BOARD_Future_2     0x09      // Future

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
