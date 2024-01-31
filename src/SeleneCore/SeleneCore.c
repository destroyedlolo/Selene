/* SeleneCore.c
 *
 * Selene's core and helpers
 *
 * 06/01/2024 First version
 */

#include "Selene/SeleneCore.h"

#include <stddef.h>	/* NULL */

static struct SeleneCore selCore;

static struct SelLog *selLog;

static void slc_SelLogInitialised(struct SelLog *aselLog){
/**
 * SelLog has been initialized.
 *
 * Initialise internal SelLog reference. After this call, SelCore's can
 * log messages.
 *
 * @function slc_SelLogInitialised
 * @param pointer to SelLog module
 */
	selLog = aselLog;
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
void InitModule( void ){
	selLog = NULL;

		/* Initialise module's glue */
	initModule((struct SelModule *)&selCore, "SeleneCore", SELENECORE_VERSION);

	selCore.SelLogInitialised = slc_SelLogInitialised;

	registerModule((struct SelModule *)&selCore);
}
