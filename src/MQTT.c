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
/* Connect to a broker
 * 1 : broker's host
 * 2 : table of arguments
 */
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	const char *host = luaL_checkstring(L, 1);	/* Host to connect to */
	const char *clientID = "Selene";

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "SelSurface.create() is expecting a table");
		return 2;
	}

	lua_pushstring(L, "keepAliveInterval");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER )
		conn_opts.keepAliveInterval = lua_tointeger(L, -1);
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "cleansession");
	lua_gettable(L, -2);
	switch( lua_type(L, -1) ){
	case LUA_TBOOLEAN:
		conn_opts.cleansession = lua_toboolean(L, -1) ? 1 : 0;
		break;
	case LUA_TNUMBER:
		conn_opts.cleansession = lua_tointeger(L, -1) ? 1 : 0;
		break;
	case LUA_TNIL:
		conn_opts.cleansession = 0;
		break;
	default :
		lua_pushnil(L);
		lua_pushstring(L, lua_typename(L, lua_type(L, -2) ));
		lua_pushstring(L," : don't know how to convert to boolean");
		lua_concat(L, 2);
		return 2;
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "reliable");
	lua_gettable(L, -2);
	switch( lua_type(L, -1) ){
	case LUA_TBOOLEAN:
		conn_opts.reliable = lua_toboolean(L, -1) ? 1 : 0;
		break;
	case LUA_TNUMBER:
		conn_opts.reliable = lua_tointeger(L, -1) ? 1 : 0;
		break;
	case LUA_TNIL:
		conn_opts.reliable = 0;
		break;
	default :
		lua_pushnil(L);
		lua_pushstring(L, lua_typename(L, lua_type(L, -2) ));
		lua_pushstring(L," : don't know how to convert to boolean");
		lua_concat(L, 2);
		return 2;

	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "username");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TSTRING )
		conn_opts.username = lua_tostring(L, -1);
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "password");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TSTRING )
		conn_opts.password = lua_tostring(L, -1);
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "clientID");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TSTRING )
		clientID = lua_tostring(L, -1);
	lua_pop(L, 1);	/* cleaning ... */

puts(clientID);

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
