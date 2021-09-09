#include <aros/config.h>
#include <intuition/preferences.h>

/*
** The intuition default preferences structure
*/

CONST struct Preferences IntuitionDefaultPreferences =
{
    8,      // FontHeight
    0,      // PrinterPort
    5,      // BaudRate

    {
	{ 0 },{ 40000 }
    }
    ,   // timeval KeyRptSpeed

    {
	{ 0 },{ 500000 }
    }
    ,   // timeval KeyRptDelay

    {
	{ 0 },{ 500000 }
    }
    ,   // timeval DoubleClick


    /* Intuition Pointer data */
    {
        0x0000, 0x0000, /* USHORT PointerMatrix[POINTERSIZE];  */

        0xC000, 0x4000,
        0x7000, 0xB000,
        0x3C00, 0x4C00,
        0x3F00, 0x4300,

        0x1FC0, 0x20C0,
        0x1FC0, 0x2000,
        0x0F00, 0x1100,
        0x0D80, 0x1280,

        0x04C0, 0x0940,
        0x0460, 0x08A0,
        0x0020, 0x0040,
        0x0000, 0x0000,

        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,

        0x0000, 0x0000
    },

    // PointerMatrix (36 entries)
    0,      // XOffset
    0,      // YOffset
    0x0E44, // color17 RED
    0x0000, // color18 BLACK
    0x0EEA, // color19 IVORY
    1,      // PointerTicks

    0x0AAA, // color0-4
    0x0000,
    0x0FFF,
    0x068B,

    0,      // ViewXOffset
    0,      // ViewYOffset
    0,      // ViewInitX
    0,      // ViewINitY

    (BOOL)MOUSE_ACCEL,      // EnableCLI

    0,      // PointerType
    {
        'g','e','n','e','r','i','c',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    }
    ,// PrinterFilename (30 chars)

    0,      // PrintPitch
    0,      // PrintQuality
    0,      // PrintSpacing
    5,      // PrintLeftMargin
    75,     // PrintRightMargin
    0,      // PrintImage
    0,      // PrintAspect
    1,      // PrintShade
    2,      // PrintTreshold

    32,     // PaperSize
    66,     // PaperLength
    0,      // PaperType

    0,      // SerRWBits
    0,      // SerStopBuf
    0,      // SerParShk

    0,      // LaceWb

    {
        0,0,0,0,0,0,0,0,0,0,0,0
    }
    ,       // pad (12 bytes)
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    }
    ,   // PrtDevName (16 chars)

    0,      // DefaultPrtUnit
    0,      // DefaultSerUnit

    0,      // RowSizeChange
    0,      // ColumnSizeChange

    0,      // PrintFlags
    0,      // PrintMaxWidth
    0,      // PrintMaxHeight
    0,      // PrintDensity
    0,      // PrintXOffset

    AROS_NOMINAL_WIDTH,  // wb_Width
    AROS_NOMINAL_HEIGHT, // wb_Height
    AROS_NOMINAL_DEPTH,  // wb_Depth

    0       // ext size
};
