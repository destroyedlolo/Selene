#!/bin/bash
#
# this script is used to 
# - enable / disable the build of particular modules
# - create and update Makefiles

# set following variables depending on which module you want to build.
# if unset, the module is not built.

# Enable Lua
# This setting impacts only shared object. Lua is mandatory for Selene itself
BUILD_LUA=1

# Build Curses plugin
BUILD_CURSES=1

# Build OLED screen plugin
BUILD_OLED=1

# Build DRMCairo plugin
BUILD_DRMCAIRO=1
# include fall-back to stock frame buffer
DRMC_WITH_FB=1

# Build directFB plugin
#BUILD_DIRECTFB=1

# DEBUG - Add debuging messages
#DEBUG=1

# MCHECK - check memory concistency (see glibc's mcheck())
#MCHECK=1

# end of customisation area

# build configuration
# ----------------

# where to install plugins
# production
#PLUGIN_DIR=/usr/local/lib/Selene
# for development
PLUGIN_DIR=$( pwd )

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
echo -e "\trm *.so" >> Makefile
echo -e "\trm src/SeleneLibrary/*.so" >> Makefile
echo -e "\trm src/SelPlugins/*/*.so" >> Makefile

echo >> Makefile
echo "# Build everything" >> Makefile
echo "all:" >> Makefile

# =============================
# Configure external components
# =============================

echo -e "\nSet build options\n=================\n"

CFLAGS="-Wall -O2 -fPIC -Wno-unused-result"
RDIR=$( pwd )

# Lua version
if true; then	# OS Version
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

if [ ${BUILD_CURSES+x} ]; then
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
	BUILD_CURSES="-DBUILD_CURSES \$(shell $NCURSES --cflags )"
	BUILD_CURSES_LIB="\$(shell $NCURSES --libs )"

	cd src/SelPlugins/Curses/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $BUILD_CURSES" *.c -so=../../../SelCurses.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/Curses' >> Makefile
else
	echo "Curse not used."
fi


echo
echo "OLED plug-in"
echo "------------"

if [ ${BUILD_OLED+x} ]; then
	echo "OLED used"
	BUILD_OLED="-DBUILD_OLED"
	BUILD_OLED_LIB="-lArduiPi_OLED"

	cd src/SelPlugins/OLED/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $BUILD_OLED" *.c -so=../../../SelOLED.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/OLED' >> Makefile
else
	echo "OLED not used"
fi


echo
echo "DRMCairo plugin"
echo "---------------"

if [ ${BUILD_DRMCAIRO+x} ]; then
	BUILD_DRMCAIRO="-DBUILD_DRMCAIRO \$(shell pkg-config --cflags libdrm cairo freetype2 )"
	BUILD_DRMCAIRO_LIB="\$(shell pkg-config --libs libdrm cairo freetype2 )"
	echo "DRMCairo used"

	if pkg-config --cflags libkms; then
		BUILD_DRMCAIRO="$BUILD_DRMCAIRO \$(shell pkg-config --cflags libkms )"
		BUILD_DRMCAIRO_LIB="$BUILD_DRMCAIRO_LIB \$(shell pkg-config --libs libkms )"
	else
		BUILD_DRMCAIRO="$BUILD_DRMCAIRO -DKMS_MISSING"
		echo "WARNING : Kms is missing"
	fi

	if [ ${DRMC_WITH_FB+x} ]; then
		BUILD_DRMCAIRO="-DDRMC_WITH_FB $BUILD_DRMCAIRO"
		echo "with Framebuffer fallback"
	fi

	cd src/SelPlugins/DRMCairo/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $BUILD_DRMCAIRO" *.c -so=../../../SelDRMCairo.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/DRMCairo' >> Makefile
else
	echo "DRMCairo not used"
fi

echo
echo "DirectFB source"
echo "-----------"

if [ ${BUILD_DIRECTFB+x} ]; then
	BUILD_DIRECTFB="-DBUILD_DIRECTFB \$(shell directfb-config --cflags )"
	BUILD_DIRECTFB_LIB="\$(shell directfb-config --libs )"
	echo "DirectFB used"

	cd src/SelPlugins/DirectFB/
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA $BUILD_DIRECTFB" *.c -so=../../../SelDirectFB.so > Makefile
	cd ../../..

	echo -e '\t$(MAKE) -C src/SelPlugins/DirectFB' >> Makefile
else
	echo "DirectFB not used"
fi


echo
echo "Selene Library"
echo "=============="
echo

cd src/SeleneLibrary
LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUA" *.c -so=../../libSelene.so > Makefile

cd ../..
echo -e '\t$(MAKE) -C src/SeleneLibrary' >> Makefile

echo
echo "Main source"
echo "==========="
echo

cd src

LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK \
	$LUA $LUALIB \
	$BUILD_DRMCAIRO $BUILD_DRMCAIRO_LIB \
	$BUILD_DIRECTFB $BUILD_DIRECTFB_LIB \
	$BUILD_CURSES $BUILD_CURSES_LIB \
	$BUILD_OLED $BUILD_OLED_LIB \
	$MCHECK_LIB \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR \
	-L$RDIR -lSelene -lpaho-mqtt3c $LUA -lm -ldl -Wl,--export-dynamic -lpthread" \
	*.c -t=../Selene > Makefile

cd ..
echo -e '\t$(MAKE) -C src/' >> Makefile

#echo
#echo "Don't forget if you want to run it without installing first"
#echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH


