#!/bin/bash

gcc -Wall -DUSE_DIRECTFB -std=c99 -o Selene src/*.c `directfb-config --cflags` `directfb-config --libs` -llua

