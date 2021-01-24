/*
    Copyright © 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

/******************************************************************************
 
    NAME
 
 	AROS_LH3(void, CachePostDMA,
 
    SYNOPSIS
 	AROS_LHA(APTR,    address, A0),
 	AROS_LHA(ULONG *, length,  A1),
 	AROS_LHA(ULONG,   flags,  D0),
 
    LOCATION
 	struct ExecBase *, SysBase, 128, Exec)
 
    FUNCTION
 	Do everything necessary to make CPU caches aware that a DMA has
 	happened.
 
    INPUTS
 	address - Virtual address of memory affected by the DMA
 	*length - Number of bytes affected
 	flags	- DMA_NoModify	  - Indicate that the memory did not change.
 		  DMA_ReadFromRAM - Indicate that the DMA goes from RAM
 				    to the device. Set this bit in bot calls.
 
    RESULT
 
    NOTES
 	DMA must follow a call to CachePreDMA() and must be followed
 	by a call to CachePostDMA().
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
 	CachePreDMA()
 
    INTERNALS
 
    HISTORY
 
******************************************************************************/

/*
   XDEF AROS_SLIB_ENTRY(CachePostDMA_00,Exec,128)  	; for 68000/10/20
   XDEF AROS_SLIB_ENTRY(CachePostDMA_30,Exec,128)	; for 68030+
   XDEF AROS_SLIB_ENTRY(CachePostDMA_40,Exec,128)	; for 68040/68060
*/

	#include "aros/m68k/asm.h"
	#include "cache.h"

	#define DMAB_Continue	    1  /* Continuation flag for CachePreDMA */
	#define DMAB_NoModify	    2  /* Set if DMA does not update memory */
	#define	DMAB_ReadFromRAM    3  /* Set if DMA goes *FROM* RAM to device */

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CachePostDMA_00,Exec,128)
	.type	AROS_SLIB_ENTRY(CachePostDMA_00,Exec,128),@function
AROS_SLIB_ENTRY(CachePostDMA_00,Exec,128):


	rts

