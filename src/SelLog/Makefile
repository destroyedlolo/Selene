# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../libSelene -Wall -O2 -fPIC -Wno-unused-result -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --cflags lua5.3 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"' -L/home/laurent/Projets/Selene.v7

SelLog.o : SelLog.c Makefile 
	$(cc) -c -o SelLog.o SelLog.c $(opts) 

../../SelLog.so : SelLog.o Makefile 
	 $(cc) -shared -o ../../SelLog.so SelLog.o $(opts) 

all: ../../SelLog.so 
