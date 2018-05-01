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

#ifdef USE_MQTT
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#include "libSelene.h"
#include "MQTT_tools.h"
#include "SelShared.h"
#include "SelFIFO.h"
#include "SelTimer.h"

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
	int qos;				/**< QoS associated to this topic */
	struct SelTimer *watchdog;	/**< Watchdog on document arrival */
	struct elastic_storage *func;	/**< Arrival callback function (run in dedicated context) */
	int trigger;			/**< application side trigger function */
	enum TaskOnce trigger_once;	/**< Avoid duplicates in waiting list */
};

/** 
 * \struct enhanced_client
 * \brief Broker client's context
 */
struct enhanced_client {
	MQTTClient client;	/**< Paho's client handle */
	struct _topic *subscriptions;	/**< Linked list of subscription */
#ifdef NOT_YET
	int onDisconnectFunc;	/**< Function called in case of disconnection with the broker */
#endif
	int onDisconnectTrig;	/**< Triggercalled in case of disconnection with the broker */
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
			if(tp->func){	/* Call back function defined */
				lua_State *tstate = createslavethread();

					/* Push arguments */
				lua_pushstring( tstate, topic);
				lua_pushstring( tstate, cpayload);

				loadandlaunch(NULL, tstate, tp->func, 2, 1, tp->trigger, tp->trigger);
			} else {
				/* No call back : set a shared variable
				 * and unconditionally push a trigger if it exists
				 */
				soc_sets( topic, cpayload );
				if(tp->trigger != LUA_REFNIL)
					pushtask( tp->trigger, tp->trigger_once );
			}

