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

static void CsRClean( void ){
	if(CsRinitialized){
		endwin();
		CsRinitialized = false;
	}
}

static int CsRGetCh( lua_State *L ){
	int c = getch();

	lua_pushinteger(L, c);
	return 1;
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
	{"echo", CsREcho},
	{"noecho", CsRNoEcho},
	{"raw", CsRRaw},
	{"cbreak", CsRCBrk},
	{"getch", CsRGetCh},
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
