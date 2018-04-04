# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Apr  3 2018 23:14:49) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I/home/laurent/Projets/lua-5.3.4/install/include -L/home/laurent/Projets/lua-5.3.4/install/lib -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=/home/laurent/Projets/Selene -L/home/laurent/Projets/Selene -lSelene -DxDEBUG -lpaho-mqtt3c -llua -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
