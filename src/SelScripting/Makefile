# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../include -Wall -O2 -fPIC -Wno-unused-result -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --cflags lua5.4 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene

SelScripting.o : SelScripting.c Makefile 
	$(cc) -c -o SelScripting.o SelScripting.c $(opts) 

../../SelScripting.so : SelScripting.o Makefile 
	 $(cc) -shared -o ../../SelScripting.so SelScripting.o $(opts) \
  

all: ../../SelScripting.so 
