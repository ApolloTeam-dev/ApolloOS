#!/bin/bash
shopt -s extglob

## BEGIN Configuration ## Place your favorite configuration here.
JOBS="$(cat /proc/cpuinfo | grep "processor" | wc -l)"
CLEAN=0
GITCLEAN=0
DL=0
WORK="apollo-os"
REPO="https://github.com/ApolloTeam-dev/AROS"
BRANCH="v4-alynna"
REZ=640x256x4
CPU=68040
FPU=68881
OPT=s
VAMP=1
CONF=0
DEBUG=0
CONFO=""
MAKEO=""
EXCLUDE=0
## END Configuration ##

## BEGIN Variables ##
setvars () {
 DIR="$(pwd)"
 SRC="$DIR/$WORK/src"
 PORTS="$DIR/$WORK/prt"
 BIN="$DIR/$WORK/bin"
 CONFOPTS="--target=amiga-m68k --with-optimization=-O$OPT --with-aros-prefs=classic --with-resolution=$REZ --with-cpu=$CPU --with-fpu=$FPU --disable-mmu --with-portssources=$PORTS"
 if [ $VAMP = 0 ]; then CONFOPTS="$CONFOPTS --with-nonvampire-support"; fi
 if [ $DEBUG = 1 ]; then CONFOPTS="$CONFOPTS --enable-debug --with-serial-debug"; fi
 MAKEOPTS="-j$JOBS"
 PKGS="git gcc g++ make cmake gawk bison flex bzip2 netpbm autoconf automake libx11-dev libxext-dev libc6-dev liblzo2-dev libxxf86vm-dev libpng-dev libsdl1.2-dev byacc python-mako libxcursor-dev gcc-multilib"
 export CONFOPTS MAKEOPTS SRC PORTS BIN DIR PKGS
}
freevars () { unset CONFOPTS MAKEOPTS SRC PORTS BIN DIR BRANCH CLEAN DL CONF CONFO GITCLEAN JOBS MAKEO REZ VAMP WORK DEBUG CPU FPU OPT; }
## END Variables ##

## BEGIN FUNCTIONS ##
defaults () {
              echo -n "--branch=$BRANCH -c$CLEAN -d$DL -f$CONF -g$GITCLEAN --jobs=$JOBS -v$VAMP "
              if [ $REZ = "640x200x4" ]; then echo -n "-n"; else echo -n "-p"; fi
              echo " --work=$WORK --conf=\"$CONFO\" --make=\"$MAKEO\" --cpu=$CPU --fpu=$FPU --opt=$OPT"
}
deposit-rom () { 
		 if [ -e $BIN/bin/amiga-m68k/gen/boot/bootdisk-amiga-m68k.adf ]; then
                  cp $BIN/bin/amiga-m68k/gen/boot/bootdisk-amiga-m68k.adf $WORK/
                  echo ">>> ADF RESULT: $(du --apparent-size -h $WORK/bootdisk-amiga-m68k.adf)"
                 fi
		 if [ -e $BIN/bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin ]; then
                  cat $BIN/bin/amiga-m68k/gen/boot/aros-amiga-m68k-ext.bin $BIN/bin/amiga-m68k/gen/boot/aros-amiga-m68k-rom.bin >$WORK/AROS.ROM
                  echo ">>> ROM RESULT: $(du --apparent-size -h $WORK/AROS.ROM)"
                 fi
		 if [ -e $BIN/distfiles/aros-amiga-m68k.iso ]; then
                  cp $BIN/distfiles/aros-amiga-m68k.iso $WORK/
                  echo ">>> ISO RESULT: $(du --apparent-size -h $WORK/aros-amiga-m68k.iso)"
                 fi
}
makeclean () { cd $BIN; make clean; cd $DIR; }
gitclean () { cd $SRC; git clean -df; cd $DIR; }
pkgcheck () { if [ $(dpkg-query -W -f '${Binary:Package} ${Status}\n' $PKGS | wc -l) -eq $(echo $PKGS | wc -w) ]; then return 0; else return 1; fi; }
download () { cd $WORK; if [ ! -d $SRC ]; then git clone --recursive $REPO --branch=$BRANCH $SRC; cd $DIR; fi }
configure () { cd $BIN; $SRC/configure $CONFOPTS $CONFO; cd $DIR; }
compile () { cd $BIN; make $1 $MAKEOPTS $MAKEO; cd $DIR; }

