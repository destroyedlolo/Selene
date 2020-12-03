#!/bin/bash

# this version compile against OS' Lua
# Only OLED plugin is compiled

# USE_DIRECTFB - Build directFB plugin
# USE_CURSES - Build Curses plugin
# USE_OLED - Build OLED screen plugin
# DEBUG - Add debuging messages

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
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -fPIC -std=c99 " *.c -so=../../../SelCurses.so > Makefile

cd ../DirectFB
echo
echo "DirectFB source"
echo "-----------"
echo
LFMakeMaker -v +f=Makefile -cc="gcc -Wall -fPIC -std=c99 " *.c -so=../../../SelDirectFB.so > Makefile

cd ../..
echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts=" -Wall -DxDEBUG \
	\`pkg-config --cflags lua\` \`pkg-config --libs lua\` \
	-DUSE_OLED -lArduiPi_OLED \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L../ -L$PLUGIN_DIR -lSelene \
	-lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread" \
	*.c -t=../Selene > Makefile


echo
echo "Don't forget if you wan to run it without installing first"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

