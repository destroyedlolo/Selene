# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (May  8 2017 00:11:26) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=/home/laurent/Projets/Selene -L/home/laurent/Projets/Selene -lSelene -DxDEBUG `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c Makefile 
	$(cc) -c -o selene.o selene.c 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o 

all: ../Selene 
