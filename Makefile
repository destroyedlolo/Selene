# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (May 30 2015 17:53:45) (c)LFSoft 1997

gotoall: all

# Warning : 'assert.h' can't be located for this node.

#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DxDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpaho-mqtt3c

src/dfb_font.o : src/dfb_font.c src/directfb.h 
	$(cc) -c -o src/dfb_font.o src/dfb_font.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_image.o : src/dfb_image.c src/directfb.h 
	$(cc) -c -o src/dfb_image.o src/dfb_image.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_layer.o : src/dfb_layer.c src/directfb.h 
	$(cc) -c -o src/dfb_layer.o src/dfb_layer.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_screen.o : src/dfb_screen.c src/directfb.h 
	$(cc) -c -o src/dfb_screen.o src/dfb_screen.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_surface.o : src/dfb_surface.c src/directfb.h 
	$(cc) -c -o src/dfb_surface.o src/dfb_surface.c 

# Warning : 'assert.h' can't be located for this node.
src/dfb_window.o : src/dfb_window.c src/directfb.h 
	$(cc) -c -o src/dfb_window.o src/dfb_window.c 

# Warning : 'assert.h' can't be located for this node.
src/directfb.o : src/directfb.c src/directfb.h 
	$(cc) -c -o src/directfb.o src/directfb.c 

# Warning : 'assert.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
src/MQTT.o : src/MQTT.c src/SelShared.h src/MQTT_tools.h src/MQTT.h 
	$(cc) -c -o src/MQTT.o src/MQTT.c 

src/MQTT_tools.o : src/MQTT_tools.c src/MQTT_tools.h 
	$(cc) -c -o src/MQTT_tools.o src/MQTT_tools.c 

# Warning : 'assert.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
src/SelCollection.o : src/SelCollection.c src/SelCollection.h 
	$(cc) -c -o src/SelCollection.o src/SelCollection.c 

# Warning : 'time.h' can't be located for this node.
# Warning : 'stdio.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'sys/poll.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'libgen.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'signal.h' can't be located for this node.
src/selene.o : src/selene.c src/SelCollection.h src/MQTT.h \
  src/SelTimer.h src/directfb.h src/SelShared.h src/selene.h 
	$(cc) -c -o src/selene.o src/selene.c 

# Warning : 'string.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'stdint.h' can't be located for this node.
# Warning : 'sys/eventfd.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
src/SelShared.o : src/SelShared.c src/SelShared.h 
	$(cc) -c -o src/SelShared.o src/SelShared.c 

# Warning : 'sys/timerfd.h' can't be located for this node.
# Warning : 'math.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
src/SelTimer.o : src/SelTimer.c src/SelShared.h src/SelTimer.h \
  src/selene.h 
	$(cc) -c -o src/SelTimer.o src/SelTimer.c 

Selene : src/SelTimer.o src/SelShared.o src/selene.o \
  src/SelCollection.o src/MQTT_tools.o src/MQTT.o src/directfb.o \
  src/dfb_window.o src/dfb_surface.o src/dfb_screen.o src/dfb_layer.o \
  src/dfb_image.o src/dfb_font.o 
	 $(cc) -o Selene src/SelTimer.o src/SelShared.o src/selene.o \
  src/SelCollection.o src/MQTT_tools.o src/MQTT.o src/directfb.o \
  src/dfb_window.o src/dfb_surface.o src/dfb_screen.o src/dfb_layer.o \
  src/dfb_image.o src/dfb_font.o 

all: Selene 
