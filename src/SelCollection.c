/*	SelCollection.c
 *
 *	Values collection
 *
 *	28/09/2015	LF : First version
 */

#include "SelCollection.h"

#include <assert.h>
#include <stdlib.h>

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
	float min,max;
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

	for(unsigned int i=col->full ? col->last - col->size : 0; i < col->last; i++)
		lua_pushnumber(L,  col->data[ i % col->size ]);
	return col->full ? col->size : col->last;
}

static int scol_getsize(lua_State *L){
	struct SelCollection *col = checkSelCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

static int scol_create(lua_State *L){
	struct SelCollection *col = (struct SelCollection *)lua_newuserdata(L, sizeof(struct SelCollection));
	luaL_getmetatable(L, "SelCollection");
	lua_setmetatable(L, -2);
	if(!(col->size = luaL_checkint( L, 1 ))){
		fputs("*E* SelCollection's size can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	assert( col->data = calloc(col->size, sizeof(float)) );
	col->last = 0;
	col->full = 0;

	return 1;
}

static const struct luaL_reg SelColLib [] = {
	{"create", scol_create},
	{NULL, NULL}
};

static const struct luaL_reg SelColM [] = {
	{"Push", scol_push},
	{"MinMax", scol_minmax},
	{"Data", scol_data},
	{"GetSize", scol_getsize},
	{"dump", scol_dump},
	{NULL, NULL}
};

void init_SelCollection( lua_State *L ){
	luaL_newmetatable(L, "SelCollection");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelColM);
	luaL_register(L,"SelCollection", SelColLib);
}
