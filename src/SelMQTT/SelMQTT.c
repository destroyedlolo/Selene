/***
MQTT messaging including connection part.

Have a look on **SeleMQTT** when the connection has to be managed externally.

@classmod SelMQTT

 * History :
 * 30/05/2015 LF : First version
 * 17/06/2015 LF : Add trigger function to topic
 * 11/11/2015 LF : Add TaskOnce enum
 * 21/01/2015 LF : Rename as SelMQTT
 * 11/04/2021 LF : add retained and dupplicate parameters to callback receiving function
 * 03/02/2024 LF : Switch to Selene v7
 
 */

#include <Selene/SelMQTT.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>
#include <Selene/SelMultitasking.h>
#include <Selene/SelElasticStorage.h>
#include <Selene/SelSharedVar.h>
#include <Selene/SelTimer.h>
#include <Selene/SelSharedFunction.h>
#include <Selene/SelScripting.h>
#include "../SelSharedFunction/SelSharedFunctionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct SelMQTT selMQTT;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;
static struct SelMultitasking *selMultitasking;
static struct SelScripting *selScripting;
struct SelElasticStorage *selElasticStorage;
struct SelSharedVar *selSharedVar;
struct SelTimer *selTimer;

static bool sqc_checkdependencies(){	/* Ensure all dependancies are met */
	return(!!selTimer);
}

static bool sqc_laterebuilddependancies(){	/* Add missing dependencies */
	selTimer = (struct SelTimer *)selCore->findModuleByName("SelTimer", SELTIMER_VERSION, 0);
	if(!selTimer){	/* We can live w/o it */
		selLog->Log('D', "SelTimer missing for SelMQTT");
	}

	return true;
}

static struct enhanced_client *checkSelMQTT(lua_State *L){
	void *r = selLua->testudata(L, 1, "SelMQTT");
	luaL_argcheck(L, r != NULL, 1, "'SelMQTT' expected");
	return (struct enhanced_client *)r;
}

/*
 * Subscription's related information
 */
struct _topic {
	struct _topic *next;	/* Link to next topic */
	char *topic;			/* Subscribed topic */
	int qos;				/* QoS associated to this topic */
	struct selTimerStorage *watchdog;	/* Watchdog on document arrival */
	struct elastic_storage *func;	/* Arrival callback function (run in dedicated context) */
	int trigger;			/* application side trigger function */
	enum TaskOnce trigger_once;	/* Avoid duplicates in waiting list */
};

static int sqc_mqttpublish(MQTTClient client, const char *topic, int length, void *payload, int retained){
/**
 * @brief Publish a message to a given topic.
 *
 * @function mqttpublish
 * @tparam MQTTClient MQTT client handle
 * @tparam string Topic to publish to
 * @tparam int payload length
 * @tparam void * payload to publish
 * @tparam int true if the document is retained
 * @return result of the publishing (see MQTTClient_publishMessage)
 */
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.retained = retained;
	pubmsg.payloadlen = length;
	pubmsg.payload = payload;

	return MQTTClient_publishMessage(client, topic, &pubmsg, NULL);
}

static int sqc_mqtttokcmp(register const char *s, register const char *t){
/**
 * @brief Compares a topic talking in consideration MQTT wildcard
 *
 * @function mqttpublish
 * @tparam string reference to compare to
 * @tparam string topic to be compared
 * @return 0 if identical
 */

	char last = 0;
	if(!s || !t)
		return -1;

	for(; ; s++, t++){
		if(!*s){ /* End of string */
			return(*s - *t);
		} else if(*s == '#') /* ignore remaining of the string */
			return (!*++s && (!last || last=='/')) ? 0 : -1;
		else if(*s == '+'){	/* ignore current level of the hierarchy */
			s++;
			if((last !='/' && last ) || (*s != '/' && *s )) /* has to be enclosed by '/' */
				return -1;
			while(*t != '/'){	/* looking for the closing '/' */
				if(!*t)
					return 0;
				t++;
			}
			if(*t != *s)	/* ensure the s isn't finished */
				return -1;
		} else if(*s != *t)
			break;
		last = *s;
	}
	return(*s - *t);
}

static const struct ConstTranscode _QoS[] = {
	{ "QoS0", 0 },
	{ "QoS1", 1 },
	{ "QoS2", 2 },
	{ NULL, 0 }
};

