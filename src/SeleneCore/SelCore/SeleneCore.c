/* SeleneCore.c
 *
 * Selene's core and helpers
 *
 * 06/01/2024 First version
 */

#include "SeleneCore.h"

static struct SeleneCore selCore;

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
void InitModule( void ){

		/* Initialise module's glue */
	initModule((struct SelModule *)&selCore, "SeleneCore", SELENECORE_VERSION);

	registerModule((struct SelModule *)&selCore);
}
