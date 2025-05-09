/* SeleneCore.h
 *
 * Selene's core and helpers
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELENECORE_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELENECORE_VERSION 6

#include "Selene/SelLog.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>

	/* Trans codification stuffs */
struct ConstTranscode {
	const char *name;
	const int value;
};

struct SeleneCore {
	struct SelModule module;

		/* Call backs */
	bool (*SelLogInitialised)(struct SelLog *);
	struct SelModule *(*loadModule)(const char *name, uint16_t minversion, uint16_t *found, char error_level);
	struct SelModule *(*findModuleByName)(const char *name, uint16_t minversion, char error_level);

	float (*getVersion)();

	const int (*findConst)(const char *, const struct ConstTranscode *tbl, bool *found);
	const char *(*rfindConst)(const int, const struct ConstTranscode *tbl);

	const char *(*ctime)(const time_t *, char *, size_t);

		/* Objects management */
	bool (*registerObject)(struct SelModule *, struct _SelObject *, const char *);
	struct _SelObject *(*findObject)(struct SelModule *, const char *, unsigned int);
	void (*lockObjList)(struct SelModule *);
	void (*unlockObjList)(struct SelModule *);
	struct _SelObject *(*getFirstObject)(struct SelModule *);
	struct _SelObject *(*getNextObject)(struct _SelObject *);
};

#ifdef __cplusplus
}
#endif

#endif