static int sql_QoSConst(lua_State *L){
	return selLua->findConst(L, _QoS);
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

static int sql_ErrCodeConst( lua_State *L ){
	return selLua->findConst(L, _ErrCode);
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

static int sql_StrError( lua_State *L ){
	return selLua->rfindConst(L, _strErrCode);
}

static const char *sqc_StrError( int arg ){
	int i;
	for(i=0; _strErrCode[i].name; i++){
		if( arg == _strErrCode[i].value ){
			return _strErrCode[i].name;
		}
	}

	return "Unknown error";
}

static int sql_publish(lua_State *L){
/**
 * @brief Publish to a topic
 *
 * @function Publish
 * @tparam string topic to publish to
 * @tparam string value to publish
 * @tparam number retain
 */
 
	struct enhanced_client *eclient = checkSelMQTT(L);

	if(!eclient){
		lua_pushnil(L);
		lua_pushstring(L, "publish() to a dead object");
		return 2;
	}

	const char *topic = luaL_checkstring(L, 2),
				*val = luaL_checkstring(L, 3);
	int retain =  lua_toboolean(L, 4);

	selMQTT.mqttpublish(eclient->client, topic, strlen(val), (void *)val, retain);

	return 0;
}

static const struct luaL_Reg SelMQTTLib [] = {
	{"QoSConst", sql_QoSConst},
	{"ErrConst", sql_ErrCodeConst},
	{"StrError", sql_StrError},
	{NULL, NULL}
};

static const struct luaL_Reg SelMQTTtM [] = {	/* Apply on all threads */
	{"Publish", sql_publish},
	{NULL, NULL}
};

static void sqc_connlost(void *actx, char *cause){
	struct enhanced_client *ctx = (struct enhanced_client *)actx;	/* Avoid casting */

	if(ctx->onDisconnectFunc){
		lua_State *tstate = selMultitasking->createSlaveState();
		lua_pushstring( tstate, cause ? cause : "????");	/* Push cause message */
		selMultitasking->loadandlaunch(NULL, tstate, ctx->onDisconnectFunc, 1, 0, LUA_REFNIL, TO_MULTIPLE);
	}

		/* Unlike for message arrival, a trigger is pushed
		 * unconditionally.
		 */
	if(ctx->onDisconnectTrig != LUA_REFNIL && selScripting)
		selScripting->pushtask( ctx->onDisconnectTrig, TO_MULTIPLE );
}

static int sqc_msgarrived(void *actx, char *topic, int tlen, MQTTClient_message *msg){
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
	selLog->Log('D', "topic : %s", topic);
#endif

	for(tp = ctx->subscriptions; tp; tp = tp->next){	/* Looks for the corresponding function */

		if(!sqc_mqtttokcmp(tp->topic, topic)){
			if(tp->func){	/* Call back function defined */
				lua_State *tstate = selMultitasking->createSlaveState();

					/* Push arguments */
				lua_pushstring(tstate, topic);			/* 1: topic */
				lua_pushstring(tstate, cpayload);			/* 2: payload */
				lua_pushboolean(tstate, msg->retained);	/* 3: Retained */
				lua_pushboolean(tstate, msg->dup);		/* 4: duplicated message */

				selMultitasking->loadandlaunch(NULL, tstate, tp->func, 4, 1, tp->trigger, tp->trigger);
			} else {
				/* No call back : set a shared variable
				 * and unconditionally push a trigger if it exists
				 */
				selSharedVar->setString(topic, cpayload, 0);
				if(tp->trigger != LUA_REFNIL && selScripting)	/* Push trigger function if defined */
					selScripting->pushtask(tp->trigger, tp->trigger_once);
			}

			if(tp->watchdog){
				selLog->Log('D', "Resetting");
				selTimer->reset(tp->watchdog); /* Reset the wathdog : data arrived on time */
			}
		}
	}

	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

static int sql_connect(lua_State *L){
/** Connect to a broker
 *
 * @function Connect
 * @tparam string broker url
 * @tparam table Connect_arguments
 *	@see Connect_arguments
 */
/**
 *	Arguments for @{Connect}
 *
 *	@table Connect_arguments
 *	@field KeepAliveInterval
 *	@field cleansession
 *	@field reliable
 *	@field persistence
 *	@field username
 *	@field password
 *	@field clientID obviously, must be uniq
 *	@field OnDisconnect function to be called when disconnected (*CAUTION* : runing in its own thread)
 *	@field OnDisconnectTrigger trigger to be added in the todo list when disconnected
 */

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	const char *host = luaL_checkstring(L, 1);	/* Host to connect to */
	const char *clientID = "Selene";
	const char *persistence = NULL;
	const char *err = NULL;
	struct enhanced_client *eclient;
	struct elastic_storage *onDisconnectFunc = NULL;
	int OnDisconnectTrig = LUA_REFNIL;

		/* Read arguments */
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

		/*
		 * Function to be called in case of broker disconnect
		 * CAUTION : this function is called in a dedicated context
		 */
#if 0 /* AF */
	struct elastic_storage **r;
#endif
	lua_pushstring(L, "OnDisconnect");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TFUNCTION){
		assert( (onDisconnectFunc = malloc( sizeof(struct elastic_storage) )) );
		assert( selElasticStorage->init(onDisconnectFunc) );

		if(lua_dump(L, selMultitasking->dumpwriter, onDisconnectFunc 
#if LUA_VERSION_NUM > 501
			,1
#endif
		) != 0){
			selElasticStorage->free(onDisconnectFunc);
			return luaL_error(L, "unable to dump given function");
		}
	}
#if 0 /* AF */
	else if( (r = selLua->testudata(L, -1, "SelSharedFunc")) )
		onDisconnectFunc = *r;
#endif
	lua_pop(L, 1);	/* Pop the unused result */

	lua_pushstring(L, "OnDisconnectTrigger");
	lua_gettable(L, -2);
	if(lua_type(L, -1) != LUA_TFUNCTION)	/* This function is optional */
		lua_pop(L, 1);	/* Pop the unused result */
	else {
		if(selScripting)
			OnDisconnectTrig = selScripting->findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
		lua_pop(L,1);
	}


		/* Creating Lua data */
	eclient = (struct enhanced_client *)lua_newuserdata(L, sizeof(struct enhanced_client));
	luaL_getmetatable(L, "SelMQTT");
	lua_setmetatable(L, -2);
	eclient->subscriptions = NULL;
	eclient->onDisconnectFunc = onDisconnectFunc;
	eclient->onDisconnectTrig = OnDisconnectTrig;

		/* Connecting */
	int nerr;
	if((nerr = MQTTClient_create( &(eclient->client), host, clientID, persistence ? MQTTCLIENT_PERSISTENCE_DEFAULT : MQTTCLIENT_PERSISTENCE_NONE, (void *)persistence )) != MQTTCLIENT_SUCCESS)
		err = "Fail to create";
	else {
		if((nerr = MQTTClient_setCallbacks( eclient->client, eclient, sqc_connlost, sqc_msgarrived, NULL)) != MQTTCLIENT_SUCCESS)
			err = "Fail to set Callbacks";
		else switch( (nerr = MQTTClient_connect( eclient->client, &conn_opts)) ){
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
	}

	if(err){
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushstring(L, err);
		lua_pushnumber(L, nerr);
		return 3;
	}

	return 1;
}

void sqc_createExternallyManaged(lua_State *L, MQTTClient client){
/** 
 * @brief create an SelMQTT from an externally managed client
 *
 * @function sqc_createExternallyManaged
 * @tparam lua_State *L Lua state to create the object to
 * @tparam MQTTClient client
 */
	struct enhanced_client *eclient = (struct enhanced_client *)lua_newuserdata(L, sizeof(struct enhanced_client));
	luaL_getmetatable(L, "SelMQTT");
	lua_setmetatable(L, -2);

	eclient->client = client;
	eclient->subscriptions = NULL;
	eclient->onDisconnectFunc = NULL;
	eclient->onDisconnectTrig = LUA_REFNIL;

	selCore->initObject((struct SelModule *)&selMQTT, (struct SelObject *)eclient);
}

static const struct luaL_Reg SelMQTTExtLib [] = {
	{"Connect", sql_connect},
	{NULL, NULL}
};

static int sql_subscribe(lua_State *L){
/** 
 * @brief Subscribe to topics
 *
 * @function Subscribe
 * @tparam table Subscribe_arguments
 *	@see Subscribe_arguments
 */
/**
 *	Arguments for @{Subscribe}
 *
 *	@table Subscribe_arguments
 *	@field topic topic name to subscribe
 *	@field func function to call when a message arrive (run in a dedicated thread)
 *	@field trigger function to be added in the todo list
 *	@field trigger_once if true, the function is only added if not already in the todo list
 *	@field qos as the name said, default 0
 *	@field watchdog **SelTimer** watchdog timer, the timer is reset every time a message arrives
 */
	struct enhanced_client *eclient = checkSelMQTT(L);
	int nbre;	/* nbre of topics */
	struct _topic *nt;

	if(eclient->subscriptions)
		return luaL_error(L, "Sorry but for the moment, multi pass subscription is not supported");

	if(!eclient){
		lua_pushnil(L);
		lua_pushstring(L, "subscribe() to a dead object");
		return 2;
	}

	if(lua_type(L, -1) != LUA_TTABLE){
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
		struct elastic_storage **r;

		int qos = 0;
		struct elastic_storage *func = NULL;
		int trigger = LUA_REFNIL;
		enum TaskOnce trigger_once = TO_ONCE;
		struct selTimerStorage *watchdog = NULL;

		lua_pushstring(L, "topic");
		lua_gettable(L, -2);
		assert( (topic = strdup( luaL_checkstring(L, -1) )) );
		lua_pop(L, 1);	/* Pop topic */

		lua_pushstring(L, "func");
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TFUNCTION){
			assert( (func = malloc( sizeof(struct elastic_storage) )) );
			assert( selElasticStorage->init(func) );

			if(lua_dump(L, selMultitasking->dumpwriter, func 
#if LUA_VERSION_NUM > 501
				,1
#endif
			) != 0){
				selElasticStorage->free( func );
				return luaL_error(L, "unable to dump given function");
			}
		} else if( (r = selLua->testudata(L, -1, "SelSharedFunction")) )
			func = &(*(struct SelSharedFunctionStorage **)r)->estorage;
		lua_pop(L, 1);	/* Pop the unused result */

			/* triggers are part of the main thread and pushed in TODO list.
			 * Consequently, they are kept in functions lookup reference table
			 */

		lua_pushstring(L, "trigger");
		lua_gettable(L, -2);
		if( lua_type(L, -1) != LUA_TFUNCTION )	/* This function is optional */
			lua_pop(L, 1);	/* Pop the unused result */
		else {
			if(selScripting)
				trigger = selScripting->findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
			lua_pop(L,1);
		}

		
		lua_pushstring(L, "trigger_once");
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TBOOLEAN)
			trigger_once = lua_toboolean(L, -1) ? TO_ONCE : TO_MULTIPLE;
		else if(lua_type(L, -1) == LUA_TNUMBER)
			trigger_once = lua_tointeger(L, -1);
		lua_pop(L, 1);	/* Pop the value */

		lua_pushstring(L, "qos");
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER)
			qos = lua_tointeger(L, -1);
		lua_pop(L, 1);	/* Pop the QoS */

		lua_pushstring(L, "watchdog");
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TUSERDATA){
			if(!selTimer)
				return luaL_error(L, "SelTimer not loaded");
			watchdog = *(struct selTimerStorage **)luaL_checkudata(L, -1, "SelTimer");
		}
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
		int i;

		assert(tpcs);
		assert(qos);

		for(i=0; i < nbre; i++){
			assert( t );	/* If failing, it means an error in the code above */
			tpcs[i] = t->topic;
			qos[i] = t->qos;

			t = t->next;
		}
		if((err = MQTTClient_subscribeMany( eclient->client, nbre, tpcs, qos)) != MQTTCLIENT_SUCCESS){
			lua_pushnil(L);
			lua_pushstring(L, sqc_StrError(err));
			return 2;
		}
	}

	return 0;
}

