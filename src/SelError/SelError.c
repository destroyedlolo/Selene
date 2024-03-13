/* SelError.h
 *
 * Error management
 *
 * 09/03/2024 First version
 */

#include <Selene/SelError.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>

#include "selErrorStorage.h"

struct SelError selError;

struct SeleneCore *selCore;
struct SelLog *selLog;
struct SelLua *selLua;

static struct selErrorStorage *checkSelError(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelError");
	luaL_argcheck(L, r != NULL, 1, "'SelError' expected");
	return (struct selErrorStorage *)r;
}

static void sec_create(lua_State *L, char level, const char *message, bool log){
/** 
 * @brief Create a new SelError.
 *
 * @function Create
 * @tparam char Error's level
 * @ŧparam const char *message
 * @tparam bool do we have to log the error ?
 */
	struct selErrorStorage *error;

	error = (struct selErrorStorage *)lua_newuserdata(L, sizeof( struct selErrorStorage ));
	luaL_getmetatable(L, "SelError");
	lua_setmetatable(L, -2);

	error->level = level;
	error->msg = message;

	selLog->Log(level, message);
}

static bool sec_isSelError(struct lua_State *L, int index){
	return(!!luaL_testudata(L, index, "SelError"));
}

static int sel_isSelError(lua_State *L){
	lua_pushboolean(L, sec_isSelError(L, 1));
	return 1;
}

static const struct luaL_Reg SelErrorLib [] = {
	{"isSelError", sel_isSelError},
	{NULL, NULL}
};

static int sel_getLevel(lua_State *L){
	struct selErrorStorage *r = checkSelError(L);

	if(!r){
		lua_pushnil(L);
		lua_pushstring(L, "getLevel() to a dead object");
		return 2;
	}

	char t[] = {r->level, 0};
	lua_pushstring(L,t);

	return 1;
}

static int sel_getMessage(lua_State *L){
	struct selErrorStorage *r = checkSelError(L);

	if(!r){
		lua_pushnil(L);
		lua_pushstring(L, "sel_getMessage() to a dead object");
		return 2;
	}

	lua_pushstring(L,r->msg);

	return 1;
}

static const struct luaL_Reg SelErrorM [] = {
	{"getLevel", sel_getLevel},
	{"getMessage", sel_getMessage},
	{NULL, NULL}
};

static void registerSelError(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelError", SelErrorLib);
	selLua->objFuncs(L, "SelError", SelErrorM);
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

	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;

		/* Other mandatory modules */

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selError, "SelError", SELERROR_VERSION, LIBSELENE_VERSION))
		return false;

	selError.create = sec_create;
	selError.isSelError = sec_isSelError;

	registerModule((struct SelModule *)&selError);

	registerSelError(NULL);
	selLua->AddStartupFunc(registerSelError);
	return true;
}
