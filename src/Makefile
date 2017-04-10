# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (Apr 23 2016 19:25:06) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DUSE_CURSES -DUSE_MQTT -DxPLUGIN_DIR=\"/usr/local/lib/Selene\" -DxDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` `ncursesw5-config --cflags` `ncursesw5-config --libs` `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c 

SelCollection.o : SelCollection.c SelCollection.h selene.h \
  configuration.h Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c 

SelLog.o : SelLog.c SelLog.h selene.h configuration.h Makefile 
	$(cc) -c -o SelLog.o SelLog.c 

SelMQTT.o : SelMQTT.c SelMQTT.h selene.h configuration.h MQTT_tools.h \
  SelShared.h SelTimer.h Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c 

SelShared.o : SelShared.c SelShared.h selene.h configuration.h \
  Makefile 
	$(cc) -c -o SelShared.o SelShared.c 

SelTimedCollection.o : SelTimedCollection.c SelTimedCollection.h \
  selene.h configuration.h Makefile 
	$(cc) -c -o SelTimedCollection.o SelTimedCollection.c 

SelTimer.o : SelTimer.c selene.h configuration.h SelTimer.h \
  SelShared.h Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c 

selene.o : selene.c selene.h configuration.h SelShared.h \
  SelPlugins/DirectFB/directfb.h SelPlugins/Curses/curses.h SelTimer.h \
  SelMQTT.h SelCollection.h SelTimedCollection.h SelLog.h Makefile 
	$(cc) -c -o selene.o selene.c 

../Selene : selene.o SelTimer.o SelTimedCollection.o SelShared.o \
  SelMQTT.o SelLog.o SelCollection.o MQTT_tools.o Makefile 
	 $(cc) -o ../Selene selene.o SelTimer.o SelTimedCollection.o \
  SelShared.o SelMQTT.o SelLog.o SelCollection.o MQTT_tools.o 

all: ../Selene 
