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
#include <stdlib.h>

static pthread_mutex_t sl_mutex;
static FILE *sl_logfile;
static MQTTClient sl_MQTT_client;
static const char *sl_MQTT_ClientID;

static enum WhereToLog sl_logto;

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

bool slc_registerTransCo( const char alv, const char *axt ){
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

	topic_root_len = strlen( sl_MQTT_ClientID ) + 5;	/* strlen( "/log/" + '\0' ) */
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
		mqttpublish( sl_MQTT_client, ttopic, strlen(msg), (void *)msg, 0);
	}

	return true;
}

/* register a new level
 * ->	string 1 : level (only the 1st char is took in account)
 * 		string 2 : corresponding topic extension
 */
static int sl_register( lua_State *L ){
	const char *ext = luaL_checkstring(L, 1);	/* Level */
	const char level = *ext;
	ext = luaL_checkstring(L, 2);	/* Extension */

	if(!slc_registerTransCo(level, ext)){
		lua_pushnil(L);
		lua_pushstring(L, "Can't register this log level");
		return 2;
	}
	return 0;
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
	{"register", sl_register},
	{"log", sl_log},
	{"status", sl_status},
	{NULL, NULL}
};

void initG_SelLog(){
	pthread_mutex_init( &sl_mutex, NULL );
	sl_logfile = NULL;
	sl_MQTT_client = NULL;
	sl_MQTT_ClientID = NULL;

	sl_levtransco = NULL;
	max_extension = 11;	/* strlen("Information") */

	if( !slc_registerTransCo( 'F', "Fatal" ) ||
	  !slc_registerTransCo( 'E', "Error" ) ||
	  !slc_registerTransCo( 'W', "Warning" ) ||
	  !slc_registerTransCo( 'T', "Trace" ) )
		slc_log('F', "Can't register logging topics");
}

int initSelLog( lua_State *L ){
	libSel_libFuncs(L, "SelLog" , SelLogLib);

	return 1;
}


