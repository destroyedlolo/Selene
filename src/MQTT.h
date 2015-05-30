/* MQTT.h
 *
 * All shared definitions for MQTT
 *
 * 30/05/2015 LF : First version
 */

#ifndef SMQTT_H
#define SMQTT_H

#	ifdef USE_MQTT
#include "selene.h"
#include <MQTTClient.h>

extern void init_mqtt(lua_State *);

#	endif
#endif
