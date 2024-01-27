/* SelLog.h
 *
 * Logging facility
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLOG_VERSION
#include "libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLOG_VERSION (LIBSELENE_VERSION + 1)

#ifdef __cplusplus
extern "C"
{
#endif


	/*
	 * Configure where to log too
	 */

enum WhereToLog {
	LOG_DECIDEYOUSELF = 0,
	LOG_FILE = 1,
	LOG_STDOUT = 2
};


struct SelLog {
	struct SelModule module;

		/* Call backs */
	bool (*Log)(const char level, const char *message, ...);	/* Logging */

	void (*ignoreList)(const char *);
	bool (*initFile)(const char *filename, enum WhereToLog logto);
};

#ifdef __cplusplus
}
#endif

#endif
