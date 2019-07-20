# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC  `pkg-config --cflags lua` -DUSE_DIRECTFB `directfb-config --cflags`

dfb_font.o : dfb_font.c directfb.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o dfb_font.o dfb_font.c $(opts) 

dfb_image.o : dfb_image.c directfb.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o dfb_image.o dfb_image.c $(opts) 

dfb_layer.o : dfb_layer.c directfb.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o dfb_layer.o dfb_layer.c $(opts) 

dfb_screen.o : dfb_screen.c directfb.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o dfb_screen.o dfb_screen.c $(opts) 

dfb_surface.o : dfb_surface.c directfb.h \
  ../../SeleneLibrary/libSelene.h Makefile 
	$(cc) -c -o dfb_surface.o dfb_surface.c $(opts) 

dfb_window.o : dfb_window.c directfb.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o dfb_window.o dfb_window.c $(opts) 

directfb.o : directfb.c directfb.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o directfb.o directfb.c $(opts) 

../../../SelDirectFB.so : directfb.o dfb_window.o dfb_surface.o \
  dfb_screen.o dfb_layer.o dfb_image.o dfb_font.o Makefile 
	 $(cc) -shared -o ../../../SelDirectFB.so directfb.o \
  dfb_window.o dfb_surface.o dfb_screen.o dfb_layer.o dfb_image.o \
  dfb_font.o $(opts) 

all: ../../../SelDirectFB.so 
