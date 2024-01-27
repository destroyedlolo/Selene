# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../libSelene -Wall -O2 -fPIC -Wno-unused-result -DDEBUG  $(shell pkg-config --cflags lua5.4 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene

SeleneCore.o : SeleneCore.c Makefile 
	$(cc) -c -o SeleneCore.o SeleneCore.c $(opts) 

../../SeleneCore.so : SeleneCore.o Makefile 
	 $(cc) -shared -o ../../SeleneCore.so SeleneCore.o $(opts) 

all: ../../SeleneCore.so 
