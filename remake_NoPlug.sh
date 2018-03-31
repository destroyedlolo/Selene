#!/bin/bash

# USE_MQTT - use Paho's MQTT layer
# DEBUG - Add debuging messages


cd src

echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile -cc='gcc -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=\"/usr/local/lib/Selene\" -DxDEBUG `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic -lpthread' *.c -t=../Selene > Makefile

