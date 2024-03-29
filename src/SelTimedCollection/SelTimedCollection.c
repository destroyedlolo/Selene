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

#define BUFFSZ	1023

static void stcc_dump(struct SelTimedCollectionStorage *col){
/** 
 * Display collection's content (for debugging purposes).
 *
 * @function dump
 *
 */
	char t[BUFFSZ+1];
	char tn[64];

	pthread_mutex_lock(&col->mutex);

	selLog->Log('D', "SelTimedCollection's Dump (size : %d x %d, last : %d)", col->size, col->ndata, col->last);

	if(col->full)
		for(size_t i = col->last - col->size; i < col->last; i++){
			strcpy(t, selCore->ctime(&col->data[i % col->size].t, NULL, 0)); 
			for(size_t j = 0; j < col->ndata; j++){
				sprintf(tn, " %lf", col->data[i % col->size].data[j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}
	else
		for(size_t i = 0; i < col->last; i++){
			strcpy(t, selCore->ctime(&col->data[i % col->size].t, NULL, 0)); 
			for(size_t j = 0; j < col->ndata; j++){
				sprintf(tn, " %lf", col->data[i].data[j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}

	pthread_mutex_unlock(&col->mutex);
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

static void sctc_clear(struct SelTimedCollectionStorage *col){
/**
 * Make the collection empty
 *
 * @function Clear
 */
	pthread_mutex_lock(&col->mutex);

	col->last = 0;
	col->full = 0;

	pthread_mutex_unlock(&col->mutex);
}

static bool sctc_push(struct SelTimedCollectionStorage *col, size_t num, time_t tm, ...){
	if(col->ndata != num){
		selLog->Log('E', "Number of arguments mismatch");
		return false;
	}

	pthread_mutex_lock(&col->mutex);

	va_list ap;
	va_start(ap, tm);
	for(size_t j=0; j<num; j++){
		lua_Number val = va_arg(ap, lua_Number);
		col->data[col->last % col->size].data[j] = val;
	}

	col->data[col->last++ % col->size].t = tm ? tm : time(NULL);

	if(col->last > col->size)
		col->full = true;

	va_end(ap);

	pthread_mutex_unlock(&col->mutex);
	return true;
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

	selTimedCollection.module.dump = stcc_dump;

	selTimedCollection.create = sctc_create;
	selTimedCollection.clear = sctc_clear;
	selTimedCollection.push= sctc_push;

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
