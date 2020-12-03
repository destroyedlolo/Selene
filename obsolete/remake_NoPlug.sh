#!/bin/bash

# DEBUG - Add debuging messages

# PLUGIN_DIR=/usr/local/lib/Selene
PLUGIN_DIR=$( pwd )

echo "Don't forget"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

cd src

cd SeleneLibrary
echo
echo "Selene Library"
echo "--------------"
LFMakeMaker -v +f=Makefile --opts='-Wall -fPIC' *.c -so=../../libSelene.so > Makefile

cd ../SelPlugins/OLED/
echo
echo "OLED plugin"
echo "-----------"
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -fPIC -std=c99 " *.c -so=../../../SelOLED.so > Makefile

cd ../Curses/
echo
echo "Curses plugin"
echo "-------------"
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -fPIC -std=c99 " *.c -so=../../../SelCurses.so > Makefile

cd ../DirectFB
echo
echo "DirectFB source"
echo "-----------"
echo
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -fPIC -std=c99 " *.c -so=../../../SelDirectFB.so > Makefile


cd ../..
echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts="-isystem -Wall -DxDEBUG \
	-I$LUA_DIR/include -L$LUA_DIR/lib \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR -lSelene \
	-lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread" \
	*.c -t=../Selene > Makefile

