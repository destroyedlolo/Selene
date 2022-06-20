/***
A collection of up to *size* value.

When a additional one is pushed in a full collection, the oldest one is pushed out.

 
----------------------
Typical usage : to store single or multivaled numbers

@classmod SelCollection

 * History :
 *	28/09/2015	LF : First version
 *	04/04/2018	LF : switch to libSelene
 *	23/09/2020	LF : Multivalue
 
@todo add Load() and Save()

@usage
-- Multi valued Collection example

col = SelCollection.create(5,2)

for i=1,4 do
	col:Push(i, 4-i)
end
col:dump()

 */

#include "SelCollection.h"

#include <assert.h>
#include <stdlib.h>

static int scol_create(lua_State *L){
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
	struct SelCollection *col = (struct SelCollection *)lua_newuserdata(L, sizeof(struct SelCollection));
	assert(col);

	luaL_getmetatable(L, "SelCollection");
	lua_setmetatable(L, -2);

	if((col->size = luaL_checkinteger( L, 1 )) < 0){
		fputs("*E* SelCollection's size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	if((col->ndata = lua_tointeger( L, 2 )) < 1)
		col->ndata = 1;

	assert( (col->data = calloc(col->size * col->ndata, sizeof(lua_Number))) );
	col->last = 0;
	col->full = 0;

	return 1;
}

static const struct luaL_Reg SelColLib [] = {
	{"create", scol_create},
	{NULL, NULL}
};

static struct SelCollection *checkSelCollection(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelCollection' expected");
	return (struct SelCollection *)r;
}

static int scol_push(lua_State *L){
/** 
 * Push a new sample.
 *
 * @function Push
 * @tparam num number single value or table of numbers in case of multi values collection
 */
	struct SelCollection *col = checkSelCollection(L);
	unsigned int j;

	if( lua_gettop(L)-1 != col->ndata )
		luaL_error(L, "Expecting %d data", col->ndata);

	for( j=1; j<lua_gettop(L); j++)
		col->data[ (col->last % col->size)*col->ndata + j-1 ] = luaL_checknumber( L, j+1 );
	col->last++;

	if(col->last > col->size)
		col->full = 1;
	return 0;
}

static int scol_minmax(lua_State *L){
/** 
 * Calculates the minimum and the maximum of this collection.
 *
 * @function MinMax
 * @treturn num minium
 * @treturn num maximum
 * @raise (**nil**, *error message*) in case the collection is empty
 */
	struct SelCollection *col = checkSelCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[col->ndata], max[col->ndata];

	if(!col->last && !col->full){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size : 0;

	for( j=0; j<col->ndata; j++ )
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

static int scol_getsize(lua_State *L){
/** 
 * Number of entries that can be stored in this collection
 *
 * @function GetSize
 * @treturn num reserved storage for this collection
 */
	struct SelCollection *col = checkSelCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

static int scol_HowMany(lua_State *L){
/** 
 * Number of entries actually stored
 *
 * @function HowMany
 * @treturn num Amount of samples stored
 */
	struct SelCollection *col = checkSelCollection(L);

	lua_pushnumber(L, col->full ? col->size : col->last);
	return 1;
}

static int scol_data(lua_State *L){
/** 
 * Push all collection's data in the pipeline.
 *
 * Useage is discouraged : **Don't do it !**
 *
 * @function Data
 */
	struct SelCollection *col = checkSelCollection(L);
	unsigned int i,j;

	if(!col->last && !col->full)
		return 0;

#ifdef DEBUG
	printf("%d : %s\n", col->full ? col->size : col->last, lua_checkstack(L, col->full ? col->size : col->last) ? "ok" : "nonok" );
#endif
	for(i=col->full ? col->last - col->size : 0; i < col->last; i++)
		if(col->ndata == 1)
			lua_pushnumber(L,  col->data[ i % col->size ]);
		else {
			lua_newtable(L);	/* table result */
			for( j=0; j<col->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, col->data[ (i % col->size)*col->ndata + j ]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
	return col->full ? col->size : col->last;
}

static int scol_inter(lua_State *L){
	struct SelCollection *col = (struct SelCollection *)lua_touserdata(L, lua_upvalueindex(1));

	if(col->cidx < col->last) {
		if(col->ndata == 1)
			lua_pushnumber(L,  col->data[ col->cidx % col->size ]);
		else {
			unsigned int j;
			lua_newtable(L);	/* table result */
			for( j=0; j<col->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, col->data[ (col->cidx % col->size)*col->ndata + j ]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		col->cidx++;
		return 1;
	} else
		return 0;
}

static int scol_idata(lua_State *L){
/** 
 * Collection's Iterator
 *
 * @function iData
 * @usage
for d in col:iData() do print(d) end
 */
	struct SelCollection *col = checkSelCollection(L);

	if(!col->last && !col->full)
		return 0;

	col->cidx = col->full ? col->last - col->size : 0;
	lua_pushcclosure(L, scol_inter, 1);

	return 1;
}

static int scol_clear(lua_State *L){
/**
 * Make the collection empty
 *
 * @function Clear
 */
	struct SelCollection *col = checkSelCollection(L);

	col->last = 0;
	col->full = 0;

	return 0;
}

static int scol_dump(lua_State *L){
/** 
 * Display collection's content (for debugging purposes).
 *
 * @function dump
 *
 */
	struct SelCollection *col = checkSelCollection(L);
	unsigned int i,j;

	printf("SelCollection's Dump (size : %d x %d, last : %d)\n", col->size, col->ndata, col->last);

	if(col->full)
		for(i = col->last - col->size; i < col->last; i++){
			for(j = 0; j < col->ndata; j++)
				printf("\t%f", col->data[(i % col->size)*col->ndata + j]);
			puts("");
		}
	else
		for(i = 0; i < col->last; i++){
			for(j = 0; j < col->ndata; j++)
				printf("\t%f", col->data[i*col->ndata + j]);
			puts("");
		}

	return 0;
}

static const struct luaL_Reg SelColM [] = {
	{"dump", scol_dump},
	{"Clear", scol_clear},
	{"Push", scol_push},
	{"MinMax", scol_minmax},
	{"Data", scol_data},
	{"iData", scol_idata},
	{"GetSize", scol_getsize},
	{"HowMany", scol_HowMany},
	{NULL, NULL}
};

int initSelCollection( lua_State *L ){
	libSel_objFuncs( L, "SelCollection", SelColM);
	libSel_libFuncs( L, "SelCollection", SelColLib );

	return 1;
}

