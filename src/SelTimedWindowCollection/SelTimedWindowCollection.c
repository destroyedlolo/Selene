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
#include <errno.h>

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

static struct SelTimedWindowCollection selTimedWindowCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

struct SelTimedWindowCollectionStorage *checkSelTimedWindowCollection(lua_State *L){
	void *r = selLua->testudata(L, 1, "SelTimedWindowCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelTimedWindowCollection' expected");
	return *(struct SelTimedWindowCollectionStorage **)r;
}

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
		selLog->Log('D', "SelTimedWindowCollection's Dump (size : %lu, group : %lu, EMPTY)", col->size, col->group);
		return;
	}

	selLog->Log('D', "SelTimedWindowCollection's Dump (size : %lu, group : %lu, last : %lu) %s size: %ld", col->size, col->group, col->last, col->full ? "Full":"Incomplete", col->group);

	if(col->full)
		for(size_t j = col->last - col->size +1; j <= col->last; j++){
			int i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			if(col->data[i].num)
				selLog->Log('D', "[%ld] min: %lf / max: %lf / avg: %lf @ %s", j, col->data[i].min_data, col->data[i].max_data, col->data[i].sum/col->data[i].num, selCore->ctime(&t, NULL, 0));
			else
				selLog->Log('D', "Empty record");
		}
	else
		for(size_t i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			if(col->data[i].num)
				selLog->Log('D', "[%ld] min: %lf / max: %lf / avg: %lf @ %s", i, col->data[i].min_data, col->data[i].max_data, col->data[i].sum/col->data[i].num, selCore->ctime(&t, NULL, 0));
			else
				selLog->Log('D', "Empty record");
		}

	pthread_mutex_unlock(&col->mutex);
}

static int stwl_dump(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	selTimedWindowCollection.module.dump(col);

	return 0;
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

static int stwl_find(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = selTimedWindowCollection.find(luaL_checkstring(L, 1), 0);
	if(!col)
		return 0;

	struct SelTimedWindowCollectionStorage **r = lua_newuserdata(L, sizeof(struct SelTimedWindowCollectionStorage *));
	assert(r);

	luaL_getmetatable(L, "SelTimedWindowCollection");
	lua_setmetatable(L, -2);
	*r = col;

	return 1;
}

static struct SelTimedWindowCollectionStorage *stwc_create(const char *name, size_t size, size_t group){
/** 
 * Create a new SelTimedWindowCollection
 *
 * @function Create
 * @tparam string name of the the collection (can be NIL)
 * @tparam number size size of the collection
 * @tparam number group seconds to be grouped in records
 */
	struct SelTimedWindowCollectionStorage *col;

	if(name){
		unsigned int h = selL_hash(name);
		col = stwc_find(name, h);
		if(col)
			return col;
	}

	col = malloc(sizeof(struct SelTimedWindowCollectionStorage));
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
	col->full = false;

		/* Register this collection */
	if(name)
		selCore->registerObject((struct SelModule *)&selTimedWindowCollection, (struct _SelObject *)col, strdup(name));

	MCHECK;
	return col;
}

static int stwl_create(lua_State *L){
	const char *name = lua_tostring(L, 1);	/* Name of the collection */
	int size, group;

	if((size = luaL_checkinteger( L, 2 )) <= 0){
		selLog->Log('F', "SelTimedWindowCollection's size can't be null or negative");
		exit(EXIT_FAILURE);
	}

	if((group = lua_tointeger( L, 3 )) < 1)
		group = 1;
	
	struct SelTimedWindowCollectionStorage **col = (struct SelTimedWindowCollectionStorage **)lua_newuserdata(L, sizeof(struct SelTimedWindowCollectionStorage *));
	assert(col);

	luaL_getmetatable(L, "SelTimedWindowCollection");
	lua_setmetatable(L, -2);

	*col = stwc_create(name, size, group);

	return 1;
}

	/* ***
	 * stwi_ : Internal functions that doesn't lock the collection
	 * ***/

static inline int stwi_secw(struct SelTimedWindowCollectionStorage *col, time_t t){
/* <- segregator corresponding to given time */
	return(t/col->group);
}

static struct timedwdata *stwi_new(struct SelTimedWindowCollectionStorage *col, time_t t){
/* Create and add a new record.
 * Notez-bien : it's an internal function which is not locking the collection.
 */
	col->last++;
	if(col->last > col->size)
		col->full = true;

	col->data[col->last % col->size].num = 0;	/* Empty record */
	col->data[col->last % col->size].t = stwi_secw(col, t);

	return &col->data[col->last % col->size];
}

static struct timedwdata *stwi_getrecord(struct SelTimedWindowCollectionStorage *col, time_t t){
/* Get the storage corresponding to the given time.
 * Check if the time belong to the last storage, otherwise, create a new one.
 *
 * Notez-bien : it's meaning data MUST be inserted in chronological order.
 */
	if(col->last == (unsigned int)-1)	/* Empty collection : create the 1st record */
		return stwi_new(col, t);

	int i = col->last % col->size;
	if(col->data[i].t == stwi_secw(col, t)) /* is it the last one ? */
		return &col->data[i];
	else
		return stwi_new(col, t);
}

static void stwi_insert(struct timedwdata *dt, lua_Number v){
/* update dt record with v value
 */

	if(!dt->num){	/* First data */
		dt->num = 1;
		dt->min_data = dt->max_data = dt->sum = v;
	} else {	/* Not a virgin record */
		dt->num++;
		dt->sum += v;

		if(v < dt->min_data)
			dt->min_data = v;
		if(v > dt->max_data)
			dt->max_data = v;
	}
}

static void stwc_push(struct SelTimedWindowCollectionStorage *col, lua_Number v, time_t t){
/** 
 * Push a new value
 *
 * @function Push
 * @tparam number value value to push
 * @tparam ?integer|nil timestamp Current timestamp by default
 */
	pthread_mutex_lock(&col->mutex);

	if(!t)
		t = time(NULL);

	struct timedwdata *dt = stwi_getrecord(col, t);
	stwi_insert(dt, v);
	
	pthread_mutex_unlock(&col->mutex);
}

static int stwl_push(lua_State *L){
/** 
 * Push a new value
 *
 * @function Push
 * @tparam ?number|table value single value or table of numbers in case of multi values collection
 * @tparam ?integer|nil timestamp Current timestamp by default
 */
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);

	lua_Number dt = luaL_checknumber( L, 2 );

	stwc_push(col, dt, (lua_type( L, 3 ) == LUA_TNUMBER) ? lua_tonumber( L, 3 ) : time(NULL));

	return 0;
}

