/***
Timed values collection.

@classmod SelTimedCollection

 * History :
 *	10/04/2017	LF : First version
 *	24/09/2020	LF : Multivalue
 *	03/02/2021	LF : storing in userdata prevents sharing b/w thread
 *		so only a pointer in now stored in the state
 *	28/03/2024	LF : Migrate to V7
 */

#include <Selene/SelTimedCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelTimedCollectionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

static struct SelTimedCollection selTimedCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SelTimedCollectionStorage *checkSelTimedCollection(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelTimedCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelTimedCollection' expected");
	return (struct SelTimedCollectionStorage *)r;
}

static struct SelTimedCollectionStorage *sctc_create(size_t size, size_t nbre_data){
/** 
 * Create a new SelTimedCollection
 *
 * @function Create
 * @tparam number size of the collection
 * @tparam number amount of values per sample (optional, default **1**)
 *
 * @usage
 col = SelTimedCollection.Create(5,3)
 */
	struct SelTimedCollectionStorage *col = malloc(sizeof(struct SelTimedCollectionStorage));
	assert(col);

	pthread_mutex_init(&col->mutex, NULL);

	if(!(col->size = size)){
		selLog->Log('F', "SelTimedCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}

	if((col->ndata = nbre_data) < 1)
		col->ndata = 1;

	assert( (col->data = calloc(col->size, sizeof(struct timeddata))) );
	for(size_t i=0; i<col->size; i++)
		assert( (col->data[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->last = 0;
	col->full = 0;

	MCHECK;

	return col;
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
	if(!initModule((struct SelModule *)&selTimedCollection, "SelTimedCollection", SELTIMEDCOLLECTION_VERSION, LIBSELENE_VERSION))
		return false;

	selTimedCollection.create = sctc_create;
	registerModule((struct SelModule *)&selTimedCollection);

#if 0
if(selLua){	/* Only if Lua is used */
		registerSelCollection(NULL);
		selLua->AddStartupFunc(registerSelCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif
#endif

	return true;
}
