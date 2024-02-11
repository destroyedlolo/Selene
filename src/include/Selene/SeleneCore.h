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
#define SELENECORE_VERSION 4

#include "Selene/SelLog.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/* Trans codification stuffs */
struct ConstTranscode {
	const char *name;
	const unsigned int value;
};

struct SeleneCore {
	struct SelModule module;

		/* Call backs */
	bool (*SelLogInitialised)(struct SelLog *);
	struct SelModule *(*loadModule)(const char *name, uint16_t minversion, uint16_t *found, char error_level);
	struct SelModule *(*findModuleByName)(const char *name, uint16_t minversion, char error_level);

	float (*getVersion)();

	 const unsigned int (*findConst)(const char *, const struct ConstTranscode *tbl);
	 const char *(*rfindConst)(const unsigned int, const struct ConstTranscode *tbl);
};

#ifdef __cplusplus
}
#endif

#endif
