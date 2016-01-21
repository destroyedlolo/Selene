/**
 * \file SelMQTT.c
 * \brief This file contains all stuffs related to MQTT messaging
 * \author Laurent Faillie (destroyedlolo)
 *
 * \verbatim
 * 30/05/2015 LF : First version
 * 17/06/2015 LF : Add trigger function to topic
 * 11/11/2015 LF : Add TaskOnce enum
 * 21/01/2015 LF : Rename as SelMQTT
 * \endverbatim
 */
#include "SelMQTT.h"
#include "SelTimer.h"

#ifdef USE_MQTT
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "MQTT_tools.h"
#include "SelShared.h"

static const struct ConstTranscode _QoS[] = {
	{ "QoS0", 0 },
	{ "QoS1", 1 },
	{ "QoS2", 2 },
	{ NULL, 0 }
};

static int smq_QoSConst( lua_State *L ){
	return findConst(L, _QoS);
}

/**
 * \struct _topic
 * \brief Subscription's related information
 */
struct _topic {
	struct _topic *next;	/**< Link to next topic */
	char *topic;			/**< Subscribed topic */
	int func;				/**< Arrival callback function (run in dedicated context) */
	int trigger;			/**< application side trigger function */
	enum TaskOnce trigger_once;	/**< Avoid duplicates in waiting list */
	int qos;				/**< QoS associated to this topic */
	struct SelTimer *watchdog;	/**< Watchdog on document arrival */
};

/** 
 * \struct enhanced_client
 * \brief Broker client's context
 */
struct enhanced_client {
	MQTTClient client;	/**< Paho's client handle */
	lua_State *L;		/**< Callbacks' local context */
	struct _topic *subscriptions;	/**< Linked list of subscription */
};

	/*
	 * Errors handling
	 */

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

/**
 *  \fn static int truc( lua_State *L )
 *	\return Error string corresponding to the provided code
 *
 *	\code
 *	SelMQTT.StrError(code))
 *	\endcode
 */
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

/*
 * Callback functions 
 */
#ifdef DEBUG
int _msgarrived
#else
int msgarrived
#endif
(void *actx, char *topic, int tlen, MQTTClient_message *msg){
/* handle message arrival and call associated function.
 * NOTE : up to now, only textual topics & messages are
 * correctly handled (lengths are simply ignored)
 */
	struct enhanced_client *ctx = actx;	/* To avoid numerous cast */
	struct _topic *tp;
	char cpayload[msg->payloadlen + 1];
	memcpy(cpayload, msg->payload, msg->payloadlen);
	cpayload[msg->payloadlen] = 0;
#ifdef DEBUG
	printf("topic : %s\n", topic);
#endif

	for(tp = ctx->subscriptions; tp; tp = tp->next){	/* Looks for the corresponding function */
		if(!mqtttokcmp(tp->topic, topic)){
			if(tp->func != LUA_REFNIL){		/* Call back function defined */
				pthread_mutex_lock( &lua_mutex );

				lua_rawgeti( ctx->L, LUA_REGISTRYINDEX, tp->func);	/* retrieves the function */
				lua_pushstring( ctx->L, topic);
				lua_pushstring( ctx->L, cpayload);
				if(lua_pcall( ctx->L, 2, 1, 0)){	/* Call Lua callback function */
					fprintf(stderr, "*E* (msg arrival) %s\n", lua_tostring(ctx->L, -1));
					lua_pop(ctx->L, 2); /* pop error message and NIL from the stack */
				} else if(tp->trigger != LUA_REFNIL){
					if(lua_toboolean(ctx->L, -1))
						pushtask( tp->trigger, tp->trigger_once );
					lua_pop(ctx->L, 1);	/* remove the return code */
				}
				pthread_mutex_unlock( &lua_mutex );
			} else {
				/* No call back : set a shared variable
				 * and unconditionnaly push a trigger if it exists
				 */
				soc_sets( topic, cpayload );
				if(tp->trigger != LUA_REFNIL)
					pushtask( tp->trigger, tp->trigger_once );
			}
		}
	}

	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

#ifdef DEBUG
int msgarrived(void *actx, char *topic, int tlen, MQTTClient_message *msg){
	puts("msgarrived ...");
	int r=_msgarrived(actx, topic, tlen, msg);
	puts("msgarrived ok");
	return r;
}
#endif

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

	assert( !eclient->subscriptions );	/* As of now, multiple pass subscription is not supported */

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

		int func = LUA_REFNIL;
		int trigger = LUA_REFNIL;
		enum TaskOnce trigger_once = TO_ONCE;
		int qos = 0;

		lua_pushstring(L, "topic");
		lua_gettable(L, -2);
		assert( topic = strdup( luaL_checkstring(L, -1) ) );
		lua_pop(L, 1);	/* Pop topic */

			/* CAUTION : func are part of dedicated thread's context and never
			 *	pushed in TODO list.
			 * 	Consequently, they are not kept in functions lookup reference table
			 */
		lua_pushstring(L, "func");
		lua_gettable(L, -2);
		if( lua_type(L, -1) != LUA_TFUNCTION )
			lua_pop(L, 1);	/* Pop the result */
		else {
			lua_xmove( L, eclient->L, 1 );	/* Move the function to the callback's stack */
			func = luaL_ref(eclient->L,LUA_REGISTRYINDEX);	/* Reference the function in callbacks' context */
		}

			/* triggers are part of the main thread and pushed in TODO list.
			 * Consequently, they are kept in functions lookup reference table
			 */
		lua_pushstring(L, "trigger");
		lua_gettable(L, -2);
		if( lua_type(L, -1) != LUA_TFUNCTION )	/* This function is optional */
			lua_pop(L, 1);	/* Pop the unused result */
		else {
			trigger = findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
			lua_pop(L,1);
		}

		
		lua_pushstring(L, "trigger_once");
		lua_gettable(L, -2);
		if( lua_type(L, -1) == LUA_TBOOLEAN )
			trigger_once = lua_toboolean(L, -1) ? TO_ONCE : TO_MULTIPLE;
		else if( lua_type(L, -1) == LUA_TNUMBER )
			trigger_once = lua_tointeger(L, -1);
		lua_pop(L, 1);	/* Pop the value */

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
		nt->trigger = trigger;
		nt->trigger_once = trigger_once;
		nt->qos = qos;
		nt->watchdog = NULL;
		eclient->subscriptions = nt;
		
		lua_pop(L, 1);	/* Pop the sub-table */
	}

		/* subscribe to topics */
	if(nbre){
		char **tpcs = calloc(nbre, sizeof( char * ));
		int *qos = calloc(nbre, sizeof( int ));
		struct _topic *t = eclient->subscriptions;
		int err;

		assert(tpcs);
		assert(qos);

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
		lua_pushstring(L, "SelMQTT.create() is expecting a table");
		return 2;
	}

	lua_pushstring(L, "KeepAliveInterval");
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
	eclient->L = luaL_newstate();
	luaL_openlibs(eclient->L);
	init_shared_Lua(eclient->L );

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
