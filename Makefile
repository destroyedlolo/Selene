# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (Sep 28 2015 20:11:32) (c)LFSoft 1997

gotoall: all

# Warning : 'assert.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.

#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpaho-mqtt3c

src/MQTT.o : src/MQTT.c src/sharedobj.h src/MQTT_tools.h src/MQTT.h 
	$(cc) -c -o src/MQTT.o src/MQTT.c 

src/MQTT_tools.o : src/MQTT_tools.c src/MQTT_tools.h 
	$(cc) -c -o src/MQTT_tools.o src/MQTT_tools.c 

# Warning : 'Collection.h' can't be located for this node.
src/SelCollection.o : src/SelCollection.c 
	$(cc) -c -o src/SelCollection.o src/SelCollection.c 

# Warning : 'sys/timerfd.h' can't be located for this node.
# Warning : 'math.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
src/Timer.o : src/Timer.c src/Timer.h src/selene.h 
	$(cc) -c -o src/Timer.o src/Timer.c 

# Warning : 'assert.h' can't be located for this node.
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

# Warning : 'time.h' can't be located for this node.
# Warning : 'stdio.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'sys/poll.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'libgen.h' can't be located for this node.
# Warning : 'Collection.h' can't be located for this node.
src/selene.o : src/selene.c src/MQTT.h src/Timer.h src/directfb.h \
  src/sharedobj.h src/selene.h 
	$(cc) -c -o src/selene.o src/selene.c 

# Warning : 'string.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'stdint.h' can't be located for this node.
# Warning : 'sys/eventfd.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
src/sharedobj.o : src/sharedobj.c src/sharedobj.h 
	$(cc) -c -o src/sharedobj.o src/sharedobj.c 

Selene : src/sharedobj.o src/selene.o src/directfb.o src/dfb_surface.o \
  src/dfb_screen.o src/dfb_image.o src/Timer.o src/SelCollection.o \
  src/MQTT_tools.o src/MQTT.o 
	 $(cc) -o Selene src/sharedobj.o src/selene.o src/directfb.o \
  src/dfb_surface.o src/dfb_screen.o src/dfb_image.o src/Timer.o \
  src/SelCollection.o src/MQTT_tools.o src/MQTT.o 

all: Selene 
