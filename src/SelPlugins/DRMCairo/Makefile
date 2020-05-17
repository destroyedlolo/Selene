# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC  `pkg-config --cflags lua` -DUSE_DRMCAIRO `pkg-config --cflags libdrm` `pkg-config --cflags libkms` `pkg-config --cflags cairo`

DRMCairo.o : DRMCairo.c DRMCairo.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o DRMCairo.o DRMCairo.c $(opts) 

SelDCCard.o : SelDCCard.c DRMCairo.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o SelDCCard.o SelDCCard.c $(opts) 

SelDCSurface.o : SelDCSurface.c DRMCairo.h \
  ../../SeleneLibrary/libSelene.h Makefile 
	$(cc) -c -o SelDCSurface.o SelDCSurface.c $(opts) 

../../../SelDRMCairo.so : SelDCSurface.o SelDCCard.o DRMCairo.o \
  Makefile 
	 $(cc) -shared -o ../../../SelDRMCairo.so SelDCSurface.o \
  SelDCCard.o DRMCairo.o $(opts) 

all: ../../../SelDRMCairo.so 
