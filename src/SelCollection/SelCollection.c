/* SelCollection.c
 *
 * Collection of values
 *
 * 15/02/2021 LF : emancipate to create shared collection
 * 24/03/2024 LF : migrate to v7
 */

#include <Selene/SelCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelCollectionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>	/* varargs */

static struct SelCollection selCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

#define BUFFSZ	1023

static void scc_dump(struct SelCollectionStorage *col){
/** 
 * Display collection's content (for debugging purposes).
 *
 * @function dump
 *
 */
	unsigned int i,j;
	char t[BUFFSZ+1];
	char tn[64];

	selLog->Log('D', "SelCollection's Dump (size : %d x %d, last : %d)", col->size, col->ndata, col->last);

	if(col->full)
		for(i = col->last - col->size; i < col->last; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%f ", col->data[(i % col->size)*col->ndata + j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}
	else
		for(i = 0; i < col->last; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%f ", col->data[i*col->ndata + j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}
}

static struct SelCollectionStorage *scc_create(size_t size, size_t nbre_data){
/** 
 * Create a new SelCollection
 *
 * @function Create
 * @tparam num size size of the collection
 * @tparam num amount of values per sample (optional, default **1**)
 *
 * @usage
 col = SelCollection.create(5)
 */
	struct SelCollectionStorage *col = malloc(sizeof(struct SelCollectionStorage));
	assert(col);

	if(!(col->size = size)){
		selLog->Log('F', "SelCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}

	if((col->ndata = nbre_data) < 1)
		col->ndata = 1;

	assert((col->data = calloc(col->size * col->ndata, sizeof(lua_Number))));
	col->last = 0;
	col->full = 0;

	return(col);
}

static bool scc_push(struct SelCollectionStorage *col, size_t num, ...){
	if(col->ndata != num){
		selLog->Log('E', "Number of arguments mismatch");
		return false;
	}

	va_list ap;
	va_start(ap, num);
	for(size_t j=0; j<num; j++){
		lua_Number val = va_arg(ap, lua_Number);
		col->data[(col->last % col->size)*col->ndata + j] = val;
	}
	col->last++;

	if(col->last > col->size)
		col->full = true;

	va_end(ap);

	return true;
}

static bool scc_minmaxs(struct SelCollectionStorage *col, lua_Number *min, lua_Number *max){
	if(col->ndata != 1){
		selLog->Log('E', "SelCollection.minmaxs() can deal only with single value collection");
		return false;
	}

	if(!col->last && !col->full){
		selLog->Log('E', "MinMax() on an empty collection");
		return false;
	}

	size_t ifirst = col->full ? col->last - col->size : 0;
	*min = *max = col->data[ifirst % col->size];

	for(size_t i = ifirst; i < col->last; i++){
		lua_Number v = col->data[i % col->size];
		if(v < *min)
			*min = v;
		if(v > *max)
			*max = v;
	}

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
	if(!initModule((struct SelModule *)&selCollection, "SelCollection", SELCOLLECTION_VERSION, LIBSELENE_VERSION))
		return false;

	selCollection.module.dump = scc_dump;

	selCollection.create = scc_create;
	selCollection.push = scc_push;
	selCollection.minmaxs = scc_minmaxs;
	
	registerModule((struct SelModule *)&selCollection);

	return true;
}
