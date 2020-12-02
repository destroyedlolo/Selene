#!/bin/bash

# this version compile against OS' Lua

# set following variable depending on which module you want to build.
# if unset, the module is not built.

# USE_DIRECTFB - Build directFB plugin
#USE_DIRECTFB=1

# USE_CURSES - Build Curses plugin
USE_CURSES=1

# USE_OLED - Build OLED screen plugin
USE_OLED=1

# USE_DRMCAIRO - Build DRMCairo plugin
USE_DRMCAIRO=1
# include fallback to stock framebuffer
DRMC_WITH_FB=1

# DEBUG - Add debuging messages
#DEBUG=1

# MCHECK - check memory concistency (see glibc's mcheck())
#MCHECK=1

# end of customisation area

# build configuration
# ----------------

# where to install plugins
# production
PLUGIN_DIR=/usr/local/lib/Selene
# for development
#PLUGIN_DIR=$( pwd )

# Lua version
# custom 5.3.4
#LUA_DIR=/home/laurent/Projets/lua-5.3.4/install
#LUA="-isystem $LUA_DIR/include"
#LUALIB="-L$LUA_DIR/lib"
# system Lua

if pkg-config --cflags lua; then
	LUA="\$(shell pkg-config --cflags lua )"
	LUALIB="\$(shell pkg-config --libs lua )"

	echo "Found Lua"
elif pkg-config --cflags lua5.1; then
	LUA="\$(shell pkg-config --cflags lua5.1 )"
	LUALIB="\$(shell pkg-config --libs lua5.1 )"

	echo "Found Lua5.1"
else
	echo "Lua not found"
	exit 1
fi

CFLAGS="-Wall -fPIC"
RDIR=$( pwd )

# Let's go

if [ ${USE_DIRECTFB+x} ]; then
	USE_DIRECTFB="-DUSE_DIRECTFB \$(shell directfb-config --cflags )"
	USE_DIRECTFB_LIB="\$(shell directfb-config --libs )"
	echo "DirectFB used"
else
	echo "DirectFB not used"
fi

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
else
	echo "Curse not used."
fi

if [ ${USE_OLED+x} ]; then
	USE_OLED="-DUSE_OLED"
	USE_OLED_LIB="-lArduiPi_OLED"
	echo "OLED used"
else
	echo "OLED not used"
fi

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

else
	echo "DRMCairo not used"
fi

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

cd src

cd SeleneLibrary
echo
echo "Selene Library"
echo "--------------"
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA" *.c -so=../../libSelene.so > Makefile

cd ../SelPlugins/DRMCairo/
echo
echo "DRMCairo plugin"
echo "---------------"
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_DRMCAIRO" *.c -so=../../../SelDRMCairo.so > Makefile

cd ../OLED/
echo
echo "OLED plugin"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_OLED" *.c -so=../../../SelOLED.so > Makefile

cd ../Curses/
echo
echo "Curses plugin"
echo "-------------"
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_CURSES" *.c -so=../../../SelCurses.so > Makefile

cd ../DirectFB
echo
echo "DirectFB source"
echo "-----------"
echo
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $USE_DIRECTFB" *.c -so=../../../SelDirectFB.so > Makefile

cd ../..
echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $LUALIB \
	$USE_DRMCAIRO $USE_DRMCAIRO_LIB \
	$USE_DIRECTFB $USE_DIRECTFB_LIB \
	$USE_CURSES $USE_CURSES_LIB \
	$USE_OLED $USE_OLED_LIB \
	$MCHECK_LIB \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR \
	-L$RDIR -lSelene -lpaho-mqtt3c $LUA -lm -ldl -Wl,--export-dynamic -lpthread" \
	*.c -t=../Selene > Makefile

echo
echo "Don't forget if you want to run it without installing first"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH


