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

static pthread_mutex_t log_mutex;
static FILE *logfile;

#define LOG_FILE 1
#define LOG_STDOUT 2
static char logto;

static int sl_init( lua_State *L ){
/* Open the log file in append mode
 * -> file name
 * -> both : 
 *  	if unset, log only on file
 *  	if false, if launched from an interactive shell, log only on stdout
 *  	if true, if launched from an interactive shell, log both on file and on stdout
 */

	if(logfile){
		fclose(logfile);
		logfile = NULL;
	}

	if(lua_type(L, 2) == LUA_TBOOLEAN) {
		if(lua_toboolean(L, 2)){	/* true, on both if possible */
			logto = LOG_FILE;
			if(isatty(STDOUT_FILENO))
				logto |= LOG_STDOUT;
		} else if(isatty(STDOUT_FILENO))	/* false & interactive */
			logto = LOG_STDOUT;
		else
			logto = LOG_FILE;
	} else
		logto = LOG_FILE;

	if( logto & LOG_FILE ){
		const char *fn = luaL_checkstring(L, 1);	/* Name of the log file */

		if(!(logfile = fopen( fn, "a" ))){
			int en = errno;
			fprintf(stderr, "*E* %s : %s\n", fn, strerror(en));
			lua_pushnil(L);
			lua_pushstring(L, strerror(en));
			return 2;
		}
	}

	return 0;
}

static int sl_log( lua_State *L ){
	const char *msg = luaL_checkstring(L, 1);	/* message to log */
	time_t tt;
	struct tm *tmt;

	pthread_mutex_lock( &log_mutex );
	time( &tt );
	tmt = localtime( &tt );	/* WARNING : C99 doesn't support _r : So we are obliged to use non-threadsafe version :( */

	if(logto & LOG_STDOUT){
		printf( "%04d-%02d-%02d %02d:%02d:%02d - %s\n", 
			tmt->tm_year+1900, tmt->tm_mon+1, tmt->tm_mday,
			tmt->tm_hour, tmt->tm_min, tmt->tm_sec,
		msg );
	}

	if(logfile){
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
		fflush(logfile);
	}

	pthread_mutex_unlock( &log_mutex );
	return 0;
}

static int sl_status( lua_State *L ){
/* Return the current status of logging
 * <- bool : logging on file
 * <- bool : logging on STDOUT
 */

	if(logfile)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	if(logto & LOG_STDOUT)
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
	logfile = NULL;
	pthread_mutex_init( &log_mutex, NULL );
}

int initSelLog( lua_State *L ){
	libSel_libFuncs(L, "SelLog" , SelLogLib);

	return 1;
}

