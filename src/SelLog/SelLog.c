/***
Logging facility.

If connected to a broker, in addition to the destination set in init() function,
the message is published as well on "clientID/Log/level" topic.

Following level are automatically registered :

- 'F' : "Fatal"
- 'E' : "Error"
- 'W' : "Warning"
- 'T' : "Trace"


@classmod SelLog

 * History :
 * 12/05/2016 LF : First version
 * 19/01/2024 LF : Switch to Selene v7
 */

#include <Selene/SelLog.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLua.h>
#include <Selene/SelMQTT.h>

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h> /*	Vararg */

static struct SelLog selLog;

static struct SeleneCore *selCore;
static struct SelLua *selLua;
static struct SelMQTT *selMQTT;

static pthread_mutex_t sl_mutex;	/* secure concurrency */
static FILE *sl_logfile;			/* file to log to */

static char *sl_LevIgnore;
static enum WhereToLog sl_logto;

static MQTTClient sl_MQTT_client;
static const char *sl_MQTT_ClientID;

	/* Trans codification b/w log level and
	 * topic extension
	 */
struct LevelTransco {
	struct LevelTransco *next;
	const char *ext_topic;
	char level;
} *sl_levtransco;
size_t max_extension;	/* Maximum length of extensions */
size_t topic_root_len;	/* Length of the topic root, \0 included */


static bool slc_checkdependencies(){	/* Ensure all dependancies are met */
	return(!!selMQTT);
}

static bool slc_laterebuilddependancies(){	/* Add missing dependencies */
	selMQTT = (struct SelMQTT *)selCore->findModuleByName("SelMQTT", SELMQTT_VERSION, 0);
	if(!selMQTT){	/* We can live w/o it */
		selLog.Log('D', "SelMQTT missing for SelLog");
	} else {
		if(!selLog.registerTransCo('F', "Fatal") ||
		  !selLog.registerTransCo('E', "Error") ||
		  !selLog.registerTransCo('W', "Warning") ||
		  !selLog.registerTransCo('T', "Trace"))
			selLog.Log('F', "Can't register logging topics");
	}

	return true;
}

#define MAXMSG	1024 /* Maximum message lenght */

static bool slc_Log(const char level, const char *message, ...){
/** 
 * @brief Log a message (C interface)
 *
 * @function Log
 * @tparam string level log level as registered ('**I**' by if 0)
 * @tparam string message text to log
 * @treturn boolean false if we faced a technical error
 */
	time_t tt;
	struct tm tmt;
	va_list args;
	char t[MAXMSG];

	if(sl_LevIgnore && strchr(sl_LevIgnore, level))	/* Do we have to ignore */
		return true;
	
	time(&tt);
	localtime_r(&tt, &tmt);

	snprintf(t, MAXMSG,
		"%04d-%02d-%02d %02d:%02d:%02d - *%c* %s\n",
		tmt.tm_year+1900, tmt.tm_mon+1, tmt.tm_mday,
		tmt.tm_hour, tmt.tm_min, tmt.tm_sec,
		level, message 
	);


	if(sl_logto & LOG_STDOUT){
		va_start(args, message);
		vprintf(t, args);
		va_end(args);
	}

	if(sl_logfile){
		va_start(args, message);
		if( vfprintf( sl_logfile, t, args ) < 0 ){
			va_end(args);
			return false;
		}
		va_end(args);
		fflush(sl_logfile);
	}

	if(selMQTT && sl_MQTT_client && MQTTClient_isConnected(sl_MQTT_client)){
		const char *sub = NULL;

		for(struct LevelTransco *n = sl_levtransco; n; n = n->next){
			if( n->level == level ){
				sub = n->ext_topic;
				break;
			}
		}

		if(!sub)
			sub = "Information";
		
		char ttopic[ topic_root_len + max_extension ];
		sprintf(ttopic, "%s/Log/%s",sl_MQTT_ClientID, sub);

		va_start(args, message);
		vsnprintf(t, MAXMSG, message, args);
		va_end(args);

		selMQTT->mqttpublish(sl_MQTT_client, ttopic, strlen(t), (void *)t, 0);
	}
	
	return true;
}

