/* csr_window.c
 *
 * This file contains all stuffs related to Curses' windows.
 *
 * 06/09/2016 LF : First version
 */
#include "curses.h"

#ifdef USE_CURSES

static WINDOW **checkSelCWindow(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelCWindow");
	luaL_argcheck(L, r != NULL, 1, "'SelCWindow' expected");
	return (WINDOW **)r;
}

static int SCW_Print( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	char *arg = luaL_checkstring(L, 2);
	waddstr(*w, arg);

	return 0;
}

static int SCW_GetCh( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	int c = wgetch(*w);

	lua_pushinteger(L, c);
	return 1;
}

static int SCW_Refresh( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	wrefresh( *w );
	return 0;
}

static int SCW_getsize(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int r,c;
	getmaxyx(*w, r,c);

	lua_pushinteger(L, c);
	lua_pushinteger(L, r);
	return 2;
}

static int SCW_delwin(lua_State *L){
	WINDOW **s = checkSelCWindow(L);

	if(!*s){
		lua_pushnil(L);
		lua_pushstring(L, "delwin() on a dead object");
		return 2;
	}

	if(*s != stdscr)
		delwin(*s);
	*s = NULL;

	return 0;
}

static const struct luaL_reg SelCWndLib [] = {
	{NULL, NULL}
};

static const struct luaL_reg SelCWndM [] = {
	{"getch", SCW_GetCh},
	{"print", SCW_Print},
	{"refresh", SCW_Refresh},
	{"GetSize", SCW_getsize},
	{"delwin", SCW_delwin},
	{"destroy", SCW_delwin},	/* Alias */
	{NULL, NULL}
};

void _include_SelCWindow( lua_State *L ){
	luaL_newmetatable(L, "SelCWindow");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelCWndM);
	luaL_register(L,"SelCWindow", SelCWndLib);
}

#endif
