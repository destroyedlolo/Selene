# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -fPIC   $(shell pkg-config --cflags lua )

elastic_storage.o : elastic_storage.c elastic_storage.h \
  sel_Shareable.h libSelene.h SelShared.h configuration.h \
  SelTimedCollection.h SelTimedWindowCollection.h Makefile 
	$(cc) -c -o elastic_storage.o elastic_storage.c $(opts) 

LuaSupportFunc.o : LuaSupportFunc.c libSelene.h elastic_storage.h \
  sel_Shareable.h configuration.h SelShared.h SelTimedCollection.h \
  SelTimedWindowCollection.h internal.h Makefile 
	$(cc) -c -o LuaSupportFunc.o LuaSupportFunc.c $(opts) 

misc.o : misc.c libSelene.h elastic_storage.h sel_Shareable.h Makefile 
	$(cc) -c -o misc.o misc.c $(opts) 

MQTT_tools.o : MQTT_tools.c libSelene.h elastic_storage.h \
  sel_Shareable.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c $(opts) 

SelAverageCollection.o : SelAverageCollection.c SelAverageCollection.h \
  libSelene.h elastic_storage.h sel_Shareable.h Makefile 
	$(cc) -c -o SelAverageCollection.o SelAverageCollection.c \
  $(opts) 

SelCollection.o : SelCollection.c SelCollection.h libSelene.h \
  elastic_storage.h sel_Shareable.h Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c $(opts) 

SeleMQTT.o : SeleMQTT.c libSelene.h elastic_storage.h sel_Shareable.h \
  internal.h Makefile 
	$(cc) -c -o SeleMQTT.o SeleMQTT.c $(opts) 

Selene.o : Selene.c libSelene.h elastic_storage.h sel_Shareable.h \
  configuration.h SelTimer.h SelShared.h SelTimedCollection.h \
  SelTimedWindowCollection.h SelEvent.h Makefile 
	$(cc) -c -o Selene.o Selene.c $(opts) 

SelEvent.o : SelEvent.c libSelene.h elastic_storage.h sel_Shareable.h \
  SelEvent.h SelShared.h configuration.h SelTimedCollection.h \
  SelTimedWindowCollection.h Makefile 
	$(cc) -c -o SelEvent.o SelEvent.c $(opts) 

SelFIFO.o : SelFIFO.c SelFIFO.h libSelene.h elastic_storage.h \
  sel_Shareable.h Makefile 
	$(cc) -c -o SelFIFO.o SelFIFO.c $(opts) 

SelLog.o : SelLog.c libSelene.h elastic_storage.h sel_Shareable.h \
  internal.h Makefile 
	$(cc) -c -o SelLog.o SelLog.c $(opts) 

SelMQTT.o : SelMQTT.c libSelene.h elastic_storage.h sel_Shareable.h \
  SelShared.h configuration.h SelTimedCollection.h \
  SelTimedWindowCollection.h SelFIFO.h SelTimer.h Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c $(opts) 

sel_Shareable.o : sel_Shareable.c sel_Shareable.h Makefile 
	$(cc) -c -o sel_Shareable.o sel_Shareable.c $(opts) 

SelShared.o : SelShared.c SelShared.h libSelene.h elastic_storage.h \
  sel_Shareable.h configuration.h SelTimedCollection.h \
  SelTimedWindowCollection.h Makefile 
	$(cc) -c -o SelShared.o SelShared.c $(opts) 

SelTimedCollection.o : SelTimedCollection.c SelTimedCollection.h \
  libSelene.h elastic_storage.h sel_Shareable.h Makefile 
	$(cc) -c -o SelTimedCollection.o SelTimedCollection.c $(opts) 

SelTimedWindowCollection.o : SelTimedWindowCollection.c \
  SelTimedWindowCollection.h libSelene.h elastic_storage.h \
  sel_Shareable.h Makefile 
	$(cc) -c -o SelTimedWindowCollection.o \
  SelTimedWindowCollection.c $(opts) 

SelTimer.o : SelTimer.c libSelene.h elastic_storage.h sel_Shareable.h \
  SelTimer.h Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c $(opts) 

../../libSelene.so : SelTimer.o SelTimedWindowCollection.o \
  SelTimedCollection.o SelShared.o sel_Shareable.o SelMQTT.o SelLog.o \
  SelFIFO.o SelEvent.o Selene.o SeleMQTT.o SelCollection.o \
  SelAverageCollection.o MQTT_tools.o misc.o LuaSupportFunc.o \
  elastic_storage.o Makefile 
	 $(cc) -shared -o ../../libSelene.so SelTimer.o \
  SelTimedWindowCollection.o SelTimedCollection.o SelShared.o \
  sel_Shareable.o SelMQTT.o SelLog.o SelFIFO.o SelEvent.o Selene.o \
  SeleMQTT.o SelCollection.o SelAverageCollection.o MQTT_tools.o \
  misc.o LuaSupportFunc.o elastic_storage.o $(opts) 

all: ../../libSelene.so 
