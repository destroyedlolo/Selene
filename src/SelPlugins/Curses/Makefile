# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG  $(shell pkg-config --cflags lua5.3 ) -DLUA -DUSE_CURSES $(shell ncursesw6-config --cflags ) $(shell ncursesw6-config --libs )

SelCurses.o : SelCurses.c Makefile 
	$(cc) -c -o SelCurses.o SelCurses.c $(opts) 

../../../lib/Selene/SelCurses.so : SelCurses.o Makefile 
	 $(cc) -shared -o ../../../lib/Selene/SelCurses.so SelCurses.o \
  $(opts) 

all: ../../../lib/Selene/SelCurses.so 
