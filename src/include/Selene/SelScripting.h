/* SelScripting.h
 *
 * Expose Lua scripting functions.
 * Because not all application using Selene framework need scripting related
 * methods.
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELSCRIPTING_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELSCRIPTING_VERSION 1

#ifdef __cplusplus
extern "C"
{
#endif

struct SelScripting {
	struct SelModule module;

		/* Call backs */
};

#ifdef __cplusplus
}
#endif

#endif
