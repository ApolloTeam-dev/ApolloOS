/*****************************************************************************

    NAME
#include <proto/kernel.h>

AROS_UFH1(void, AMMXSaveContext,
	AROS_UFHA(struct AMMXContext *, ammx, A0))

    SYNOPSIS

    LOCATION

    FUNCTION
	Save AMMX Context

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
    .globl	AMMXSaveContext

AMMXSaveContext:

    .short    0x10C8		//move.l (a0)+,b0
    .short    0x10C9		//move.l (a0)+,b1
    .short    0x10CA		//move.l (a0)+,b2
    .short    0x10CB		//move.l (a0)+,b3
    .short    0x10CC		//move.l (a0)+,b4
    .short    0x10CD		//move.l (a0)+,b5
    .short    0x10CE		//move.l (a0)+,b6
    .short    0x10CF		//move.l (a0)+,b7

    .short    0xFE58,0x0004		//Load (a0)+,B0
    .short    0xFE58,0x1004		//Load (a0)+,B1
    .short    0xFE58,0x2004		//Load (a0)+,B2
    .short    0xFE58,0x3004		//Load (a0)+,B3
    .short    0xFE58,0x4004		//Load (a0)+,B4
    .short    0xFE58,0x5004		//Load (a0)+,B5
    .short    0xFE58,0x6004		//Load (a0)+,B6
    .short    0xFE58,0x7004		//Load (a0)+,B7
    .short    0xFE58,0x8004		//Load (a0)+,B8
    .short    0xFE58,0x9004		//Load (a0)+,B9
    .short    0xFE58,0xa004		//Load (a0)+,B10
    .short    0xFE58,0xb004		//Load (a0)+,B11
    .short    0xFE58,0xc004		//Load (a0)+,B12
    .short    0xFE58,0xd004		//Load (a0)+,B13
    .short    0xFE58,0xe004		//Load (a0)+,B14
    .short    0xFE58,0xf004		//Load (a0)+,B15

    .short    0xFE18,0x8004		//Load (a0)+,e0
    .short    0xFE18,0x9004		//Load (a0)+,e1
    .short    0xFE18,0xA004		//Load (a0)+,e2
    .short    0xFE18,0xB004		//Load (a0)+,e3
    .short    0xFE18,0xC004		//Load (a0)+,e4
    .short    0xFE18,0xD004		//Load (a0)+,e5
    .short    0xFE18,0xE004		//Load (a0)+,e6
    .short    0xFE18,0xF004		//Load (a0)+,e7

    .short    0xFE18,0x0004		//Load (a0)+,d0	
    .short    0xFE18,0x1004		//Load (a0)+,d1	
    .short    0xFE18,0x2004		//Load (a0)+,d2	
    .short    0xFE18,0x3004		//Load (a0)+,d3	
    .short    0xFE18,0x4004		//Load (a0)+,d4	
    .short    0xFE18,0x5004		//Load (a0)+,d5	
    .short    0xFE18,0x6004		//Load (a0)+,d6	
    .short    0xFE18,0x7004		//Load (a0)+,d7	

    rts
