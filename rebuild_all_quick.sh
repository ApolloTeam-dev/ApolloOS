#!/bin/bash
WORK="apollo-os"
DISTRONAME="ApolloOS"
DISTROVERSION="$(cat version)"
DISTRODATE="$(date +%Y-%m-%d)"
AMIGADATE="$(date +"%-d.%-m.%Y")"

mkdir -p "bin/amiga-m68k/gen/"
VERSION_FILE="bin/amiga-m68k/gen/dist_config.h"

printf "#ifndef AROS_DIST_CONFIG_H\n#define AROS_DIST_CONFIG_H\n\n" > "${VERSION_FILE}"
# shellcheck disable=SC2129
printf "#define __DISTRONAME__\t\t\"%s\"\n" "${DISTRONAME}" >> "${VERSION_FILE}"
printf "#define __DISTROVERSION__\t\"%s\"\n" "${DISTROVERSION}" >> "${VERSION_FILE}"
printf "#define __DISTRODATE__\t\t\"%s\"\n" "${DISTRODATE}" >> "${VERSION_FILE}"
printf "#define __AMIGADATE__\t\t\"%s\"\n" "${AMIGADATE}" >> "${VERSION_FILE}"
printf "\n#endif //AROS_DIST_CONFIG_H\n" >> "${VERSION_FILE}"

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

make -j${THREADS} distfiles
cat bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin > aros.rom
ls -lah aros.rom

