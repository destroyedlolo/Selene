/* SelTimedCollection.h
 *
 * Collection of timed values
 *
 */

#ifndef SELTIMEDCOLLECTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELTIMEDCOLLECTION_VERSION 1

struct SelTimedCollectionStorage;

struct SelTimedCollection {
	struct SelModule module;

		/* Call backs */
	struct SelTimedCollectionStorage *(*create)(size_t, size_t);
};

#endif
