# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Oct 18 2020 17:58:02) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC   -I/usr/include/lua5.1 -DUSE_OLED

SelOLED.o : SelOLED.c SelOLED.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o SelOLED.o SelOLED.c $(opts) 

../../../SelOLED.so : SelOLED.o Makefile 
	 $(cc) -shared -o ../../../SelOLED.so SelOLED.o $(opts) 

all: ../../../SelOLED.so 
