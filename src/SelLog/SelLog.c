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

#include "Selene/SelLog.h"

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h> /*	Vararg */

static struct SelLog selLog;

static pthread_mutex_t sl_mutex;	/* secure concurrency */
static FILE *sl_logfile;			/* file to log to */

static char *sl_LevIgnore;
static enum WhereToLog sl_logto;

#define MAXMSG	1024 /* Maximum message lenght */

static bool slc_Log(const char level, const char *message, ...){
/** 
 * Log a message (C interface)
 *
 * @function Log
 * @tparam string level log level as registered ('**I**' by if 0)
 * @tparam string message text to log
 */
	time_t tt;
	struct tm tmt;
	va_list args;

	char t[MAXMSG];
	va_start(args, message);
	
	time( &tt );
	localtime_r( &tt, &tmt );

	snprintf(t, MAXMSG,
		"%04d-%02d-%02d %02d:%02d:%02d - *%c* %s\n",
		tmt.tm_year+1900, tmt.tm_mon+1, tmt.tm_mday,
		tmt.tm_hour, tmt.tm_min, tmt.tm_sec,
		level, message 
	);


	if(sl_logto & LOG_STDOUT)
		vprintf(t, args);

	if(sl_logfile){
		if( vfprintf( sl_logfile, t, args ) < 0 ){
			va_end(args);
			return false;
		}
		fflush(sl_logfile);
	}

	va_end(args);
	return true;
}


bool slc_init(const char *fn, enum WhereToLog logto){
/** 
 * Initialise logging to file
 *
 * Notez-bien :
 * ---
 *
 * - Whatever the value of *where*, the message is always published if connected to a broker.
 * - depending on logging level, the message is output on *stdout* or *stderr*
 *
 * @function init
 * @tparam string filename file to log to
 * @tparam WhereToLog 
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

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
void InitModule(void){
		/* Initialise configurations */
	pthread_mutex_init( &sl_mutex, NULL );
	sl_logfile = NULL;
	sl_LevIgnore = NULL;
	sl_logto = LOG_STDOUT;	/* Without initialisation, log to STDOUT */

		/* Initialise module's glue */
	initModule((struct SelModule *)&selLog, "SelLog", SELLOG_VERSION);

	selLog.Log = slc_Log;
	selLog.initFile = slc_init;

	registerModule((struct SelModule *)&selLog);
}
