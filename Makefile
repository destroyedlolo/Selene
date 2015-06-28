# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (May 30 2015 17:53:45) (c)LFSoft 1997

gotoall: all

# Warning : 'assert.h' can't be located for this node.

#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpaho-mqtt3c

src/dfb_image.o : src/dfb_image.c src/directfb.h 
	$(cc) -c -o src/dfb_image.o src/dfb_image.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_screen.o : src/dfb_screen.c src/directfb.h 
	$(cc) -c -o src/dfb_screen.o src/dfb_screen.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_surface.o : src/dfb_surface.c src/directfb.h 
	$(cc) -c -o src/dfb_surface.o src/dfb_surface.c 

# Warning : 'assert.h' can't be located for this node.
src/directfb.o : src/directfb.c src/directfb.h 
	$(cc) -c -o src/directfb.o src/directfb.c 

# Warning : 'assert.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
src/MQTT.o : src/MQTT.c src/sharedobj.h src/MQTT.h 
	$(cc) -c -o src/MQTT.o src/MQTT.c 

# Warning : 'stdio.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
src/selene.o : src/selene.c src/MQTT.h src/directfb.h src/sharedobj.h \
  src/selene.h 
	$(cc) -c -o src/selene.o src/selene.c 

# Warning : 'string.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'pthread.h' can't be located for this node.
# Warning : 'stdint.h' can't be located for this node.
# Warning : 'sys/eventfd.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
src/sharedobj.o : src/sharedobj.c src/sharedobj.h 
	$(cc) -c -o src/sharedobj.o src/sharedobj.c 

# Warning : 'assert.h' can't be located for this node.
src/timer.o : src/timer.c src/selene.h 
	$(cc) -c -o src/timer.o src/timer.c 

Selene : src/timer.o src/sharedobj.o src/selene.o src/MQTT.o \
  src/directfb.o src/dfb_surface.o src/dfb_screen.o src/dfb_image.o 
	 $(cc) -o Selene src/timer.o src/sharedobj.o src/selene.o \
  src/MQTT.o src/directfb.o src/dfb_surface.o src/dfb_screen.o \
  src/dfb_image.o 

all: Selene 
