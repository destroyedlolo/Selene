# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (Apr 13 2016 21:34:06) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_CURSES `ncursesw5-config --cflags` -fPIC -std=c99 

curses.o : curses.c curses.h ../../selene.h Makefile 
	$(cc) -c -o curses.o curses.c 

../../../SelCurses.so : curses.o Makefile 
	 $(cc) -shared -o ../../../SelCurses.so curses.o 

all: ../../../SelCurses.so 
