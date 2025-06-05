/***
This collection stores **immediate** and **average** values sub collection.


  - **Immediate** values : a collection of up to *isize* values;
When a additional one is pushed, the oldest one is pushed out.

  - **Average** values : when *group* **new** data are available in the immediate collection,
their average value is calculated and then pushed into this list.


----------------------
Typical usage : to store frequent metrics (like energy counter) and display both the recent values
and a long term trend to avoid too large curve.

@classmod SelAverageCollection

* History :
* 11/06/2022 LF : creation
* 27/03/2024 LF : migrate to v7
* 07/04/2024 LF : Switch to named collection

@usage
local col = SelAverageCollection.Create("my collection",5,7,3)

for i=1,4 do
	col:Push(i)
end
print( "Size : ", col:GetSize() )
print( "HowMany : ", col:HowMany() )
col:dump() 
 */

#include <Selene/SelAverageCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelAverageCollectionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if LUA_VERSION_NUM == 501
#	define lua_rawlen lua_objlen
#endif

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

static struct SelAverageCollection selAverageCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SelAverageCollectionStorage *checkSelAverageCollection(lua_State *L){
	void *r = selLua->testudata(L, 1, "SelAverageCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelAverageCollection' expected");
	return *(struct SelAverageCollectionStorage **)r;
}

#define BUFFSZ	1023

