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
	lua_Number (*getsI)(struct SelAverageCollectionStorage *, size_t);	/* get single */
	lua_Number (*getsA)(struct SelAverageCollectionStorage *, size_t);	/* get single */
	lua_Number *(*getI)(struct SelAverageCollectionStorage *, size_t, lua_Number *);	/* get multiple */
	lua_Number *(*getA)(struct SelAverageCollectionStorage *, size_t, lua_Number *);	/* get multiple */
	lua_Number (*getatI)(struct SelAverageCollectionStorage *, size_t, size_t);	/* get single at a place */
	lua_Number (*getatA)(struct SelAverageCollectionStorage *, size_t, size_t);	/* get single at a place */
	bool (*save)(struct SelAverageCollectionStorage *, const char *, bool);
	bool (*load)(struct SelAverageCollectionStorage *, const char *);
};

#endif
