# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (Sep  7 2016 11:21:46) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_CURSES -DUSE_MQTT -DPLUGIN_DIR=\".\" -DxDEBUG -std=c99 `ncursesw5-config --cflags` `ncursesw5-config --libs` `pkg-config --cflags lua` `pkg-config --libs lua` -lpaho-mqtt3c -ldl -Wl,--export-dynamic -lpthread

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

SelTimer.o : SelTimer.c selene.h configuration.h SelTimer.h \
  SelShared.h Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c 

selene.o : selene.c selene.h configuration.h SelShared.h \
  SelPlugins/DirectFB/directfb.h SelPlugins/Curses/curses.h SelTimer.h \
  SelMQTT.h SelCollection.h SelLog.h Makefile 
	$(cc) -c -o selene.o selene.c 

../Selene : selene.o SelTimer.o SelShared.o SelMQTT.o SelLog.o \
  SelCollection.o MQTT_tools.o Makefile 
	 $(cc) -o ../Selene selene.o SelTimer.o SelShared.o SelMQTT.o \
  SelLog.o SelCollection.o MQTT_tools.o 

all: ../Selene 
