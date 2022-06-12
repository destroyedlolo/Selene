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

/* printf("*D* isz: %d asz: %d grp: %d\n", col->isize, col->asize, col->group); */

	if(col->isize < col->group){
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

static int sacol_push(lua_State *L){
/* Arguments are : 
 * 	1 : number ( To be compatible with previous version )
 * 			(only if only 1 value is stored)
 * 		table with the same amount as number of data
 */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	sel_shareable_lock( &(*col)->shareme ); /* Avoid concurrent access during modification */

		/****
		 * Insert immediate value
		 ****/

	if(!lua_istable(L, 2)){	/* One value, old interface */
		if((*col)->ndata > 1){
			sel_shareable_unlock( &(*col)->shareme ); /* Error : the collection needs to be released before raising the error */
			luaL_error(L, "Pushing a single number on multi-valued AverageCollection");
		}

		(*col)->immediate[ (*col)->ilast % (*col)->isize].data[0] = luaL_checknumber( L, 2 );
	} else {	/* Table provided */
		unsigned int j;

		if( lua_rawlen(L,2) != (*col)->ndata ){
			sel_shareable_unlock( &(*col)->shareme );
			luaL_error(L, "Expecting %d data per sample", (*col)->ndata);
		}

		for( j = 0; j < (*col)->ndata; j++){
			lua_rawgeti(L, 2, j+1);
			(*col)->immediate[ (*col)->ilast % (*col)->isize].data[j] = luaL_checknumber( L, -1 );
			lua_pop(L,1);
		}
	}

	if((*col)->ilast++ > (*col)->isize)
		(*col)->ifull = 1;


		/****
		 * Update average values if needed
		 ****/

	if(!((*col)->ilast % (*col)->group)){	/* push a new average */
		unsigned int i;

		if((*col)->ndata > 1){	/* Multi value */
			unsigned int j;
			for( j = 0; j < (*col)->ndata; j++)
				(*col)->average[ (*col)->alast % (*col)->asize].data[j] = 0;
		} else
			(*col)->average[ (*col)->alast % (*col)->asize].data[0] = 0;

		for(i = (*col)->ilast - (*col)->group; i < (*col)->ilast; i++){
			if((*col)->ndata > 1){	/* Multi value */
				unsigned int j;
				for( j = 0; j < (*col)->ndata; j++)
					(*col)->average[(*col)->alast % (*col)->asize].data[j] += (*col)->immediate[i % (*col)->isize].data[j];
			} else
				(*col)->average[(*col)->alast % (*col)->asize].data[0] += (*col)->immediate[i % (*col)->isize].data[0];
		}

		if((*col)->ndata > 1){	/* Multi value */
			unsigned int j;
			for( j = 0; j < (*col)->ndata; j++)
				(*col)->average[(*col)->alast % (*col)->asize].data[j] /= (*col)->group;
		} else
			(*col)->average[ (*col)->alast % (*col)->asize].data[0] /= (*col)->group;

		if((*col)->alast++ > (*col)->asize)
			(*col)->afull = 1;
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 0;
}

static int sacol_minmaxI(lua_State *L){
/* MinMax for immediate values */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[(*col)->ndata], max[(*col)->ndata];

	if(!(*col)->ilast && !(*col)->ifull){
		lua_pushnil(L);
		lua_pushstring(L, "MinMaxI() on an empty collection");
		return 2;
	}

	sel_shareable_lock( &(*col)->shareme );	/* Avoid modification while running through the collection */
  
	ifirst = (*col)->ifull ? (*col)->ilast - (*col)->isize : 0;
	for( j=0; j<(*col)->ndata; j++ )
		min[j] = max[j] = (*col)->immediate[ ifirst % (*col)->isize ].data[j];

	for(i = ifirst; i < (*col)->ilast; i++){
		for( j=0; j<(*col)->ndata; j++ ){
			lua_Number v = (*col)->immediate[ i % (*col)->isize ].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}

	if((*col)->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 2;
}

static int sacol_minmaxA(lua_State *L){
/* MinMax for average values */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[(*col)->ndata], max[(*col)->ndata];

	if(!(*col)->alast && !(*col)->afull){
		lua_pushnil(L);
		lua_pushstring(L, "MinMaxA() on an empty collection");
		return 2;
	}

	sel_shareable_lock( &(*col)->shareme );	/* Avoid modification while running through the collection */
  
	ifirst = (*col)->afull ? (*col)->alast - (*col)->asize : 0;
	for( j=0; j<(*col)->ndata; j++ )
		min[j] = max[j] = (*col)->average[ ifirst % (*col)->asize ].data[j];

	for(i = ifirst; i < (*col)->alast; i++){
		for( j=0; j<(*col)->ndata; j++ ){
			lua_Number v = (*col)->average[ i % (*col)->asize ].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}

	if((*col)->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 2;
}

	/* Debug function */
static int sacol_dump(lua_State *L){
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int i,j;

	sel_shareable_lock( &(*col)->shareme );

	printf("SelAverageCollection's Dump (size : %d x %d, last : %d)\nimmediate :\n", (*col)->isize, (*col)->ndata, (*col)->ilast);

	if((*col)->ifull)
		for(i = (*col)->ilast - (*col)->isize; i < (*col)->ilast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->immediate[i % (*col)->isize].data[j]);
			puts("");
		}
	else
		for(i = 0; i < (*col)->ilast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->immediate[i].data[j]);
			puts("");
		}

	puts("Average :");

	if((*col)->afull)
		for(i = (*col)->alast - (*col)->asize; i < (*col)->alast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->average[i % (*col)->asize].data[j]);
			puts("");
		}
	else
		for(i = 0; i < (*col)->alast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->average[i].data[j]);
			puts("");
		}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 0;
}

static int sacol_clear(lua_State *L){
/* Make the list empty */
	struct SelAverageCollection **col = checkSelAverageCollection(L);

	sel_shareable_lock( &(*col)->shareme );
	(*col)->ilast = 0;
	(*col)->ifull = 0;
	(*col)->alast = 0;
	(*col)->afull = 0;
	sel_shareable_unlock( &(*col)->shareme );

	return 0;
}

static const struct luaL_Reg SelAverageColLib [] = {
	{"Create", sacol_create}, 
	{NULL, NULL}
};

static const struct luaL_Reg SelAverageColM [] = {
	{"Push", sacol_push},
	{"MinMaxI", sacol_minmaxI},
	{"MinMaxImmediate", sacol_minmaxI},
	{"MinMaxA", sacol_minmaxA},
	{"MinMaxAverage", sacol_minmaxA},
#if 0
/*	{"Data", scol_data}, */
	{"iData", stcol_idata},
	{"GetSize", stcol_getsize},
	{"HowMany", stcol_HowMany},
	{"Save", stcol_Save},
	{"Load", stcol_Load},
#endif
	{"dump", sacol_dump},
	{"Clear", sacol_clear},
	{NULL, NULL}
};


int initSelAverageCollection( lua_State *L ){
	libSel_objFuncs( L, "SelAverageCollection", SelAverageColM);
	libSel_libFuncs( L, "SelAverageCollection", SelAverageColLib );

	return 1;
}
