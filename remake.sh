#!/bin/bash

LFMakeMaker -v -g -cc='gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpthread -lpaho-mqtt3c' src/*.c -t=Selene > Makefile


