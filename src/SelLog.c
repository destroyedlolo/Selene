/* SelLog.c
 *
 * Loging facility
 *
 * 12/05/2016 LF : First version
 */

#include "SelLog.h"

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static pthread_mutex_t log_mutex;
static FILE *logfile;

static int sl_init( lua_State *L ){
/* Open the log file in append mode
 * -> file name
 */
	const char *fn = luaL_checkstring(L, 1);	/* Name of the log file */

	if(!(logfile = fopen( fn, "a" ))){
		int en = errno;
		fprintf(stderr, "*E* %s : %s\n", fn, strerror(en));
		lua_pushnil(L);
		lua_pushstring(L, strerror(en));
		return 2;
	}
	return 0;
}

static int sl_log( lua_State *L ){
	const char *msg = luaL_checkstring(L, 1);	/* message to log */
	time_t tt;
	struct tm *tmt;

	if(!logfile)
		return 0;

	pthread_mutex_lock( &log_mutex );
	time( &tt );
	tmt = localtime( &tt );	/* WARNING : C99 doesn't support _r : So we are obliged to use non-threadsafe version :( */

	if( fprintf( logfile, "%04d-%02d-%02d %02d:%02d:%02d - %s\n", 
			tmt->tm_year+1900, tmt->tm_mon+1, tmt->tm_mday,
			tmt->tm_hour, tmt->tm_min, tmt->tm_sec,
	msg ) < 0 ){
		int en = errno;
		fprintf(stderr, "*E* Log writing : %s\n", strerror(en));
		lua_pushnil(L);
		lua_pushstring(L, strerror(en));
		pthread_mutex_unlock( &log_mutex );
		return 2;
	}

	pthread_mutex_unlock( &log_mutex );
	return 0;
}

static const struct luaL_reg SelLogLib [] = {
	{"init", sl_init},
	{"log", sl_log},
	{NULL, NULL}
};

void init_log( lua_State *L ){
	luaL_register(L, "SelLog" , SelLogLib);

	logfile = NULL;
	pthread_mutex_init( &log_mutex, NULL );
}

