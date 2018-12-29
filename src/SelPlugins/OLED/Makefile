# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall `pkg-config --cflags lua` -DUSE_OLED -fPIC -std=c99 
opts=

SelOLED.o : SelOLED.c SelOLED.h ../../SeleneLibrary/libSelene.h \
  Makefile 
	$(cc) -c -o SelOLED.o SelOLED.c $(opts) 

../../../SelOLED.so : SelOLED.o Makefile 
	 $(cc) -shared -o ../../../SelOLED.so SelOLED.o $(opts) 

all: ../../../SelOLED.so 