valid-cpu () { 
 if [ "$1" = "" ]; then
  echo "68000 68010 68020 68030 68040 68060"
  return 0
 else 
  if [ "$1" == "680[12346]0" ]; then return 1; else return 0; fi
 fi
}

valid-fpu () {
 if [ "$1" = "" ]; then
  echo "68881 soft-float hard-float"
 else
  if [ "$1" == "68881|soft-float|hard-float" ]; then return 1; else return 0; fi
 fi
}

loop-through-opts () {
for i in "$@"; do
 case $i in
  -b=*|--branch=*)
   BRANCH="${i#*=}"
   shift
  ;;
  -b0|--no-branch)
   BRANCH=""
   shift
  ;;
  -c|-c1|--clean)
   CLEAN=1
   shift
  ;;
  -c0|--no-clean)
   CLEAN=0
   shift
  ;;
  -d|-d1|--download)
   DL=1
   shift
  ;;
  -d0|--no-download)
   DL=0
   shift
  ;;
  -f|-f1|--conf)
   CONF=1
   shift
  ;;
  -f0|--no-conf)
   CONF=0
   shift
  ;;
  -f=*|--conf=*)
   CONFO="${i#*=}"
   shift
  ;;
  -g|-g1|--gitclean)
   GITCLEAN=1
   shift
  ;;
  -g0|--no-gitclean)
   GITCLEAN=0
   shift
  ;;
  -j=*|--jobs=*)
   JOBS="${i#*=}"
   shift
  ;;
  -j)
   JOBS=$(cat /proc/cpuinfo | grep "processor" | wc -l)
   shift
  ;;
  -m=*|--make=*)
   MAKEO="${i#*=}"
   shift
  ;;
  -m0|--no-make)
   MAKEO=""
   shift
  ;;
  -n|--ntsc)
   REZ="640x200x4"
   shift
  ;;
  -p|--pal)
   REZ="640x256x4"
   shift
  ;;
  -v|-v1|--vamp)
   VAMP=1
   shift
  ;;
  -v0|--no-vamp)
   VAMP=0
   shift
  ;;
  -w=*|--work=*)
   WORK="${i#*=}"
   shift
  ;;
  -x|-x1|--exclude)
   EXCLUDE=1
   shift
  ;;
  -x0|--no-exclude)
   EXCLUDE=0
   shift
  ;;
  --cpu=*)
   CPU="${i#*=}"
   shift
  ;;
  --fpu=*)
   FPU="${i#*=}"
   shift
  ;;
  -o=*|--opt=*)
   OPT="${i#*=}"
   shift
  ;;
  --debug|--debug=1)
   DEBUG=1
   shift
  ;;
  --no-debug|--debug=0)
   DEBUG=0
   shift
  ;;
  *)
   if [ "$CMD" = "" ]; then CMD=$i; shift;
   else (echo "!!! There seems to be an issue with your Options." >&2); shift; exit 2
   fi
 esac
 export BRANCH CLEAN DL CONF CONFO GITCLEAN JOBS MAKEO REZ VAMP WORK CMD CPU FPU OPT DEBUG EXCLUDE
done
}

