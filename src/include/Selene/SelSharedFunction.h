/* SelSharedFunction.h
 *
 * Share function among threads
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELSHAREDFUNCTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelElasticStorage.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELSHAREDFUNCTION_VERSION 1

struct SelSharedFunction {
	struct SelModule module;

		/* Call backs */

};

#endif
