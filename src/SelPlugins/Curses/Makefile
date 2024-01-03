# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -Wno-unused-result   $(shell pkg-config --cflags lua5.3 ) -DLUA -DUSE_CURSES $(shell ncursesw6-config --cflags ) $(shell ncursesw6-config --libs )

csr_window.o : csr_window.c curses.h ../../SeleneLibrary/libSelene.h \
  ../../SeleneLibrary/Module.h Makefile 
	$(cc) -c -o csr_window.o csr_window.c $(opts) 

curses.o : curses.c curses.h ../../SeleneLibrary/libSelene.h \
  ../../SeleneLibrary/Module.h Makefile 
	$(cc) -c -o curses.o curses.c $(opts) 

../../../SelCurses.so : curses.o csr_window.o Makefile 
	 $(cc) -shared -o ../../../SelCurses.so curses.o csr_window.o \
  $(opts) 

all: ../../../SelCurses.so 
