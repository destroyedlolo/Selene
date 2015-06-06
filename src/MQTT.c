/* MQTT.c
 *
 * This file contains all stuffs related to MQTT messaging
 *
 * 30/05/2015 LF : First version
 */
#include "MQTT.h"

#ifdef USE_MQTT
#include <assert.h>
#include <stdlib.h>

static const struct ConstTranscode _QoS[] = {
	{ "QoS0", 0 },
	{ "QoS1", 1 },
	{ "QoS2", 2 },
	{ NULL, 0 }
};

static int smq_QoSConst( lua_State *L ){
	return findConst(L, _QoS);
}

static const struct ConstTranscode _ErrCode[] = {
	{ "MQTTCLIENT_SUCCESS", MQTTCLIENT_SUCCESS },
	{ "MQTTCLIENT_FAILURE", MQTTCLIENT_FAILURE },
	{ "MQTTCLIENT_DISCONNECTED", MQTTCLIENT_DISCONNECTED },
	{ "MQTTCLIENT_MAX_MESSAGES_INFLIGHT", MQTTCLIENT_MAX_MESSAGES_INFLIGHT },
	{ "MQTTCLIENT_BAD_UTF8_STRING", MQTTCLIENT_BAD_UTF8_STRING },
	{ "MQTTCLIENT_NULL_PARAMETER", MQTTCLIENT_NULL_PARAMETER },
	{ "MQTTCLIENT_TOPICNAME_TRUNCATED", MQTTCLIENT_TOPICNAME_TRUNCATED},
	{ "MQTTCLIENT_BAD_STRUCTURE", MQTTCLIENT_BAD_STRUCTURE },
	{ "MQTTCLIENT_BAD_QOS", MQTTCLIENT_BAD_QOS },
	{ NULL, 0 }
};

static int smq_ErrCodeConst( lua_State *L ){
	return findConst(L, _ErrCode);
}

static const struct ConstTranscode _strErrCode[] = {	/* Caution, reverse tables */
	{ "No error", MQTTCLIENT_SUCCESS },
	{ "A generic error code indicating the failure of an MQTT client operation", MQTTCLIENT_FAILURE },
	{ "The client is disconnected", MQTTCLIENT_DISCONNECTED },
	{ "The maximum number of messages allowed to be simultaneously in-flight has been reached", MQTTCLIENT_MAX_MESSAGES_INFLIGHT },
	{ "An invalid UTF-8 string has been detected", MQTTCLIENT_BAD_UTF8_STRING },
	{ "A NULL parameter has been supplied when this is invalid", MQTTCLIENT_NULL_PARAMETER },
	{ "The topic has been truncated (the topic string includes embedded NULL characters)", MQTTCLIENT_TOPICNAME_TRUNCATED },
	{ "A structure parameter does not have the correct eyecatcher and version number", MQTTCLIENT_BAD_STRUCTURE },
	{ "A QoS value that falls outside of the acceptable range (0,1,2)", MQTTCLIENT_BAD_QOS },
	{ NULL, 0 }
};

static int smq_StrError( lua_State *L ){
	return rfindConst(L, _strErrCode);
}

static const char *smq_CStrError( int arg ){
	for(int i=0; _strErrCode[i].name; i++){
		if( arg == _strErrCode[i].value ){
			return _strErrCode[i].name;
		}
	}

	return "Unknown error";
}

struct _topic {
	struct _topic *next;
	char *topic;
	int func;
	int qos;
};

struct enhanced_client {
	MQTTClient client;
	struct _topic *subscriptions;
};

/*
 * Callback functions 
 */

