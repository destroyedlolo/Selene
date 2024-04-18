/***
Timed window values collection.

The current implementation rely on :

  - time_t is an integer kind of,
  - it represents the number of seconds since era

@classmod SelTimedWindowCollection

 * History :
 *	10/04/2017	LF : First version
 *	17/03/2021	LF : storing in userdata prevents sharing b/w thread
 *		so only a pointer in now stored in the state
 */

#include <Selene/SelTimedWindowCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelTimedWindowCollectionStorage.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

static struct SelTimedWindowCollection selTimedWindowCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;


static void stwc_dump(struct SelTimedWindowCollectionStorage *col){
/** 
 * Display collection's content (for debugging purposes).
 *
 * @function dump
 *
 */
	pthread_mutex_lock(&col->mutex);

	if(col->last == (unsigned int)-1){
		pthread_mutex_unlock(&col->mutex);
		selLog->Log('D', "SelTimedWindowCollection's Dump (size : %d, EMPTY)", col->size);
		return;
	}

	selLog->Log('D', "SelTimedWindowCollection's Dump (size : %d, last : %d) %s size: %ld", col->size, col->last, col->full ? "Full":"Incomplet", col->group);

	if(col->full)
		for(size_t j = col->last - col->size +1; j <= col->last; j++){
			int i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			selLog->Log('D', "%lf / %lf @ %s", col->data[i].min_data, col->data[i].max_data, selCore->ctime(&t, NULL, 0));
		}
	else
		for(size_t i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			selLog->Log('D', "%lf / %lf @ %s", col->data[i].min_data, col->data[i].max_data, selCore->ctime(&t, NULL, 0));
		}

	pthread_mutex_unlock(&col->mutex);
}

static struct SelTimedWindowCollectionStorage *stwc_find(const char *name, unsigned int h){
/** 
 * Find a SelTimedWindowCollection by its name.
 *
 * @function Find
 * @tparam string name Name of the Collection
 * @param int hash code (recomputed if null)
 * @treturn ?SelTimedWindowCollection|nil
 */
	return((struct SelTimedWindowCollectionStorage *)selCore->findObject((struct SelModule *)&selTimedWindowCollection, name, h));
}

#if 0
static int sctl_find(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = SelTimedWindowCollection.find(luaL_checkstring(L, 1), 0);
	if(!col)
		return 0;

	struct SelTimedWindowCollectionStorage **r = lua_newuserdata(L, sizeof(struct SelTimedWindowCollectionStorage *));
	assert(r);

	luaL_getmetatable(L, "SelTimedWindowCollection");
	lua_setmetatable(L, -2);
	*r = col;

	return 1;
}
#endif

static struct SelTimedWindowCollectionStorage *stwc_create(const char *name, size_t size, size_t group){
/** 
 * Create a new SelTimedWindowCollection
 *
 * @function Create
 * @tparam number size size of the collection
 * @tparam number group seconds to be grouped in records
 */
	struct SelTimedWindowCollectionStorage *col = malloc(sizeof(struct SelTimedWindowCollectionStorage));
	assert(col);

	pthread_mutex_init(&col->mutex, NULL);

	if(!(col->size = size)){
		selLog->Log('F', "SelTimedWindowCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}
	if(!(col->group = group)){
		selLog->Log('F', "SelTimedWindowCollection's group can't be null or negative");
		exit(EXIT_FAILURE);
	}

	assert( (col->data = calloc(col->size, sizeof(struct timedwdata))) );
	col->last = (unsigned int)-1;
	col->full = 0;

		/* Register this collection */
	selCore->registerObject((struct SelModule *)&selTimedWindowCollection, (struct _SelObject *)col, strdup(name));

	MCHECK;
	return col;
}

static bool stwc_push(struct SelTimedWindowCollectionStorage *col, size_t v, time_t t){
/** 
 * Push a new value
 *
 * @function Push
 * @tparam ?number|table value single value or table of numbers in case of multi values collection
 * @tparam ?integer|nil timestamp Current timestamp by default
 */
	pthread_mutex_lock(&col->mutex);
	pthread_mutex_unlock(&col->mutex);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Other mandatory modules */

		/* optional modules */
	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'E');

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selTimedWindowCollection, "SelTimedWindowCollection", SELTIMEDWINDOWCOLLECTION_VERSION, LIBSELENE_VERSION))
		return false;

	selTimedWindowCollection.module.dump = stwc_dump;

	selTimedWindowCollection.create = stwc_create;
	selTimedWindowCollection.find = stwc_find;
	selTimedWindowCollection.push = stwc_push;
/*
	selTimedCollection.clear = sctc_clear;
	selTimedCollection.minmaxs= sctc_minmaxs;
	selTimedCollection.minmax= sctc_minmax;
	selTimedCollection.getsize = sctc_getsize;
	selTimedCollection.howmany = sctc_howmany;
	selTimedCollection.getn = sctc_getn;
	selTimedCollection.gets = sctc_gets;
	selTimedCollection.get = sctc_get;
	selTimedCollection.getat = sctc_getat;
	selTimedCollection.save = sctc_save;
	selTimedCollection.load = sctc_load;
*/

	registerModule((struct SelModule *)&selTimedWindowCollection);

#if 0
	if(selLua){	/* Only if Lua is used */
		registerSelTimedCollection(NULL);
		selLua->AddStartupFunc(registerSelTimedCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif
#endif

	return true;
}