static const struct luaL_Reg SelMQTTM [] = {
	{"Subscribe", sql_subscribe},
	{NULL, NULL}
};

static void registerSelMQTT(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelMQTT", SelMQTTLib);
	selLua->objFuncs(L, "SelMQTT", SelMQTTtM);
}


/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	uint16_t found;

		/* Core modules */
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;

		/* Other mandatory modules */
	selElasticStorage = (struct SelElasticStorage *)selCore->loadModule("SelElasticStorage", SELELASTIC_STORAGE_VERSION, &found, 'F');
	if(!selElasticStorage)
		return false;
	
	selMultitasking = (struct SelMultitasking *)selCore->loadModule("SelMultitasking", SELMULTITASKING_VERSION, &found, 'F');
	if(!selMultitasking)
		return false;

	selSharedVar = (struct SelSharedVar *)selCore->loadModule("SelSharedVar", SELSHAREDVAR_VERSION, &found, 'F');
	if(!selSharedVar)
		return false;

		/* optional modules */
	selScripting =  (struct SelScripting *)selCore->findModuleByName("SelScripting", SELSCRIPTING_VERSION,0);

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selMQTT, "SelMQTT", SELMQTT_VERSION, LIBSELENE_VERSION))
		return false;

	selMQTT.module.checkdependencies = sqc_checkdependencies;
	selMQTT.module.laterebuilddependancies = sqc_laterebuilddependancies;

	selMQTT.mqttpublish = sqc_mqttpublish;
	selMQTT.mqtttokcmp = sqc_mqtttokcmp;
	selMQTT.checkSelMQTT = checkSelMQTT;
	selMQTT.createExternallyManaged = sqc_createExternallyManaged;

	registerModule((struct SelModule *)&selMQTT);

	selLua->libCreateOrAddFuncs(NULL, "SelMQTT", SelMQTTLib);
	selLua->libCreateOrAddFuncs(NULL, "SelMQTT", SelMQTTExtLib);
	selLua->objFuncs(NULL, "SelMQTT", SelMQTTtM);
	selLua->objFuncs(NULL, "SelMQTT", SelMQTTM);

	selLua->AddStartupFunc(registerSelMQTT);

	return true;
}
