# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (Nov 18 2023 14:49:11) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../libSelene -Wall -O2 -fPIC -Wno-unused-result -DDEBUG  $(shell pkg-config --cflags lua5.4 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene"' -L/home/laurent/Projets/Selene

SelLog.o : SelLog.c Makefile 
	$(cc) -c -o SelLog.o SelLog.c $(opts) 

../../SelLog.so : SelLog.o Makefile 
	 $(cc) -shared -o ../../SelLog.so SelLog.o $(opts) 

all: ../../SelLog.so 