static void sacc_dump(void *acol){
/** 
 * Display collection's content (for debugging purposes).
 *
 * @function dump
 *
 */
	struct SelAverageCollectionStorage *col = acol;	/* Avoid zillion of casts */
	unsigned int i,j;
	char t[BUFFSZ+1];
	char tn[64];

	pthread_mutex_lock(&col->mutex);

	selLog->Log('D', "SelAverageCollection's Dump (size : %d x %d (g:%d), last : %d)", col->isize, col->ndata, col->group, col->ilast);
	selLog->Log('D', "immediate :");

	if(col->ifull)
		for(i = col->ilast - col->isize; i < col->ilast; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%lf ", col->immediate[i % col->isize].data[j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}
	else
		for(i = 0; i < col->ilast; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%lf ", col->immediate[i].data[j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}

	selLog->Log('D', "Average :");

	if(col->afull)
		for(i = col->alast - col->asize; i < col->alast; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%lf ", col->average[i % col->asize].data[j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}
	else
		for(i = 0; i < col->alast; i++){
			*t = 0;
			for(j = 0; j < col->ndata; j++){
				sprintf(tn, "%lf ", col->average[i].data[j]);
				strncat(t, tn, BUFFSZ);
			}
			selLog->Log('D', "\t%s", t);
		}

	pthread_mutex_unlock(&col->mutex);
}

static int sacl_dump(lua_State *L){
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	selAverageCollection.module.dump(col);

	return 0;
}

static struct SelAverageCollectionStorage *sacc_find(const char *name, unsigned int h){
/** 
 * Find a SelAverageCollection by its name.
 *
 * @function Find
 * @tparam string name Name of the Collection
 * @param int hash code (recomputed if null)
 * @treturn ?SelAverageCollection|nil
 */
	return((struct SelAverageCollectionStorage *)selCore->findNamedObject((struct SelModule *)&selAverageCollection, name, h));

}

static int sacl_find(lua_State *L){
	struct SelAverageCollectionStorage *col = selAverageCollection.find(luaL_checkstring(L, 1), 0);
	if(!col)
		return 0;

	struct SelAverageCollectionStorage **r = lua_newuserdata(L, sizeof(struct SelAverageCollectionStorage *));
	assert(r);
	luaL_getmetatable(L, "SelAverageCollection");
	lua_setmetatable(L, -2);
	*r = col;

	return 1;
}

static struct SelAverageCollectionStorage *sacc_create(const char *name, size_t isize, size_t asize, size_t grouping, size_t ndata){
/** 
 * Create a new SelAverageCollection
 *
 * @function Create
 * @tparam string collection's name (can be nil for unamed)
 * @tparam num isize size of the immediate collection
 * @tparam num asize size of the average collection
 * @tparam num grouping value
 * @tparam num amount of values per sample (optional, default **1**)
 *
 * @usage
 col = SelAverageCollection.Create("my name",5,7,3)
 */
	struct SelAverageCollectionStorage *col;

	if(name){
		unsigned int h = selL_hash(name);
		col = sacc_find(name, h);
		if(col)
			return col;
	}

	col = malloc(sizeof(struct SelAverageCollectionStorage));
	assert(col);

	pthread_mutex_init(&col->mutex, NULL);

	col->isize = isize;
	col->asize = asize;
	col->group = grouping;
	col->ndata = ndata ? ndata:1;

	if(!isize || !asize || !grouping){
		selLog->Log('F', "SelAverageCollection's size can't be null");
		exit(EXIT_FAILURE);
	}
	if(grouping > isize){
		selLog->Log('F', "SelAverageCollection's grouping can't be larger than immediate size");
		exit(EXIT_FAILURE);
	}

	assert( (col->immediate = calloc(col->isize, sizeof(struct imaveragedata))) );
	for(size_t i=0; i<col->isize; i++)
		assert( (col->immediate[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->ilast = 0;
	col->ifull = false;

	assert( (col->average = calloc(col->asize, sizeof(struct imaveragedata))) );
	for(size_t i=0; i<col->asize; i++)
		assert( (col->average[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->alast = 0;
	col->afull = false;

		/* Register this collection */
	if(name)
		selCore->registerNamedObject((struct SelModule *)&selAverageCollection, (struct _SelNamedObject *)col, strdup(name));
	else
		selCore->initObject((struct SelModule *)&selAverageCollection, (struct SelObject *)col);

	MCHECK;
	return col;
}

static int sacl_create(lua_State *L){
	const char *name = lua_tostring(L, 1);	/* Name of the collection */
	size_t isize, asize, group, ndata;
	if((isize = luaL_checkinteger( L, 2 )) <= 0)
		return luaL_error(L, "SelAverageCollection's immediate size can't be null or negative");
	if((asize = luaL_checkinteger( L, 3 )) <= 0)
		return luaL_error(L, "SelAverageCollection's average size can't be null or negative\n");
	if((group = luaL_checkinteger( L, 4 )) <= 0)
		return luaL_error(L, "SelAverageCollection's grouping can't be null or negative");
	if((ndata = lua_tointeger( L, 5 )) < 1)
		ndata = 1;

	if(isize < group)
		return luaL_error(L, "SelAverageCollection's grouping can't be > to immediate sample size");

	struct SelAverageCollectionStorage **p = (struct SelAverageCollectionStorage **)lua_newuserdata(L, sizeof(struct SelAverageCollectionStorage *));
	assert(p);
	*p = sacc_create(name, isize, asize, group, ndata);

	luaL_getmetatable(L, "SelAverageCollection");
	lua_setmetatable(L, -2);

	MCHECK;
	return 1;
}

static void sacc_postinsert(struct SelAverageCollectionStorage *col){
/* Common processing to be done after data insertion in 
 * sacc_push() and sacl_push()
 */
	col->ilast++;

	if(col->ilast > col->isize)
		col->ifull = true;

		/****
		 * Update average values if needed
		 ****/

	if(!(col->ilast % col->group)){	/* push a new average */
		unsigned int i,j;

		for( j = 0; j < col->ndata; j++)
			col->average[col->alast % col->asize].data[j] = 0;

		for(i = col->ilast - col->group; i < col->ilast; i++){
			for(j = 0; j < col->ndata; j++)
				col->average[col->alast % col->asize].data[j] += col->immediate[i % col->isize].data[j];
		}

		for(j = 0; j < col->ndata; j++)
			col->average[col->alast % col->asize].data[j] /= col->group;

		if(++col->alast > col->asize)
			col->afull = true;
	}
}

static bool sacc_push(struct SelAverageCollectionStorage *col, size_t num, ...){
	if(col->ndata != num){
		selLog->Log('E', "Number of arguments mismatch");
		return false;
	}

	va_list ap;
	va_start(ap, num);
	pthread_mutex_lock(&col->mutex);

	for(size_t j=0; j<num; j++){
		lua_Number val = va_arg(ap, lua_Number);
		col->immediate[col->ilast % col->isize].data[j] = val;
	}
	va_end(ap);
	
	sacc_postinsert(col);

	pthread_mutex_unlock(&col->mutex);

	return true;
}

static int sacl_push(lua_State *L){
/** 
 * Push a new sample.
 *
 * If afterward *group* samples are waiting, a new average is calculated and inserted.
 *
 * @function Push
 * @tparam ?number|table value single value or table of numbers in case of multi values collection
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	pthread_mutex_lock(&col->mutex);	/* Lock the collection */

	if(!lua_istable(L, 2)){	/* One value, old interface */
		if(col->ndata > 1){
			pthread_mutex_unlock(&col->mutex); /* Error : the collection needs to be released before raising the error */
			luaL_error(L, "Pushing a single number on multi-valued AverageCollection");
		}

		col->immediate[col->ilast % col->isize].data[0] = luaL_checknumber(L, 2);
	} else {	/* Table provided */
		if(lua_rawlen(L,2) != col->ndata){
			pthread_mutex_unlock(&col->mutex);
			luaL_error(L, "Expecting %d data per sample", col->ndata);
		}

		for(size_t j = 0; j < col->ndata; j++){
			lua_rawgeti(L, 2, j+1);
			col->immediate[col->ilast % col->isize].data[j] = luaL_checknumber(L, -1);
			lua_pop(L,1);
		}
	}

	sacc_postinsert(col);

	pthread_mutex_unlock(&col->mutex);

	return 0;
}

static bool sacc_minmaxIs(struct SelAverageCollectionStorage *col, lua_Number *min, lua_Number *max){
	if(col->ndata != 1){
		selLog->Log('E', "SelAverageCollectionStorage.minmaxis() can deal only with single value collection");
		return false;
	}

	if(!col->ilast && !col->ifull){
		selLog->Log('D', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);
	size_t ifirst = col->ifull ? col->ilast - col->isize : 0;
	*min = *max = *col->immediate[ifirst % col->isize].data;

	for(size_t i = ifirst; i < col->ilast; i++){
		lua_Number v = *col->immediate[i % col->isize].data;
		if(v < *min)
			*min = v;
		if(v > *max)
			*max = v;
	}
	pthread_mutex_unlock(&col->mutex);

	return true;
}

static bool sacc_minmaxI(struct SelAverageCollectionStorage *col, lua_Number *min, lua_Number *max){
	if(!col->ilast && !col->ifull){
		selLog->Log('D', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);
	size_t ifirst = col->ifull ? col->ilast - col->isize : 0;
	for(size_t j=0; j<col->ndata; j++)
		min[j] = max[j] = col->immediate[ifirst % col->isize].data[j];

	for(size_t i = ifirst; i < col->ilast; i++){
		for(size_t j = 0; j < col->ndata; j++){
			lua_Number v = col->immediate[i % col->isize].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}
	pthread_mutex_unlock(&col->mutex);

	return true;
}

static void sacl_pubminmax(lua_State *L, struct SelAverageCollectionStorage *col, lua_Number *min, lua_Number *max){
/* push min / max tables */
	
	if(selAverageCollection.getn(col) == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for(size_t j=0; j<selAverageCollection.getn(col); j++){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for(size_t j=0; j<selAverageCollection.getn(col); j++){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}
}

static int sacl_minmaxI(lua_State *L){
/** 
 * Calculates the minimum and the maximum of the **immediate** part.
 *
 * @function MinMaxI
 * @treturn ?number|table minium
 * @treturn ?number|table maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	lua_Number min[selAverageCollection.getn(col)], max[selAverageCollection.getn(col)];
	selAverageCollection.minmaxI(col, min, max);

	sacl_pubminmax(L, col, min, max);

	MCHECK;
	return 2;
}

static bool sacc_minmaxAs(struct SelAverageCollectionStorage *col, lua_Number *min, lua_Number *max){
	if(col->ndata != 1){
		selLog->Log('E', "SelAverageCollectionStorage.minmaxi() can deal only with single value collection");
		return false;
	}

	if(!col->alast && !col->afull){
		selLog->Log('D', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);
	size_t afirst = col->afull ? col->alast - col->asize : 0;
	*min = *max = *col->average[afirst % col->asize].data;

	for(size_t i = afirst; i < col->alast; i++){
		lua_Number v = *col->average[i % col->asize].data;
		if(v < *min)
			*min = v;
		if(v > *max)
			*max = v;
	}
	pthread_mutex_unlock(&col->mutex);

	return true;
}

static bool sacc_minmaxA(struct SelAverageCollectionStorage *col, lua_Number *min, lua_Number *max){
	if(!col->alast && !col->afull){
		selLog->Log('D', "MinMax() on an empty collection");
		return false;
	}

	pthread_mutex_lock(&col->mutex);
	size_t afirst = col->afull ? col->alast - col->asize : 0;
	for(size_t j=0; j<col->ndata; j++)
		min[j] = max[j] = col->average[afirst % col->asize].data[j];

	for(size_t i = afirst; i < col->alast; i++){
		for(size_t j = 0; j < col->ndata; j++){
			lua_Number v = col->average[i % col->asize].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}
	pthread_mutex_unlock(&col->mutex);

	return true;
}

static int sacl_minmaxA(lua_State *L){
/** 
 * Calculates the minimum and the maximum of the **average** part.
 *
 * @function MinMaxA
 * @treturn ?number|table minimum
 * @treturn ?number|table maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	lua_Number min[selAverageCollection.getn(col)], max[selAverageCollection.getn(col)];
	selAverageCollection.minmaxA(col, min, max);

	sacl_pubminmax(L, col, min, max);

	MCHECK;
	return 2;
}

static int sacl_minmax(lua_State *L){
/** 
 * MinMax for both immediate & average value
 *
 * @function MinMax
 * @treturn ?number|table minium immediate
 * @treturn ?number|table maximum immediate
 * @treturn ?number|table minium average
 * @treturn ?number|table maximum average
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	lua_Number min[selAverageCollection.getn(col)], max[selAverageCollection.getn(col)];
	selAverageCollection.minmaxI(col, min, max);
	sacl_pubminmax(L, col, min, max);

	selAverageCollection.minmaxA(col, min, max);
	sacl_pubminmax(L, col, min, max);

	MCHECK;
	return 4;
}

static size_t sacc_getn(struct SelAverageCollectionStorage *col){
/** 
 * Number of entries per sample
 *
 * @function Getn
 * @treturn num Amount of data per sample
 */
	return(col->ndata);
}

static size_t sacc_getsizeI(struct SelAverageCollectionStorage *col){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	return(col->isize);
}

static size_t sacc_getsizeA(struct SelAverageCollectionStorage *col){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	return(col->asize);
}

static int sacl_getsize(lua_State *L){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num immediate
 * @treturn num average
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);

	pthread_mutex_lock(&col->mutex);
	lua_pushnumber(L, col->isize);
	lua_pushnumber(L, col->asize);
	pthread_mutex_unlock(&col->mutex);

	return 2;
}

static size_t sacc_howmanyI(struct SelAverageCollectionStorage *col){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	return(col->ifull ? col->isize : col->ilast);
}

static size_t sacc_howmanyA(struct SelAverageCollectionStorage *col){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	return(col->afull ? col->asize : col->alast);
}

static int sacl_HowMany(lua_State *L){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num immediate
 * @treturn num average
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);

	pthread_mutex_lock(&col->mutex);
	lua_pushnumber(L, col->ifull ? col->isize : col->ilast);
	lua_pushnumber(L, col->afull ? col->asize : col->alast);
	pthread_mutex_unlock(&col->mutex);

	return 2;
}

static void sacc_clear(struct SelAverageCollectionStorage *col){
/**
 * Make the collection empty
 *
 * @function Clear
 */
	pthread_mutex_lock(&col->mutex);
	col->ilast = 0;
	col->ifull = 0;

	col->alast = 0;
	col->afull = 0;
	pthread_mutex_unlock(&col->mutex);
}

static int sacl_clear(lua_State *L){
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	selAverageCollection.clear(col);

	return 0;
}

static lua_Number sacc_getsI(struct SelAverageCollectionStorage *col, size_t idx){
/**
 * Returns the value at the given position (0.0 if invalid)
 * 1st value for multi valued collection.
 * 
 * @function gets
 * @treturn lua_Number value
 */
 	lua_Number ret;
	if(idx >= selAverageCollection.howmanyI(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->ifull)
		idx += col->ilast - col->isize;	/* normalize to physical index */
	ret = *col->immediate[idx % col->isize].data;
	pthread_mutex_unlock(&col->mutex);

	return ret;
}

static lua_Number sacc_getsA(struct SelAverageCollectionStorage *col, size_t idx){
/**
 * Returns the value at the given position (0.0 if invalid)
 * 1st value for multi valued collection.
 * 
 * @function gets
 * @treturn lua_Number value
 */
	if(idx >= selAverageCollection.howmanyA(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->afull)
		idx += col->alast - col->asize;	/* normalize to physical index */
	lua_Number ret = *col->average[idx % col->asize].data;
	pthread_mutex_unlock(&col->mutex);

	return ret;
}

static lua_Number *sacc_getI(struct SelAverageCollectionStorage *col, size_t idx, lua_Number *res){
/**
 * Returns the values at the given position (0.0 if invalid)
 * 
 * @function get
 * @treturn lua_Number value
 */
	if(idx >= selAverageCollection.howmanyI(col)){
		for(size_t j=0; j<col->ndata; j++)
			res[j] = 0.0;
		return res;
	}

	pthread_mutex_lock(&col->mutex);
	if(col->ifull)
		idx += col->ilast - col->isize;	/* normalize to physical index */
	for(size_t j=0; j<col->ndata; j++)
		res[j] = col->immediate[idx % col->isize].data[j];
	pthread_mutex_unlock(&col->mutex);

	return res;
}

static lua_Number *sacc_getA(struct SelAverageCollectionStorage *col, size_t idx, lua_Number *res){
/**
 * Returns the values at the given position (0.0 if invalid)
 * 
 * @function get
 * @treturn lua_Number value
 */
	if(idx >= selAverageCollection.howmanyA(col)){
		for(size_t j=0; j<col->ndata; j++)
			res[j] = 0.0;
		return res;
	}

	pthread_mutex_lock(&col->mutex);
	if(col->afull)
		idx += col->alast - col->asize;	/* normalize to physical index */
	for(size_t j=0; j<col->ndata; j++)
		res[j] = col->average[idx % col->asize].data[j];
	pthread_mutex_unlock(&col->mutex);

	return res;
}

static lua_Number sacc_getatI(struct SelAverageCollectionStorage *col, size_t idx, size_t at){
	if(idx >= selAverageCollection.howmanyI(col) || at >= selAverageCollection.getn(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->ifull)
		idx += col->ilast - col->isize;	/* normalize to physical index */
	lua_Number ret = col->immediate[idx % col->isize].data[at];
	pthread_mutex_unlock(&col->mutex);

	return ret;
}

static lua_Number sacc_getatA(struct SelAverageCollectionStorage *col, size_t idx, size_t at){
	if(idx >= selAverageCollection.howmanyA(col) || at >= selAverageCollection.getn(col))
		return 0.0;

	pthread_mutex_lock(&col->mutex);
	if(col->afull)
		idx += col->alast - col->asize;	/* normalize to physical index */
	lua_Number ret = col->average[idx % col->asize].data[at];
	pthread_mutex_unlock(&col->mutex);

	return ret;
}

static bool sacc_save(struct SelAverageCollectionStorage *col, const char *filename, bool average_only){
/** 
 * Save the collection to a file
 *
 * @function Save
 * @tparam string filename
 * @tparam boolean Save only average values ? Immediate are lost (optional, default **false**)
 * @usage
col:Save('/tmp/tst.dt', false)
 */
 	FILE *f = fopen(filename, "w");
	if(!f){
		selLog->Log('E', "%s : %s", filename, strerror(errno));
		return false;
	}

	pthread_mutex_lock(&col->mutex);
		/* Write Header */
	fprintf(f, "SaCMV %ld %ld\n", col->ndata, col->group);

		/* Immediate values */
	if(!average_only){
		if(col->ifull)
			for(size_t i = col->ilast - col->isize; i < col->ilast; i++){
				fputc('i', f);
				for(size_t j = 0; j < col->ndata; j++)
					fprintf(f, "\t%lf", col->immediate[i % col->isize].data[j]);
				fputs("\n",f);
			}
		else
			for(size_t i = 0; i < col->ilast; i++){
				fputc('i', f);
				for(size_t j = 0; j < col->ndata; j++)
					fprintf(f, "\t%lf", col->immediate[i].data[j]);
				fputs("\n",f);
			}
	}

		/* Average values */
	if(col->afull)
		for(size_t i = col->alast - col->asize; i < col->alast; i++){
			fputc('a', f);
			for(size_t j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->average[i % col->asize].data[j]);
			fputs("\n",f);
		}
	else
		for(size_t i = 0; i < col->alast; i++){
			fputc('a', f);
			for(size_t j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->average[i].data[j]);
			fputs("\n",f);
		}

	pthread_mutex_unlock(&col->mutex);

	fclose(f);
	return true;
}

static int sacl_save(lua_State *L){
/** 
 * Save the collection to a file
 *
 * @function Save
 * @tparam string filename
 * @tparam boolean Save only average values ? Immediate are lost (optional, default **false**)
 * @usage
col:Save('/tmp/tst.dt')
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	const char *s = lua_tostring( L, 2 );
	bool avonly = false;

	if(lua_isboolean(L,3))
		avonly = lua_toboolean(L, 3);

	selAverageCollection.save(col, s, avonly);

	return 0;
}

static bool sacc_load(struct SelAverageCollectionStorage *col, const char *filename){
	size_t i,j;

 	FILE *f = fopen(filename, "r");
	if(!f){
		selLog->Log('E', "%s : %s", filename, strerror(errno));
		return false;
	}

	if(!fscanf(f, "SaCMV %ld %ld", &j, &i)){
		selLog->Log('E', "Nagic not found");
		fclose(f);
		return false;
	}
	
	if(j != col->ndata){
		selLog->Log('E', "Amount of data doesn't match");
		fclose(f);
		return false;
	}
	
	if(i != col->group){
		selLog->Log('E', "This grouping doesn't match");
		fclose(f);
		return false;
	}

	pthread_mutex_lock(&col->mutex);
	for(;;){
		char cat;
		fscanf(f, "\n%c", &cat);
		if(feof(f))
			break;

		if(cat == 'i'){
			for(size_t j = 0; j < col->ndata; j++)
				fscanf(f, "%lf", &col->immediate[col->ilast % col->isize].data[j] );
			col->ilast++;
		} else if(cat == 'a'){
			for(size_t j = 0; j < col->ndata; j++)
				fscanf(f, "%lf", &col->average[col->alast % col->asize].data[j] );
			col->alast++;
		} else {
			pthread_mutex_unlock(&col->mutex);
			selLog->Log('E', "This grouping doesn't match");
			fclose(f);
			return false;
		}
		
		if(col->ilast > col->isize)
		col->ifull = 1;

		if(col->alast > col->asize)
			col->afull = 1;
	}
	pthread_mutex_unlock(&col->mutex);

	fclose(f);
	return true;
}

static int sacl_load(lua_State *L){
/** 
 * Load the collection from a file
 *
 * @function Load
 * @tparam string filename
 * @usage
col:Save('/tmp/tst.dt', false)
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);
	const char *s = lua_tostring( L, 2 );

	selAverageCollection.load(col, s);

	return 0;
}

static int scl_iinter(lua_State *L){
	struct SelAverageCollectionStorage *col = *(struct SelAverageCollectionStorage **)lua_touserdata(L, lua_upvalueindex(1));

	pthread_mutex_lock(&col->mutex);
	if(col->icidx < col->ilast) {
		if(col->ndata == 1)
			lua_pushnumber(L, col->immediate[col->icidx % col->isize].data[0]);
		else {
			lua_newtable(L);	/* table result */
			for(size_t j=0; j<col->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, col->immediate[col->icidx % col->isize].data[j]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		col->icidx++;
		pthread_mutex_unlock(&col->mutex);
		return 1;
	} else {
		pthread_mutex_unlock(&col->mutex);
		return 0;
	}
}

static int sacl_idata(lua_State *L){
/** 
 * Iterator for **immediate** data
 *
 * @function iData
 * @usage
for d in col:iData() do print(d) end
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);

	pthread_mutex_lock(&col->mutex);
	if(!col->ilast && !col->ifull)
		return 0;

	col->icidx = col->ifull ? col->ilast - col->isize : 0;
	lua_pushcclosure(L, scl_iinter, 1);
	pthread_mutex_unlock(&col->mutex);

	return 1;
}

static int scl_ainter(lua_State *L){
	struct SelAverageCollectionStorage *col = *(struct SelAverageCollectionStorage **)lua_touserdata(L, lua_upvalueindex(1));

	pthread_mutex_lock(&col->mutex);
	if(col->acidx < col->alast) {
		if(col->ndata == 1)
			lua_pushnumber(L, col->average[col->acidx % col->asize].data[0]);
		else {
			lua_newtable(L);	/* table result */
			for(size_t j=0; j<col->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, col->average[col->acidx % col->asize].data[j]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		col->acidx++;
		pthread_mutex_unlock(&col->mutex);
		return 1;
	} else {
		pthread_mutex_unlock(&col->mutex);
		return 0;
	}
}

static int sacl_adata(lua_State *L){
/** 
 * Iterator for **immediate** data
 *
 * @function iData
 * @usage
for d in col:iData() do print(d) end
 */
	struct SelAverageCollectionStorage *col = checkSelAverageCollection(L);

	pthread_mutex_lock(&col->mutex);
	if(!col->alast && !col->afull)
		return 0;

	col->acidx = col->afull ? col->alast - col->asize : 0;
	lua_pushcclosure(L, scl_ainter, 1);
	pthread_mutex_unlock(&col->mutex);

	return 1;
}

static const struct luaL_Reg SelAverageCollectionM [] = {
	{"dump", sacl_dump},
	{"Push", sacl_push},
	{"GetSize", sacl_getsize},
	{"HowMany", sacl_HowMany},
	{"MinMaxI", sacl_minmaxI},
	{"MinMaxImmediate", sacl_minmaxI},
	{"MinMaxA", sacl_minmaxA},
	{"MinMaxAverage", sacl_minmaxA},
	{"MinMax", sacl_minmax},
	{"iData", sacl_idata},
	{"aData", sacl_adata},
	{"Save", sacl_save},
	{"Load", sacl_load},
	{"Clear", sacl_clear},
	{NULL, NULL}
};

static const struct luaL_Reg SelAverageCollectionLib [] = {
	{"Create", sacl_create},
	{"Find", sacl_find},
	{NULL, NULL}
};

static void registerSelAverageCollection(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelAverageCollection", SelAverageCollectionLib);
	selLua->objFuncs(L, "SelAverageCollection", SelAverageCollectionM);
}


/** 
 * Load a collection from a file
 *
 * @function Load
 * @tparam string filename
 *
 * @usage
col:Load('/tmp/tst.dt')
 */
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
	if(!initModule((struct SelModule *)&selAverageCollection, "SelAverageCollection", SELAVERAGECOLLECTION_VERSION, LIBSELENE_VERSION))
		return false;

	selAverageCollection.module.dump = sacc_dump;

	selAverageCollection.create = sacc_create;
	selAverageCollection.find = sacc_find;
	selAverageCollection.push = sacc_push;
	selAverageCollection.minmaxIs = sacc_minmaxIs;
	selAverageCollection.minmaxAs = sacc_minmaxAs;
	selAverageCollection.minmaxI = sacc_minmaxI;
	selAverageCollection.minmaxA = sacc_minmaxA;
	selAverageCollection.getn = sacc_getn;
	selAverageCollection.getsizeI = sacc_getsizeI;
	selAverageCollection.howmanyI = sacc_howmanyI;
	selAverageCollection.getsizeA = sacc_getsizeA;
	selAverageCollection.howmanyA = sacc_howmanyA;
	selAverageCollection.clear = sacc_clear;
	selAverageCollection.getsI = sacc_getsI;
	selAverageCollection.getsA = sacc_getsA;
	selAverageCollection.getI = sacc_getI;
	selAverageCollection.getA = sacc_getA;
	selAverageCollection.getatI = sacc_getatI;
	selAverageCollection.getatA = sacc_getatA;
	selAverageCollection.save = sacc_save;
	selAverageCollection.load = sacc_load;

	registerModule((struct SelModule *)&selAverageCollection);

	if(selLua){	/* Only if Lua is used */
		registerSelAverageCollection(NULL);
		selLua->AddStartupFunc(registerSelAverageCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

	return true;
}
