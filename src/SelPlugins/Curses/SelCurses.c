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

static const struct luaL_Reg CsRLib[] = {
	{"CharAttrConst", CharAttrConst},
#if 0
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
#endif
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
