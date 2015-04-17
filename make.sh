#!/bin/bash

gcc -s -O2 -Wall -DUSE_DIRECTFB -std=c99 -o Selene src/*.c `directfb-config --cflags` `directfb-config --libs` -llua

