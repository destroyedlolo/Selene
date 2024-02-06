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
#define SELENECORE_VERSION 3

#include "Selene/SelLog.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct SeleneCore {
	struct SelModule module;

		/* Call backs */
	bool (*SelLogInitialised)(struct SelLog *);
	struct SelModule *(*loadModule)(const char *name, uint16_t minversion, uint16_t *found, char error_level);
	struct SelModule *(*findModuleByName)(const char *name, uint16_t minversion, char error_level);

	float (*getVersion)();

};

#ifdef __cplusplus
}
#endif

#endif