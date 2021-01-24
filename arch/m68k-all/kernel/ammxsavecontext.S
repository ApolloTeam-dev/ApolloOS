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

    .short    0xFE18,0x0004		//Store (a0)+,D0
    .short    0xFE18,0x1004		//Store (a0)+,D1
    .short    0xFE18,0x2004		//Store (a0)+,D2
    .short    0xFE18,0x3004		//Store (a0)+,D3
    .short    0xFE18,0x4004		//Store (a0)+,D4
    .short    0xFE18,0x5004		//Store (a0)+,D5
    .short    0xFE18,0x6004		//Store (a0)+,D6
    .short    0xFE18,0x7004		//Store (a0)+,D7
    .short    0xFE18,0x8004		//Store (a0)+,E0
    .short    0xFE18,0x9004		//Store (a0)+,E1
    .short    0xFE18,0xa004		//Store (a0)+,E2
    .short    0xFE18,0xb004		//Store (a0)+,E3
    .short    0xFE18,0xc004		//Store (a0)+,E4
    .short    0xFE18,0xd004		//Store (a0)+,E5
    .short    0xFE18,0xe004		//Store (a0)+,E6
    .short    0xFE18,0xf004		//Store (a0)+,E7


    .short    0xFE98,0x0004		//Store (a0)+,E8
    .short    0xFE98,0x1004		//Store (a0)+,E9
    .short    0xFE98,0x2004		//Store (a0)+,E10
    .short    0xFE98,0x3004		//Store (a0)+,E11
    .short    0xFE98,0x4004		//Store (a0)+,E12
    .short    0xFE98,0x5004		//Store (a0)+,E13
    .short    0xFE98,0x6004		//Store (a0)+,E14
    .short    0xFE98,0x7004		//Store (a0)+,E15
    .short    0xFE98,0x8004		//Store (a0)+,E16
    .short    0xFE98,0x9004		//Store (a0)+,E17
    .short    0xFE98,0xa004		//Store (a0)+,E18
    .short    0xFE98,0xb004		//Store (a0)+,E19
    .short    0xFE98,0xc004		//Store (a0)+,E20
    .short    0xFE98,0xd004		//Store (a0)+,E21
    .short    0xFE98,0xe004		//Store (a0)+,E22
    .short    0xFE98,0xf004		//Store (a0)+,E23

    rts
