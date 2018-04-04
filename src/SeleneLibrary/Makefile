# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Apr  4 2018 10:07:32) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I/home/laurent/Projets/lua-5.3.4/install/include -Wall -fPIC

LuaSupportFunc.o : LuaSupportFunc.c libSelene.h Makefile 
	$(cc) -c -o LuaSupportFunc.o LuaSupportFunc.c $(opts) 

Selene.o : Selene.c libSelene.h Makefile 
	$(cc) -c -o Selene.o Selene.c $(opts) 

misc.o : misc.c libSelene.h Makefile 
	$(cc) -c -o misc.o misc.c $(opts) 

../../libSelene.so : misc.o Selene.o LuaSupportFunc.o Makefile 
	 $(cc) -shared -o ../../libSelene.so misc.o Selene.o \
  LuaSupportFunc.o $(opts) 

all: ../../libSelene.so 
