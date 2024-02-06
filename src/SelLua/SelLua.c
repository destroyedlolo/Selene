/* SeleneCore.h
 *
 * Selene's core and helpers
 *
 * 06/02/2024 First version
 */

#include "Selene/SelLua.h"

static struct SelLua selLua;

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selLua, "SelLua", SELLUA_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selLua);

	return true;
}