static void slc_ignoreList(const char *list){
/**
 * @brief Initialise ignore list.
 *
 * Each character in the string correspond to an error level
 * that will be ignored, that is not logged or published.
 *
 * @function ignoreList
 * @tparam string string representing errors level to be ignored (NULL to clear)
 */
	if(sl_LevIgnore){
		free(sl_LevIgnore);
		sl_LevIgnore = NULL;
	}

	if(list)
		sl_LevIgnore = strdup(list);
}

static int sll_log(lua_State *L){
/** 
 * Log a message
 *
 * @function log
 * @tparam string level log level as registered (optional, '**I**' by default)
 * @tparam string message text to log
 * @usage
 * SelLog.log("my informative message")
 * SelLog.log('E', "my error message")
 *
 */
	const char *msg = luaL_checkstring(L, 1);	/* message to log */
	char level = 'I';
	if( lua_type(L, 2) == LUA_TSTRING ){
		level = *msg;
	   	msg = luaL_checkstring(L, 2);	/* loggin level (optional) */
	}

	if( sl_LevIgnore && strchr(sl_LevIgnore, level) )
		return 0;

	if(!slc_Log( level, msg )){
		int en = errno;
		fprintf(stderr, "*E* Log writing : %s\n", strerror(en));
		lua_pushnil(L);
		lua_pushstring(L, strerror(en));
		return 2;
	}
	return 0;
}

bool slc_configure(const char *fn, enum WhereToLog logto){
/** 
 * @brief Initialise logging to file
 *
 * Notez-bien :
 * ---
 *
 * - Whatever the value of *where*, the message is always published if connected to a broker.
 * - depending on logging level, the message is output on *stdout* or *stderr*
 *
 * @function initFile
 * @tparam string filename file to log to
 * @tparam WhereToLog
 * @treturn boolean did we succeed ?
 */
	if(sl_logfile){
		fclose(sl_logfile);
		sl_logfile = NULL;
	}

	if(logto == LOG_DECIDEYOUSELF) /* Log depending on the context */
		sl_logto = isatty(STDOUT_FILENO) ? LOG_STDOUT : LOG_FILE;
	else {	/* if both is provided, log on STDOut only if interractive */
		sl_logto = logto;
		if( sl_logto & LOG_FILE )
			if( !isatty(STDOUT_FILENO) )
				sl_logto &= ~LOG_STDOUT;
	}

	if( sl_logto & LOG_FILE ){
		if(!fn){
			errno = EINVAL;
			return false;
		}
		if(!(sl_logfile = fopen( fn, "a" )))
			return false;
	}
	return true;
}


static int sll_configure( lua_State *L ){
/** 
 * Initialise logging
 *
 * Notez-bien :
 * ---
 *
 * - Whatever the value of *where*, the message is always published if connected to a broker.
 * - depending on logging level, the message is output on *stdout* or *stderr*
 *
 * @function configure
 * @tparam string filename file to log to
 * @tparam boolean where **true** both on *stdout* and file, **false** only on *stdout*, **unset** only on file
 */
	enum WhereToLog csl_logto;
	const char *fn = NULL;

	if(lua_type(L, 2) == LUA_TBOOLEAN) {
		if(lua_toboolean(L, 2)){	/* true, on both if possible */
			csl_logto = LOG_FILE;
			if(isatty(STDOUT_FILENO))
				csl_logto |= LOG_STDOUT;
		} else if(isatty(STDOUT_FILENO))	/* false & interactive */
			csl_logto = LOG_STDOUT;
		else
			csl_logto = LOG_FILE;
	} else
		csl_logto = LOG_FILE;

	if( csl_logto & LOG_FILE )
		fn = luaL_checkstring(L, 1);	/* Name of the log file */

	if(!slc_configure(fn, csl_logto)){
		int en = errno;
		fprintf(stderr, "*E* %s : %s\n", fn, strerror(en));
		lua_pushnil(L);
		lua_pushstring(L, strerror(en));
		return 2;
	}

	return 0;
}

static void slc_initMQTT(MQTTClient aClient, const char *cID){
	sl_MQTT_client = aClient;
	sl_MQTT_ClientID = cID;

	topic_root_len = strlen( sl_MQTT_ClientID ) + 5;	/* strlen( "/log/" + '\0' ) */
}

