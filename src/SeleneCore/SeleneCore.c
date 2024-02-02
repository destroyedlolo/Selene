/* SeleneCore.c
 *
 * Selene's core and helpers
 *
 * 06/01/2024 First version
 */

#include "Selene/SeleneCore.h"

#include <stddef.h>		/* NULL */
#include <dlfcn.h>		/* dlopen(), ... */

static struct SeleneCore selCore;

static struct SelLog *selLog;

static void scc_SelLogInitialised(struct SelLog *aselLog){
/**
 * @brief SelLog has been initialized.
 *
 * Initialise internal SelLog reference. After this call, SelCore's can
 * log messages.
 *
 * @function slc_SelLogInitialised
 * @param pointer to SelLog module
 */
	selLog = aselLog;
}

static struct SelModule *scc_loadModule(const char *name, uint16_t minversion, uint16_t *verfound, char error_level){
/**
 * @brief Load a module
 *
 * @function loadModule
 * @param name Name of the module to load
 * @param minversion minimum version to load
 * @param found version of the found library (0 if not found, use dlerror() for explanation)
 * @param Error level to use in case of issue
 * @return pointer to the module or NULL if not found
 */
	struct SelModule *res = loadModule(name, minversion, verfound);

	if(!res && selLog){	/* An error occurred */
		if(*verfound)
			selLog->Log(error_level, "Can't load %s (%u instead of expected %u)",
				name, *verfound, minversion
			);
		else {
			char *err = dlerror();
			if(!err)
				selLog->Log(error_level, "Can't load %s : missing InitModule() or newer SelModule expected", name);
			else
				selLog->Log(error_level, "Can't load %s (%s)", name, err);
		}
	}

	return res;
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selLog = NULL;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selCore, "SeleneCore", SELENECORE_VERSION, LIBSELENE_VERSION))
		return false;

	selCore.SelLogInitialised = scc_SelLogInitialised;
	selCore.loadModule = scc_loadModule;

	registerModule((struct SelModule *)&selCore);

	return true;
}
