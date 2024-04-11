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
	return *(struct SelTimedCollectionStorage **)r;
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

static int sctl_dump(lua_State *L){
	struct SelTimedCollectionStorage *col = checkSelTimedCollection(L);
	selTimedCollection.module.dump(col);

	return 0;
}

static struct SelTimedCollectionStorage *sctc_find(const char *name, unsigned int h){
/** 
 * Find a SelCollection by its name.
 *
 * @function Find
 * @tparam string name Name of the Collection
 * @param int hash code (recomputed if null)
 * @treturn ?SelTimedCollection|nil
 */
	return((struct SelTimedCollectionStorage *)selCore->findObject((struct SelModule *)&selTimedCollection, name, h));
}

static struct SelTimedCollectionStorage *sctc_create(const char *name, size_t size, size_t nbre_data){
/** 
 * Create a new SelTimedCollection
 *
 * @function Create
 * @tparam string name of the the collection
 * @tparam number size of the collection
 * @tparam number amount of values per sample (optional, default **1**)
 *
 * @usage
 col = SelTimedCollection.Create(5,3)
 */
	unsigned int h = selL_hash(name);
	struct SelTimedCollectionStorage *col = sctc_find(name, h);
	if(col)
		return col;

	col = malloc(sizeof(struct SelTimedCollectionStorage));
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

		/* Register this collection */
	selCore->registerObject((struct SelModule *)&selTimedCollection, (struct _SelObject *)col, strdup(name));

	MCHECK;
	return col;
}

