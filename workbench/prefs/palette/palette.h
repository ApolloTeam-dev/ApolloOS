/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.
*/

#ifndef _PEPALETTE_H
#define _PEPALETTE_H

/*** Identifier base ********************************************************/
#define MUIB_PEPalette                  (TAG_USER | 0x11000000)

#define MUIA_PEPalette_Pens             (MUIB_PEPalette + 1)
#define MUIA_PEPalette_Penmap4          (MUIB_PEPalette + 2)
#define MUIA_PEPalette_Penmap8          (MUIB_PEPalette + 3)

extern struct MUI_CustomClass *PEPalette_CLASS;
#define PEPaletteObject BOOPSIOBJMACRO_START(PEPalette_CLASS->mcc_Class)

#endif

