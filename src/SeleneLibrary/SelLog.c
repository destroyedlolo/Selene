/* SelLog.c
 *
 * Loging facility
 *
 * 12/05/2016 LF : First version
 */

#include "libSelene.h"
#include "internal.h"

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t sl_mutex;
static FILE *sl_logfile;
static MQTTClient sl_MQTT_client;
static const char *sl_MQTT_ClientID;

static enum WhereToLog sl_logto;

bool slc_init( const char *fn, enum WhereToLog asl_logto ){
	if(sl_logfile){
		fclose(sl_logfile);
		sl_logfile = NULL;
	}

	if(!asl_logto) /* Log depending on the context */
		sl_logto = isatty(STDOUT_FILENO) ? LOG_STDOUT : LOG_FILE;
	else {	/* if both is provided, log on STDOut only if interractive */
		sl_logto = asl_logto;
		if( sl_logto & LOG_FILE )
			if( !isatty(STDOUT_FILENO) )
				sl_logto &= ~LOG_STDOUT;
	}

	if( sl_logto & LOG_FILE ){
		if(!(sl_logfile = fopen( fn, "a" )))
			return false;
	}
	return true;
}

void slc_initMQTT( MQTTClient aClient, const char *cID ){
	sl_MQTT_client = aClient;
	sl_MQTT_ClientID = cID;
}

static int sl_init( lua_State *L ){
/* Open the log file in append mode
 * -> file name
 * -> both : 
 *  	if unset, log only on file
 *  	if false, if launched from an interactive shell, log only on stdout
 *  	if true, if launched from an interactive shell, log both on file and on stdout
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

	if(!slc_init(fn, csl_logto)){
		int en = errno;
		fprintf(stderr, "*E* %s : %s\n", fn, strerror(en));
		lua_pushnil(L);
		lua_pushstring(L, strerror(en));
		return 2;
	}

	return 0;
}

bool slc_log( const char level, const char *msg){
	time_t tt;
	struct tm *tmt;

	pthread_mutex_lock( &sl_mutex );
	time( &tt );
	tmt = localtime( &tt );	/* WARNING : C99 doesn't support _r : So we are obliged to use non-threadsafe version :( */

	if(sl_logto & LOG_STDOUT){
		fprintf( 
			(level=='E' || level=='F') ? stderr : stdout,
			"%04d-%02d-%02d %02d:%02d:%02d - *%c* %s\n", 
			tmt->tm_year+1900, tmt->tm_mon+1, tmt->tm_mday,
			tmt->tm_hour, tmt->tm_min, tmt->tm_sec,
		level, msg );
	}

	if(sl_logfile){
		if( fprintf( sl_logfile, "%04d-%02d-%02d %02d:%02d:%02d - *%c* %s\n", 
			tmt->tm_year+1900, tmt->tm_mon+1, tmt->tm_mday,
			tmt->tm_hour, tmt->tm_min, tmt->tm_sec,
		level, msg ) < 0 ){
			pthread_mutex_unlock( &sl_mutex );
			return false;
		}
		fflush(sl_logfile);
	}
	pthread_mutex_unlock( &sl_mutex );

	if(sl_MQTT_client && MQTTClient_isConnected(sl_MQTT_client)){
		char *sub;
		switch(level){
		case 'F':
			sub = "/Log/Fatal";
			break;
		case 'E':
			sub = "/Log/Error";
			break;
		case 'W':
			sub = "/Log/Warning";
			break;
		default :
			sub = "/Log/Information";
		}

		char ttopic[ strlen(sl_MQTT_ClientID) + strlen(sub) + 1 ];
		sprintf(ttopic, "%s%s",sl_MQTT_ClientID, sub);
		mqttpublish( sl_MQTT_client, ttopic, strlen(msg), (void *)msg, 0);
	}

	return true;
}

/* Log some information depending on current configuration on
 * . stdout/err depending on the criticality
 * . MQTT topics
 * . log file
 *
 * -> if only 1 argument : the string to log.
 * -> if 2 arguments :
 *  	- string 1 : level to log ('I', 'W', 'E', 'F')
 *  	- string 2 : the message to log
 */
static int sl_log( lua_State *L ){
	const char *msg = luaL_checkstring(L, 1);	/* message to log */
	char level = 'I';
	if( lua_type(L, 2) == LUA_TSTRING ){
		level = *msg;
	   	msg = luaL_checkstring(L, 2);	/* loggin level (optional) */
	}

	if(!slc_log( level, msg )){
		int en = errno;
		fprintf(stderr, "*E* Log writing : %s\n", strerror(en));
		lua_pushnil(L);
		lua_pushstring(L, strerror(en));
		return 2;
	}
	return 0;
}

static int sl_status( lua_State *L ){
/* Return the current status of logging
 * <- bool : logging on file
 * <- bool : logging on STDOUT
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
	{"init", sl_init},
	{"log", sl_log},
	{"status", sl_status},
	{NULL, NULL}
};

void initG_SelLog(){
	pthread_mutex_init( &sl_mutex, NULL );
	sl_logfile = NULL;
	sl_MQTT_client = NULL;
	sl_MQTT_ClientID = NULL;
}

int initSelLog( lua_State *L ){
	libSel_libFuncs(L, "SelLog" , SelLogLib);

	return 1;
}


