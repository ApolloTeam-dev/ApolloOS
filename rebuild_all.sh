#!/bin/bash
CPU_COUNT=$(grep processor /proc/cpuinfo | wc -l)
THREADS=${CPU_COUNT}

#Some how, running more than 8 tasks doesn't succeed every time
if [ ${THREADS} -gt  8 ]
then
	THREADS=8
fi

args=("$@")
if [ "${args[ 0 ]}" == "-h" ]
then
	echo "Options are:"
	echo "-h				prints this help text"
	echo "--with-nonvampire-support	building for non-vampire platforms"
	exit
fi

echo "##############################################"
echo "# Rebuilding all"
echo "# CPU Count: ${CPU_COUNT}"
echo "# Rebuilding with ${THREADS} parallel tasks"
if [ $# -gt 0 ]; then echo "# Building with $@"; fi
echo "##############################################"
sleep 3

make clean
git clean -df
rm -rf bin/amiga-m68k

source ./make_dist_config.sh
DISTOPTNAME="--enable-dist-name=${DISTRONAME}"
DISTOPTVER="--enable-dist-version=${DISTROVERSION}"

./configure "${DISTOPTNAME}" "${DISTOPTVER}" --target=amiga-m68k --with-optimization="-O2" --enable-ccache --with-aros-prefs=classic --with-resolution=640x256x4 --with-cpu=68040 --disable-mmu $@


make -j${THREADS}
make -j${THREADS} distfiles
cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > aros.rom
ls -lah aros.rom

