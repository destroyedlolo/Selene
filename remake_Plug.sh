#!/bin/bash

# USE_DIRECTFB - Build directFB plugin
# USE_CURSES - Build Curses plugin
# USE_MQTT - use Paho's MQTT layer
# DEBUG - Add debuging messages
# PLUGIN_DIR - where plugins .so can be found (default is /usr/local/lib/Selene)

cd src

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_DIRECTFB -DUSE_CURSES -DUSE_MQTT -DxPLUGIN_DIR=\".\" -DxDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` `ncursesw5-config --cflags` `ncursesw5-config --libs` `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic' *.c -t=../Selene > Makefile

cd SelPlugins/DirectFB

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_DIRECTFB `directfb-config --cflags` -fPIC -std=c99 ' *.c -so=../../../SelDirectFB.so > Makefile

cd ../Curses

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_CURSES `ncursesw5-config --cflags` -fPIC -std=c99 ' *.c -so=../../../SelCurses.so > Makefile

cd ../../..

