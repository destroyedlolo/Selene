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

static int SCW_attrset(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int a = luaL_checkint(L, 2);

	if(wattrset(*w, a) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wattrset() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_attron(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int a = luaL_checkint(L, 2);

	if(wattron(*w, a) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wattron() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_attroff(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int a = luaL_checkint(L, 2);

	if(wattroff(*w, a) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wattroff() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_Move(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);

	if(wmove(*w, y,x) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wmove() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_Print( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	char *arg = luaL_checkstring(L, 2);
	waddstr(*w, arg);

	return 0;
}

static int SCW_PrintAt(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);

	char *arg = luaL_checkstring(L, 4);
	mvwaddstr(*w, y, x, arg);

	return 0;
}

static int SCW_GetCh( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	int c = wgetch(*w);

	lua_pushinteger(L, c);
	return 1;
}

static int SCW_addch(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int c;
	switch( lua_type(L, 2 )){
	case LUA_TNUMBER:
		c = lua_tointeger(L, 2 );
		break;
	case LUA_TSTRING:
		c = *lua_tostring(L, 2 );
		break;
	default :
		lua_pushnil(L);
		lua_pushstring(L, "addch() expects an integer or a string");
		return 2;
	}

	if(waddch(*w, c) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "waddch() returned an error");
		return 2;
	}

	return 0;
}

static int internal_getnstr(lua_State *L, WINDOW **w, int n){
	char s[n+1];

	if(wgetnstr(*w, s, n) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wgetnstr() returned an error");
		return 2;
	}
	lua_pushstring(L, s);
	return 1;
}

static int SCW_getstr(lua_State *L){
	WINDOW **w = checkSelCWindow(L);

	return internal_getnstr(L, w, COLS);
}

static int SCW_getnstr(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int n = luaL_checkint(L, 2);

	if(!n){
		lua_pushnil(L);
		lua_pushstring(L, "zero sized getnstr()");
		return 2;
	}

	return internal_getnstr(L, w, n);
}

static int SCW_GetstrAt(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);

	if(wmove(*w, y,x) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wmove() returned an error");
		return 2;
	}

	return internal_getnstr(L, w, COLS);
}

static int SCW_GetnstrAt(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	int n = luaL_checkint(L, 4);

	if(!n){
		lua_pushnil(L);
		lua_pushstring(L, "zero sized GetnstrAt()");
		return 2;
	}

	if(wmove(*w, y,x) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "wmove() returned an error");
		return 2;
	}

	return internal_getnstr(L, w, n);
}

static int SCW_AddchAt(lua_State *L){
	WINDOW **w = checkSelCWindow(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	int c;
	switch( lua_type(L, 4 )){
	case LUA_TNUMBER:
		c = lua_tointeger(L, 4 );
		break;
	case LUA_TSTRING:
		c = *lua_tostring(L, 4 );
		break;
	default :
		lua_pushnil(L);
		lua_pushstring(L, "AddchAt() expects an integer or a string");
		return 2;
	}

	if(mvwaddch(*w, y,x, c) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "mvwaddch() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_Refresh( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	wrefresh( *w );
	return 0;
}

static int SCW_Erase( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	werase( *w );
	return 0;
}

static int SCW_Clear( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	wclear( *w );
	return 0;
}

static int SCW_ClrToEol( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	wclrtoeol( *w );
	return 0;
}

static int SCW_ClrToBot( lua_State *L ){
	WINDOW **w = checkSelCWindow(L);

	wclrtobot( *w );
	return 0;
}

static int SCW_GetSize(lua_State *L){
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
	{"attrset", SCW_attrset},
	{"attron", SCW_attron},
	{"attroff", SCW_attroff},
	{"getch", SCW_GetCh},
	{"addch", SCW_addch},
	{"getstr", SCW_getstr},
	{"GetstrAt", SCW_GetstrAt},
	{"getnstr", SCW_getnstr},
	{"GetnstrAt", SCW_GetnstrAt},
	{"AddchAt", SCW_AddchAt},
	{"print", SCW_Print},
	{"PrintAt", SCW_PrintAt},
	{"GetSize", SCW_GetSize},
	{"Move", SCW_Move},
	{"Cursor", SCW_Move},		/* Alias */
	{"refresh", SCW_Refresh},
	{"erase", SCW_Erase},
	{"clear", SCW_Clear},
	{"clrtoeol", SCW_ClrToEol},
	{"clrtobot", SCW_ClrToBot},
	{"delwin", SCW_delwin},
	{"Destroy", SCW_delwin},	/* Alias */
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
