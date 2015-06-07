# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (May 30 2015 17:53:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpaho-mqtt3c

src/MQTT.o : src/MQTT.c 
	$(cc) -c -o src/MQTT.o src/MQTT.c 

src/dfb_image.o : src/dfb_image.c 
	$(cc) -c -o src/dfb_image.o src/dfb_image.c 

src/dfb_screen.o : src/dfb_screen.c 
	$(cc) -c -o src/dfb_screen.o src/dfb_screen.c 

src/dfb_surface.o : src/dfb_surface.c 
	$(cc) -c -o src/dfb_surface.o src/dfb_surface.c 

src/directfb.o : src/directfb.c 
	$(cc) -c -o src/directfb.o src/directfb.c 

src/selene.o : src/selene.c 
	$(cc) -c -o src/selene.o src/selene.c 

src/sharedobj.o : src/sharedobj.c 
	$(cc) -c -o src/sharedobj.o src/sharedobj.c 

Selene : src/sharedobj.o src/selene.o src/directfb.o src/dfb_surface.o \
  src/dfb_screen.o src/dfb_image.o src/MQTT.o 
	 $(cc) -o Selene src/sharedobj.o src/selene.o src/directfb.o \
  src/dfb_surface.o src/dfb_screen.o src/dfb_image.o src/MQTT.o 

all: Selene 
