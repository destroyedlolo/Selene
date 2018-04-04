#!/bin/bash

# this version compile against Lua 5.3.4

# USE_MQTT - use Paho's MQTT layer
# DEBUG - Add debuging messages

# PLUGIN_DIR=/usr/local/lib/Selene
PLUGIN_DIR=$( pwd )

LUA_DIR=/home/laurent/Projets/lua-5.3.4/install

cd src

cd SeleneLibrary
echo
echo "Selene Library"
echo "--------------"
LFMakeMaker -v +f=Makefile --opts="-I$LUA_DIR/include -Wall -fPIC" *.c -so=../../libSelene.so > Makefile

cd ..

echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts="-I$LUA_DIR/include -L$LUA_DIR/lib -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=$PLUGIN_DIR -L$PLUGIN_DIR -lSelene -DxDEBUG -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread" *.c -t=../Selene > Makefile

echo
echo
echo "Don't forget"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

