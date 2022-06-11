/* SelAverageCollection.c
 *
 * Collection of immediate and average values
 *
 * 11/06/2022 LF : creation
 */

#include "SelAverageCollection.h"

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

struct SelAverageCollection **checkSelAverageCollection(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelAverageCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelAverageCollection' expected");
	return (struct SelAverageCollection **)r;
}

static int sacol_create(lua_State *L){
/* Create a new collection
 * -> 1: size of the immediate collection
 * -> 2: size of the average collection
 * -> 3: grouping value
 * -> 4: number of value per sample
 */
	struct SelAverageCollection *col = malloc(sizeof(struct SelAverageCollection));
	struct SelAverageCollection **p = (struct SelAverageCollection **)lua_newuserdata(L, sizeof(struct SelAverageCollection *));
	unsigned int i;

	assert(col);
	assert(p);
	*p = col;

	sel_shareable_init(&col->shareme);

	luaL_getmetatable(L, "SelAverageCollection");
	lua_setmetatable(L, -2);

	if((col->isize = luaL_checkinteger( L, 1 )) <= 0 ){
		fputs("*E* SelAverageCollection's immediate size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	if((col->asize = luaL_checkinteger( L, 2 )) <= 0 ){
		fputs("*E* SelAverageCollection's average size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	if((col->group = luaL_checkinteger( L, 3 )) <= 0 ){
		fputs("*E* SelAverageCollection's grouping can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}
	if(col->isize > col->group){
		fputs("*E* SelAverageCollection's grouping can't be > to immediate sample size\n", stderr);
		exit(EXIT_FAILURE);
	}
	
	if((col->ndata = lua_tointeger( L, 4 )) < 1)
		col->ndata = 1;

	assert( (col->immediate = calloc(col->isize, sizeof(struct imaveragedata))) );
	for( i=0; i<col->isize; i++)
		assert( (col->immediate[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->ilast = 0;
	col->ifull = 0;

	assert( (col->average = calloc(col->asize, sizeof(struct imaveragedata))) );
	for( i=0; i<col->asize; i++)
		assert( (col->average[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->alast = 0;
	col->afull = 0;

	MCHECK;
	return 1;
}

static const struct luaL_Reg SelAverageColLib [] = {
	{"Create", sacol_create}, 
	{NULL, NULL}
};

static const struct luaL_Reg SelAverageColM [] = {
#if 0
	{"Push", sacol_push},
	{"MinMax", stcol_minmax},
/*	{"Data", scol_data}, */
	{"iData", stcol_idata},
	{"GetSize", stcol_getsize},
	{"HowMany", stcol_HowMany},
	{"Save", stcol_Save},
	{"Load", stcol_Load},
	{"dump", stcol_dump},
	{"Clear", stcol_clear},
#endif
	{NULL, NULL}
};


int initSelAverageCollection( lua_State *L ){
	libSel_objFuncs( L, "SelAverageCollection", SelAverageColM);
	libSel_libFuncs( L, "SelAverageCollection", SelAverageColLib );

	return 1;
}
