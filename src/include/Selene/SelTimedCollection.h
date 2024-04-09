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

#include <time.h>

struct SelTimedCollectionStorage;

struct SelTimedCollection {
	struct SelModule module;

		/* Call backs */
	struct SelTimedCollectionStorage *(*create)(const char *, size_t, size_t);
	struct SelTimedCollectionStorage *(*find)(const char *, unsigned int);
	void (*clear)(struct SelTimedCollectionStorage *);
	bool (*push)(struct SelTimedCollectionStorage *, size_t, time_t, ...);
	bool (*minmaxs)(struct SelTimedCollectionStorage *, lua_Number *, lua_Number *);	/* shortcut for single value collection */
	size_t (*getsize)(struct SelTimedCollectionStorage *);
	size_t (*howmany)(struct SelTimedCollectionStorage *);
	size_t (*getn)(struct SelTimedCollectionStorage *);
};

#endif
