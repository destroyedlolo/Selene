/**
 * \file SelMQTT.h
 * \brief All shared definitions for MQTT
 *
 * 30/05/2015 LF : First version
 * 21/01/2015 LF : Rename as SelMQTT
 */

#ifndef SMQTT_H
#define SMQTT_H

#	ifdef USE_MQTT
#include "selene.h"
#include <MQTTClient.h>

extern void init_mqtt(lua_State *);

#	endif
#endif
