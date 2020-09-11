/*
    Copyright © 2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Optimized CopyMem by Gunnar
    Lang: english
*/

	#include "aros/m68k/asm.h"

	.text

	.globl	AROS_SLIB_ENTRY(CopyMem,Exec,104)
	.type	AROS_SLIB_ENTRY(CopyMem,Exec,104),@function
AROS_SLIB_ENTRY(CopyMem,Exec,104):
#ifdef NONVAMPIRE
	cmp.l #8,%d0		// max. 8 left, 7 right and at least 8 in the middle (the latter two are checked against 0)
	blt   zend

	move.l	%a1,%d1	// dest pointer
	and.l	#7,%d1		// 0..7
	sub.l	%d1,%d0		// width minus maxbytes(phase1)
	bra	aend

aloop:
	move.b (%a0)+,(%a1)+
aend:	dbra   %d1,aloop


	move.l  %d0,%d1		// bytes remaining after phase 1
	and.l 	#7,%d0		// leftover bytes for phase 3
	lsr.l 	#3,%d1		// remaining bytes / 8 (for main phase)
	beq	zend

	// phase 2: main copy loop (8 Bytes)
xloop:
	move.l (%a0)+,(%a1)+
	move.l (%a0)+,(%a1)+
	subq.l #1,%d1
	bnes   xloop

	bra.s  zend

zloop:
	move.b (%a0)+,(%a1)+
zend:
	dbf %d0,zloop


end:
	rts
#else
	cmp.l #8,%d0		// max. 8 left, 7 right and at least 8 in the middle (the latter two are checked against 0)
	blt   zend

        .short 0x0809,0x0000    //      btst   #0,%a1   
        beq    aligned1
        move.b (%a0)+,(%a1)+
        subq.l  #1,%d0
aligned1:
        .short 0x0809,0x0001    //      btst   #1,%a1   
        beq    aligned2
        move.w (%a0)+,(%a1)+
        subq.l  #2,%d0
aligned2:
        .short 0x0809,0x0002    //      btst   #2,%a1   
        beq    aligned4
        move.l (%a0)+,(%a1)+
        subq.l  #4,%d0
aligned4:

	move.l  %d0,%d1		// bytes remaining after phase 1
	and.l 	#7,%d0		// leftover bytes for phase 3
	lsr.l 	#3,%d1		// remaining bytes / 8 (for main phase)
	beq	zend

	// phase 2: main copy loop (8 Bytes)
xloop:
	move.l (%a0)+,(%a1)+
	move.l (%a0)+,(%a1)+
	subq.l #1,%d1
	bnes   xloop

	bra.s  zend

zloop:
	move.b (%a0)+,(%a1)+
zend:
	dbf %d0,zloop

end:
	rts
#endif
