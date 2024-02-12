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

#include "Selene/SelMQTT.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"
#include "Selene/SelLua.h"

static struct SelMQTT selMQTT;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static int smc_mqttpublish(MQTTClient client, const char *topic, int length, void *payload, int retained){
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

static int smc_mqtttokcmp(register const char *s, register const char *t){
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

int sql_StrError( lua_State *L ){
	return selLua->rfindConst(L, _strErrCode);
}


static const struct luaL_Reg SelMQTTLib [] = {
	{"QoSConst", sql_QoSConst},
	{"ErrConst", sql_ErrCodeConst},
	{"StrError", sql_StrError},
#if 0
	{"Connect", smq_connect},
#ifdef COMPATIBILITY
	{"connect", smq_connect},
#endif
#endif
	{NULL, NULL}
};

static bool smc_initLua(){
	selLua->libFuncs(NULL, "SelMQTT", SelMQTTLib);
	return true;
}


/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;

	/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selMQTT, "SelMQTT", SELMQTT_VERSION, LIBSELENE_VERSION))
		return false;

	selMQTT.module.initLua = smc_initLua;

	selMQTT.mqttpublish = smc_mqttpublish;
	selMQTT.mqtttokcmp = smc_mqtttokcmp;

	registerModule((struct SelModule *)&selMQTT);

	return true;
}
