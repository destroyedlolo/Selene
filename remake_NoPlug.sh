#!/bin/bash

# USE_DIRECTFB - Build directFB plugin
# USE_MQTT - use Paho's MQTT layer
# DEBUG - Add debuging messages
# PLUGIN_DIR - where plugins .so can be found (default is /usr/local/lib/Selene)

cd src

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -pthread -DUSE_MQTT -DxPLUGIN_DIR=\".\" -DxDEBUG -std=c99 `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic' *.c -t=../Selene > Makefile

cd SelPlugins/DirectFB

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -pthread -fPIC -std=c99 ' *.c -so=../../../SelDirectFB.so > Makefile

cd ../../..
