# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Apr  3 2018 23:14:49) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-I/home/laurent/Projets/lua-5.3.4/install/include -Wall -fPIC -DxUSE_MQTT

LuaSupportFunc.o : LuaSupportFunc.c libSelene.h configuration.h \
  SelShared.h Makefile 
	$(cc) -c -o LuaSupportFunc.o LuaSupportFunc.c $(opts) 

misc.o : misc.c libSelene.h Makefile 
	$(cc) -c -o misc.o misc.c $(opts) 

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c $(opts) 

SelCollection.o : SelCollection.c libSelene.h Makefile 
	$(cc) -c -o SelCollection.o SelCollection.c $(opts) 

Selene.o : Selene.c libSelene.h configuration.h SelTimer.h SelShared.h \
  Makefile 
	$(cc) -c -o Selene.o Selene.c $(opts) 

SelLog.o : SelLog.c libSelene.h Makefile 
	$(cc) -c -o SelLog.o SelLog.c $(opts) 

SelMQTT.o : SelMQTT.c libSelene.h MQTT_tools.h SelShared.h \
  configuration.h SelTimer.h Makefile 
	$(cc) -c -o SelMQTT.o SelMQTT.c $(opts) 

SelShared.o : SelShared.c SelShared.h libSelene.h configuration.h \
  Makefile 
	$(cc) -c -o SelShared.o SelShared.c $(opts) 

SelTimedCollection.o : SelTimedCollection.c libSelene.h Makefile 
	$(cc) -c -o SelTimedCollection.o SelTimedCollection.c $(opts) 

SelTimedWindowCollection.o : SelTimedWindowCollection.c libSelene.h \
  Makefile 
	$(cc) -c -o SelTimedWindowCollection.o \
  SelTimedWindowCollection.c $(opts) 

SelTimer.o : SelTimer.c libSelene.h SelTimer.h Makefile 
	$(cc) -c -o SelTimer.o SelTimer.c $(opts) 

../../libSelene.so : SelTimer.o SelTimedWindowCollection.o \
  SelTimedCollection.o SelShared.o SelMQTT.o SelLog.o Selene.o \
  SelCollection.o MQTT_tools.o misc.o LuaSupportFunc.o Makefile 
	 $(cc) -shared -o ../../libSelene.so SelTimer.o \
  SelTimedWindowCollection.o SelTimedCollection.o SelShared.o \
  SelMQTT.o SelLog.o Selene.o SelCollection.o MQTT_tools.o misc.o \
  LuaSupportFunc.o $(opts) 

all: ../../libSelene.so 
