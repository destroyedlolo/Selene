/* curses.c
 *
 * This file contains all stuffs related to Curses.
 *
 * 06/09/2016 LF : First version
 */
#include "curses.h"

#include <stdlib.h>

#ifdef USE_CURSES
bool CsRinitialized;

static void CsRClean( void ){
	if(CsRinitialized){
		endwin();
		CsRinitialized = false;
	}
}

static int CsRPrintw( lua_State *L ){
	char *arg = luaL_checkstring(L, 1);
	printw(arg);
	return 0;
}

static int CsRRefresh( lua_State *L ){
	refresh();
	return 0;
}

static int CsRGetCh( lua_State *L ){
	getch();
	return 0;
}

static int CsRInit( lua_State *L ){
	initscr();
	CsRinitialized = true;
	atexit(CsRClean);

	return 0;
}

static const struct luaL_reg CsRLib[] = {
	{"printw", CsRPrintw},
	{"refresh", CsRRefresh},
	{"getch", CsRGetCh},
	{"init", CsRInit},	
	{NULL, NULL}    /* End of definition */
};

void init_curses(lua_State *L){
	luaL_openlib(L,"SelCurses", CsRLib, 0);
	CsRinitialized = false;
}
#endif
