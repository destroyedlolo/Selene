# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -Wno-unused-result -DDEBUG  	$(shell pkg-config --cflags lua5.4 ) -DLUA $(shell pkg-config --libs lua5.4 ) 	 	 	 	 	 	-DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene 	-lSelene -lpaho-mqtt3c $(shell pkg-config --cflags lua5.4 ) -DLUA -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
