# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --cflags lua5.3 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"' -L/home/laurent/Projets/Selene.v7

SelFIFO.o : SelFIFO.c Makefile 
	$(cc) -c -o SelFIFO.o SelFIFO.c $(opts) 

../../SelFIFO.so : SelFIFO.o Makefile 
	 $(cc) -shared -o ../../SelFIFO.so SelFIFO.o $(opts) 

all: ../../SelFIFO.so 
