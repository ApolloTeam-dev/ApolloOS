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
	exit
fi

echo "##############################################"
echo "# Rebuilding rom quick"
echo "# CPU Count: ${CPU_COUNT}"
echo "# Rebuilding with ${THREADS} parallel tasks"
echo "##############################################"
sleep 1

rm -rf config/features.status

make -j${THREADS} kernel-amiga-m68k

if [ "${args[ 0 ]}" == "--with-nonvampire-support" ]
then
	cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > ApolloROM.UAE
	ls -lah ApolloROM.UAE
else
	cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > ApolloROM.V4
	ls -lah ApolloROM.V4
fi

