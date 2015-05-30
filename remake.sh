#!/bin/bash

LFMakeMaker -v -g -cc='gcc -Wall -DUSE_DIRECTFB -DDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua' src/*.c -t=Selene > Makefile


