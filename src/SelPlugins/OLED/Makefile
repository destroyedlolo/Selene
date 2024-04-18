# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG  $(shell pkg-config --cflags lua ) -DLUA -DUSE_OLED -lArduiPi_OLED

SelOLED.o : SelOLED.c Makefile 
	$(cc) -c -o SelOLED.o SelOLED.c $(opts) 

../../../lib/Selene/SelOLED.so : SelOLED.o Makefile 
	 $(cc) -shared -o ../../../lib/Selene/SelOLED.so SelOLED.o \
  $(opts) 

all: ../../../lib/Selene/SelOLED.so 
