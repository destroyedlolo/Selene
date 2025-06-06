/* SeleneCore.h
 *
 * Selene's core and helpers
 *
 * Have a look and respect Selene Licence.
 *
 *	History :
 *	---------
 *	v8	- Add lock/unlock in SelGenericSurface
 */

#ifndef SELENECORE_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELENECORE_VERSION 8

#include "Selene/SelLog.h"

#ifdef __cplusplus
extern "C"
{
#endif


	/* Avoid unneeded dependencies */
struct SelGenericSurface;
struct SGS_callbacks;

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

		/* Named Objects management */
	bool (*registerNamedObject)(struct SelModule *, struct _SelNamedObject *, const char *);
	struct _SelNamedObject *(*findNamedObject)(struct SelModule *, const char *, unsigned int);
	void (*lockObjList)(struct SelModule *);
	void (*unlockObjList)(struct SelModule *);
	struct _SelNamedObject *(*getFirstNamedObject)(struct SelModule *);
	struct _SelNamedObject *(*getNextNamedObject)(struct _SelNamedObject *);

		/* Objects management */
	void (*initObject)(struct SelModule *, struct SelObject *);
	void (*initGenericSurface)(struct SelModule *, struct SelGenericSurface *);
	void (*initGenericSurfaceCallBacks)(struct SGS_callbacks *);
};

#ifdef __cplusplus
}
#endif

#endif
