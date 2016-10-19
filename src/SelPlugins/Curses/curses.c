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

/* grep KEY_ /usr/include/ncurses.h | awk '$2 !~ /KEY_F\(n\)/ { gsub("KEY_","",$2); print "\t{\""$2"\", KEY_"$2"}," }' */
static const struct ConstTranscode _cursKeys[] = {
	{"DOWN", KEY_DOWN},
	{"UP", KEY_UP},
	{"LEFT", KEY_LEFT},
	{"RIGHT", KEY_RIGHT},
	{"HOME", KEY_HOME},
	{"BACKSPACE", KEY_BACKSPACE},
	{"F0", KEY_F0},
	{"DL", KEY_DL},
	{"IL", KEY_IL},
	{"DC", KEY_DC},
	{"IC", KEY_IC},
	{"EIC", KEY_EIC},
	{"CLEAR", KEY_CLEAR},
	{"EOS", KEY_EOS},
	{"EOL", KEY_EOL},
	{"SF", KEY_SF},
	{"SR", KEY_SR},
	{"NPAGE", KEY_NPAGE},
	{"PPAGE", KEY_PPAGE},
	{"STAB", KEY_STAB},
	{"CTAB", KEY_CTAB},
	{"CATAB", KEY_CATAB},
	{"ENTER", KEY_ENTER},
	{"PRINT", KEY_PRINT},
	{"LL", KEY_LL},
	{"A1", KEY_A1},
	{"A3", KEY_A3},
	{"B2", KEY_B2},
	{"C1", KEY_C1},
	{"C3", KEY_C3},
	{"BTAB", KEY_BTAB},
	{"BEG", KEY_BEG},
	{"CANCEL", KEY_CANCEL},
	{"CLOSE", KEY_CLOSE},
	{"COMMAND", KEY_COMMAND},
	{"COPY", KEY_COPY},
	{"CREATE", KEY_CREATE},
	{"END", KEY_END},
	{"EXIT", KEY_EXIT},
	{"FIND", KEY_FIND},
	{"HELP", KEY_HELP},
	{"MARK", KEY_MARK},
	{"MESSAGE", KEY_MESSAGE},
	{"MOVE", KEY_MOVE},
	{"NEXT", KEY_NEXT},
	{"OPEN", KEY_OPEN},
	{"OPTIONS", KEY_OPTIONS},
	{"PREVIOUS", KEY_PREVIOUS},
	{"REDO", KEY_REDO},
	{"REFERENCE", KEY_REFERENCE},
	{"REFRESH", KEY_REFRESH},
	{"REPLACE", KEY_REPLACE},
	{"RESTART", KEY_RESTART},
	{"RESUME", KEY_RESUME},
	{"SAVE", KEY_SAVE},
	{"SBEG", KEY_SBEG},
	{"SCANCEL", KEY_SCANCEL},
	{"SCOMMAND", KEY_SCOMMAND},
	{"SCOPY", KEY_SCOPY},
	{"SCREATE", KEY_SCREATE},
	{"SDC", KEY_SDC},
	{"SDL", KEY_SDL},
	{"SELECT", KEY_SELECT},
	{"SEND", KEY_SEND},
	{"SEOL", KEY_SEOL},
	{"SEXIT", KEY_SEXIT},
	{"SFIND", KEY_SFIND},
	{"SHELP", KEY_SHELP},
	{"SHOME", KEY_SHOME},
	{"SIC", KEY_SIC},
	{"SLEFT", KEY_SLEFT},
	{"SMESSAGE", KEY_SMESSAGE},
	{"SMOVE", KEY_SMOVE},
	{"SNEXT", KEY_SNEXT},
	{"SOPTIONS", KEY_SOPTIONS},
	{"SPREVIOUS", KEY_SPREVIOUS},
	{"SPRINT", KEY_SPRINT},
	{"SREDO", KEY_SREDO},
	{"SREPLACE", KEY_SREPLACE},
	{"SRIGHT", KEY_SRIGHT},
	{"SRSUME", KEY_SRSUME},
	{"SSAVE", KEY_SSAVE},
	{"SSUSPEND", KEY_SSUSPEND},
	{"SUNDO", KEY_SUNDO},
	{"SUSPEND", KEY_SUSPEND},
	{"UNDO", KEY_UNDO},
	{"MOUSE", KEY_MOUSE},
	{"RESIZE", KEY_RESIZE},
	{"EVENT", KEY_EVENT},

	{"F1", KEY_F(1)},
	{"F2", KEY_F(2)},
	{"F3", KEY_F(3)},
	{"F4", KEY_F(4)},
	{"F5", KEY_F(5)},
	{"F6", KEY_F(6)},
	{"F7", KEY_F(7)},
	{"F8", KEY_F(8)},
	{"F9", KEY_F(9)},
	{"F10", KEY_F(10)},
	{"F11", KEY_F(11)},
	{"F12", KEY_F(12)},

	{ NULL, 0 }
};

static int CsRKey(lua_State *L ){
	return findConst(L, _cursKeys);
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
	bool res = true;
	if( lua_isboolean( L, 1 ) )
		res = lua_toboolean( L, 1 );

	if(res)
		raw();
	else
		noraw();

	return 0;
}

static int CsRNoRaw( lua_State *L ){
	noraw();
	return 0;
}

static int CsRCBrk( lua_State *L ){
	bool res = true;
	if( lua_isboolean( L, 1 ) )
		res = lua_toboolean( L, 1 );

	if(res)
		cbreak();
	else
		nocbreak();
	return 0;
}

static int CsRCNoBrk( lua_State *L ){
	nocbreak();
	return 0;
}

static const struct luaL_reg CsRLib[] = {
	{"CharAttrConst", CharAttrConst},
	{"CursorVisibilityConst", CursorVisibilityConst},
	{"Key", CsRKey},
	{"beep", CsRbeep},
	{"flash", CsRflash},
	{"curs_set", CsRcurs_set},
	{"echo", CsREcho},
	{"noecho", CsRNoEcho},
	{"raw", CsRRaw},
	{"noraw", CsRNoRaw},
	{"cbreak", CsRCBrk},
	{"nocbreak", CsRCNoBrk},
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
