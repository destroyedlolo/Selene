/* SelSharedVar.c
 *
 * Variable shared among threads
 *
 * 05/03/2024 First version
 */

#include <Selene/SelSharedVar.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

struct SelSharedVar selSharedVar;


struct SeleneCore *selCore;
struct SelLog *selLog;
struct SelLua *selLua;

static const struct luaL_Reg SelSharedVarLib [] = {
/*	{"Set", sll_ignore}, */
	{NULL, NULL}
};

static void registerSelSharedVar(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelSharedVar", SelSharedVarLib);
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

		/* Not mandatory as may be used by C code */
	selLua =  (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selSharedVar, "SelSharedVar", SELSHAREDVAR_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selSharedVar);

	if(selLua){	/* Only if Lua is used */
		selLua->libCreateOrAddFuncs(NULL, "SelSharedVar", SelSharedVarLib);
		selLua->AddStartupFunc(registerSelSharedVar);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

	return true;
}
