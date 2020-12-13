/*
    Copyright � 1995-2012, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Optimized CopyMemQuick by Gunnar von Boehn.
    Lang: english
*/

	#include "aros/m68k/asm.h"

#define UNALIGNEDWARNING 1
	.text
	.balign 4

	.globl	AROS_SLIB_ENTRY(CopyMemQuick,Exec,105)
	.type	AROS_SLIB_ENTRY(CopyMemQuick,Exec,105),@function
AROS_SLIB_ENTRY(CopyMemQuick,Exec,105):
#ifdef NONVAMPIRE

	subq.l #4,%d0		// size is 4.b less than actual
	bls.b ism4or0		// if d0<=0
prem4:
m4loop:
	move.l (%a0)+,(%a1)+
	move.l (%a0)+,(%a1)+
	subq.l #8,%d0			// 8 less bytes to MOVE
	bhi.b m4loop			// if d0>0
ism4or0:
	beq.b lastm4
	rts
lastm4:
	move.l (%a0),(%a1)
	rts
#else
	lsr.l #2,%d0		//
	beq   exit

	.short 0x0809,0x0002	//	btst   #2,%a1	
	beq    aligned
	move.l (%a0)+,(%a1)+
	subq.l	#1,%d0
aligned:
	move.l	%d0,%d1
	and.l	#3,%d0
	lsr.l	#2,%d1
	beq	zend

loop:
	move16 (%a0)+,(%a1)+
	subq.l #1,%d1
	bne.b  loop
	bra    zend
zloop:
	move.l (%a0)+,(%a1)+
zend:
	dbra %d0,zloop

exit:	rts
#endif
