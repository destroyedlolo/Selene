/* SelQueue.c
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 */
#include "SelQueue.h"

static const struct luaL_reg SelQLib [] = {
//	{"create", sq_create},
	{NULL, NULL}
};

static const struct luaL_reg SelQM [] = {
//	{"Push", sq_push},
//	{"iData", sq_idata},
//	{"GetSize", sq_getsize},
//	{"HowMany", sq_HowMany},
//	{"dump", sq_dump},
	{NULL, NULL}
};


void init_SelQueue( lua_State *L ){
	luaL_newmetatable(L, "SelQueue");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelQM);
	luaL_register(L,"SelQueue", SelQLib);
}

