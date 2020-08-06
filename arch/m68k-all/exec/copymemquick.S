/*
    Copyright © 1995-2012, The AROS Development Team. All rights reserved.
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


	subq.l #4,%d0		// size is 4.b less than actual
	bls.b ism4or0		// if d0<=0
prem4:
m4loop:
	move.l (%a0)+,(%a1)+
	move.l (%a0)+,(%a1)+
	subq.w #8,%d0			// 8 less bytes to MOVE
	bhi.b m4loop			// if d0>0
ism4or0:
	beq.b lastm4
	rts
lastm4:
	move.l (%a0),(%a1)
	rts



