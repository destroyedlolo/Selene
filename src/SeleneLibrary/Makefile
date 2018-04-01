# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (May  8 2017 00:11:26) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -fPIC

misc.o : misc.c libSelene.h Makefile 
	$(cc) -c -o misc.o misc.c 

../../libSelene.so : misc.o Makefile 
	 $(cc) -shared -o ../../libSelene.so misc.o 

all: ../../libSelene.so 
