/* SelQueue.c
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 */
#include "SelQueue.h"

#include <assert.h>

static struct SelQueue *checkSelQueue(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelQueue");
	luaL_argcheck(L, r != NULL, 1, "'SelQueue' expected");
	return (struct SelQueue *)r;
}

static int sq_create(lua_State *L){
	struct SelQueue *q = (struct SelQueue *)lua_newuserdata(L, sizeof(struct SelQueue));
	assert(q);
	luaL_getmetatable(L, "SelQueue");
	lua_setmetatable(L, -2);

	q->first = q->last = NULL;

	return 1;
}

static const struct luaL_reg SelQLib [] = {
	{"create", sq_create},
	{NULL, NULL}
};

static const struct luaL_reg SelQM [] = {
//	{"Push", sq_push},
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

