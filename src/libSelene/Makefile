# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -Wno-unused-result -DDEBUG  -DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene

libSelene.o : libSelene.c Makefile 
	$(cc) -c -o libSelene.o libSelene.c $(opts) 

../../libSelene.so : libSelene.o Makefile 
	 $(cc) -shared -o ../../libSelene.so libSelene.o $(opts) 

all: ../../libSelene.so 
