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

static bool scc_minmax(struct SelCollectionStorage *col, lua_Number *min, lua_Number *max){

	if(!col->last && !col->full){
		selLog->Log('E', "MinMax() on an empty collection");
		return false;
	}

	size_t ifirst = col->full ? col->last - col->size : 0;
	for(size_t j=0; j<col->ndata; j++)
		min[j] = max[j] = col->data[(ifirst % col->size)*col->ndata + j];

	for(size_t i = ifirst; i < col->last; i++){
		for(size_t j=0; j<col->ndata; j++){
			lua_Number v = col->data[(i % col->size)*col->ndata + j];
			if(v < min[j])
				min[j] = v;
			if(v > max[j])
				max[j] = v;
		}
	}
	
	return true;
}

static void scc_clear(struct SelCollectionStorage *col){
/**
 * Make the collection empty
 *
 * @function Clear
 */
	col->last = 0;
	col->full = 0;
}

static size_t scc_getsize(struct SelCollectionStorage *col){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	return(col->size);
}

static size_t scc_howmany(struct SelCollectionStorage *col){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	return(col->full ? col->size : col->last);
}

static size_t scc_getn(struct SelCollectionStorage *col){
/** 
 * Number of entries per sample
 *
 * @function Getn
 * @treturn num Amount of data per sample
 */
	return(col->ndata);
}

static lua_Number scc_gets(struct SelCollectionStorage *col, size_t idx){
/**
 * Returns the value at the given position (0.0 if invalid)
 * 1st value for multi valued collection.
 * 
 * @function gets
 * @treturn lua_Number value
 */
	if(idx >= selCollection.howmany(col))
		return 0.0;

	return(col->data[((col->last - col->size + idx) % col->size)*col->ndata]);
}

static lua_Number *scc_get(struct SelCollectionStorage *col, size_t idx, lua_Number *res){
/**
 * Returns the value at the given position (0.0 if invalid)
 * 1st value for multi valued collection.
 * 
 * @function gets
 * @treturn lua_Number value
 */
	if(idx >= selCollection.howmany(col)){
		for(size_t j=0; j<col->ndata; j++)
			res[j] = 0.0;
		return res;
	}

	idx += col->last - col->size;	/* normalize to physical index */
	for(size_t j=0; j<col->ndata; j++)
		res[j] = col->data[(idx % col->size)*col->ndata + j];

	return res;
}

static lua_Number scc_getat(struct SelCollectionStorage *col, size_t idx, size_t at){
	if(idx >= selCollection.howmany(col) || at >= col->ndata)
		return 0.0;

	idx += col->last - col->size;	/* normalize to physical index */
	return(col->data[(idx % col->size)*col->ndata + at]);
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
	selCollection.minmax = scc_minmax;
	selCollection.clear = scc_clear;
	selCollection.getsize = scc_getsize;
	selCollection.howmany = scc_howmany;
	selCollection.getn = scc_getn;
	selCollection.gets = scc_gets;
	selCollection.get = scc_get;
	selCollection.getat = scc_getat;
	
	registerModule((struct SelModule *)&selCollection);

	return true;
}