static int sctl_create(lua_State *L){
	const char *name = luaL_checkstring(L, 1);	/* Name of the collection */
	int size, ndata;

	if((size = luaL_checkinteger( L, 2 )) <= 0){
		selLog->Log('F', "SelTimedCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}

	if((ndata = lua_tointeger( L, 3 )) < 1)
		ndata = 1;
	
	struct SelTimedCollectionStorage **col = (struct SelTimedCollectionStorage **)lua_newuserdata(L, sizeof(struct SelTimedCollectionStorage));
	assert(col);

	luaL_getmetatable(L, "SelTimedCollection");
	lua_setmetatable(L, -2);

	*col = sctc_create(name, size, ndata);

	return 1;
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

static bool sctc_minmaxs(struct SelTimedCollectionStorage *col, lua_Number *min, lua_Number *max){
	if(col->ndata != 1){
		selLog->Log('E', "SelTimedCollection.minmaxs() can deal only with single value collection");
		return false;
	}

	if(!col->last && !col->full){
		selLog->Log('E', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);

	size_t ifirst = col->full ? col->last - col->size : 0;
	*min = *max = *col->data[ifirst % col->size].data;

	for(size_t i = ifirst; i < col->last; i++){
		lua_Number v = *col->data[i % col->size].data;
		if(v < *min)
			*min = v;
		if(v > *max)
			*max = v;
	}

	pthread_mutex_unlock(&col->mutex);

	return true;
}

static bool sctc_minmax(struct SelTimedCollectionStorage *col, lua_Number *min, lua_Number *max){

	if(!col->last && !col->full){
		selLog->Log('E', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);
	size_t ifirst = col->full ? col->last - col->size : 0;
	for(size_t j=0; j<col->ndata; j++)
		min[j] = max[j] = col->data[ifirst % col->size].data[j];

	for(size_t i = ifirst; i < col->last; i++){
		for(size_t j=0; j<col->ndata; j++){
			lua_Number v = col->data[i % col->size].data[j];
			if(v < min[j])
				min[j] = v;
			if(v > max[j])
				max[j] = v;
		}
	}
	pthread_mutex_unlock(&col->mutex);
	
	return true;
}

#if 0
static int scl_minmax(lua_State *L){
/** 
 * Calculates the minimum and the maximum of this collection.
 *
 * @function MinMax
 * @treturn ?number|table minium
 * @treturn ?number|table maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelCollectionStorage *col = checkSelCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[col->ndata], max[col->ndata];

	if(!col->last && !col->full){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	pthread_mutex_lock(&col->mutex);
	ifirst = col->full ? col->last - col->size : 0;

	for(j=0; j<col->ndata; j++)
		min[j] = max[j] = col->data[ (ifirst % col->size)*col->ndata + j ];

	for(i = ifirst; i < col->last; i++){
		for( j=0; j<col->ndata; j++ ){
			lua_Number v = col->data[ (i % col->size)*col->ndata + j ];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}
	pthread_mutex_unlock(&col->mutex);

	if(col->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<col->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<col->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	return 2;
}
#endif

static size_t sctc_getsize(struct SelTimedCollectionStorage *col){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	return(col->size);
}

static size_t sctc_howmany(struct SelTimedCollectionStorage *col){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	return(col->full ? col->size : col->last);
}

#if 0
static int sctl_getsize(lua_State *L){
	struct SelTimedCollectionStorage *col = checkSelCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

static int sctl_getn(lua_State *L){
	struct SelTimedCollectionStorage *col = checkSelCollection(L);

	lua_pushnumber(L, col->ndata);
	return 1;
}

static int sctl_HowMany(lua_State *L){
	struct SelCollectionStorage *col = checkSelCollection(L);

	lua_pushnumber(L, col->full ? col->size : col->last);
	return 1;
}
#endif

static size_t sctc_getn(struct SelTimedCollectionStorage *col){
/** 
 * Number of entries per sample
 *
 * @function Getn
 * @treturn num Amount of data per sample
 */
	return(col->ndata);
}

static lua_Number sctc_gets(struct SelTimedCollectionStorage *col, time_t *t, size_t idx){
/**
 * Returns the value at the given position (0.0 if invalid)
 * 1st value for multi valued collection.
 * 
 * @function gets
 * @argument t pointer to a time_t to hold the time of the sample (ignored if NULL)
 * @treturn lua_Number value
 */
	if(idx >= selTimedCollection.howmany(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->full)
		idx += col->last - col->size;
	lua_Number ret = *col->data[idx % col->size].data;
	if(t)
		*t = col->data[idx % col->size].t;
	pthread_mutex_unlock(&col->mutex);

	return ret;
}

static lua_Number *sctc_get(struct SelTimedCollectionStorage *col, time_t *t, size_t idx, lua_Number *res){
/**
 * Returns the values at the given position (0.0 if invalid)
 * 
 * @function get
 * @treturn lua_Number value
 */
	if(idx >= selTimedCollection.howmany(col)){
		for(size_t j=0; j<col->ndata; j++)
			res[j] = 0.0;
		return res;
	}

	pthread_mutex_lock(&col->mutex);
	if(col->full)
		idx += col->last - col->size;	/* normalize to physical index */
	for(size_t j=0; j<col->ndata; j++)
		res[j] = col->data[idx % col->size].data[j];
	if(t)
		*t = col->data[idx % col->size].t;
	pthread_mutex_unlock(&col->mutex);

	return res;
}

static lua_Number sctc_getat(struct SelTimedCollectionStorage *col, time_t *t, size_t idx, size_t at){
	if(idx >= selTimedCollection.howmany(col) || at >= selTimedCollection.getn(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->full)
		idx += col->last - col->size;	/* normalize to physical index */
	lua_Number res = col->data[idx % col->size].data[at];
	if(t)
		*t = col->data[idx % col->size].t;
	pthread_mutex_unlock(&col->mutex);

	return res;
}

static bool sctc_save(struct SelTimedCollectionStorage *col, const char *filename){
/** 
 * Save the collection to a file
 *
 * @function Save
 * @tparam string filename
 * @usage
col:Save('/tmp/tst.dt')
 */
 	FILE *f = fopen(filename, "w");
	if(!f){
		selLog->Log('E', "%s : %s", filename, strerror(errno));
		return false;
	}

	pthread_mutex_lock(&col->mutex);
		/* Write Header */
	fprintf(f, "STCMV %d\n", col->ndata);

		/* Average values */
	if(col->full)
		for(size_t i = col->last - col->size; i < col->last; i++){
			fprintf(f, "d %ld", col->data[i % col->size].t);
			for(size_t j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->data[i % col->size].data[j]);
			fputs("\n",f);
		}
	else
		for(size_t i = 0; i < col->last; i++){
			fprintf(f, "d %ld", col->data[i % col->size].t);
			for(size_t j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->data[i % col->size].data[j]);
			fputs("\n",f);
		}
	pthread_mutex_unlock(&col->mutex);

	fclose(f);
	return true;
}

static bool sctc_load(struct SelTimedCollectionStorage *col, const char *filename){
	size_t j;

 	FILE *f = fopen(filename, "r");
	if(!f){
		selLog->Log('E', "%s : %s", filename, strerror(errno));
		return false;
	}

	if(!fscanf(f, "STCMV %ld", &j)){
		selLog->Log('E', "Nagic not found");
		fclose(f);
		return false;
	}
	
	if(j != col->ndata){
		selLog->Log('E', "Amount of data doesn't match");
		fclose(f);
		return false;
	}
	

	pthread_mutex_lock(&col->mutex);
	for(;;){
		char cat;
		fscanf(f, "\n%c", &cat);
		if(feof(f))
			break;

		if(cat == 'd'){
			fscanf(f, "%ld", &col->data[col->last % col->size].t);
			for(size_t j = 0; j < col->ndata; j++)
				fscanf(f, "%lf", &col->data[col->last % col->size].data[j]);
			col->last++;
		} else {
			pthread_mutex_unlock(&col->mutex);
			selLog->Log('E', "This grouping doesn't match");
			fclose(f);
			return false;
		}

		if(col->last > col->size)
			col->full = true;
	}
	pthread_mutex_unlock(&col->mutex);

	fclose(f);
	return true;
}

static const struct luaL_Reg SelTimedCollectionM [] = {
	{"dump", sctl_dump},
#if 0
	{"Push", scl_push},
	{"MinMax", scl_minmax},
	{"iData", scl_idata},
	{"Clear", scl_clear},
	{"GetSize", scl_getsize},
	{"Getn", scl_getn},
	{"HowMany", scl_HowMany},
	{"Save", scl_save},
	{"Load", scl_load},
#endif
	{NULL, NULL}
};

static const struct luaL_Reg SelTimedCollectionLib [] = {
	{"Create", sctl_create},
/*	{"Find", sctl_find}, */
	{NULL, NULL}
};

static void registerSelTimedCollection(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelTimedCollection", SelTimedCollectionLib);
	selLua->objFuncs(L, "SelTimedCollection", SelTimedCollectionM);
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

	registerModule((struct SelModule *)&selTimedCollection);

	if(selLua){	/* Only if Lua is used */
		registerSelTimedCollection(NULL);
		selLua->AddStartupFunc(registerSelTimedCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

	return true;
}
