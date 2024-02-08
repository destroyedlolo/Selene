# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --cflags lua5.4 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene

SelMQTT.o : SelMQTT.c Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c $(opts) 

../../SelMQTT.so : SelMQTT.o Makefile 
	 $(cc) -shared -o ../../SelMQTT.so SelMQTT.o $(opts) 

all: ../../SelMQTT.so 
