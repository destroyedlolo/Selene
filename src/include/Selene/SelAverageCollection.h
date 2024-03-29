/* SelAverageCollection.h
 *
 * This collection stores **immediate** and **average** values sub collection.
 *
 * 11/06/2022 LF : creation
 * 27/03/2024 LF : migrate to v7
 */

#ifndef SELAVERAGECOLLECTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELAVERAGECOLLECTION_VERSION 1

struct SelAverageCollectionStorage;

struct SelAverageCollection {
	struct SelModule module;

		/* Call backs */
	struct SelAverageCollectionStorage *(*create)(size_t, size_t, size_t, size_t);
	bool (*push)(struct SelAverageCollectionStorage *, size_t, ...);
	bool (*minmaxIs)(struct SelAverageCollectionStorage *, lua_Number *, lua_Number *);	/* shortcut for single value collection (immediate) */
	bool (*minmaxAs)(struct SelAverageCollectionStorage *, lua_Number *, lua_Number *);	/* shortcut for single value collection (average) */
	bool (*minmaxI)(struct SelAverageCollectionStorage *, lua_Number *, lua_Number *);	/* shortcut for single value collection (immediate) */
	bool (*minmaxA)(struct SelAverageCollectionStorage *, lua_Number *, lua_Number *);	/* shortcut for single value collection (immediate) */
	size_t (*getn)(struct SelAverageCollectionStorage *);
	size_t (*getsizeI)(struct SelAverageCollectionStorage *);
	size_t (*howmanyI)(struct SelAverageCollectionStorage *);
	size_t (*getsizeA)(struct SelAverageCollectionStorage *);
	size_t (*howmanyA)(struct SelAverageCollectionStorage *);
	void (*clear)(struct SelAverageCollectionStorage *);
};

#endif
