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
#include <unistd.h>

#include <linux/input.h>

static struct SelEvent *checkSelEvent(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelEvent");
	luaL_argcheck(L, r != NULL, 1, "'SelEvent' expected");
	return (struct SelEvent *)r;
}

static int EventCreate(lua_State *L){
/*	Create an events' handler
 *	-> 1: /dev/input/event's file
 *	-> 2: function to be called (must be as fast as possible)
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

	if((t = open( fn, O_RDONLY | O_NOCTTY )) == -1){
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

static int EventRead(lua_State *L){
	struct SelEvent *event = checkSelEvent(L);
	struct input_event ev;
	int r;

	if((r=read(event->fd, &ev, sizeof( struct input_event ))) != sizeof( struct input_event )){
#ifdef DEBUG
		printf("*D* Read input_event : only %d bytes read\n", r);
#endif
		lua_pushnil(L);
		lua_pushstring(L, "Read input_event : unexpected read");
		return 2;
	}
	lua_Number t = ev.time.tv_sec + (lua_Number)ev.time.tv_usec/1000000.0;
	lua_pushnumber( L, t );
	lua_pushnumber( L, ev.type );
	lua_pushnumber( L, ev.code );
	lua_pushnumber( L, ev.value );
	return 4;
}

static const struct luaL_reg SelEventLib [] = {
	{"create", EventCreate},
	{NULL, NULL}
};

static const struct luaL_reg SelEventM [] = {
	{"read", EventRead},
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
