/*	SelCollection.c
 *
 *	Values collection
 *
 *	28/09/2015	LF : First version
 */

#include "SelCollection.h"

#include <assert.h>
#include <stdlib.h>

static int scol_push(lua_State *L){
	return 0;
}

static int scol_create(lua_State *L){
	struct SelCollection *col;
	assert( col = (struct SelCollection *)malloc( sizeof(struct SelCollection)) );
	if(!(col->len = luaL_checkint( L, 1 ))){
		fputs("*E* SelCollection's size can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	assert( col->data = calloc(col->len, sizeof(float)) );
	col->last = 0;
	col->full = 0;

	return 0;
}
static const struct luaL_reg SelColLib [] = {
	{"create", scol_create},
	{NULL, NULL}
};

static const struct luaL_reg SelColM [] = {
	{"push", scol_push},
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
