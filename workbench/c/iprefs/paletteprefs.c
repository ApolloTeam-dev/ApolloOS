/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.

    Desc:
*/

/*********************************************************************************************/

#include <aros/debug.h>

#include "global.h"

#include <prefs/prefhdr.h>
#include <datatypes/pictureclass.h>
#include <prefs/palette.h>

#include <intuition/iprefs.h>

/*********************************************************************************************/

static LONG stopchunks[] =
{
    ID_PREF, ID_PALT
};

/*********************************************************************************************/

void PalettePrefs_Handler(STRPTR filename)
{
    struct PalettePrefs paletteprefs;
    struct IFFHandle *iff;

    D(bug("[IPrefs:Palette] %s('%s')\n", __func__, filename));
    
    if ((iff = CreateIFF(filename, stopchunks, 2)))
    {
        while(ParseIFF(iff, IFFPARSE_SCAN) == 0)
        {
            struct ContextNode *cn;

            D(bug("[IPrefs:Palette] %s: ParseIFF okay\n", __func__));
            cn = CurrentChunk(iff);

            switch (cn->cn_ID)
            {
            case ID_PALT:
                {
                    struct PalettePrefs loadprefs;
                    int i;

                    D(bug("[IPrefs:Palette] %s: ID_PALT chunk\n", __func__));

                    if (ReadChunkBytes(iff, &loadprefs, sizeof(struct PalettePrefs)) == sizeof(struct PalettePrefs))
                    {
                        struct ColorSpec palette[32];
                        struct IOldPenPrefs penmap4 = { 0 };
                        struct IOldPenPrefs penmap8 = { 0 };
                        penmap8.Type = 1;

                        D(bug("[IPrefs:Palette]  %s:  Reading chunk successful.\n", __func__));

                        for (i = 0; i < 32; i++)
                        {
                            if (i < NUMDRIPENS)
                            {
                                penmap4.PenTable[i] = AROS_BE2WORD(loadprefs.pap_4ColorPens[i]);
                                penmap4.Count++;

                                penmap8.PenTable[i] = AROS_BE2WORD(loadprefs.pap_8ColorPens[i]);
                                penmap8.Count++;
                                if (penmap8.PenTable[i] >= PEN_C3)
                                    penmap8.PenTable[i] -= (PEN_C3 - 4);
                            }
                            palette[i].ColorIndex   = AROS_BE2WORD(loadprefs.pap_Colors[i].ColorIndex);
                            palette[i].Red          = AROS_BE2WORD(loadprefs.pap_Colors[i].Red);
                            palette[i].Green        = AROS_BE2WORD(loadprefs.pap_Colors[i].Green);
                            palette[i].Blue         = AROS_BE2WORD(loadprefs.pap_Colors[i].Blue);
                        }

                        /* load the palette in .. */
                        SetIPrefs(&palette[0], sizeof(struct ColorSpec) * 32, IPREFS_TYPE_PALETTE_V39);

                        /* load the pen mappings in .. */
                        SetIPrefs(&penmap4, sizeof(struct IOldPenPrefs), IPREFS_TYPE_PENS_V39);
                        SetIPrefs(&penmap8, sizeof(struct IOldPenPrefs), IPREFS_TYPE_PENS_V39);

                        D(bug("[IPrefs:Palette]  %s:  ID_PALT chunk parsed.\n", __func__));
                    }
                }
                break;
            }
        }

        KillIFF(iff);

    }

    D(bug("[IPrefs:Palette] %s: Done.\n", __func__));
}

/*********************************************************************************************/
