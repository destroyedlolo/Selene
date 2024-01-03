# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -Wno-unused-result   $(shell pkg-config --cflags lua5.3 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"'

libSelene.o : libSelene.c libSelene.h Makefile 
	$(cc) -c -o libSelene.o libSelene.c $(opts) 

../../libSelene.so : libSelene.o Makefile 
	 $(cc) -shared -o ../../libSelene.so libSelene.o $(opts) 

all: ../../libSelene.so 
