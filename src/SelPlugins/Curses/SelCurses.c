/***
 *
 * Generate Curses based textual interface.
 *

@classmod SelCurses

 * 06/09/2016 LF : First version
 * 18/04/2024 LF : Migrate to V7
 */

#include <Selene/SelCurses/SelCurses.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <ncurses.h>
#include <stdlib.h>

static struct SelCurses selCurses;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static bool CsRinitialized;

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
/** Find out numeric value of a character attribute
 *
 * @function CharAttrConst
 * @tparam string name Attribute name in CAPITAL
 * @treturn number numeric value
 */

	return selLua->findConst(L, _chATTR);
}

static const struct ConstTranscode _cursVisibilit[] = {
	{ "INVISIBLE", 0 },
	{ "INVIS", 0 },
	{ "NORMAL", 1 },
	{ "SHOWY", 2 },
	{ NULL, 0 }
};

static int CursorVisibilityConst(lua_State *L ){
/** Find out numeric value of a cursor visiblity
 *
 * @function CursorVisibilityConst
 * @tparam string name Visibility name
 * @treturn number numeric value
 */
	return selLua->findConst(L, _cursVisibilit);
}

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
#ifdef KEY_EVENT	/* deprecated */
	{"EVENT", KEY_EVENT},
#endif

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
/** Find out numeric value of a key
 *
 * @function Key
 * @tparam string name key name
 * @treturn number numeric value
 */
	return selLua->findConst(L, _cursKeys);
}

static void CsRClean( void ){
	if(CsRinitialized){
		endwin();
		CsRinitialized = false;
	}
}

static int CsRInit( lua_State *L ){
/** Initialisation functions of Curses engine
 *
 * @function init
 * @treturn SelCWindow
 */
	initscr();
	CsRinitialized = true;
	atexit(CsRClean);

	WINDOW **wp = (WINDOW **)lua_newuserdata(L, sizeof(WINDOW *));
	*wp = stdscr;
	luaL_getmetatable(L, "SelCWindow");
	lua_setmetatable(L, -2);

	return 1;
}

static const struct luaL_Reg CsRLib[] = {
	{"CharAttrConst", CharAttrConst},
	{"CursorVisibilityConst", CursorVisibilityConst},
	{"Key", CsRKey},
#if 0
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
#endif
	{"init", CsRInit},
	{NULL, NULL}    /* End of definition */
};

static void registerSelCurses(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelCurses", CsRLib);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Other mandatory modules */
	selLua =  (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selCurses, "SelCurses", SELCURSES_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selCurses);

	registerSelCurses(NULL);
	selLua->AddStartupFunc(registerSelCurses);

	return true;
}