static bool stwc_minmax(struct SelTimedWindowCollectionStorage *col, lua_Number *min, lua_Number *max, lua_Number *avg, double *dtime){
/** 
 * Calculates the minimum and the maximum of this collection.
 *
 * @function MinMax
 * @treturn number minium
 * @treturn number maximum
 * @treturn number average
 * @treturn number time span
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	if(col->last == (unsigned int)-1)
		return false;

	pthread_mutex_lock(&col->mutex);

	lua_Number sum;
	size_t num;
	time_t first;
	size_t ifirst = col->full ? col->last - col->size +1 : 0;;	/* Index of the 1st data */

	*min = col->data[ ifirst % col->size ].min_data;
	*max = col->data[ ifirst % col->size ].max_data;
	sum = col->data[ ifirst % col->size ].sum;
	num = col->data[ ifirst % col->size ].num;
	first = col->data[ ifirst % col->size ].t * col->group;

	for(size_t i = ifirst+1; i <= col->last; i++){
		if(col->data[ i % col->size ].min_data < *min)
			*min = col->data[ i % col->size ].min_data;
		if(col->data[ i % col->size ].max_data > *max)
			*max = col->data[ i % col->size ].max_data;
		sum += col->data[ i % col->size ].sum;
		num += col->data[ i % col->size ].num;
		*dtime = difftime(col->data[ i % col->size ].t * col->group, first);
	}

	pthread_mutex_unlock(&col->mutex);

	if(!num)	/* Normally, not needed : safety first */
		return false;
	*avg = sum/num;

	return true;
}

static int stwl_minmax(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);

	lua_Number min, max, avg;
	double diff;

	if(!stwc_minmax(col, &min, &max, &avg, &diff)){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		selLog->Log('D', "MinMax() on an empty collection");
		return 2;
	}

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	lua_pushnumber(L, avg);
	lua_pushnumber(L, diff);

	return 4;
}

static bool stwc_diffminmax(struct SelTimedWindowCollectionStorage *col, lua_Number *min, lua_Number *max){
/** 
 * Calculates the minimum and maximum data windows of the collection.
 *
 * @function DiffMinMax
 * @treturn number minium
 * @treturn number maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	if(col->last == (unsigned int)-1)
		return false;

	pthread_mutex_lock(&col->mutex);

	size_t ifirst = col->full ? col->last - col->size +1 : 0;;	/* Index of the 1st data */
	*min = *max = col->data[ ifirst % col->size ].max_data - col->data[ ifirst % col->size ].min_data;

	for(size_t i = ifirst+1; i <= col->last; i++){
		lua_Number d = col->data[ i % col->size ].max_data - col->data[ i % col->size ].min_data;
		if( d < *min )
			*min = d;
		if( d > *max )
			*max = d;
	}

	pthread_mutex_unlock(&col->mutex);
	return true;
}

