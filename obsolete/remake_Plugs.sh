#!/bin/bash

# this version compile against OS' Lua

# USE_DIRECTFB - Build directFB plugin
# USE_CURSES - Build Curses plugin
# USE_OLED - Build OLED screen plugin
# DEBUG - Add debuging messages

if which ncursesw6-config; then
        echo "Found ncurses v6"
        NCURSES=ncursesw6-config
elif which ncursesw5-config; then
        echo "Found ncurses v5"
        NCURSES=ncursesw5-config
else
        echo "No ncurses found"
        exit 20
fi

#PLUGIN_DIR=/usr/local/lib/Selene
PLUGIN_DIR=$( pwd )

cd src

cd SeleneLibrary
echo
echo "Selene Library"
echo "--------------"
LFMakeMaker -v +f=Makefile --opts="\`pkg-config --cflags lua\` -Wall -fPIC" *.c -so=../../libSelene.so > Makefile

cd ../SelPlugins/OLED/
echo
echo "OLED plugin"
echo "-----------"
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -DUSE_OLED -fPIC -std=c99 " *.c -so=../../../SelOLED.so > Makefile

cd ../Curses/
echo
echo "Curses plugin"
echo "-------------"
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -DUSE_CURSES `$NCURSES --cflags` -fPIC -std=c99 " *.c -so=../../../SelCurses.so > Makefile

cd ../DirectFB
echo
echo "DirectFB source"
echo "-----------"
echo
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -DUSE_DIRECTFB `directfb-config --cflags` -fPIC -std=c99 " *.c -so=../../../SelDirectFB.so > Makefile

cd ../..
echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts=" -Wall -DxDEBUG \
	\`pkg-config --cflags lua\` \`pkg-config --libs lua\` \
	-DUSE_OLED -lArduiPi_OLED \
	-DUSE_CURSES `$NCURSES --cflags` `$NCURSES --libs` \
	-DUSE_DIRECTFB `directfb-config --cflags` `directfb-config --libs` \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L../ -L$PLUGIN_DIR -lSelene \
	-lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread" \
	*.c -t=../Selene > Makefile


echo
echo "Don't forget if you wan to run it without installing first"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

