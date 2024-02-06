/* SelLua.h
 *
 * Lua support
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLUA_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLUA_VERSION 1

#ifdef __cplusplus
extern "C"
{
#endif

struct SelLua {
	struct SelModule module;

		/* Call backs */
};

#ifdef __cplusplus
}
#endif

#endif
