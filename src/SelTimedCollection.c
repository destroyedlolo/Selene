/*	SelTimedCollection.c
 *
 *	Timed values collection
 *
 *	10/04/2017	LF : First version
 */

#include "SelTimedCollection.h"

#include <assert.h>
#include <stdlib.h>

static int stcol_create(lua_State *L){
	struct SelTimedCollection *col = (struct SelTimedCollection *)lua_newuserdata(L, sizeof(struct SelTimedCollection));
	luaL_getmetatable(L, "SelTimedCollection");
	lua_setmetatable(L, -2);
	if(!(col->size = luaL_checkint( L, 1 ))){
		fputs("*E* SelTimedCollection's size can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	assert( col->data = calloc(col->size, sizeof(struct timeddata)) );
	col->last = 0;
	col->full = 0;

	return 1;
}

static const struct luaL_reg SelTimedColLib [] = {
	{"create", stcol_create}, 
	{NULL, NULL}
};

static const struct luaL_reg SelTimedColM [] = {
/*	{"Push", scol_push},
	{"MinMax", scol_minmax},
	{"Data", scol_data},
	{"iData", scol_idata},
	{"GetSize", scol_getsize},
	{"HowMany", scol_HowMany},
	{"dump", scol_dump}, */
	{NULL, NULL}
};


void init_SelTimedCollection( lua_State *L ){
	luaL_newmetatable(L, "SelTimedCollection");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelTimedColM);
	luaL_register(L,"SelCollection", SelTimedColLib);
}