int msgarrived(void *ctx, char *topic, int tlen, MQTTClient_message *msg){

printf("*AF* message arrived (%s)\n", topic);

	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

void connlost(void *client, char *cause){
/*AF : probably better to do ... */
	printf("*W* Broker connection lost due to %s\n", cause ? cause : "???");
}

static struct enhanced_client *checkSelMQTT(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelMQTT");
	luaL_argcheck(L, r != NULL, 1, "'SelMQTT' expected");
	return (struct enhanced_client *)r;
}

static int smq_subscribe(lua_State *L){
/* Subscribe to topics
 * 1 : table
 * 	topic : topic name to subscribe
 * 	func : function to call when a message arrive
 * 	qos : as the name said, default 0
 */
	struct enhanced_client *eclient = checkSelMQTT(L);
	int nbre;	/* nbre of topics */
	struct _topic *nt;

	if(!eclient){
		lua_pushnil(L);
		lua_pushstring(L, "subscribe() to a dead object");
		return 2;
	}

	if( lua_type(L, -1) != LUA_TTABLE ){
		lua_pushnil(L);
		lua_pushstring(L, "subscribe() needs a table");
		return 2;
	}
	nbre = lua_objlen(L, -1);	/* nbre of entries in the table */

		/* Walk thru arguments */

	lua_pushnil(L);
	while(lua_next(L, -2) != 0){
		char *topic;

		int func;
		int qos = 0;

		lua_pushstring(L, "topic");
		lua_gettable(L, -2);
		assert( topic = strdup( luaL_checkstring(L, -1) ) );
		lua_pop(L, 1);	/* Pop topic */

		lua_pushstring(L, "func");
		lua_gettable(L, -2);
		if( lua_type(L, -1) != LUA_TFUNCTION ){
			lua_pop(L, 1);	/* Pop the result */
			lua_pushnil(L);
			lua_pushstring(L, "Subscribe() : topics needs associated function");
			return 2;
		}
		func = luaL_ref(L,LUA_REGISTRYINDEX);	/* pop the value by itself */

		lua_pushstring(L, "qos");
		lua_gettable(L, -2);
		if( lua_type(L, -1) == LUA_TNUMBER )
			qos = lua_tointeger(L, -1);
		lua_pop(L, 1);	/* Pop the QoS */

			/* Allocating the new topic */
		assert( nt = malloc(sizeof(struct _topic)) );
		nt->next = eclient->subscriptions;
		nt->topic = topic;
		nt->func = func;
		nt->qos = qos;
		eclient->subscriptions = nt;
		
		lua_pop(L, 1);	/* Pop the sub-table */
	}

		/* subscribe to topics */
	if(nbre){
		char *tpcs[nbre];
		int qos[nbre];
		struct _topic *t = eclient->subscriptions;
		int err;

		for(int i=0; i < nbre; i++){
			assert( t );	/* If failing, it means an error in the code above */
			tpcs[i] = t->topic;
			qos[i] = t->qos;

			t = t->next;
		}
		if( (err = MQTTClient_subscribeMany( eclient->client, nbre, tpcs, qos)) != MQTTCLIENT_SUCCESS ){
			lua_pushnil(L);
			lua_pushstring(L, smq_CStrError(err));
			return 2;
		}
	}

	return 0;
}

static int smq_connect(lua_State *L){
/* Connect to a broker
 * 1 : broker's host
 * 2 : table of arguments
 */
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	const char *host = luaL_checkstring(L, 1);	/* Host to connect to */
	const char *clientID = "Selene";
	const char *persistence = NULL;
	const char *err = NULL;
	struct enhanced_client *eclient;

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

	lua_pushstring(L, "persistence");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TSTRING )
		persistence = lua_tostring(L, -1);
	lua_pop(L, 1);	/* cleaning ... */
		/* Creating Lua data */
	eclient = (struct enhanced_client *)lua_newuserdata(L, sizeof(struct enhanced_client));
	luaL_getmetatable(L, "SelMQTT");
	lua_setmetatable(L, -2);
	eclient->subscriptions = NULL;

		/* Connecting */
	MQTTClient_create( &(eclient->client), host, clientID, persistence ? MQTTCLIENT_PERSISTENCE_DEFAULT : MQTTCLIENT_PERSISTENCE_NONE, (void *)persistence );
	MQTTClient_setCallbacks( eclient->client, eclient, connlost, msgarrived, NULL);

	switch( MQTTClient_connect( eclient->client, &conn_opts) ){
	case MQTTCLIENT_SUCCESS : 
		break;
	case 1 : err = "Unable to connect : Unacceptable protocol version";
		break;
	case 2 : err = "Unable to connect : Identifier rejected";
		break;
	case 3 : err = "Unable to connect : Server unavailable";
		break;
	case 4 : err = "Unable to connect : Bad user name or password";
		break;
	case 5 : err = "Unable to connect : Not authorized";
		break;
	default :
		err = "Unable to connect : Unknown error";
	}

	if(err){
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushstring(L, err);
		return 2;
	}

	return 1;
}

static const struct luaL_reg SelMQTTLib [] = {
	{"connect", smq_connect},
	{"QoSConst", smq_QoSConst},
	{"ErrConst", smq_ErrCodeConst},
	{"StrError", smq_StrError},
	{NULL, NULL}
};

static const struct luaL_reg SelMQTTM [] = {
	{"subscribe", smq_subscribe},
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
