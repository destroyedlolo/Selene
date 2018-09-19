/**
 * SeleMQTT.c
 *
 * "external" MQTT messaging.
 * Unlike SelMQTT, connection is managed externaly
 *
 * 24/07/2018 LF : First version
 */
#include <string.h>

#include "libSelene.h"
#include "internal.h"

/**
 * broker client context
 */

static struct external_client {
	MQTTClient client;		/* Paho's client handle */
	const char *clientID;	/* Pointer to clientID */
} eclient;

void semc_initializeSeleMQTT( MQTTClient aclient, const char *acid ){
	eclient.client = aclient;
	eclient.clientID = acid;
}

static int sme_publish(lua_State *L){
/* Publish to a topic
 *	1 : topic
 *	2 : valeur
 *	3 : retain
 */
	if( !eclient.client ){
		lua_pushnil(L);
		lua_pushstring(L, "Publish() without MQTT initialized");
		return 2;
	}

	const char *topic = luaL_checkstring(L, 1),
				*val = luaL_checkstring(L, 2);
	int retain =  lua_toboolean(L, 3);

	mqttpublish( eclient.client, topic, strlen(val), (void *)val, retain );

	return 0;
}

static const struct luaL_Reg SeleMQTTLib [] = {
	{"Publish", sme_publish},
/*	{"QoSConst", smq_QoSConst}, */
	{"ErrConst", smq_ErrCodeConst},
	{"StrError", smq_StrError},
	{NULL, NULL}
};

int initSeleMQTT(lua_State *L){
	libSel_libFuncs( L, "SeleMQTT", SeleMQTTLib );

	return 1;
}

void initG_SeleMQTT(){
	eclient.client = NULL;
	eclient.clientID = NULL;
}
