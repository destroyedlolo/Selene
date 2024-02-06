/* SeleneCore.h
 *
 * Selene's core and helpers
 *
 * 06/02/2024 First version
 */

#include "Selene/SelLua.h"
#include "Selene/SeleneVersion.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"

static struct SelLua selLua;

static lua_State *L;	/* Main thread Lua's state (to make the initialisation easier */
static struct SeleneCore *selCore;
static struct SelLog *selLog;

static lua_State *slc_getLuaState(){
/**
 * @brief Returns main thread Lua state
 *
 * @function getLuaState
 * @return LuaState
 */
	return L;
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selLua, "SelLua", SELLUA_VERSION, LIBSELENE_VERSION))
		return false;

	selLua.getLuaState = slc_getLuaState;

	registerModule((struct SelModule *)&selLua);

		/* Initialize Lua */
	L = luaL_newstate();
	luaL_openlibs(L);

		/* Define globals functions */
	lua_pushnumber(L, SELENE_VERSION);	/* Expose version to lua side */
	lua_setglobal(L, "SELENE_VERSION");

	return true;
}
