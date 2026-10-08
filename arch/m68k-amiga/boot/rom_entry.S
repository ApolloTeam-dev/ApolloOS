
#include <aros/config.h>

Version = 46
Revision= 12

	.text

	.globl software_reset
	.globl _start
	.align 4 

rom_start:
	.word	0x1114
_start:
software_reset:
	jmp rom_init

    // make programs that peek rom header happy

    .word 0x0000
    .word 0xffff
    .word Version
    .word Revision
    .word Version
    .word Revision
    .word 0xffff
    .word 0xffff
    .asciz "ApolloOS ROM, OS and Libraries"
    .asciz "Copyright \251 1995-2026 "
    .asciz "Parts are developed by AROS Development Team"
    .asciz ADATE

    /* Distribution version (contents of the top-level 'version' file),
     * at a fixed offset so external tools can read it:
     *   $F800A0: "AOSV" magic, followed by the NUL-terminated string.
     * The .org below fails the build if the strings above overrun it,
     * and the .org 0xd0 fails it if the version string is too long
     * (max 43 characters). */
    .org   0xa0
    .ascii "AOSV"
    .asciz __DISTROVERSION__

    /* Magic reset vector, used by some expansion ROMs and
     * softkick programs */
    .align 4
    .org   0xd0
    .global rom_magic_reset
rom_magic_reset:
    reset
    jmp rom_init
