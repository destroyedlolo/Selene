# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --cflags lua5.3 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"' -L/home/laurent/Projets/Selene.v7

SelScripting.o : SelScripting.c Makefile 
	$(cc) -c -o SelScripting.o SelScripting.c $(opts) 

../../SelScripting.so : SelScripting.o Makefile 
	 $(cc) -shared -o ../../SelScripting.so SelScripting.o $(opts) \
  

all: ../../SelScripting.so 
