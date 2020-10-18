# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Oct 18 2020 17:58:02) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC   -I/usr/include/lua5.1 -DUSE_CURSES $( ncursesw6-config --cflags )

csr_window.o : csr_window.c curses.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o csr_window.o csr_window.c $(opts) 

curses.o : curses.c curses.h ../../SeleneLibrary/libSelene.h Makefile 
	$(cc) -c -o curses.o curses.c $(opts) 

../../../SelCurses.so : curses.o csr_window.o Makefile 
	 $(cc) -shared -o ../../../SelCurses.so curses.o csr_window.o \
  $(opts) 

all: ../../../SelCurses.so 
