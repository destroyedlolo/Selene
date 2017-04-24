/* SelEvent.c
 *
 * This file contains Events stuffs
 *
 * 24/04/2017 LF : First version
 */

#include "selene.h"
#include "SelEvent.h"
#include "SelShared.h"

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


static struct SelEvent *checkSelEvent(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelEvent");
	luaL_argcheck(L, r != NULL, 1, "'SelEvent' expected");
	return (struct SelEvent *)r;
}

static int EventCreate(lua_State *L){
/*	Create an events' handler
 *	-> 1: /dev/input/event's file
 *	-> 2: function to be called
 */
	struct SelEvent *event;
	const char *fn = luaL_checkstring(L, 1);	/* Event's file */
	int t,f;

	if( lua_type(L, 2) != LUA_TFUNCTION ){
		lua_pushstring(L, "Expecting function for argument #2 of SelEvent.create()");
		lua_error(L);
		exit(EXIT_FAILURE);
	} else
		f = findFuncRef(L,2);

	if((t = open( fn, O_NOCTTY )) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	event = (struct SelEvent *)lua_newuserdata(L, sizeof( struct SelEvent ));
	luaL_getmetatable(L, "SelEvent");
	lua_setmetatable(L, -2);
	event->fd = t;
	event->func = f;

	return 1;
}

static const struct luaL_reg SelEventLib [] = {
	{"create", EventCreate},
	{NULL, NULL}
};

static const struct luaL_reg SelEventM [] = {
	{NULL, NULL}
};

void init_SelEvent( lua_State *L ){
	luaL_newmetatable(L, "SelEvent");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelEventM);
	luaL_register(L,"SelEvent", SelEventLib);
}
