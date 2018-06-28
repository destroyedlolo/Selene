#!/bin/bash

# this version compile against OS' Lua

# USE_DIRECTFB - Build directFB plugin
# USE_MQTT - use Paho's MQTT layer
# USE_CURSES - Build Curses plugin
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
LFMakeMaker -v +f=Makefile --opts="\`pkg-config --cflags lua\` -Wall -fPIC -DUSE_MQTT" *.c -so=../../libSelene.so > Makefile

cd ../SelPlugins/Curses/
echo
echo "Curses plugin"
echo "-------------"
LFMakeMaker -v +f=Makefile -cc="gcc -Wall \`pkg-config --cflags lua\` -DUSE_CURSES `$NCURSES --cflags` -fPIC -std=c99 " *.c -so=../../../SelCurses.so > Makefile

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
LFMakeMaker -v +f=Makefile --opts=" -Wall \`pkg-config --cflags lua\` \`pkg-config --libs lua\` -DUSE_CURSES `$NCURSES --cflags` `$NCURSES --libs` -DUSE_MQTT -DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L../ -L$PLUGIN_DIR -lSelene -DxDEBUG -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread" *.c -t=../Selene > Makefile


echo
echo "Don't forget if you wan to run it without installing first"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