help () {
cat << EOF
mkapollo.sh -- Roll your own ApolloOS image and ROM

 It does what NintenDon't.
 Syntax: mkapollo.sh [options] <command> [args]

 Commands:
  all                         Run configure, make distfiles and ROM (Start here!)
  dist                        Just make distfiles
  kernel                      Just make ROM
  wipe                        Starts from near fresh (leaves ports)
  null                        Just parse options

 Options:
  -b,--branch=<branch>        Select different branch
  -c,--clean                  Make clean first
  -d,--download               Delete source and redownload
  -f,--conf                   Run configure first
  --conf=<opts>               Add configure options
  -g,--gitclean               Clean for git committing (after other operations)
  --jobs=#                    Specify number of compile threads
  -j                          One compile thread for each cpu
  --make=<opts>               Add make options
  -n,--ntsc                   Select NTSC build
  -o,--opt=#                  Select optimization level
  -p,--pal                    Select PAL build
  -v,--vamp                   Compile for True Vampires
  -w,--work=<dir>             Work directory (will be created if not present)
  -x,--exclude                When wiping, do not save cross compile tools
  --cpu=<cpu>                 Specify target CPU
  --fpu=<fpu>                 Specify target FPU
  --debug                     Enable debugging

  Defaults can be changed at the top of this file.
  Nearly all options take a boolean 0/1 to override defaults, as well as --no-<option>

  Valid optimization levels: s 0 1 2 3
EOF
echo "  Valid CPUs               : $(valid-cpu)"
echo "  Valid FPUs               : $(valid-fpu)"
cat << EOF

 It will automatically install the packages needed for Vampire Goodness.
 Note that sources will be downloaded if it is not found at the location.
 If you just want your ROM, just do:  mkapollo.sh all
 Then go get a coffee. Or a meal.

 The current defaults are:
EOF
echo "  $(defaults)"
echo -e "\nPlease Amiga responsibly."
}
## END FUNCTIONS ##

## BEGIN MAIN ##
loop-through-opts $@
if [ "$CMD" = "" ]; 	then help; exit 1; 	fi

setvars
echo "Making ApolloOS:     $CMD $(defaults)"
echo "Configuration:       $CONFOPTS"
echo "Make options:        $MAKEOPTS"
echo "Work directory:      $WORK"
echo -n "3..."; sleep 1; echo -n "2..."; sleep 1; echo -n "1..."; sleep 1; echo "Go!";

mkdir -p $BIN

if [ $(pkgcheck; echo $?) = 1 ]; then
 echo ">>> You are missing required packages to build ApolloOS.  Attempting to install."
 apt -y update
 apt -y install $PKGS
fi
if [ $DL = 1 ]; 	         then echo ">>> Source wipe requested, deleting."; 	rm -rf $SRC; 	fi
if [ ! -e $SRC ]; 	         then echo ">>> Source not detected, downloading.";	download;	fi
if [ $CLEAN = 1 ]; 	         then echo ">>> Fresh build requested, cleaning.";	makeclean; 	fi

case $CMD in
 dist)
  if [ $CONF = 1 ] || [ ! -e $BIN/config.status ]; then configure; fi
  compile distfiles
 ;;
 kernel)
  if [ $CONF = 1 ] || [ ! -e $BIN/config.status ]; then configure; fi
  compile kernel
  deposit-rom
 ;;
 all)
  configure
  compile distfiles
  deposit-rom
 ;;
 wipe)
  echo "!!! CTRL-AMIGA-AMIGA Pressed !!!"
  echo ">>> Removing sources"
  rm -rf $SRC
  if [ "$EXCLUDE" = "0" ]; then
   echo ">>> Preserving Crosstools (saves serious recompile time)"
   mv $BIN/bin/linux* $PORTS/
  fi
  echo ">>> Removing Binaries (Things in the work dir root are preserved)"
  rm -rf $BIN
  if [ "$EXCLUDE" = "0" ]; then
   echo ">>> Restoring Crosstools"
   mv $PORTS/linux* $BIN/bin/
  fi
  echo ">>> Done.  You may now redownload or even choose another branch."
 ;;
 *)
  help
  exit 1
 ;;
esac
if [ $GITCLEAN = 1 ]; 	then echo ">>> Cleaning git artifacts.";		gitclean; 	fi
freevars
echo "Please Amiga responsibly."
## END MAIN ##
