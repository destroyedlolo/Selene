#!/bin/bash
#
# this script is used to 
# - enable / disable the build of particular modules
# - create and update Makefiles

# set following variables depending on which module you want to build.
# if unset, the module is not built.

# Build Curses plugin
#USE_CURSES=1

# Build OLED screen plugin
#USE_OLED=1

# Build DRMCairo plugin
#USE_DRMCAIRO=1
# include fall-back to stock frame buffer
#DRMC_WITH_FB=1

# Build directFB plugin
#USE_DIRECTFB=1

# DEBUG - Add debuging messages
DEBUG=1

# MCHECK - check memory concistency (see glibc's mcheck())
MCHECK=1

# end of customisation area

# build configuration
# ----------------

# where to install plugins
# production
#PLUGIN_DIR=/usr/local/lib/Selene
# for development
PLUGIN_DIR=$( pwd )/lib/Selene

if [ ${PLUGIN_DIR+x} ]
then
	USE_PLUGDIR="-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR"
fi

# -------------------------------------
#      END OF CONFIGURATION AREA
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# -------------------------------------

# Error is fatal
set -e

# =================
# Rebuild Makefiles
# =================

echo -e "\nBuild Makefiles\n===============\n"

echo "# global Makefile that is calling sub directories ones" > Makefile
echo >> Makefile
echo "gotoall: all" >> Makefile
echo >> Makefile

echo "# Clean previous builds sequels" >> Makefile
echo "clean:" >> Makefile
echo -e "\t-rm -f Selene src/testSelene/testSelene" >> Makefile
echo -e "\t-rm -f lib/Selene/*.so" >> Makefile
echo -e "\t-rm -f src/*/*.o" >> Makefile

echo >> Makefile
echo "# Build everything" >> Makefile
echo "all:" >> Makefile

# =============================
# Configure external components
# =============================

echo -e "\nSet build options\n=================\n"

CFLAGS="-Wall -O2 -fPIC -Wno-unused-result"

# Lua version
if true; then	# OS Version
	if ! command -v lua &> /dev/null
	then
		echo "No Lua found"
		exit 1
	fi

	VERLUA=$( lua -v 2>&1 | grep -o -E '[0-9]\.[0-9]' )
	echo -n "Lua's version :" $VERLUA

	if pkg-config --cflags lua$VERLUA > /dev/null 2>&1; then
		echo "  (Packaged)"
		LUA="\$(shell pkg-config --cflags lua$VERLUA ) -DLUA"
		LUALIB="\$(shell pkg-config --libs lua$VERLUA )"
	elif pkg-config --cflags lua > /dev/null 2>&1; then
		echo " (unpackaged)"
		LUA="\$(shell pkg-config --cflags lua ) -DLUA"
		LUALIB="\$(shell pkg-config --libs lua )"
	else
		echo " - No package found"
		echo "Workaround : edit this remake file to hardcode where Lua is installed."
		echo
		exit 1
	fi
else
# Force custom 5.3.4 version
	LUA_DIR=/home/laurent/Projets/lua-5.3.4/install
	LUA="-isystem $LUA_DIR/include"
	LUALIB="-L$LUA_DIR/lib"
fi


# Let's go

echo
echo "Global settings"
echo "==============="
echo

if [ ${DEBUG+x} ]; then
	DEBUG="-DDEBUG"
else
	echo "DEBUG not defined"
fi

if [ ${MCHECK+x} ]; then
	echo "Memory checking activated"

	MCHECK='-DMCHECK="mcheck(NULL)"'
	MCHECK_LIB="-lmcheck"
else
	echo "No memory checking"
fi

echo
echo "Plug-ins settings"
echo "================="
echo

echo
echo "Curses plug-in"
echo "--------------"

