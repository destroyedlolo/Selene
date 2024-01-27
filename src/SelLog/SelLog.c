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

static struct SelLog selLog;

static pthread_mutex_t sl_mutex;	/* secure concurrency */
static FILE *sl_logfile;			/* file to log to */

static char *sl_LevIgnore;
static enum WhereToLog sl_logto;

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
void InitModule( void ){

		/* Initialise module's glue */
	initModule((struct SelModule *)&selLog, "SelLog", SELLOG_VERSION);

	registerModule((struct SelModule *)&selLog);
}
