# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I../libSelene -Wall -O2 -fPIC -Wno-unused-result -DDEBUG  $(shell pkg-config --cflags lua5.3 ) -DLUA -DPLUGIN_DIR='"/home/laurent/Projets/Selene.v7"' -L/home/laurent/Projets/Selene.v7

SelMQTT.o : SelMQTT.c Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c $(opts) 

../../SelMQTT.so : SelMQTT.o Makefile 
	 $(cc) -shared -o ../../SelMQTT.so SelMQTT.o $(opts) 

all: ../../SelMQTT.so 