if [ ${USE_CURSES+x} ]; then
	if which ncursesw6-config > /dev/null 2>&1; then
		echo "Found ncurses v6"
        NCURSES=ncursesw6-config
	elif which ncursesw5-config > /dev/null 2>&1; then
        echo "Found ncurses v5"
        NCURSES=ncursesw5-config
	else
		echo "Curse not found : Failing ..."
		exit 20
	fi
	USE_CURSES="-DUSE_CURSES \$(shell $NCURSES --cflags )"
	USE_CURSES_LIB="\$(shell $NCURSES --libs )"

	cd src/SelPlugins/Curses/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_CURSES $USE_CURSES_LIB" *.c -so=../../../lib/Selene/SelCurses.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/Curses' >> Makefile
else
	echo "Curse not used."
fi


echo
echo "OLED plug-in"
echo "------------"

if [ ${USE_OLED+x} ]; then
	echo "OLED used"
	USE_OLED="-DUSE_OLED"
	USE_OLED_LIB="-lArduiPi_OLED"

	cd src/SelPlugins/OLED/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_OLED $USE_OLED_LIB" *.c -so=../../../lib/Selene/SelOLED.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/OLED' >> Makefile
else
	echo "OLED not used"
fi


echo
echo "DRMCairo plugin"
echo "---------------"

if [ ${USE_DRMCAIRO+x} ]; then
	USE_DRMCAIRO="-DUSE_DRMCAIRO \$(shell pkg-config --cflags libdrm cairo freetype2 )"
	USE_DRMCAIRO_LIB="\$(shell pkg-config --libs libdrm cairo freetype2 )"
	echo "DRMCairo used"

	if pkg-config --cflags libkms; then
		USE_DRMCAIRO="$USE_DRMCAIRO \$(shell pkg-config --cflags libkms )"
		USE_DRMCAIRO_LIB="$USE_DRMCAIRO_LIB \$(shell pkg-config --libs libkms )"
	else
		USE_DRMCAIRO="$USE_DRMCAIRO -DKMS_MISSING"
		echo "WARNING : Kms is missing"
	fi

	if [ ${DRMC_WITH_FB+x} ]; then
		USE_DRMCAIRO="-DDRMC_WITH_FB $USE_DRMCAIRO"
		echo "with Framebuffer fallback"
	fi

	cd src/SelPlugins/DRMCairo/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_DRMCAIRO $USE_DRMCAIRO_LIB" *.c -so=../../../lib/Selene/SelDRMCairo.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/DRMCairo' >> Makefile
else
	echo "DRMCairo not used"
fi

echo
echo "DirectFB source"
echo "-----------"

if [ ${USE_DIRECTFB+x} ]; then
	USE_DIRECTFB="-DUSE_DIRECTFB \$(shell directfb-config --cflags )"
	USE_DIRECTFB_LIB="\$(shell directfb-config --libs )"
	echo "DirectFB used"

	cd src/SelPlugins/DirectFB/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_DIRECTFB $USE_DIRECTFB_LIB" *.c -so=../../../lib/Selene/SelDirectFB.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/DirectFB' >> Makefile
else
	echo "DirectFB not used"
fi


echo
echo "Selene Library"
echo "=============="
echo

cd src/libSelene
LFMakeMaker -v -I../include/ +f=Makefile --opts="-I../include $CFLAGS $DEBUG $MCHECK $USE_PLUGDIR" *.c -so=../../lib/Selene/libSelene.so.2 > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/libSelene' >> Makefile

echo
echo "SeleneCore"
echo "=========="
echo

cd src/SeleneCore
LFMakeMaker -v -I../include/ +f=Makefile \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SeleneCore.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SeleneCore' >> Makefile

echo
echo "SelLog"
echo "======"
echo

cd src/SelLog
LFMakeMaker -v -I../include/ +f=Makefile \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelLog.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelLog' >> Makefile

echo
echo "SelLua"
echo "======="
echo

cd src/SelLua
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelLua.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelLua' >> Makefile

echo
echo "SelScripting"
echo "============"
echo

cd src/SelScripting
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelScripting.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelScripting' >> Makefile

echo
echo "SelElasticStorage"
echo "================="
echo

