/* SelSharedVar.h
 *
 * Variable shared among threads
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELSHAREDVAR_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELSHAREDVAR_VERSION 1

struct SelSharedVar {
	struct SelModule module;

		/* Call backs */

};

#endif
