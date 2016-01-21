#!/bin/bash

cd src

LFMakeMaker -v -cc='gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DxDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpaho-mqtt3c' *.c -t=../Selene > Makefile