cd src/SelElasticStorage
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
    --opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
    *.c -so=../../lib/Selene/SelElasticStorage.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelElasticStorage' >> Makefile

echo
echo "SelMultitasking"
echo "==============="
echo

cd src/SelMultitasking
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
    --opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
    *.c -so=../../lib/Selene/SelMultitasking.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelMultitasking' >> Makefile

# Not yet implemented
# echo
# echo "SelSharedFunction"
# echo "================="
# echo

# cd src/SelSharedFunction
# LFMakeMaker -v -I../include/ +f=Makefile -I../include \
#    --opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
#    *.c -so=../../lib/Selene/SelSharedFunction.so > Makefile
# cd ../..
# echo -e '\t$(MAKE) -C src/SelSharedFunction' >> Makefile

echo
echo "SelSharedVar"
echo "============"
echo

cd src/SelSharedVar
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
   --opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
   *.c -so=../../lib/Selene/SelSharedVar.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelSharedVar' >> Makefile

echo
echo "SelMQTT"
echo "======="
echo

cd src/SelMQTT
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelMQTT.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelMQTT' >> Makefile

echo
echo "SelError"
echo "========"
echo

cd src/SelError
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelError.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelError' >> Makefile

echo
echo "SelTimer"
echo "========"
echo

cd src/SelTimer
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelTimer.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelTimer' >> Makefile

echo
echo "SelFIFO"
echo "======="
echo

cd src/SelFIFO
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelFIFO.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelFIFO' >> Makefile

echo
echo "SelEvent"
echo "========"
echo

cd src/SelEvent
LFMakeMaker -v -I../include/ +f=Makefile -I../include \
	--opts="-I../include $CFLAGS $DEBUG $MCHECK $LUA $USE_PLUGDIR" \
	*.c -so=../../lib/Selene/SelEvent.so > Makefile
cd ../..
echo -e '\t$(MAKE) -C src/SelEvent' >> Makefile

if [ ${DEBUG+x} ]; then
	echo
	echo "Test source"
	echo "==========="
	echo

	cd src/testSelene

	LFMakeMaker -v -I../include/ +f=Makefile --opts="-I../include $CFLAGS $DEBUG $MCHECK \
		$LUA $LUALIB \
		$USE_DRMCAIRO \
		$USE_DIRECTFB \
		$USE_CURSES \
		$USE_OLED \
		$MCHECK_LIB \
		$USE_PLUGDIR \
		-l:libSelene.so.2 -lpaho-mqtt3c $LUA -lm -ldl -Wl,--export-dynamic -lpthread" \
		*.c -t=testSelene > Makefile

	cd ../..
	echo -e '\t$(MAKE) -C src/testSelene' >> Makefile
fi

echo
echo "Selene"
echo "======"
echo

cd src/Selene

LFMakeMaker -g -v -I../include/ +f=Makefile --opts="-I../include $CFLAGS $DEBUG $MCHECK \
	$LUA $LUALIB \
	$USE_DRMCAIRO \
	$USE_DIRECTFB \
	$USE_CURSES \
	$USE_OLED \
	$MCHECK_LIB \
	$USE_PLUGDIR \
	-l:libSelene.so.2 -lpaho-mqtt3c $LUA -lm -ldl -Wl,--export-dynamic -lpthread" \
*.c -t=../../Selene > Makefile

cd ../..
echo -e '\t$(MAKE) -C src/Selene' >> Makefile

echo
echo "C examples"
echo "=========="
echo

cd Clenites
rm -f make.sh

for f in *.c 
do
	echo "cc -I../src/include/ $CFLAGS $DEBUG $MCHECK $MCHECK_LIB $USE_PLUGDIR -l:libSelene.so.2 -lpaho-mqtt3c -lm -ldl -Wl,--export-dynamic -lpthread $f -o $( basename $f .c )" >> make.sh
done

cd ../..

if [ ${PLUGIN_DIR+x} ]
then
	echo
	echo "Don't forget if you want to run it without installing first"
	echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH
fi
