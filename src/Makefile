# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts= -Wall `pkg-config --cflags lua` `pkg-config --libs lua` -DUSE_CURSES -D_GNU_SOURCE -D_DEFAULT_SOURCE -I/usr/include/ncursesw -lncursesw -DUSE_DIRECTFB -I/usr/include/directfb -D_REENTRANT -L/usr/lib -ldirectfb -lfusion -ldirect -lpthread -DUSE_MQTT -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v4"' -L/home/laurent/Projets/Selene.v4 -lSelene -DxDEBUG -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
