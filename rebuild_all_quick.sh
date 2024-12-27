#!/bin/bash
CPU_COUNT=$(grep processor /proc/cpuinfo | wc -l)
THREADS=${CPU_COUNT}

#Some how, running more than 8 tasks doesn't succeed every time
#if [ ${THREADS} -gt  8 ]
#then
#	THREADS=8
#fi

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

rm -rf config/features.status

make -j${THREADS} distfiles

if [ "${args[ 0 ]}" == "--with-nonvampire-support" ]
then
	cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > ApolloROM.UAE
	ls -lah ApolloROM.UAE
else
	cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > ApolloROM.V4
	ls -lah ApolloROM.V4
fi

