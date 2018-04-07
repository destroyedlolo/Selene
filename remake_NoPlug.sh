#!/bin/bash

# USE_MQTT - use Paho's MQTT layer
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
LFMakeMaker -v +f=Makefile --opts='-Wall -fPIC -DUSE_MQTT' *.c -so=../../libSelene.so > Makefile

cd ..

echo
echo "Main source"
echo "-----------"
LFMakeMaker -v +f=Makefile --opts="-Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=$PLUGIN_DIR -L$PLUGIN_DIR -lSelene -DxDEBUG \`pkg-config --cflags lua\` \`pkg-config --libs lua\` -lpaho-mqtt3c -ldl -Wl,--export-dynamic -lpthread" *.c -t=../Selene > Makefile

