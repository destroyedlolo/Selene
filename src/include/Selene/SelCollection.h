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
#define SELCOLLECTION_VERSION 3

struct SelCollectionStorage;

struct SelCollection {
	struct SelModule module;

		/* Call backs */
	struct SelCollectionStorage *(*create)(const char *, size_t, size_t);
	struct SelCollectionStorage *(*find)(const char *, unsigned int);
	bool (*push)(struct SelCollectionStorage *, size_t, ...);
	bool (*minmaxs)(struct SelCollectionStorage *, lua_Number *, lua_Number *);	/* shortcut for single value collection */
	bool (*minmax)(struct SelCollectionStorage *, lua_Number *, lua_Number *);	/* multi value collection */
	void (*clear)(struct SelCollectionStorage *);
	size_t (*getsize)(struct SelCollectionStorage *);
	size_t (*howmany)(struct SelCollectionStorage *);
	size_t (*getn)(struct SelCollectionStorage *);
	lua_Number (*gets)(struct SelCollectionStorage *, size_t);	/* get single */
	lua_Number *(*get)(struct SelCollectionStorage *, size_t, lua_Number *);	/* get multiple */
	lua_Number (*getat)(struct SelCollectionStorage *, size_t, size_t);	/* get single at a place */
	bool (*save)(struct SelCollectionStorage *, const char *);
	bool (*load)(struct SelCollectionStorage *, const char *);

};

#endif
