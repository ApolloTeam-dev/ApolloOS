/*****************************************************************************

    NAME
#include <proto/kernel.h>

AROS_UFH1(void, AMMXRestoreContext,
	AROS_UFHA(struct AMMXContext *, ammx, A0))

    SYNOPSIS

    LOCATION

    FUNCTION
	Restore AMMX Context

    INPUTS
	None

    RESULT
	None

    NOTES
	Must be called in supervisor mode.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

******************************************************************************/
    #include "aros/m68k/asm.h"

    .text
    .balign 4
    .globl	AMMXRestoreContext

AMMXRestoreContext:

    .short    0x1058		//move.l (a0)+,b0
    .short    0x1258		//move.l (a0)+,b1
    .short    0x1458		//move.l (a0)+,b2
    .short    0x1658		//move.l (a0)+,b3
    .short    0x1858		//move.l (a0)+,b4
    .short    0x1A58		//move.l (a0)+,b5
    .short    0x1C58		//move.l (a0)+,b6
    .short    0x1E58		//move.l (a0)+,b7

    .short    0xFE18,0x0001		//Load (a0)+,D0
    .short    0xFE18,0x0101		//Load (a0)+,D1
    .short    0xFE18,0x0201		//Load (a0)+,D2
    .short    0xFE18,0x0301		//Load (a0)+,D3
    .short    0xFE18,0x0401		//Load (a0)+,D4
    .short    0xFE18,0x0501		//Load (a0)+,D5
    .short    0xFE18,0x0601		//Load (a0)+,D6
    .short    0xFE18,0x0701		//Load (a0)+,D7
    .short    0xFE18,0x0801		//Load (a0)+,E0
    .short    0xFE18,0x0901		//Load (a0)+,E1
    .short    0xFE18,0x0a01		//Load (a0)+,E2
    .short    0xFE18,0x0b01		//Load (a0)+,E3
    .short    0xFE18,0x0c01		//Load (a0)+,E4
    .short    0xFE18,0x0d01		//Load (a0)+,E5
    .short    0xFE18,0x0e01		//Load (a0)+,E6
    .short    0xFE18,0x0f01		//Load (a0)+,E7

    .short    0xFE58,0x0001		//Load (a0)+,E8
    .short    0xFE58,0x0101		//Load (a0)+,E9
    .short    0xFE58,0x0201		//Load (a0)+,E10
    .short    0xFE58,0x0301		//Load (a0)+,E11
    .short    0xFE58,0x0401		//Load (a0)+,E12
    .short    0xFE58,0x0501		//Load (a0)+,E13
    .short    0xFE58,0x0601		//Load (a0)+,E14
    .short    0xFE58,0x0701		//Load (a0)+,E15
    .short    0xFE58,0x0801		//Load (a0)+,E16
    .short    0xFE58,0x0901		//Load (a0)+,E17
    .short    0xFE58,0x0a01		//Load (a0)+,E18
    .short    0xFE58,0x0b01		//Load (a0)+,E19
    .short    0xFE58,0x0c01		//Load (a0)+,E20
    .short    0xFE58,0x0d01		//Load (a0)+,E21
    .short    0xFE58,0x0e01		//Load (a0)+,E22
    .short    0xFE58,0x0f01		//Load (a0)+,E23


    rts
