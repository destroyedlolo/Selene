/* SelSharedRef.h
 *
 * Store Lua reference.
 *
 * 07/05/2024 First version
 */

#ifndef SELSHAREDREF_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelElasticStorage.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELSHAREDREF_VERSION 1

struct SelSharedRefStorage;

struct SelSharedRef {
	struct SelModule module;

		/* Call backs */
	struct SelSharedRefStorage *(*find)(const char *, unsigned int);
};

#endif
