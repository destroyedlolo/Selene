# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC  `pkg-config --cflags lua` `pkg-config --libs lua` 	-DUSE_DRMCAIRO `pkg-config --cflags libdrm` `pkg-config --cflags libkms` `pkg-config --cflags cairo` `pkg-config --libs libdrm` `pkg-config --libs libkms` `pkg-config --libs cairo` 	  	-DUSE_CURSES `ncursesw6-config --cflags` `ncursesw6-config --libs` 	-DUSE_OLED -lArduiPi_OLED 	-DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene 	-L/home/laurent/Projets/Selene -lSelene -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