static int stwl_diffminmax(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	lua_Number min,max;
	
	if(!stwc_diffminmax(col, &min, &max)){
		lua_pushnil(L);
		lua_pushstring(L, "DiffMinMax() on an empty collection");
		selLog->Log('D', "DiffMinMax() on an empty collection");
		return 2;
	}

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);

	return 2;
}

static size_t stwc_getsize(struct SelTimedWindowCollectionStorage *col){
	return(col->size);
}

static int stwl_getsize(lua_State *L){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	lua_pushnumber(L, col->size);
	return 1;
}

static size_t stwc_howmany(struct SelTimedWindowCollectionStorage *col){
	return(col->full ? col->size : col->last+1);
}

static int stwl_howmany(lua_State *L){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	lua_pushnumber(L, col->full ? col->size : col->last+1);
	return 1;
}

static size_t stwc_getgrouping(struct SelTimedWindowCollectionStorage *col){
	return(col->group);
}

static int stwl_getgrouping(lua_State *L){
/** 
 * Number of seconds grouped in a window
 *
 * @function GetGrouping
 * @treturn number Number of seconds grouped in a window
 */
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	lua_pushnumber(L, col->group);
	return 1;
}

static void stwc_clear(struct SelTimedWindowCollectionStorage *col){
/**
 * Make the collection empty
 *
 * @function Clear
 */
	pthread_mutex_lock(&col->mutex);

	col->last = (unsigned int)-1;
	col->full = 0;

	pthread_mutex_unlock(&col->mutex);
}

static int stwl_clear(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	stwc_clear(col);
	return 0;
}

static size_t stwc_firstidx(struct SelTimedWindowCollectionStorage *col){
	return(col->full ? col->last - col->size +1 : 0);
}

static size_t stwc_lastidx(struct SelTimedWindowCollectionStorage *col){
	return(col->last);
}

static bool stwc_get(struct SelTimedWindowCollectionStorage *col, size_t i, lua_Number *min,  lua_Number *max, lua_Number *avg, time_t *time){
/**
 * Retrieves the records at the given index
 *
 * @function get
 * @tparam number index
 * @treturn boolean true if data exists
 * @treturn number minimum value
 * @treturn number maximum value
 * @treturn number average value
 * @treturn time_t corresponding timestamp
 */
	if(col->last == (unsigned int)-1 || i > col->last)	/* Out of range */
		return false;

	if(i < stwc_firstidx(col))	/* already ejected */
		return false;

	pthread_mutex_lock(&col->mutex);
	
	i %= col->size;
	if(!col->data[i].num){	/* Normally, shouldn't happen */
		pthread_mutex_unlock(&col->mutex);
		return false;
	}

	*min = col->data[i].min_data;
	*max = col->data[i].max_data;
	*avg = col->data[i].sum/col->data[i].num;
	*time = col->data[i].t * col->group;

	pthread_mutex_unlock(&col->mutex);

	return true;
}

static bool stwc_save(struct SelTimedWindowCollectionStorage *col, const char *fch){
/** 
 * Save the collection to a file
 *
 * @function Save
 * @tparam string filename
 * @usage
col:Save('/tmp/tst.dt')
 */
	if(col->last == (unsigned int)-1){
		selLog->Log('D', "Save() on an empty collection");
		return false;
	}

	FILE *f = fopen( fch, "w" );
	if(!f){
		selLog->Log('E', "%s : %s", fch, strerror(errno));
		return false;
	}

	fprintf(f, "STWC %lu\n", col->group);

	pthread_mutex_lock(&col->mutex);

	if(col->full)
		for(size_t j = col->last - col->size +1; j <= col->last; j++){
			size_t i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			fprintf(f, "%lf/%lf/%lf/%lu@%ld\n", col->data[i].min_data, col->data[i].max_data, col->data[i].sum, col->data[i].num, t);
		}
	else
		for(size_t i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			fprintf(f, "%lf/%lf/%lf/%lu@%ld\n", col->data[i].min_data, col->data[i].max_data, col->data[i].sum, col->data[i].num, t);
		}

	pthread_mutex_unlock(&col->mutex);

	fclose(f);
	return true;
}

static int stwl_Save(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	const char *s = luaL_checkstring(L, 2);

	if(!stwc_save(col, s)){
		lua_pushnil(L);
		lua_pushstring(L, "Save() failed");
		return 2;
	}

	return 0;
}

