# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (May 30 2015 17:53:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua

src/dfb_image.o : src/dfb_image.c 
	$(cc) -c -o src/dfb_image.o src/dfb_image.c 

src/dfb_screen.o : src/dfb_screen.c 
	$(cc) -c -o src/dfb_screen.o src/dfb_screen.c 

src/dfb_surface.o : src/dfb_surface.c 
	$(cc) -c -o src/dfb_surface.o src/dfb_surface.c 

src/directfb.o : src/directfb.c 
	$(cc) -c -o src/directfb.o src/directfb.c 

src/selene.o : src/selene.c 
	$(cc) -c -o src/selene.o src/selene.c 

Selene : src/selene.o src/directfb.o src/dfb_surface.o \
  src/dfb_screen.o src/dfb_image.o 
	 $(cc) -o Selene src/selene.o src/directfb.o src/dfb_surface.o \
  src/dfb_screen.o src/dfb_image.o 

all: Selene 
