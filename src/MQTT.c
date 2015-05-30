/* MQTT.c
 *
 * This file contains all stuffs related to MQTT messaging
 *
 * 30/05/2015 LF : First version
 */
#include "MQTT.h"

#ifdef USE_MQTT
#include <assert.h>

static int smq_connect(lua_State *L){
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	const char *host = luaL_checkstring(L, 1);	/* Host to connect to */

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "SelSurface.create() is expecting a table");
		return 2;
	}


	return 0;
}

static const struct luaL_reg SelMQTTLib [] = {
	{"connect", smq_connect},
	{NULL, NULL}
};

static const struct luaL_reg SelMQTTM [] = {
	{NULL, NULL}
};

void init_mqtt(lua_State *L){
	luaL_newmetatable(L, "SelMQTT");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelMQTTM);
	luaL_register(L,"SelMQTT", SelMQTTLib);
}
#endif
