# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=`pkg-config --cflags lua` -Wall -fPIC

LuaSupportFunc.o : LuaSupportFunc.c libSelene.h elastic_storage.h \
  configuration.h SelShared.h internal.h Makefile 
	$(cc) -c -o LuaSupportFunc.o LuaSupportFunc.c $(opts) 

MQTT_tools.o : MQTT_tools.c libSelene.h elastic_storage.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c $(opts) 

SelCollection.o : SelCollection.c libSelene.h elastic_storage.h \
  Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c $(opts) 

SelEvent.o : SelEvent.c libSelene.h elastic_storage.h SelEvent.h \
  SelShared.h configuration.h Makefile 
	$(cc) -c -o SelEvent.o SelEvent.c $(opts) 

SelFIFO.o : SelFIFO.c SelFIFO.h libSelene.h elastic_storage.h Makefile 
	$(cc) -c -o SelFIFO.o SelFIFO.c $(opts) 

SelLog.o : SelLog.c libSelene.h elastic_storage.h Makefile 
	$(cc) -c -o SelLog.o SelLog.c $(opts) 

SelMQTT.o : SelMQTT.c libSelene.h elastic_storage.h SelShared.h \
  configuration.h SelFIFO.h SelTimer.h Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c $(opts) 

SelShared.o : SelShared.c SelShared.h libSelene.h elastic_storage.h \
  configuration.h Makefile 
	$(cc) -c -o SelShared.o SelShared.c $(opts) 

SelTimedCollection.o : SelTimedCollection.c libSelene.h \
  elastic_storage.h Makefile 
	$(cc) -c -o SelTimedCollection.o SelTimedCollection.c $(opts) 

SelTimedWindowCollection.o : SelTimedWindowCollection.c libSelene.h \
  elastic_storage.h Makefile 
	$(cc) -c -o SelTimedWindowCollection.o \
  SelTimedWindowCollection.c $(opts) 

SelTimer.o : SelTimer.c libSelene.h elastic_storage.h SelTimer.h \
  Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c $(opts) 

Selene.o : Selene.c libSelene.h elastic_storage.h configuration.h \
  SelTimer.h SelShared.h SelEvent.h Makefile 
	$(cc) -c -o Selene.o Selene.c $(opts) 

elastic_storage.o : elastic_storage.c elastic_storage.h libSelene.h \
  SelShared.h configuration.h Makefile 
	$(cc) -c -o elastic_storage.o elastic_storage.c $(opts) 

misc.o : misc.c libSelene.h elastic_storage.h Makefile 
	$(cc) -c -o misc.o misc.c $(opts) 

../../libSelene.so : misc.o elastic_storage.o Selene.o SelTimer.o \
  SelTimedWindowCollection.o SelTimedCollection.o SelShared.o \
  SelMQTT.o SelLog.o SelFIFO.o SelEvent.o SelCollection.o MQTT_tools.o \
  LuaSupportFunc.o Makefile 
	 $(cc) -shared -o ../../libSelene.so misc.o elastic_storage.o \
  Selene.o SelTimer.o SelTimedWindowCollection.o SelTimedCollection.o \
  SelShared.o SelMQTT.o SelLog.o SelFIFO.o SelEvent.o SelCollection.o \
  MQTT_tools.o LuaSupportFunc.o $(opts) 

all: ../../libSelene.so 
