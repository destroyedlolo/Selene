# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (May  8 2017 00:11:26) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=\"/usr/local/lib/Selene\" -DxDEBUG -std=c99 `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic -lpthread

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c 

SelCollection.o : SelCollection.c SelCollection.h selene.h \
  configuration.h Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c 

selene.o : selene.c selene.h configuration.h SelShared.h SelTimer.h \
  SelMQTT.h SelCollection.h SelTimedCollection.h \
  SelTimedWindowCollection.h SelFIFO.h SelLog.h SelEvent.h Makefile 
	$(cc) -c -o selene.o selene.c 

SelEvent.o : SelEvent.c selene.h configuration.h SelEvent.h \
  SelShared.h Makefile 
	$(cc) -c -o SelEvent.o SelEvent.c 

SelFIFO.o : SelFIFO.c SelFIFO.h selene.h configuration.h Makefile 
	$(cc) -c -o SelFIFO.o SelFIFO.c 

SelLog.o : SelLog.c SelLog.h selene.h configuration.h Makefile 
	$(cc) -c -o SelLog.o SelLog.c 

SelMQTT.o : SelMQTT.c SelMQTT.h selene.h configuration.h MQTT_tools.h \
  SelShared.h SelFIFO.h SelTimer.h Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c 

SelShared.o : SelShared.c SelShared.h selene.h configuration.h \
  Makefile 
	$(cc) -c -o SelShared.o SelShared.c 

SelTimedCollection.o : SelTimedCollection.c SelTimedCollection.h \
  selene.h configuration.h Makefile 
	$(cc) -c -o SelTimedCollection.o SelTimedCollection.c 

SelTimedWindowCollection.o : SelTimedWindowCollection.c \
  SelTimedWindowCollection.h selene.h configuration.h Makefile 
	$(cc) -c -o SelTimedWindowCollection.o \
  SelTimedWindowCollection.c 

SelTimer.o : SelTimer.c selene.h configuration.h SelTimer.h \
  SelShared.h Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c 

../Selene : SelTimer.o SelTimedWindowCollection.o SelTimedCollection.o \
  SelShared.o SelMQTT.o SelLog.o SelFIFO.o SelEvent.o selene.o \
  SelCollection.o MQTT_tools.o Makefile 
	 $(cc) -o ../Selene SelTimer.o SelTimedWindowCollection.o \
  SelTimedCollection.o SelShared.o SelMQTT.o SelLog.o SelFIFO.o \
  SelEvent.o selene.o SelCollection.o MQTT_tools.o 

all: ../Selene 
