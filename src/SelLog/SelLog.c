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

static pthread_mutex_t sl_mutex;	/* secure concurrency */
static FILE *sl_logfile;			/* file to log to */

static char *sl_LevIgnore;
static enum WhereToLog sl_logto;

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
 * @function init
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
/*
	{"register", sl_register},
*/
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

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule(void){
	selLua = NULL;

	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

		/* Initialise configurations */
	sl_logfile = NULL;
	sl_LevIgnore = NULL;
	sl_logto = LOG_STDOUT;	/* Without initialisation, log to STDOUT */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selLog, "SelLog", SELLOG_VERSION, LIBSELENE_VERSION))
		return false;

	selLog.module.initLua = slc_initLua;

	selLog.Log = slc_Log;
	selLog.ignoreList = slc_ignoreList;
	selLog.configure = slc_configure;

	registerModule((struct SelModule *)&selLog);

	selCore->SelLogInitialised(&selLog);

	pthread_mutex_init( &sl_mutex, NULL );

	return true;
}
