/***
 *
 * Curses based textual interface.
 *

@classmod SelCurses

 * 06/09/2016 LF : First version
 * 18/04/2024 LF : Migrate to V7
 * 08/01/2026 LF : Migrate to v9
 */

#include <Selene/SelPlug-in/SelCurses/SelCurses.h>

#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

struct SelCurses scr_selCurses;

struct SeleneCore *scr_selCore;
struct SelLog *scr_selLog;
struct SelLua *scr_selLua;

static const struct luaL_Reg CsRLib[] = {
/*
	{"ColorsConst", ColorsConst},
	{"listColors", CsRListColors},
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
	{"has_colors", CsRhasColors},
	{"maxPairs", CsRmaxPairs},
	{"newPairs", CsRnewPairs},
	{"init_pair", CsRinitpair},
*/
	{NULL, NULL}    /* End of definition */
};

extern const struct luaL_Reg SelCWndM [];

static void registerSelCurses(lua_State *L){
	scr_selLua->libCreateOrAddFuncs(L, "SelCurses", CsRLib);
	scr_selLua->objFuncs( L, "SelCWindow", SelCWndM );
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	scr_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!scr_selCore)
		return false;

	scr_selLog = (struct SelLog *)scr_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!scr_selLog)
		return false;

		/* Other mandatory modules */

		/* optional modules */
	scr_selLua =  (struct SelLua *)scr_selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&scr_selCurses, "SelCurses", SELCURSES_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&scr_selCurses);

	if(scr_selLua){	/* Only if Lua is used */
		registerSelCurses(NULL);
		scr_selLua->AddStartupFunc(registerSelCurses);
	}
#ifdef DEBUG
	else
		slcd_selLog->Log('D', "SelLua not loaded");
#endif


	return true;
}
