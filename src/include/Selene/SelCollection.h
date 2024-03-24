/* SelCollection.h
 *
 * Collection of values
 *
 * 15/02/2021 LF : emancipate to create shared collection
 * 24/03/2024 LF : migrate to v7
 */

#ifndef SELCOLLECTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELCOLLECTION_VERSION 1

struct SelCollection {
	struct SelModule module;

		/* Call backs */
};

#endif
