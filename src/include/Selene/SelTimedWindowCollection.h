/* SelTimedWindowCollection.h
 *
 * Collection of timed windowed values
 *
 */

#ifndef SELTIMEDWINDOWCOLLECTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELTIMEDWINDOWCOLLECTION_VERSION 1

#include <time.h>

struct SelTimedWindowCollectionStorage;

struct SelTimedWindowCollection {
	struct SelModule module;

		/* Call backs */
	struct SelTimedWindowCollectionStorage *(*create)(const char *, size_t, size_t);
	struct SelTimedWindowCollectionStorage *(*find)(const char *, unsigned int);
	void (*push)(struct SelTimedWindowCollectionStorage *, lua_Number, time_t);
	bool (*minmax)(struct SelTimedWindowCollectionStorage *, lua_Number *, lua_Number *, lua_Number *, double *);
	bool (*diffminmax)(struct SelTimedWindowCollectionStorage *, lua_Number *, lua_Number *);
};

#endif
