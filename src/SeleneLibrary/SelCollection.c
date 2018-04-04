/*	SelCollection.c
 *
 *	Values collection
 *
 *	28/09/2015	LF : First version
 *	04/04/2018	LF : switch to libSelene
 */

#include "libSelene.h"

#include <assert.h>
#include <stdlib.h>

struct SelCollection {
	lua_Number *data;		/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};

static int scol_create(lua_State *L){
	struct SelCollection *col = (struct SelCollection *)lua_newuserdata(L, sizeof(struct SelCollection));
	assert(col);

	luaL_getmetatable(L, "SelCollection");
	lua_setmetatable(L, -2);
	if(!(col->size = luaL_checkinteger( L, 1 ))){
		fputs("*E* SelCollection's size can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	assert( (col->data = calloc(col->size, sizeof(lua_Number))) );
	col->last = 0;
	col->full = 0;

	return 1;
}

static const struct luaL_Reg SelColLib [] = {
	{"create", scol_create},
	{NULL, NULL}
};

static struct SelCollection *checkSelCollection(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelCollection' expected");
	return (struct SelCollection *)r;
}

static int scol_dump(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	printf("SelCollection's Dump (size : %d, last : %d)\n", col->size, col->last);

	if(col->full)
		for(unsigned int i = col->last - col->size; i < col->last; i++)
			printf("\t%f\n", col->data[i % col->size]);
	else
		for(unsigned int i = 0; i < col->last; i++)
			printf("\t%f\n", col->data[i]);

	return 0;
}

static int scol_push(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	col->data[ col->last++ % col->size] = luaL_checknumber( L, 2 );
	if(col->last > col->size)
		col->full = 1;
	return 0;
}

static int scol_minmax(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);
	lua_Number min,max;
	unsigned int ifirst;	/* First data */

	if(!col->last && !col->full){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size : 0;
	min = max = col->data[ ifirst % col->size ];

	for(unsigned int i = ifirst; i < col->last; i++){
		if( col->data[ i % col->size ] < min )
			min = col->data[ i % col->size ];
		if( col->data[ i % col->size ] > max )
			max = col->data[ i % col->size ];
	}

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);

	return 2;
}

static int scol_data(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	if(!col->last && !col->full)
		return 0;

#ifdef DEBUG
	printf("%d : %s\n", col->full ? col->size : col->last, lua_checkstack(L, col->full ? col->size : col->last) ? "ok" : "nonok" );
#endif
	for(unsigned int i=col->full ? col->last - col->size : 0; i < col->last; i++)
		lua_pushnumber(L,  col->data[ i % col->size ]);
	return col->full ? col->size : col->last;
}

	/* Number of entries than can be stored in this collection */
static int scol_getsize(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

	/* Number of entries really stored */
static int scol_HowMany(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	lua_pushnumber(L, col->full ? col->size : col->last);
	return 1;
}

static int scol_inter(lua_State *L){
	struct SelCollection *col = (struct SelCollection *)lua_touserdata(L, lua_upvalueindex(1));

	if(col->cidx < col->last) {
		lua_pushnumber(L,  col->data[ col->cidx % col->size ]);
		col->cidx++;
		return 1;
	} else
		return 0;
}

static int scol_idata(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	if(!col->last && !col->full)
		return 0;

	col->cidx = col->full ? col->last - col->size : 0;
	lua_pushcclosure(L, scol_inter, 1);

	return 1;
}

static const struct luaL_Reg SelColM [] = {
	{"dump", scol_dump},
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

