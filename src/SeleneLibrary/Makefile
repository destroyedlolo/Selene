# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Apr  3 2018 23:14:49) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I/home/laurent/Projets/lua-5.3.4/install/include -Wall -fPIC

LuaSupportFunc.o : LuaSupportFunc.c libSelene.h Makefile 
	$(cc) -c -o LuaSupportFunc.o LuaSupportFunc.c $(opts) 

misc.o : misc.c libSelene.h Makefile 
	$(cc) -c -o misc.o misc.c $(opts) 

SelCollection.o : SelCollection.c libSelene.h Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c $(opts) 

Selene.o : Selene.c libSelene.h Makefile 
	$(cc) -c -o Selene.o Selene.c $(opts) 

SelLog.o : SelLog.c libSelene.h Makefile 
	$(cc) -c -o SelLog.o SelLog.c $(opts) 

SelTimedCollection.o : SelTimedCollection.c libSelene.h Makefile 
	$(cc) -c -o SelTimedCollection.o SelTimedCollection.c $(opts) 

SelTimedWindowCollection.o : SelTimedWindowCollection.c libSelene.h \
  Makefile 
	$(cc) -c -o SelTimedWindowCollection.o \
  SelTimedWindowCollection.c $(opts) 

../../libSelene.so : SelTimedWindowCollection.o SelTimedCollection.o \
  SelLog.o Selene.o SelCollection.o misc.o LuaSupportFunc.o Makefile 
	 $(cc) -shared -o ../../libSelene.so \
  SelTimedWindowCollection.o SelTimedCollection.o SelLog.o Selene.o \
  SelCollection.o misc.o LuaSupportFunc.o $(opts) 

all: ../../libSelene.so 
