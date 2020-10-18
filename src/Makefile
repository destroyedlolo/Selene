# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Oct 18 2020 17:58:02) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC   -I/usr/include/lua5.1 -llua5.1 	-DDRMC_WITH_FB -DUSE_DRMCAIRO -I/usr/include/libdrm -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -DKMS_MISSING -ldrm -lcairo -lfreetype 	  	-DUSE_CURSES $( ncursesw6-config --cflags ) $( ncursesw6-config --libs ) 	-DUSE_OLED -lArduiPi_OLED 	 	-DPLUGIN_DIR='"/usr/local/lib/Selene"' -L/usr/local/lib/Selene 	-L/home/laurent/Projets/Selene -lSelene -lpaho-mqtt3c -I/usr/include/lua5.1 -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
