#!/bin/bash
make -j8 kernel-amiga-m68k
cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > aros.rom
ls -lah aros.rom

