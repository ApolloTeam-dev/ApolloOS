/*****************************************************************************

    NAME
#include <proto/kernel.h>

AROS_UFH1(void, FpuSaveContext,
	AROS_UFHA(struct FpuContext *, fpu, A0)
	AROS_UFHA(UWORD, nulloffset, D0))

    SYNOPSIS

    LOCATION

    FUNCTION
	Save FPU Context

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
	.globl	FpuSaveContext

FpuSaveContext:
        fmovem.l        %fpcr/%fpsr/%fpiar,64(%a0)
        fmove.d         %fp0,(%a0)+
        fmove.d         %fp1,(%a0)+
        fmove.d         %fp2,(%a0)+
        fmove.d         %fp3,(%a0)+
        fmove.d         %fp4,(%a0)+
        fmove.d         %fp5,(%a0)+
        fmove.d         %fp6,(%a0)+
        fmove.d         %fp7,(%a0)+
0:	rts
