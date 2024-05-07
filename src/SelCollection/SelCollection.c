/* SelCollection.c
 *
 * Collection of values
 *

When a additional one is pushed in a full collection, the oldest one is pushed out.

----------------------
Typical usage : to store single or multivaled numbers

@classmod SelCollection

 * History :
 *	28/09/2015	LF : First version
 *	04/04/2018	LF : switch to libSelene
 *	23/09/2020	LF : Multivalue
 *	15/02/2021	LF : emancipate to create shared collection
 *	24/03/2024	LF : migrate to v7
 
@usage
-- Multi valued Collection example

col = SelCollection.create("my collection", 5,2)

for i=1,4 do
	col:Push(i, 4-i)
end
col:dump()

 */

#include <Selene/SelCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelCollectionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>	/* varargs */
#include <errno.h>

static struct SelCollection selCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SelCollectionStorage *checkSelCollection(lua_State *L){
	struct SelCollectionStorage **r = selLua->testudata(L, 1, "SelCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelCollection' expected");
	return *r;
}

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

	pthread_mutex_lock(&col->mutex);

	selLog->Log('D', "SelCollection's Dump (size : %d x %d, last : %d)", col->size, col->ndata, col->last);

	if(col->full)
		for(i = col->last - col->size; i < col->last; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%lf ", col->data[(i % col->size)*col->ndata + j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}
	else
		for(i = 0; i < col->last; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%lf ", col->data[i*col->ndata + j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}

	pthread_mutex_unlock(&col->mutex);
}

static int scl_dump(lua_State *L){
	struct SelCollectionStorage *col = checkSelCollection(L);
	selCollection.module.dump(col);

	return 0;
}

static struct SelCollectionStorage *scc_find(const char *name, unsigned int h){
/** 
 * Find a SelCollection by its name.
 *
 * @function Find
 * @tparam string name Name of the Collection
 * @param int hash code (recomputed if null)
 * @treturn ?SelCollection|nil
 */
	return((struct SelCollectionStorage *)selCore->findObject((struct SelModule *)&selCollection, name, h));
}

static int scl_find(lua_State *L){
	struct SelCollectionStorage *col = selCollection.find(luaL_checkstring(L, 1), 0);
	if(!col)
		return 0;

	struct SelCollectionStorage **r = lua_newuserdata(L, sizeof(struct SelCollectionStorage *));
	assert(r);
	luaL_getmetatable(L, "SelCollection");
	lua_setmetatable(L, -2);
	*r = col;

	return 1;
}

static struct SelCollectionStorage *scc_create(const char *name, size_t size, size_t nbre_data){
/** 
 * Create a new SelCollection
 *
 * @function Create
 * @tparam string collection's name (can be nil for unamed)
 * @tparam num size size of the collection
 * @tparam num amount of values per sample (optional, default **1**)
 *
 * @usage
 col = SelCollection.create("my name", 5)
 */
	struct SelCollectionStorage *col;

	if(name){
		unsigned int h = selL_hash(name);
		col = scc_find(name, h);
		if(col)
			return col;
	}

	col = malloc(sizeof(struct SelCollectionStorage));
	assert(col);

	pthread_mutex_init(&col->mutex, NULL);

	if(!(col->size = size)){
		selLog->Log('F', "SelCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}

	if((col->ndata = nbre_data) < 1)
		col->ndata = 1;

	assert((col->data = calloc(col->size * col->ndata, sizeof(lua_Number))));
	col->last = 0;
	col->full = 0;

		/* Register this collection */
	if(name)
		selCore->registerObject((struct SelModule *)&selCollection, (struct _SelObject *)col, strdup(name));

	return(col);
}

static int scl_create(lua_State *L){
	const char *name = lua_tostring(L, 1);	/* Name of the collection */
	int size, ndata;

	if((size = luaL_checkinteger( L, 2 )) <= 0){
		selLog->Log('F', "SelCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}

	if((ndata = lua_tointeger( L, 3 )) < 1)
		ndata = 1;
	
	struct SelCollectionStorage **col = (struct SelCollectionStorage **)lua_newuserdata(L, sizeof(struct SelCollectionStorage));
	assert(col);

	luaL_getmetatable(L, "SelCollection");
	lua_setmetatable(L, -2);

	*col = scc_create(name, size, ndata);

	return 1;
}

static bool scc_push(struct SelCollectionStorage *col, size_t num, ...){
	if(col->ndata != num){
		selLog->Log('E', "Number of arguments mismatch");
		return false;
	}

	va_list ap;
	va_start(ap, num);
	pthread_mutex_lock(&col->mutex);

	for(size_t j=0; j<num; j++){
		lua_Number val = va_arg(ap, lua_Number);
		col->data[(col->last % col->size)*col->ndata + j] = val;
	}
	col->last++;

	if(col->last > col->size)
		col->full = true;

	va_end(ap);

	pthread_mutex_unlock(&col->mutex);

	return true;
}

static int scl_push(lua_State *L){
/** 
 * Push a new sample.
 *
 * @function Push
 * @tparam ?number|table value single value or table of numbers in case of multi values collection
 */
	struct SelCollectionStorage *col = checkSelCollection(L);
	unsigned int j;

	if( lua_gettop(L)-1 != col->ndata )
		luaL_error(L, "Expecting %d data", col->ndata);

	pthread_mutex_lock(&col->mutex);

	for( j=1; j<lua_gettop(L); j++)
		col->data[ (col->last % col->size)*col->ndata + j-1 ] = luaL_checknumber( L, j+1 );
	col->last++;

	if(col->last > col->size)
		col->full = 1;

	pthread_mutex_unlock(&col->mutex);

	return 0;
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

	pthread_mutex_lock(&col->mutex);

	size_t ifirst = col->full ? col->last - col->size : 0;
	*min = *max = col->data[ifirst % col->size];

	for(size_t i = ifirst; i < col->last; i++){
		lua_Number v = col->data[i % col->size];
		if(v < *min)
			*min = v;
		if(v > *max)
			*max = v;
	}

	pthread_mutex_unlock(&col->mutex);

	return true;
}

static bool scc_minmax(struct SelCollectionStorage *col, lua_Number *min, lua_Number *max){

	if(!col->last && !col->full){
		selLog->Log('E', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);
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
	pthread_mutex_unlock(&col->mutex);
	
	return true;
}

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

static void scc_clear(struct SelCollectionStorage *col){
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

static int scl_clear(lua_State *L){
	struct SelCollectionStorage *col = checkSelCollection(L);
	selCollection.clear(col);

	return 0;
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

static int scl_getsize(lua_State *L){
	struct SelCollectionStorage *col = checkSelCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

static int scl_getn(lua_State *L){
	struct SelCollectionStorage *col = checkSelCollection(L);

	lua_pushnumber(L, col->ndata);
	return 1;
}

static int scl_HowMany(lua_State *L){
	struct SelCollectionStorage *col = checkSelCollection(L);

	lua_pushnumber(L, col->full ? col->size : col->last);
	return 1;
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

	pthread_mutex_lock(&col->mutex);
	if(col->full)
		idx += col->last - col->size;
	lua_Number ret = col->data[(idx % col->size)*col->ndata];
	pthread_mutex_unlock(&col->mutex);

	return ret;
}

static lua_Number *scc_get(struct SelCollectionStorage *col, size_t idx, lua_Number *res){
/**
 * Returns the values at the given position (0.0 if invalid)
 * 
 * @function get
 * @treturn lua_Number value
 */
	if(idx >= selCollection.howmany(col)){
		for(size_t j=0; j<col->ndata; j++)
			res[j] = 0.0;
		return res;
	}

	pthread_mutex_lock(&col->mutex);
	if(col->full)
		idx += col->last - col->size;	/* normalize to physical index */
	for(size_t j=0; j<col->ndata; j++)
		res[j] = col->data[(idx % col->size)*col->ndata + j];
	pthread_mutex_unlock(&col->mutex);

	return res;
}

static lua_Number scc_getat(struct SelCollectionStorage *col, size_t idx, size_t at){
	if(idx >= selCollection.howmany(col) || at >= selCollection.getn(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->full)
		idx += col->last - col->size;	/* normalize to physical index */
	lua_Number res = col->data[(idx % col->size)*col->ndata + at];
	pthread_mutex_unlock(&col->mutex);

	return res;
}

static int scl_inter(lua_State *L){
	struct SelCollectionStorage **p = (struct SelCollectionStorage **)lua_touserdata(L, lua_upvalueindex(1));
	struct SelCollectionStorage *col = *p;

	pthread_mutex_trylock(&col->mutex);
	if(col->cidx < col->last) {
		if(col->ndata == 1)
			lua_pushnumber(L,  col->data[ col->cidx % col->size ]);
		else {
			lua_newtable(L);	/* table result */
			for(size_t j=0; j<col->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, col->data[ (col->cidx % col->size)*col->ndata + j ]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		col->cidx++;
		pthread_mutex_unlock(&col->mutex);
		return 1;
	} else {
		pthread_mutex_unlock(&col->mutex);
		return 0;
	}
}

static int scl_idata(lua_State *L){
/** 
 * Collection's Iterator
 *
 * @function iData
 * @usage
for d in col:iData() do print(d) end
 */
	struct SelCollectionStorage *col = checkSelCollection(L);

	if(!col->last && !col->full)
		return 0;

	pthread_mutex_lock(&col->mutex);
	col->cidx = col->full ? col->last - col->size : 0;
	lua_pushcclosure(L, scl_inter, 1);
	pthread_mutex_unlock(&col->mutex);

	return 1;
}

static bool scc_save(struct SelCollectionStorage *col, const char *filename){
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
	fprintf(f, "SCMV %ld\n", col->ndata);

		/* Average values */
	if(col->full)
		for(size_t i = col->last - col->size; i < col->last; i++){
			fputc('d', f);
			for(size_t j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->data[(i % col->size)*col->ndata + j]);
			fputs("\n",f);
		}
	else
		for(size_t i = 0; i < col->last; i++){
			fputc('d', f);
			for(size_t j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->data[i*col->ndata + j]);
			fputs("\n",f);
		}

	pthread_mutex_unlock(&col->mutex);

	fclose(f);
	return true;
}

static bool scc_load(struct SelCollectionStorage *col, const char *filename){
	size_t j;

 	FILE *f = fopen(filename, "r");
	if(!f){
		selLog->Log('E', "%s : %s", filename, strerror(errno));
		return false;
	}

	if(!fscanf(f, "SCMV %ld", &j)){
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
			for(size_t j = 0; j < col->ndata; j++)
				fscanf(f, "%lf", &col->data[(col->last % col->size)*col->ndata + j]);
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

static int scl_save(lua_State *L){
/** 
 * Save the collection to a file
 *
 * @function Save
 * @tparam string filename
 * @usage
col:Save('/tmp/tst.dt', false)
 */
	struct SelCollectionStorage *col = checkSelCollection(L);
	const char *s = lua_tostring( L, 2 );

	selCollection.save(col, s);

	return 0;
}

static int scl_load(lua_State *L){
/** 
 * Load the collection from a file
 *
 * @function Load
 * @tparam string filename
 * @usage
col:Save('/tmp/tst.dt', false)
 */
	struct SelCollectionStorage *col = checkSelCollection(L);
	const char *s = lua_tostring( L, 2 );

	selCollection.load(col, s);

	return 0;
}

static const struct luaL_Reg SelCollectionM [] = {
	{"dump", scl_dump},
	{"Push", scl_push},
	{"MinMax", scl_minmax},
	{"iData", scl_idata},
	{"Clear", scl_clear},
	{"GetSize", scl_getsize},
	{"Getn", scl_getn},
	{"HowMany", scl_HowMany},
	{"Save", scl_save},
	{"Load", scl_load},
	{NULL, NULL}
};

static const struct luaL_Reg SelCollectionLib [] = {
	{"Create", scl_create},
	{"Find", scl_find},
	{NULL, NULL}
};

static void registerSelCollection(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelCollection", SelCollectionLib);
	selLua->objFuncs(L, "SelCollection", SelCollectionM);
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
	selCollection.find = scc_find;
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
	selCollection.save = scc_save;
	selCollection.load = scc_load;

	registerModule((struct SelModule *)&selCollection);

	if(selLua){	/* Only if Lua is used */
		registerSelCollection(NULL);
		selLua->AddStartupFunc(registerSelCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

	return true;
}