static bool stwc_load(struct SelTimedWindowCollectionStorage *col, const char *fch){
/** 
 * load the collection to a file
 *
 * @function Load
 * @tparam string filename
 * @usage
col:Load('/tmp/tst.dt')
 */
	size_t j;

 	FILE *f = fopen(fch, "r");
	if(!f){
		selLog->Log('E', "%s : %s", fch, strerror(errno));
		return false;
	}

	if(!fscanf(f, "STWC %lu", &j)){
		selLog->Log('E', "Nagic not found");
		fclose(f);
		return false;
	}
	
	if(j != col->group){
		selLog->Log('E', "Grouping doesn't match (%lu vs %lu)", j, col->group);
		fclose(f);
		return false;
	}
	
	pthread_mutex_lock(&col->mutex);

		/* As SelTimedWindowCollection contains only boundaries and
		 * summaries, data can't be simply pushed on an existing
		 * collection
		 */
	if(col->last != (unsigned int)-1){
		selLog->Log('E', "Collection must be empty");
		fclose(f);
		pthread_mutex_unlock(&col->mutex);
		return false;
	}

	lua_Number min, max, sum;
	size_t num;
	time_t t;

	while( fscanf(f, "%lf/%lf/%lf/%lu@%ld\n", &min, &max, &sum, &num, &t) != EOF){
		/* allocate a new record */
		col->last++;
		if(col->last > col->size)
			col->full = true;

		col->data[col->last % col->size].min_data = min;
		col->data[col->last % col->size].max_data = max;
		col->data[col->last % col->size].sum = sum;
		col->data[col->last % col->size].num = num;
		col->data[col->last % col->size].t = t / col->group;
	}

	pthread_mutex_unlock(&col->mutex);
	fclose(f);
	return true;
}

static int stwl_Load(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);
	const char *s = luaL_checkstring(L, 2);

	if(!stwc_load(col, s)){
		lua_pushnil(L);
		lua_pushstring(L, "Load() failed");
		return 2;
	}

	return 0;
}
	/* Iterator */
static int stwi_inter(lua_State *L){
	struct SelTimedWindowCollectionStorage *col = *(struct SelTimedWindowCollectionStorage **)lua_touserdata(L, lua_upvalueindex(1));

	if(col->cidx <= col->last) {
		pthread_mutex_lock(&col->mutex);

		lua_pushnumber(L,  col->data[ col->cidx % col->size ].min_data);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].max_data);
		if(col->data[ col->cidx % col->size ].num)
			lua_pushnumber(L, col->data[ col->cidx % col->size ].sum/col->data[ col->cidx % col->size ].num);
		else
			lua_pushnil(L);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].t * col->group);
		col->cidx++;

		pthread_mutex_unlock(&col->mutex);
		return 4;
	} else
		return 0;
}

static int stwl_idata(lua_State *L){
/** 
 * Collection's Iterator
 *
 * @function iData
 */
	struct SelTimedWindowCollectionStorage *col = checkSelTimedWindowCollection(L);

	if(col->last == (unsigned int)-1)
		return 0;

	pthread_mutex_lock(&col->mutex);
	col->cidx = col->full ? col->last - col->size +1 : 0;
	lua_pushcclosure(L, stwi_inter, 1);
	pthread_mutex_unlock(&col->mutex);
	
	return 1;
}

static const struct luaL_Reg SelTimedWindowCollectionLib [] = {
	{"Create", stwl_create}, 
	{"Find", stwl_find},
	{NULL, NULL}
};

static const struct luaL_Reg SelTimedWindowCollectionM [] = {
	{"Push", stwl_push},
	{"MinMax", stwl_minmax},
	{"DiffMinMax", stwl_diffminmax},
	{"iData", stwl_idata},
	{"GetSize", stwl_getsize},
	{"HowMany", stwl_howmany},
	{"GetGrouping", stwl_getgrouping},
	{"Save", stwl_Save},
	{"Load", stwl_Load},
	{"Clear", stwl_clear},
	{"dump", stwl_dump},
	{NULL, NULL}
};

static void registerSelTimedWindowCollection(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelTimedWindowCollection", SelTimedWindowCollectionLib);
	selLua->objFuncs(L, "SelTimedWindowCollection", SelTimedWindowCollectionM);
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
	selTimedWindowCollection.minmax = stwc_minmax;
	selTimedWindowCollection.diffminmax = stwc_diffminmax;
	selTimedWindowCollection.getsize = stwc_getsize;
	selTimedWindowCollection.howmany = stwc_howmany;
	selTimedWindowCollection.getgrouping = stwc_getgrouping;
	selTimedWindowCollection.clear = stwc_clear;
	selTimedWindowCollection.get = stwc_get;
	selTimedWindowCollection.firstidx = stwc_firstidx;
	selTimedWindowCollection.lastidx = stwc_lastidx;
	selTimedWindowCollection.save = stwc_save;
	selTimedWindowCollection.load = stwc_load;

	registerModule((struct SelModule *)&selTimedWindowCollection);

	if(selLua){	/* Only if Lua is used */
		registerSelTimedWindowCollection(NULL);
		selLua->AddStartupFunc(registerSelTimedWindowCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

	return true;
}