			if(tp->watchdog)
				_TimerReset( tp->watchdog ); /* Reset the wathdog : data arrived on time */
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

void connlost(void *actx, char *cause){
	struct enhanced_client *ctx = (struct enhanced_client *)actx;	/* Avoid casting */

#ifdef NOT_YET
	if(ctx->onDisconnectFunc != LUA_REFNIL){
		pthread_mutex_lock( &ctx->access_ctrl );
		lua_rawgeti( ctx->L, LUA_REGISTRYINDEX, ctx->onDisconnectFunc);	/* retrieves the function */
		lua_pushstring( ctx->L, cause ? cause : "????" );	/* Push the cause of the disconnect */
		if(lua_pcall( ctx->L, 1, 0, 0)){	/* Call Lua callback function */
			fprintf(stderr, "*E* (Broker disconnect callback) %s\n", lua_tostring(ctx->L, -1));
			lua_pop(ctx->L, 2); /* pop error message and NIL from the stack */
		}
		pthread_mutex_unlock( &ctx->access_ctrl );
	}
#endif

	if(ctx->onDisconnectTrig != LUA_REFNIL)
		pushtask( ctx->onDisconnectTrig, 0 );
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
#if LUA_VERSION_NUM > 501
	nbre = lua_rawlen(L, -1);
#else
	nbre = lua_objlen(L, -1);	/* nbre of entries in the table */
#endif
		/* Walking thru arguments */

	lua_pushnil(L);
	while(lua_next(L, -2) != 0){
		char *topic;

		int qos = 0;
		struct elastic_storage *func = NULL;
		int trigger = LUA_REFNIL;
		enum TaskOnce trigger_once = TO_ONCE;
		struct SelTimer *watchdog = NULL;

		lua_pushstring(L, "topic");
		lua_gettable(L, -2);
		assert( (topic = strdup( luaL_checkstring(L, -1) )) );
		lua_pop(L, 1);	/* Pop topic */

		lua_pushstring(L, "func");
		lua_gettable(L, -2);
		if( lua_type(L, -1) == LUA_TFUNCTION ){
			assert( (func = malloc( sizeof(struct elastic_storage) )) );
			assert( EStorage_init(func) );

			if(lua_dump(L, ssf_dumpwriter, func 
#if LUA_VERSION_NUM > 501
				,1
#endif
			) != 0){
				EStorage_free( func );
				return luaL_error(L, "unable to dump given function");
			}
			lua_pop(L,1);	/* remove the function from the stack */
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

		lua_pushstring(L, "watchdog");
		lua_gettable(L, -2);
		if( lua_type(L, -1) == LUA_TUSERDATA )
			watchdog = luaL_checkudata(L, -1, "SelTimer");
		lua_pop(L, 1);	/* Pop the watchdog */

			/* Allocating the new topic */
		assert( (nt = malloc(sizeof(struct _topic))) );
		nt->next = eclient->subscriptions;
		nt->topic = topic;
		nt->qos = qos;
		nt->watchdog = watchdog;
		nt->func = func;
		nt->trigger = trigger;
		nt->trigger_once = trigger_once;
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

static int smq_publish(lua_State *L){
/* Publish to a topic
 *	1 : topic
 *	2 : valeur
 *	3: retain
 */
	struct enhanced_client *eclient = checkSelMQTT(L);

	if(!eclient){
		lua_pushnil(L);
		lua_pushstring(L, "subscribe() to a dead object");
		return 2;
	}

	const char *topic = luaL_checkstring(L, 2),
				*val = luaL_checkstring(L, 3);
	int retain =  lua_toboolean(L, 4);

	mqttpublish( eclient->client, topic, strlen(val), (void *)val, retain );

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
#ifdef NOT_YET
	int onDisconnectFunc = LUA_REFNIL;
#endif
	int OnDisconnectTrig = LUA_REFNIL;
	lua_State *brk_L;	/* Lua stats for this broker client */

		/* initialize the broker's own state
		 * It is used mostly to store functions' references
		 * but we have to correctly initialize it as well as
		 * it may be used at broker connection loss
		 */
	brk_L = luaL_newstate();
	luaL_openlibs( brk_L );
	initSelShared( brk_L );
	initSelFIFO( brk_L );

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "SelMQTT.connect() is expecting a table");
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

		/**
		 * Function to be called in case of broker disconnect
		 * CAUTION : this function is called in a dedicated context
		 */
#ifdef NOT_YET
	lua_pushstring(L, "OnDisconnect");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TFUNCTION ){
		lua_xmove( L, brk_L, 1 );	/* Move the function to the callback's stack */
		onDisconnectFunc = luaL_ref(brk_L,LUA_REGISTRYINDEX);	/* Reference the function in callbacks' context */
	} else
		lua_pop(L, 1);	/* cleaning ... */
#endif

	lua_pushstring(L, "OnDisconnectTrigger");
	lua_gettable(L, -2);
		if( lua_type(L, -1) != LUA_TFUNCTION )	/* This function is optional */
			lua_pop(L, 1);	/* Pop the unused result */
		else {
			OnDisconnectTrig = findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
			lua_pop(L,1);
		}


		/* Creating Lua data */
	eclient = (struct enhanced_client *)lua_newuserdata(L, sizeof(struct enhanced_client));
	luaL_getmetatable(L, "SelMQTT");
	lua_setmetatable(L, -2);
	eclient->subscriptions = NULL;
#ifdef NOT_YET
	eclient->L = brk_L;
	pthread_mutex_init( &eclient->access_ctrl, NULL);
	eclient->onDisconnectFunc = onDisconnectFunc;
#endif
	eclient->onDisconnectTrig = OnDisconnectTrig;

		/* Connecting */
	MQTTClient_create( (void *)eclient, host, clientID, persistence ? MQTTCLIENT_PERSISTENCE_DEFAULT : MQTTCLIENT_PERSISTENCE_NONE, (void *)persistence );
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

static const struct luaL_Reg SelMQTTLib [] = {
	{"Connect", smq_connect},
#ifdef COMPATIBILITY
	{"connect", smq_connect},
#endif
	{"QoSConst", smq_QoSConst},
	{"ErrConst", smq_ErrCodeConst},
	{"StrError", smq_StrError},
	{NULL, NULL}
};

static const struct luaL_Reg SelMQTTM [] = {
	{"Subscribe", smq_subscribe},
#ifdef COMPATIBILITY
	{"subscribe", smq_subscribe},
#endif
	{"Publish", smq_publish},
	{NULL, NULL}
};

int initSelMQTT(lua_State *L){
	libSel_objFuncs( L, "SelMQTT", SelMQTTM);
	libSel_libFuncs( L, "SelMQTT", SelMQTTLib );

	return 1;
}
#endif
