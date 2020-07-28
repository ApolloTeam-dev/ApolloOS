#!/bin/bash
make clean
git clean -df
rm -rf bin/amiga-m68k
./configure --target=amiga-m68k --with-optimization="-Os" --enable-ccache --with-aros-prefs=classic --with-resolution=640x256x4 --with-cpu=68020
make -j12
make -j12 distfiles
cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > aros.rom
ls -lah aros.rom

