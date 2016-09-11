/* curses.c
 *
 * This file contains all stuffs related to Curses.
 *
 * 06/09/2016 LF : First version
 */
#include "curses.h"

#ifdef USE_CURSES
#include <stdlib.h>

bool CsRinitialized;

static const struct ConstTranscode _chATTR[] = {
	{ "NORMAL", A_NORMAL },
	{ "STANDOUT", A_STANDOUT },
	{ "UNDERLINE", A_UNDERLINE },
	{ "REVERSE", A_REVERSE },
	{ "BLINK", A_BLINK },
	{ "DIM", A_DIM },
	{ "BOLD", A_BOLD },
	{ "PROTECT", A_PROTECT },
	{ "INVIS", A_INVIS },
	{ "ALTCHARSET", A_ALTCHARSET },
	{ "CHARTEXT", A_CHARTEXT },
	{ NULL, 0 }
};

static int CharAttrConst(lua_State *L ){
	return findConst(L, _chATTR);
}

static const struct ConstTranscode _cursVisibilit[] = {
	{ "INVISIBLE", 0 },
	{ "INVIS", 0 },
	{ "NORMAL", 1 },
	{ "SHOWY", 2 },
	{ NULL, 0 }
};

static int CursorVisibilityConst(lua_State *L ){
	return findConst(L, _cursVisibilit);
}

static int CsRbeep( lua_State *L ){
	if(beep() == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "beep() returned an error");
		return 2;
	}

	return 0;
}

static int CsRflash( lua_State *L ){
	if(flash() == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "beep() returned an error");
		return 2;
	}

	return 0;
}

static int CsRcurs_set( lua_State *L ){
	int v = luaL_checkint(L, 1);

	if(curs_set(v) == ERR){
		lua_pushnil(L);
		lua_pushstring(L, "curs_set() returned an error");
		return 2;
	}

	return 0;
}

static void CsRClean( void ){
	if(CsRinitialized){
		endwin();
		CsRinitialized = false;
	}
}

static int CsREnd( lua_State *L ){
	endwin();
	CsRinitialized = false;
	return 0;
}

static int CsRInit( lua_State *L ){
	initscr();
	CsRinitialized = true;
	atexit(CsRClean);

	WINDOW **wp = (WINDOW **)lua_newuserdata(L, sizeof(WINDOW *));
	*wp = stdscr;
	luaL_getmetatable(L, "SelCWindow");
	lua_setmetatable(L, -2);

	return 1;
}

static int CsREcho( lua_State *L ){
	bool res = true;
	if( lua_isboolean( L, 1 ) )
		res = lua_toboolean( L, 1 );

	if(res)
		echo();
	else
		noecho();

	return 0;
}

static int CsRNoEcho( lua_State *L ){
	noecho();
	return 0;
}

static int CsRRaw( lua_State *L ){
	raw();
	return 0;
}

static int CsRCBrk( lua_State *L ){
	cbreak();
	return 0;
}

static const struct luaL_reg CsRLib[] = {
	{"CharAttrConst", CharAttrConst},
	{"CursorVisibilityConst", CursorVisibilityConst},
	{"beep", CsRbeep},
	{"flash", CsRflash},
	{"curs_set", CsRcurs_set},
	{"echo", CsREcho},
	{"noecho", CsRNoEcho},
	{"raw", CsRRaw},
	{"cbreak", CsRCBrk},
	{"endwin", CsREnd},
	{"init", CsRInit},	
	{NULL, NULL}    /* End of definition */
};

void init_curses(lua_State *L){
	luaL_openlib(L,"SelCurses", CsRLib, 0);
	CsRinitialized = false;

	_include_SelCWindow( L );
}
#endif
