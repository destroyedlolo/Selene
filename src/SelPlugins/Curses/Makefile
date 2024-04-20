# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG  $(shell pkg-config --cflags lua5.4 ) -DLUA -DUSE_CURSES $(shell ncursesw6-config --cflags ) $(shell ncursesw6-config --libs )

SelCurses.o : SelCurses.c Makefile 
	$(cc) -c -o SelCurses.o SelCurses.c $(opts) 

SelCWindow.o : SelCWindow.c Makefile 
	$(cc) -c -o SelCWindow.o SelCWindow.c $(opts) 

../../../lib/Selene/SelCurses.so : SelCWindow.o SelCurses.o Makefile 
	 $(cc) -shared -o ../../../lib/Selene/SelCurses.so \
  SelCWindow.o SelCurses.o $(opts) 

all: ../../../lib/Selene/SelCurses.so 
