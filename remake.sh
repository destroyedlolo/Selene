#!/bin/bash

# USE_DIRECTFB - Build directFB plugin
# USE_MQTT - use Paho's MQTT layer
# DEBUG - Add debuging messages

cd src

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DxDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl' *.c -t=../Selene > Makefile

cd SelPlugins/DirectFB

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_DIRECTFB -fPIC -std=c99 ' *.c -so=../../SelDirectFB.so > Makefile

cd ../../..

