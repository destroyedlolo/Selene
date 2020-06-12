# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC  $( pkg-config --cflags lua ) -DDRMC_WITH_FB -DUSE_DRMCAIRO -I/usr/include/libkms -I/usr/include/libdrm -I/usr/include/cairo -I/usr/lib/libffi/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16

DRMCairo.o : DRMCairo.c DRMCairo.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o DRMCairo.o DRMCairo.c $(opts) 

SelDCCard.o : SelDCCard.c DRMCairo.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o SelDCCard.o SelDCCard.c $(opts) 

SelDCFont.o : SelDCFont.c DRMCairo.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o SelDCFont.o SelDCFont.c $(opts) 

SelDCPattern.o : SelDCPattern.c DRMCairo.h \
  ../../SeleneLibrary/libSelene.h Makefile 
	$(cc) -c -o SelDCPattern.o SelDCPattern.c $(opts) 

SelDCSurface.o : SelDCSurface.c DRMCairo.h \
  ../../SeleneLibrary/libSelene.h Makefile 
	$(cc) -c -o SelDCSurface.o SelDCSurface.c $(opts) 

SelDCSurfaceImage.o : SelDCSurfaceImage.c DRMCairo.h \
  ../../SeleneLibrary/libSelene.h Makefile 
	$(cc) -c -o SelDCSurfaceImage.o SelDCSurfaceImage.c $(opts) 

../../../SelDRMCairo.so : SelDCSurfaceImage.o SelDCSurface.o \
  SelDCPattern.o SelDCFont.o SelDCCard.o DRMCairo.o Makefile 
	 $(cc) -shared -o ../../../SelDRMCairo.so SelDCSurfaceImage.o \
  SelDCSurface.o SelDCPattern.o SelDCFont.o SelDCCard.o DRMCairo.o \
  $(opts) 

all: ../../../SelDRMCairo.so 
