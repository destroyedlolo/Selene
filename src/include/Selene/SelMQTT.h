/* SelLog.h
 *
 * Logging facility
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELMQTT_VERSION
#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELMQTT_VERSION 2

#include <Selene/SelLua.h>

#include <MQTTClient.h>	/* Paho MQTT library needed */


#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Broker client's context
 */
struct enhanced_client {
	MQTTClient client;	/* Paho's client handle */
	struct _topic *subscriptions;	/* Linked list of subscription */
	struct elastic_storage *onDisconnectFunc;	/* Function called in case of disconnection with the broker */
	int onDisconnectTrig;	/* Triggercalled in case of disconnection with the broker */
};

struct SelMQTT {
	struct SelModule module;

		/* Call backs */
	int (*mqttpublish)(MQTTClient, const char *topic, int length, void *payload, int retained);
	int (*mqtttokcmp)(const char *s, const char *t);
	struct enhanced_client *(*checkSelMQTT)(lua_State *);
};

#ifdef __cplusplus
}
#endif

#endif
