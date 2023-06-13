# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -Wno-unused-result   $(shell pkg-config --cflags lua5.3 ) -DLUA $(shell pkg-config --libs lua5.3 ) 	  	  	  	  	 	-DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"' -L/home/laurent/Projets/Selene.v7 	-L/home/laurent/Projets/Selene.v7 -lSelene -lpaho-mqtt3c $(shell pkg-config --cflags lua5.3 ) -DLUA -lm -ldl -Wl,--export-dynamic -lpthread

selene.o : selene.c SeleneLibrary/libSelene.h version.h Makefile 
	$(cc) -c -o selene.o selene.c $(opts) 

../Selene : selene.o Makefile 
	 $(cc) -o ../Selene selene.o $(opts) 

all: ../Selene 
