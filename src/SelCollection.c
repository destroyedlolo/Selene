/*	SelCollection.c
 *
 *	Values collection
 *
 *	28/09/2015	LF : First version
 */

#include "SelCollection.h"

#include <assert.h>
#include <stdlib.h>

static int scol_init(lua_State *L){
	struct SelCollection *col;
	assert( col = (struct SelCollection *)malloc( sizeof(struct SelCollection)) );
	col->len = luaL_checkint( L, 1 );
	assert( col->len );

	return 0;
}

static const struct luaL_reg SelColM [] = {
	{"Init", scol_init},
	{NULL, NULL}
};

void init_SelCollection( lua_State *L ){
	luaL_newmetatable(L, "SelCollection");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelColM);
/*	luaL_register(L,"SelMQTT", SelMQTTLib); */
}
