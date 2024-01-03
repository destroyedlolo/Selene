# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -Wno-unused-result   $(shell pkg-config --cflags lua5.3 ) -DLUA -DBUILD_OLED

SelOLED.o : SelOLED.c SelOLED.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o SelOLED.o SelOLED.c $(opts) 

../../../SelOLED.so : SelOLED.o Makefile 
	 $(cc) -shared -o ../../../SelOLED.so SelOLED.o $(opts) 

all: ../../../SelOLED.so 
