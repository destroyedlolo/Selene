# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../libSelene -Wall -O2 -fPIC -Wno-unused-result   $(shell pkg-config --cflags lua5.3 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"'

SeleneCore.o : SeleneCore.c SeleneCore.h Makefile 
	$(cc) -c -o SeleneCore.o SeleneCore.c $(opts) 

../../SeleneCore.so : SeleneCore.o Makefile 
	 $(cc) -shared -o ../../SeleneCore.so SeleneCore.o $(opts) 

all: ../../SeleneCore.so 
