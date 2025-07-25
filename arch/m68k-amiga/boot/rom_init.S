/*
 * Copyright (C) 2013-2019, The AROS Development Team
 * All right reserved.
 * Author: Jason S. McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the AROS PUBLIC LICENSE (APL) Version 1.1
 * 
 * $Id$
 */

#include <aros/m68k/asm.h>
#include <aros/config.h>

	.text
	.chip 68000
	.align 4 

	.globl __MemoryTest
	.globl rom_init
rom_init:
	bra.w normal_init
	bra.w arosbootstrap_init

normal_init:

	/* Wait a bit for reset to finish.
	 * Simulate ROM checksum check.
	 */
	lea		0x00f80000,%a0
	move.l	#524288/4,%d0
	moveq	#0,%d1
0:	add.l	(%a0)+,%d1
	bcc.s	1f
	addq.l	#1,%d1
1:	subq.l	#1,%d0
	bne.s	0b

	/* Check for cartridge ROM */
	lea	0xf00000,%a0
	move.w	%a0@(0),%d0	/* Get the ROM ID (0x1111) */
	cmp.w	#0x1111,%d0
	bne.s	0f		/* Must be 0x1111 */
	lea %pc@(0),%a1
	move.l %a1,%d0
	clr.w %d0
	cmp.l %d0,%a0
	beq.s 0f /* skip if we booted at 0xF00000 */
	lea	0x40000,%sp	/* Safe stack at Chip Mem 128K */
	lea	0f,%a5
	jmp	%a0@(2)
0:

	/* Disable the overlay, power led off. */
	move.b	#00,0xbfe001
	move.b	#03,0xbfe201

arosbootstrap_init:

	/* Initial Custom chipset setup. Does same writes as AOS.
	 * It is possible some bus snooping hardware assumes this.
	 */
	lea.l	0xdff000,%a0
	move.w	#0x7fff,%d0
	move.w	%d0,0x9a(%a0)	/* Interrupt disable */
	move.w	%d0,0x9c(%a0)	/* Clear interrupts */
	move.w	%d0,0x96(%a0)	/* Stop DMA */
	move.w	#0x0174,0x032(%a0)	/* SERPER */
	move.w	#0x0200,0x100(%a0)	/* BPLCON0 */
	move.w	#0x0000,0x110(%a0)	/* BPL1DAT */
	move.w	#0x0111,0x180(%a0)	/* COLOR00 */

	/* temp system stack */
	lea.l	_ss_end,%sp		/* System stack */
	/* Put the memory bank array at the bottom of the stack, reserve space for extra memory bank */
	lea.l	_ss,%a5

	/* Do we have a Gayle? */
	sub.l	%a6,%a6
	jsr	AROS_SLIB_ENTRY(ReadGayle,Exec,136)
	moveq	#0,%d6
	tst.b	%d0
	beq.s	.nopcmcia
	moveq	#1,%d6
	/* Disable PCMCIA slot */
	move.b	#1,0xda8000
.nopcmcia:
	swap	%d6

	move.l	%a5,%a4	/* Space for CPU/FPU data array */
	lea	16(%a5),%a5	/* Space also reserved for extra membank */
	move.l	%a4,%a0
	jsr	_cpu_detect
	/* High word = gayle flag, low word = AttnFlags */
	move.w	%a4@(2),%d6


	/* set AttnFlags AFB_ADDR32 */
	or.w	#0x2000,%a4@(2)

	#if VAMPIRECARDSERIES==4
	//V4 version
	move.l	#0x01800000,%a0		// Reserve 8MB for ApolloFloppy
	move.l	#0x40000000,%a1		// SA 1Gb
	move.l	#0x00100000,%d0
	jsr	__MemoryTest
	#else

		#if VAMPIRECARDSERIES==8
		//V4 version
		move.l	#0x01800000,%a0		// Reserve 8MB for ApolloFloppy
		move.l	#0x80000000,%a1		// UniCorn 2Gb
		move.l	#0x00100000,%d0
		jsr	__MemoryTest

		#else
		move.l	#0x08000000,%a0
		move.l	#0x20000000,%a1
		move.l	#0x00100000,%d0
		jsr	__MemoryTest

		move.l	#0x01000000,%a0
		move.l	#0x08000000,%a1
		move.l	#0x00100000,%d0
		jsr	__MemoryTest
		#endif

	#endif


	/* Memory region array is put into the
	 * system stack from the bottom up, so
	 * we check the non-autoconfig memory areas
	 * in order of fastest to slowest, going
	 * up the system stack in A5.
	 */

	/* slow ram */
	move.l	#0x00c00000,%a0
	move.l	#0x00d80000,%a1
	move.l	#0x00040000,%d0
	jsr	__MemoryTest
	
	/* chip ram */
	sub.l	%a0,%a0
	move.l	#0x00B80000,%a1
	move.l	#0x00040000,%d0
	jsr	__MemoryTest
	beq	chipfault

	/* end of memory region list */
	clr.l	%a5@+
	clr.l	%a5@

	lea.l	_ss,%a5
	move.l	%a5,%sp@-	/* CPU/FPU data array */
	lea		16(%a5),%a5
	move.l	%a5,%sp@-	/* memory bank array */
	jsr		exec_boot

chipfault:
	move.w	#0x0f0,0xdff180
looping:
	nop
	bra.s	looping

#define STACK_OFFSET (6*4)
	.globl Early_Exception
Early_Exception:
	or.w	#0x0700,%sr
#if AROS_SERIAL_DEBUG
	movem.l	%d0-%d2/%a0-%a2,-(%sp)

	lea		DebugPutHex,%a2

	move.l	STACK_OFFSET+2(%sp),-(%sp)
	pea		.earlystr1(%pc)
	jsr		(%a2)
	addq.l	#8,%sp
	
	moveq	#0,%d0
	move.w	STACK_OFFSET+6(%sp),%d0
	move.l	%d0,-(%sp)
	pea		.earlystr2(%pc)
	jsr		(%a2)
	addq.l	#8,%sp

	move.l	STACK_OFFSET+8(%sp),-(%sp)
	pea		.earlystr3(%pc)
	jsr		(%a2)
	addq.l	#8,%sp

	movem.l	(%sp)+,%d0-%d2/%a0-%a2
#endif
	jmp		Early_TrapHandler

        .section .rodata
.earlystr1:
	.string "Early Exception!\nPC"
.earlystr2:
	.string "Vector"
.earlystr3:
	.string "Address"
