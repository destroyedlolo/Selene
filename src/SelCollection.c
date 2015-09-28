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
	for(int i=0; i<col->last; i++){	/*AF : A changer pour les roulements */
		printf("%f\n", col->data[i]);
	}
	return 0;
}

static int scol_push(lua_State *L){
	return 0;
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
