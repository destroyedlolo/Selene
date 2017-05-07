#!/bin/bash

# USE_DIRECTFB - Build directFB plugin
# USE_CURSES - Build Curses plugin
# USE_MQTT - use Paho's MQTT layer
# DEBUG - Add debuging messages
# PLUGIN_DIR - where plugins .so can be found (default is /usr/local/lib/Selene)

cd src

echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=\"/usr/local/lib/Selene\" -DxDEBUG -std=c99 `'$NCURSES' --cflags` `'$NCURSES' --libs` `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic -lpthread' *.c -t=../Selene > Makefile

cd SelPlugins/DirectFB

echo
echo "DirectFB source"
echo "-----------"
LFMakeMaker -v +f=Makefile -cc='gcc -Wall -fPIC -std=c99 ' *.c -so=../../../SelDirectFB.so > Makefile

cd ../Curses

echo
echo "Curses source"
echo "-----------"
LFMakeMaker -v +f=Makefile -cc='gcc -Wall --cflags` -fPIC -std=c99 ' *.c -so=../../../SelCurses.so > Makefile

cd ../../..