static int sll_configureMQTT(lua_State *L){
/** 
 * Initialise MQTT logging
 *
 */
	struct enhanced_client *client = selMQTT->checkSelMQTT(L);
	if(lua_type(L, 2) != LUA_TSTRING)
		luaL_error(L, "String expected as second argument");

	slc_initMQTT(client->client, lua_tostring(L, 2));

	return 0;
}

static int sll_register( lua_State *L ){
/** 
 * Register a logging level
 *
 * @function register
 * @tparam string level (only the 1st char is took in account)
 * @tparam string topic corresponding topic extension
 */	const char *ext = luaL_checkstring(L, 1);	/* Level */
	const char level = *ext;
	ext = luaL_checkstring(L, 2);	/* Extension */

	if(!selLog.registerTransCo(level, ext)){
		lua_pushnil(L);
		lua_pushstring(L, "Can't register this log level");
		return 2;
	}
	return 0;
}

static int sll_ignore( lua_State *L ){
/** 
 * Ignore logging levels
 *
 * @function ignore
 * @tparam string levels list of all levels to ignore
 * @usage
 * -- to ignore Trace and Warning
 *  SelLog.ignore("TW")
 */
	const char *ext = luaL_checkstring(L, 1);	/* Level to ignore */

	if(sl_LevIgnore)
		free(sl_LevIgnore);

	sl_LevIgnore = strdup(ext);

	return 0;
}

static int sll_status( lua_State *L ){
/** Return the current status of logging
 *
 * @function status
 * @treturn boolean file logging on file
 * @treturn boolean std logging on STDOUT/STDERR
 */

	if(sl_logfile)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	if(sl_logto & LOG_STDOUT)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	return 2;
}

static const struct luaL_Reg SelLogLib [] = {
	{"Log", sll_log},
	{NULL, NULL}
};

static const struct luaL_Reg SelLogExtLib [] = {
	{"configure", sll_configure},
	{"configureMQTT", sll_configureMQTT},
	{"register", sll_register},
	{"ignore", sll_ignore},
	{"status", sll_status},
	{NULL, NULL}
};

static void registerSelLog(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelLog", SelLogLib);
}

static bool slc_initLua(struct SelLua *aselLua){
	selLua = aselLua;
	selLua->libCreateOrAddFuncs(NULL, "SelLog", SelLogExtLib);
	selLua->libCreateOrAddFuncs(NULL, "SelLog", SelLogLib);
	selLua->AddStartupFunc(registerSelLog);

	return(selLua->module.version >= SELLUA_VERSION);
}

static bool slc_registerTransCo(const char alv, const char *axt){
	struct LevelTransco *n;

		/* Check if it is not already registered */
	for(n = sl_levtransco; n; n = n->next)
		if( n->level == alv )
			return false;
	
	n = malloc( sizeof(struct LevelTransco) );
	if(!n)
		return false;

	n->ext_topic = strdup( axt );
	if(!n->ext_topic){
		free(n);
		return false;
	}
	size_t ts = strlen( axt );
	if(ts > max_extension)
		max_extension = ts;
	
	n->level = alv;

	n->next = sl_levtransco;
	sl_levtransco = n;
	return true;
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule(void){
	selLua = NULL;
	selMQTT = NULL;

	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

		/* Initialise configurations */
	sl_logfile = NULL;
	sl_LevIgnore = NULL;
	sl_logto = LOG_STDOUT;	/* Without initialisation, log to STDOUT */
	sl_MQTT_client = NULL;
	sl_MQTT_ClientID = NULL;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selLog, "SelLog", SELLOG_VERSION, LIBSELENE_VERSION))
		return false;

	selLog.module.initLua = slc_initLua;
	selLog.module.checkdependencies = slc_checkdependencies;
	selLog.module.laterebuilddependancies = slc_laterebuilddependancies;

	selLog.Log = slc_Log;
	selLog.ignoreList = slc_ignoreList;
	selLog.configure = slc_configure;
	selLog.initMQTT = slc_initMQTT;
	selLog.registerTransCo = slc_registerTransCo;

	registerModule((struct SelModule *)&selLog);

	selCore->SelLogInitialised(&selLog);

	pthread_mutex_init( &sl_mutex, NULL );

	return true;
}
