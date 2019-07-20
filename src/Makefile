# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC  `pkg-config --cflags lua` `pkg-config --libs lua` 	-DUSE_DIRECTFB `directfb-config --cflags` `directfb-config --libs` 	-DUSE_CURSES `ncursesw6-config --cflags` `ncursesw6-config --libs` 	-DUSE_OLED -lArduiPi_OLED 	-DPLUGIN_DIR='"/usr/local/lib/Selene"' -L/usr/local/lib/Selene 	-L/home/laurent/Projets/Selene.v5 -lSelene -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
