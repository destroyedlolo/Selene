# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (Apr 13 2016 21:34:06) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -fPIC -std=c99 

dfb_font.o : dfb_font.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o dfb_font.o dfb_font.c 

dfb_image.o : dfb_image.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o dfb_image.o dfb_image.c 

dfb_layer.o : dfb_layer.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o dfb_layer.o dfb_layer.c 

dfb_screen.o : dfb_screen.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o dfb_screen.o dfb_screen.c 

dfb_surface.o : dfb_surface.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o dfb_surface.o dfb_surface.c 

dfb_window.o : dfb_window.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o dfb_window.o dfb_window.c 

directfb.o : directfb.c directfb.h ../../selene.h Makefile 
	$(cc) -c -o directfb.o directfb.c 

../../SelDirectFB.so : directfb.o dfb_window.o dfb_surface.o \
  dfb_screen.o dfb_layer.o dfb_image.o dfb_font.o Makefile 
	 $(cc) -shared -o ../../SelDirectFB.so directfb.o dfb_window.o \
  dfb_surface.o dfb_screen.o dfb_layer.o dfb_image.o dfb_font.o 

all: ../../SelDirectFB.so 
