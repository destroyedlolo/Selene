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
	bool (*minmax)(struct SelTimedCollectionStorage *, lua_Number *, lua_Number *);	/* multi value collection */
	size_t (*getsize)(struct SelTimedCollectionStorage *);
	size_t (*howmany)(struct SelTimedCollectionStorage *);
	size_t (*getn)(struct SelTimedCollectionStorage *);
	lua_Number (*gets)(struct SelTimedCollectionStorage *, time_t *,size_t);	/* get single */
	lua_Number *(*get)(struct SelTimedCollectionStorage *, time_t *, size_t, lua_Number *);	/* get multiple */
	lua_Number (*getat)(struct SelTimedCollectionStorage *, time_t *, size_t, size_t);	/* get single at a place */
};

#endif
