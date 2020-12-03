# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC   $( pkg-config --cflags lua ) $( pkg-config --libs lua ) 	-DDRMC_WITH_FB -DUSE_DRMCAIRO $(shell pkg-config --cflags libdrm libkms cairo ) $(shell pkg-config --libs libdrm libkms cairo ) 	  	-DUSE_CURSES $(shell ncursesw6-config --cflags ) $(shell ncursesw6-config --libs ) 	-DUSE_OLED -lArduiPi_OLED 	 	-DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene 	-L/home/laurent/Projets/Selene -lSelene -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
