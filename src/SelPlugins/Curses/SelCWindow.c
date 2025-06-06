/***
 *
 * Curses' based textual windows.
 *

@classmod SelCWindow

 *
 * 06/09/2016 LF : First version
 * 18/04/2024 LF : Migrate to V7
 */

#include <Selene/SelPlug-in/SelCurses.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include "SelCurseStorage.h"

#include <ncurses.h>
#include <stdlib.h>

extern struct SelCurses scr_selCurses;
extern struct SeleneCore *scr_selCore;
extern struct SelLog *scr_selLog;

static struct SelCurseStorage *checkSelCWindow(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelCWindow");
	luaL_argcheck(L, r != NULL, 1, "'SelCWindow' expected");
	return (struct SelCurseStorage *)r;
}

static int SCW_keypad(lua_State *L){
/** enable/disable abbreviation of function keys
 *
 * @function keypad
 * @tparam boolean keypad
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	if(keypad(w->window, lua_toboolean( L, 2)) == ERR){
		scr_selLog->Log('E', "keypad() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "keypad() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_attrset(lua_State *L){
/** Set all attributs of the window.
 *
 * @function attrset
 * @tparam integer value
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int a = luaL_checkinteger(L, 2);

	if(wattrset(w->window, a) == ERR){
		scr_selLog->Log('E', "wattrset() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wattrset() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_attron(lua_State *L){
/** Turn ON given attributs of the window.
 *
 * @function attron
 * @tparam integer value
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int a = luaL_checkinteger(L, 2);

	if(wattron(w->window, a) == ERR){
		scr_selLog->Log('E', "wattron() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wattron() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_attroff(lua_State *L){
/** Turn OFF given attributs of the window.
 *
 * @function attroff
 * @tparam integer value
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int a = luaL_checkinteger(L, 2);

	if(wattroff(w->window, a) == ERR){
		scr_selLog->Log('E', "wattroff() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wattroff() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_Move(lua_State *L){
/** move curses window cursor.
 *
 * @function Move
 * @tparam integer x
 * @tparam integer y
 */
/** move curses window cursor.
 *
 * @function Cursor
 * @tparam integer x
 * @tparam integer y
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);

	if(wmove(w->window, y,x) == ERR){
		scr_selLog->Log('E', "wmove() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wmove() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_GetXY(lua_State *L){
/** get curses cursor coordinates.
 *
 * @function GetXY
 * @treturn integer x
 * @treturn integer y
 */
/** get curses cursor coordinates.
 *
 * @function GetCursor
 * @treturn integer x
 * @treturn integer y
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x,y;
	getyx(w->window, y,x);

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	return 2;

}

static int SCW_Print( lua_State *L ){
/** write a string on the window.
 *
 * @function print
 * @tparam string text
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	const char *arg = luaL_checkstring(L, 2);

	waddstr(w->window, arg);

	return 0;
}

static int SCW_PrintAt(lua_State *L){
/** write a string on the window at the given position
 *
 * @function PrintAt
 * @tparam string text
 * @tparam integer x
 * @tparam integer y
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);

	const char *arg = luaL_checkstring(L, 4);
	mvwaddstr(w->window, y, x, arg);

	return 0;
}

static int SCW_GetCh( lua_State *L ){
/** read a character from stdin.
 *
 * @function getch
 * @treturn integer character read
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	int c = wgetch(w->window);

	lua_pushinteger(L, c);
	return 1;
}

static int SCW_addch(lua_State *L){
/** add a character (with attributes) to a curses window, then advance the cursor.
 *
 * @function addch
 * @treturn integer character read
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	int c;
	switch( lua_type(L, 2 )){
	case LUA_TNUMBER:
		c = lua_tointeger(L, 2 );
		break;
	case LUA_TSTRING:
		c = *lua_tostring(L, 2 );
		break;
	default :
		scr_selLog->Log('E', "addch() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "addch() expects an integer or a string");
		return 2;
	}

	if(waddch(w->window, c) == ERR){
		scr_selLog->Log('E', "waddch() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "waddch() returned an error");
		return 2;
	}

	return 0;
}

static int internal_getnstr(lua_State *L, WINDOW *w, int n){
	char s[n+1];

	if(wgetnstr(w, s, n) == ERR){
		scr_selLog->Log('E', "wgetnstr() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wgetnstr() returned an error");
		return 2;
	}
	lua_pushstring(L, s);
	return 1;
}

static int SCW_getstr(lua_State *L){
/** read a string from stdin.
 *
 * @function getstr
 * @treturn string
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	return internal_getnstr(L, w->window, COLS);
}

static int SCW_getnstr(lua_State *L){
/** read a fixed sized string from stdin.
 *
 * @function getnstr
 * @tparam integer max max amount of characters to read
 * @treturn string
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int n = luaL_checkinteger(L, 2);

	if(!n){
		scr_selLog->Log('E', "zero sized getnstr()");
		lua_pushnil(L);
		lua_pushstring(L, "zero sized getnstr()");
		return 2;
	}

	return internal_getnstr(L, w->window, n);
}

static int SCW_GetstrAt(lua_State *L){
/** read a string from stdin and provide its position.
 *
 * @function GetstrAt
 * @tparam integer x cursor position
 * @tparam integer y cursor position
 * @treturn string
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);

	if(wmove(w->window, y,x) == ERR){
		scr_selLog->Log('E', "wmove() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wmove() returned an error");
		return 2;
	}

	return internal_getnstr(L, w->window, COLS);
}

static int SCW_GetnstrAt(lua_State *L){
/** read a fixed sized string from stdin and provide its position.
 *
 * @function GetnstrAt
 * @tparam integer x cursor position
 * @tparam integer y cursor position
 * @tparam integer max max amount of characters to read
 * @treturn string
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int n = luaL_checkinteger(L, 4);

	if(!n){
		scr_selLog->Log('E', "zero sized GetnstrAt()");
		lua_pushnil(L);
		lua_pushstring(L, "zero sized GetnstrAt()");
		return 2;
	}

	if(wmove(w->window, y,x) == ERR){
		scr_selLog->Log('E', "wmove() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wmove() returned an error");
		return 2;
	}

	return internal_getnstr(L, w->window, n);
}

static int SCW_AddchAt(lua_State *L){
/** add a character (with attributes) to a curses window, provide the cursor position then advance the cursor.
 *
 * @function AddchAt
 * @tparam integer x cursor position
 * @tparam integer y cursor position
 * @treturn integer character read
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);

	int c;
	switch( lua_type(L, 4 )){
	case LUA_TNUMBER:
		c = lua_tointeger(L, 4 );
		break;
	case LUA_TSTRING:
		c = *lua_tostring(L, 4 );
		break;
	default :
		scr_selLog->Log('E', "AddchAt() expects an integer or a string");
		lua_pushnil(L);
		lua_pushstring(L, "AddchAt() expects an integer or a string");
		return 2;
	}

	if(mvwaddch(w->window, y,x, c) == ERR){
		scr_selLog->Log('E', "mvwaddch() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "mvwaddch() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_Refresh( lua_State *L ){
/** refresh terminal from curses buffer.
 *
 * @function refresh
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	wrefresh(w->window);
	return 0;
}

static int SCW_Border( lua_State *L ){
/** Draw window border.
 *
 * The borders generated by this function are "inside borders".
 *
 * @function border
 * @todo Handle parameters to specify characters to use.
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	chtype ls, rs, ts, bs, tl, tr, bl, br;
	ls = rs = ts = bs = tl = tr = bl = br = 0;

/* TODO :Here argument reading from an associative table */

	if(wborder( w->window, ls, rs, ts, bs, tl, tr, bl, br ) == ERR){
		scr_selLog->Log('E', "wborder() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wborder() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_Erase( lua_State *L ){
/** copy blanks to every position in the window.
 *
 * @function erase
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	werase(w->window);
	return 0;
}

static int SCW_Clear( lua_State *L ){
/** copy blanks to every position in the window and mark all of the buffer as dirty.
 *
 * @function clear
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	wclear(w->window);
	return 0;
}

static int SCW_ClrToEol( lua_State *L ){
/** erase the current line to the right of the cursor, inclusive, to the end of the current line.
 *
 * @function clrtoeol
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	wclrtoeol(w->window);
	return 0;
}

static int SCW_ClrToBot( lua_State *L ){
/** erase from the cursor to the end of window.
 *
 * @function clrtobot
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	wclrtobot(w->window);
	return 0;
}

static int SCW_GetSize(lua_State *L){
/** Get window's size.
 *
 * @function GetSize
 * @treturn integer x width
 * @treturn integer y length
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int r,c;

	getmaxyx(w->window, r,c);

	lua_pushinteger(L, c);
	lua_pushinteger(L, r);
	return 2;
}

static int SCW_HLine(lua_State *L){
/** draw a horizontal (left to right) line starting at the current cursor position.
 *
 * The current cursor position is not changed.
 *
 * @function HLine
 * @tparam integer length
 * @tparam ?integer|nil char Character to use for drawing (default : '**-**')
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int n = luaL_checkinteger(L, 2);
	char ch = '-';

	if(lua_gettop(L) > 2) switch( lua_type(L, 3 )){
	case LUA_TNUMBER:
		ch = lua_tointeger(L, 3 );
		break;
	case LUA_TSTRING:
		ch = *lua_tostring(L, 3 );
		break;
	default :
		scr_selLog->Log('E', "Hline() expects an integer or a string");
		lua_pushnil(L);
		lua_pushstring(L, "Hline() expects an integer or a string");
		return 2;
	}

	if(whline( w->window, ch, n ) == ERR){
		scr_selLog->Log('E', "whline() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "whline() returned an error");
		return 2;
	}

	return 0;	
}

static int SCW_HLineAt(lua_State *L){
/** draw a horizontal (left to right) line starting at the given position.
 *
 * @function HLineAt
 * @tparam integer x origin
 * @tparam integer y origin
 * @tparam integer length
 * @tparam ?integer|nil char Character to use for drawing (default : '**-**')
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int n = luaL_checkinteger(L, 4);
	char ch = '-';

	if(lua_gettop(L) > 4) switch( lua_type(L, 5 )){
	case LUA_TNUMBER:
		ch = lua_tointeger(L, 5 );
		break;
	case LUA_TSTRING:
		ch = *lua_tostring(L, 5 );
		break;
	default :
		scr_selLog->Log('E', "HlineAt() expects an integer or a string");
		lua_pushnil(L);
		lua_pushstring(L, "HlineAt() expects an integer or a string");
		return 2;
	}

	if(mvwhline( w->window, y ,x, ch, n ) == ERR){
		scr_selLog->Log('E', "whline() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "whline() returned an error");
		return 2;
	}

	return 0;	
}

static int SCW_VLine(lua_State *L){
/** draw a vertical (top to bottom) line starting at the current cursor position.
 *
 * The current cursor position is not changed.
 *
 * @function VLine
 * @tparam integer length
 * @tparam ?integer|nil char Character to use for drawing (default : '**|**')
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int n = luaL_checkinteger(L, 2);
	char ch = '|';

	if(lua_gettop(L) > 2) switch( lua_type(L, 3 )){
	case LUA_TNUMBER:
		ch = lua_tointeger(L, 3 );
		break;
	case LUA_TSTRING:
		ch = *lua_tostring(L, 3 );
		break;
	default :
		scr_selLog->Log('E', "Vline() expects an integer or a string");
		lua_pushnil(L);
		lua_pushstring(L, "Vline() expects an integer or a string");
		return 2;
	}

	if(wvline( w->window, ch, n ) == ERR){
		scr_selLog->Log('E', "wvline() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wvline() returned an error");
		return 2;
	}

	return 0;	
}

static int SCW_VLineAt(lua_State *L){
/** draw a vertical (top to bottom) line starting at the given position.
 *
 * @function VLineAt
 * @tparam integer x origin
 * @tparam integer y origin
 * @tparam integer length
 * @tparam ?integer|nil char Character to use for drawing (default : '**|**')
 */
	struct SelCurseStorage *w = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int n = luaL_checkinteger(L, 4);
	char ch = '|';

	if(lua_gettop(L) > 4) switch( lua_type(L, 5 )){
	case LUA_TNUMBER:
		ch = lua_tointeger(L, 5 );
		break;
	case LUA_TSTRING:
		ch = *lua_tostring(L, 5 );
		break;
	default :
		scr_selLog->Log('E', "VlineAt() expects an integer or a string");
		lua_pushnil(L);
		lua_pushstring(L, "VlineAt() expects an integer or a string");
		return 2;
	}

	if(mvwvline( w->window, y ,x, ch, n ) == ERR){
		scr_selLog->Log('E', "wvline() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wvline() returned an error");
		return 2;
	}

	return 0;	
}

static int SCW_DerWin(lua_State *L){
/** Create a subwindow
 *
 * @function DerWin
 * @tparam integer x origin related to its parent
 * @tparam integer y origin related to its parent
 * @tparam integer width
 * @tparam integer length
 */
	struct SelCurseStorage *win = checkSelCWindow(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int w = luaL_checkinteger(L, 4);
	int h = luaL_checkinteger(L, 5);
	struct SelCurseStorage *wp = (struct SelCurseStorage *)lua_newuserdata(L, sizeof(struct SelCurseStorage));

	scr_selCore->initObject((struct SelModule *)&scr_selCurses, (struct SelObject *)wp);

	if(!(wp->window = derwin(win->window,h,w,y,x))){
		lua_pop(L,1);	/* Remove user data */
		scr_selLog->Log('E', "derwin() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "derwin() returned an error");
		return 2;
	}

	luaL_getmetatable(L, "SelCWindow");
	lua_setmetatable(L, -2);

	return 1;

}

static int SCW_delwin(lua_State *L){
/** Delete a window.
 *
 *	NOTEZ-BIEN : window's content remains unchanged on screen
 *	(Curses doesn't support layers overlapping as such)
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	if(!w->window){
		scr_selLog->Log('E', "delwin() on a dead object");
		lua_pushnil(L);
		lua_pushstring(L, "delwin() on a dead object");
		return 2;
	}

	if(w->window != stdscr)
		delwin(w->window);
	w->window = NULL;

	return 0;
}

static int SCW_applyP( lua_State *L ){
/** Apply a pair
 *
 * @function applyPair
 * @tparameter Number pair index
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	short i = luaL_checkinteger(L, 2);
	if(wattron(w->window, COLOR_PAIR(i)) == ERR){
		scr_selLog->Log('E', "wattron() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wattron() returned an error");
		return 2;
	}

	return 0;
}

static int SCW_rstP( lua_State *L ){
/** Desactivate a pair
 *
 * @function resetPair
 * @tparameter Number pair index
 */
	struct SelCurseStorage *w = checkSelCWindow(L);

	short i = luaL_checkinteger(L, 2);
	if(wattroff(w->window, COLOR_PAIR(i)) == ERR){
		scr_selLog->Log('E', "wattroff() returned an error");
		lua_pushnil(L);
		lua_pushstring(L, "wattroff() returned an error");
		return 2;
	}

	return 0;
}

const struct luaL_Reg SelCWndM [] = {
	{"keypad", SCW_keypad},
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
	{"HLine", SCW_HLine},
	{"HLineAt", SCW_HLineAt},
	{"VLine", SCW_VLine},
	{"VLineAt", SCW_VLineAt},
	{"GetSize", SCW_GetSize},
	{"Move", SCW_Move},
	{"Cursor", SCW_Move},		/* Alias */
	{"GetXY", SCW_GetXY},
	{"GetCursor", SCW_GetXY},	/* Alias */
	{"border", SCW_Border},
	{"refresh", SCW_Refresh},
	{"erase", SCW_Erase},
	{"clear", SCW_Clear},
	{"clrtoeol", SCW_ClrToEol},
	{"clrtobot", SCW_ClrToBot},
	{"DerWin", SCW_DerWin},
	{"delwin", SCW_delwin},
	{"Destroy", SCW_delwin},	/* Alias */
	{"applyPair", SCW_applyP},
	{"resetPair", SCW_rstP},
	{NULL, NULL}
};


	/* ***
	 * Toile's exported function
	 * ***/

static const char * const LuaName(){
	return "SelCWindow";
}

static bool SCWt_getSize(struct SelGenericSurface *win, uint16_t *width, uint16_t *height){
	getmaxyx(((struct SelCurseStorage *)win)->window, (*height), (*width));

	return true;
}

static bool SCWt_setCursor(struct SelGenericSurface *win, uint16_t x, uint16_t y){
	if(wmove(((struct SelCurseStorage *)win)->window, y,x) == ERR)
		return false;
	else
		return true;
}

bool initExportedWindow(struct SelCurseStorage *win, WINDOW *csrw){
	scr_selCore->initGenericSurface((struct SelModule *)&scr_selCurses, (struct SelGenericSurface *)win);

	win->window = csrw;

	win->obj.LuaObjectName = LuaName;
	win->obj.getSize = SCWt_getSize;
	win->obj.setCursor = SCWt_setCursor;

	return true;
}
