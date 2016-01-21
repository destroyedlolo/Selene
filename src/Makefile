# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (May 30 2015 17:53:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DUSE_DIRECTFB -DUSE_MQTT -DxDEBUG -std=c99 `directfb-config --cflags` `directfb-config --libs` -llua -lpaho-mqtt3c

dfb_font.o : dfb_font.c directfb.h selene.h configuration.h Makefile 
	$(cc) -c -o dfb_font.o dfb_font.c 

dfb_image.o : dfb_image.c directfb.h selene.h configuration.h Makefile 
	$(cc) -c -o dfb_image.o dfb_image.c 

dfb_layer.o : dfb_layer.c directfb.h selene.h configuration.h Makefile 
	$(cc) -c -o dfb_layer.o dfb_layer.c 

dfb_screen.o : dfb_screen.c directfb.h selene.h configuration.h \
  Makefile 
	$(cc) -c -o dfb_screen.o dfb_screen.c 

dfb_surface.o : dfb_surface.c directfb.h selene.h configuration.h \
  Makefile 
	$(cc) -c -o dfb_surface.o dfb_surface.c 

dfb_window.o : dfb_window.c directfb.h selene.h configuration.h \
  Makefile 
	$(cc) -c -o dfb_window.o dfb_window.c 

directfb.o : directfb.c directfb.h selene.h configuration.h Makefile 
	$(cc) -c -o directfb.o directfb.c 

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c 

SelCollection.o : SelCollection.c SelCollection.h selene.h \
  configuration.h Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c 

selene.o : selene.c selene.h configuration.h SelShared.h directfb.h \
  SelTimer.h SelMQTT.h SelCollection.h Makefile 
	$(cc) -c -o selene.o selene.c 

SelMQTT.o : SelMQTT.c SelMQTT.h selene.h configuration.h SelTimer.h \
  MQTT_tools.h SelShared.h Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c 

SelShared.o : SelShared.c SelShared.h selene.h configuration.h \
  Makefile 
	$(cc) -c -o SelShared.o SelShared.c 

SelTimer.o : SelTimer.c selene.h configuration.h SelTimer.h \
  SelShared.h Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c 

../Selene : SelTimer.o SelShared.o SelMQTT.o selene.o SelCollection.o \
  MQTT_tools.o directfb.o dfb_window.o dfb_surface.o dfb_screen.o \
  dfb_layer.o dfb_image.o dfb_font.o Makefile 
	 $(cc) -o ../Selene SelTimer.o SelShared.o SelMQTT.o selene.o \
  SelCollection.o MQTT_tools.o directfb.o dfb_window.o dfb_surface.o \
  dfb_screen.o dfb_layer.o dfb_image.o dfb_font.o 

all: ../Selene 
