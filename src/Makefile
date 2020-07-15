# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC  $( pkg-config --cflags lua ) $( pkg-config --libs lua ) 	-DDRMC_WITH_FB -DUSE_DRMCAIRO -I/usr/include/libkms -I/usr/include/libdrm -I/usr/include/cairo -I/usr/lib/libffi/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -ldrm -lkms -lcairo 	  	-DUSE_CURSES $( ncursesw6-config --cflags ) $( ncursesw6-config --libs ) 	-DUSE_OLED -lArduiPi_OLED 	-DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene 	-L/home/laurent/Projets/Selene -lSelene -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
