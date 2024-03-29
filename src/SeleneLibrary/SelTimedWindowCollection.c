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

#include "SelTimedWindowCollection.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static int stwcol_create(lua_State *L){
/** 
 * Create a new SelTimedCollection
 *
 * @function Create
 * @tparam number size size of the collection
 * @tparam number group seconds to be grouped in records
 */
	struct SelTimedWindowCollection *col = malloc(sizeof(struct SelTimedWindowCollection));
	struct SelTimedWindowCollection **p = (struct SelTimedWindowCollection **)lua_newuserdata(L, sizeof(struct SelTimedWindowCollection *));

	assert(col);
	assert(p);
	*p = col;

	sel_shareable_init(&col->shareme);

	luaL_getmetatable(L, "SelTimedWindowCollection");
	lua_setmetatable(L, -2);

	if((col->size = luaL_checkinteger( L, 1 )) <= 0){
		fputs("*E* SelTimedWindowCollection's size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}
	if((col->group = luaL_checkinteger( L, 2 )) <= 0){
		fputs("*E* SelTimedWindowCollection's group can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	assert( (col->data = calloc(col->size, sizeof(struct timedwdata))) );
	col->last = (unsigned int)-1;
	col->full = 0;

	return 1;
}

struct SelTimedWindowCollection **checkSelTimedWindowCollection(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelTimedWindowCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelTimedWindowCollection' expected");
	return (struct SelTimedWindowCollection **)r;
}

static inline int secw( struct SelTimedWindowCollection *col, time_t t ){
/* <- segregator corresponding to given time */ 
	return( t/col->group );
}

static void stwcol_new(lua_State *L, struct SelTimedWindowCollection *col, lua_Number amin, lua_Number amax, time_t t){
/* Create and add a new record.
 * Notez-bien : it's an internal function which is not locking the collection.
 */
	col->last++;
	if(col->last > col->size)
		col->full = 1;

	col->data[ col->last % col->size].min_data = amin;
	col->data[ col->last % col->size].max_data = amax;
	col->data[ col->last % col->size].t = secw( col, t );
}

static void stwcol_insert(lua_State *L, struct SelTimedWindowCollection *col, lua_Number amin, lua_Number amax, time_t t){
/* Insert a new value in the collection
 * check if the time match an existing record and if not, create a new one
 * Notez-bien : it's an internal function which is not locking the collection 
 * which MUST be locked before.
 */
	if(col->last == (unsigned int)-1)	/* Empty collection : create the 1st record */
		stwcol_new( L, col, amin, amax, t );
	else {
		int i = col->last % col->size;
		if( col->data[i].t == secw( col, t ) ){	/* Check if the time match an existing record */
			if( col->data[i].min_data > amin )	/* if so, update boundaries if needed */
				col->data[i].min_data = amin;
			if( col->data[i].max_data < amax )
				col->data[i].max_data = amax;
		} else
			stwcol_new( L, col, amin, amax, t );
	}
}

static int stwcol_push(lua_State *L){
/** 
 * Push a new value
 *
 * @function Push
 * @tparam ?number|table value single value or table of numbers in case of multi values collection
 * @tparam ?integer|nil timestamp Current timestamp by default
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	lua_Number dt = luaL_checknumber( L, 2 );

	sel_shareable_lock( &col->shareme ); /* Avoid concurrent access during modification */
	stwcol_insert(L, col, dt, dt, (lua_type( L, 3 ) == LUA_TNUMBER) ? lua_tonumber( L, 3 ) : time(NULL));
	sel_shareable_unlock( &col->shareme );

	return 0;
}

static int stwcol_minmax(lua_State *L){
/** 
 * Calculates the minimum and the maximum of this collection.
 *
 * @function MinMax
 * @treturn number minium
 * @treturn number maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;
	lua_Number min,max;
	unsigned int ifirst;	/* First data */
	unsigned int i;

	sel_shareable_lock( &col->shareme ); /* Avoid concurrent access during modification */

	if(col->last == (unsigned int)-1){
		sel_shareable_unlock( &col->shareme );
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size +1 : 0;
	min = col->data[ ifirst % col->size ].min_data;
	max = col->data[ ifirst % col->size ].max_data;

	for(i = ifirst; i <= col->last; i++){
		if( col->data[ i % col->size ].min_data < min )
			min = col->data[ i % col->size ].min_data;
		if( col->data[ i % col->size ].max_data > max )
			max = col->data[ i % col->size ].max_data;
	}

	sel_shareable_unlock( &col->shareme );

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);

	return 2;
}

static int stwcol_diffminmax(lua_State *L){
/** 
 * Calculates the minimum and maximum data windows of the collection.
 *
 * @function DiffMinMax
 * @treturn number minium
 * @treturn number maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;
	lua_Number min,max;
	unsigned int ifirst;	/* First data */
	unsigned int i;

	sel_shareable_lock( &col->shareme ); /* Avoid concurrent access during modification */
	
	if(col->last == (unsigned int)-1){
		sel_shareable_unlock( &col->shareme );
		lua_pushnil(L);
		lua_pushstring(L, "DiffMinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size +1 : 0;
	min = max = col->data[ ifirst % col->size ].max_data - col->data[ ifirst % col->size ].min_data;

	for(i = ifirst; i <= col->last; i++){
		lua_Number d = col->data[ i % col->size ].max_data - col->data[ i % col->size ].min_data;
		if( d < min )
			min = d;
		if( d > max )
			max = d;
	}

	sel_shareable_unlock( &col->shareme );

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);

	return 2;
}


static int stwcol_getsize(lua_State *L){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	sel_shareable_lock( &col->shareme );
	lua_pushnumber(L, col->size);
	sel_shareable_unlock( &col->shareme );

	return 1;
}

static int stwcol_HowMany(lua_State *L){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	sel_shareable_lock( &col->shareme );
	lua_pushnumber(L, col->full ? col->size : col->last+1);
	sel_shareable_unlock( &col->shareme );

	return 1;
}

static int stwcol_getgrouping(lua_State *L){
/** 
 * Number of seconds grouped in a window
 *
 * @function GetGrouping
 * @treturn number Number of seconds grouped in a window
 */
/* 
 * No need to lock as this parameter doesn't change during collection life
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	lua_pushnumber(L, col->group);

	return 1;
}

	/* Iterator */
static int stwcol_inter(lua_State *L){
	struct SelTimedWindowCollection **p = (struct SelTimedWindowCollection **)lua_touserdata(L, lua_upvalueindex(1));
	struct SelTimedWindowCollection *col = *p;

	if(col->cidx <= col->last) {
		sel_shareable_lock( &col->shareme );

		lua_pushnumber(L,  col->data[ col->cidx % col->size ].min_data);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].max_data);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].t * col->group);
		col->cidx++;

		sel_shareable_unlock( &col->shareme );

		return 3;
	} else
		return 0;
}

static int stwcol_idata(lua_State *L){
/** 
 * Collection's Iterator
 *
 * @function iData
 */
struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	sel_shareable_lock( &col->shareme );

	if(col->last == (unsigned int)-1){
		sel_shareable_unlock( &col->shareme );
		return 0;
	}

	col->cidx = col->full ? col->last - col->size +1 : 0;
	lua_pushcclosure(L, stwcol_inter, 1);

	sel_shareable_unlock( &col->shareme );

	return 1;
}

	/* Backup / Restore */
static int stwcol_Save(lua_State *L){
/** 
 * Save the collection to a file
 *
 * @function Save
 * @tparam string filename
 * @usage
col:Save('/tmp/tst.dt')
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;
	const char *s = lua_tostring( L, -1 );
	unsigned int i,j;

	FILE *f = fopen( s, "w" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	sel_shareable_lock( &col->shareme );

	if(col->last == (unsigned int)-1){
		sel_shareable_unlock( &col->shareme );

		lua_pushnil(L);
		lua_pushstring(L, "Save() on an empty collection");
		fclose(f);
		return 2;
	}


	if(col->full)
		for(j = col->last - col->size +1; j <= col->last; j++){
			int i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			fprintf(f, "%lf/%lf@%ld\n", col->data[i].min_data, col->data[i].max_data, t);
		}
	else
		for(i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			fprintf(f,"%lf/%lf@%ld\n", col->data[i].min_data, col->data[i].max_data, t );
		}
	fclose(f);

	sel_shareable_unlock( &col->shareme );

	return 0;
}

static int stwcol_Load(lua_State *L){
/** 
 * Load a collection from a file
 *
 * @function Load
 * @tparam string filename
 *
 * @usage
col:Load('/tmp/tst.dt')
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;
	const char *s = lua_tostring( L, -1 );
	lua_Number di,da;
	long int t;
	FILE *f = fopen( s, "r" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	sel_shareable_lock( &col->shareme );

	while( fscanf(f, "%lf/%lf@%ld\n", &di, &da, &t) != EOF)
		stwcol_insert( L, col, di, da, t );

	sel_shareable_unlock( &col->shareme );

	fclose(f);
	return 0;
}

static int stwcol_dump(lua_State *L){
/** 
 * Display collection's content (for debugging purposes).
 *
 * @function dump
 *
 */
	unsigned int i,j;
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	sel_shareable_lock( &col->shareme );

	if(col->last == (unsigned int)-1){
		sel_shareable_unlock( &col->shareme );
		printf("SelTimedWindowCollection's Dump (size : %d, EMPTY)\n", col->size);
		return 0;
	}

	printf("SelTimedWindowCollection's Dump (size : %d, last : %d) %s size: %ld\n", col->size, col->last, col->full ? "Full":"Incomplet", col->group);
	if(col->full)
		for(j = col->last - col->size +1; j <= col->last; j++){
			int i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			printf("\t%lf / %lf @ %s", col->data[i].min_data, col->data[i].max_data, ctime( &t ) );
		}
	else
		for(i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			printf("\t%lf / %lf @ %s", col->data[i].min_data, col->data[i].max_data, ctime( &t ) );
		}

	sel_shareable_unlock( &col->shareme );
	return 0;
}

static int stwcol_clear(lua_State *L){
/**
 * Make the collection empty
 *
 * @function Clear
 */
	struct SelTimedWindowCollection **p = checkSelTimedWindowCollection(L);
	struct SelTimedWindowCollection *col = *p;

	sel_shareable_lock( &col->shareme );
	col->last = (unsigned int)-1;
	col->full = 0;
	sel_shareable_unlock( &col->shareme );

	return 0;
}

static const struct luaL_Reg SelTimedColLib [] = {
	{"Create", stwcol_create}, 
#ifdef COMPATIBILITY
	{"create", stwcol_create},
#endif
	{NULL, NULL}
};

static const struct luaL_Reg SelTimedColM [] = {
	{"Push", stwcol_push},
	{"MinMax", stwcol_minmax},
	{"DiffMinMax", stwcol_diffminmax},
/*	{"Data", scol_data}, */
	{"iData", stwcol_idata},
	{"GetSize", stwcol_getsize},
	{"HowMany", stwcol_HowMany},
	{"GetGrouping", stwcol_getgrouping},
	{"Save", stwcol_Save},
	{"Load", stwcol_Load},
	{"dump", stwcol_dump},
	{"Clear", stwcol_clear},
	{NULL, NULL}
};


int initSelTimedWindowCollection( lua_State *L ){
	libSel_objFuncs( L, "SelTimedWindowCollection", SelTimedColM);
	libSel_libFuncs( L, "SelTimedWindowCollection", SelTimedColLib );

	return 1;
}
