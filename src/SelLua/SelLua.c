/* SeleneLua.c
 *
 * Selene's Lua language support
 *
 * 06/02/2024 First version
 */

#include "Selene/SelLua.h"
#include "Selene/SeleneVersion.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"

static struct SelLua selLua;

static lua_State *mainL;	/* Main thread Lua's state (to make the initialisation easier */
static struct SeleneCore *selCore;
static struct SelLog *selLog;

#if LUA_VERSION_NUM <= 501
void *luaL_testudata(lua_State *L, int ud, const char *tname){
/* Like luaL_checkudata() but w/o crashing if doesn't march
 * From luaL_checkudata() source code
 * This function appeared with 5.2 so it's a workaround for 5.1
 */
	void *p = lua_touserdata(L, ud);
	if(p){
		if(lua_getmetatable(L, ud)){  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			if(!lua_rawequal(L, -1, -2))  /* does it have the correct mt ? */
				p = NULL;	/* No */
			lua_pop(L, 2);  /* remove both metatables */
			return p;
		}
	}
	return NULL;	/* Not an user data */
}
#endif

static lua_State *slc_getLuaState(){
/**
 * @brief Returns main thread Lua state
 *
 * @function getLuaState
 * @return LuaState
 */
	return mainL;
}

static bool slc_libFuncs(lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = mainL;

#if LUA_VERSION_NUM > 501
	lua_newtable(L);
	luaL_setfuncs (L, funcs, 0);
	lua_pushvalue(L, -1);	// pluck these lines out if they offend you
	lua_setglobal(L, name); // for they clobber the Holy _G
#else
	luaL_register(L, name, funcs);
#endif

	return true;
}

static bool slc_libAddFuncs(lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = mainL;

	lua_getglobal(L, name);
	if(!lua_istable(L, -1)){
		selLog->Log('E', "Can't add functions to unknown library \"%s\"", name);
		return false;
	}
#if LUA_VERSION_NUM > 501
	luaL_setfuncs (L, funcs, 0);
#else
	luaL_register(L, NULL, funcs);
#endif

	return true;
}

static bool slc_objFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = mainL;

	luaL_newmetatable(L, name);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */

#if LUA_VERSION_NUM < 503
	/* Insert __name field if Lua < 5.3
	 * on 5.3+, it's provided out of the box
	 */
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__name");
#endif

	if(funcs){	/* May be NULL if we're creating an empty metatable */
#if LUA_VERSION_NUM > 501
		luaL_setfuncs( L, funcs, 0);
#else
		luaL_register(L, NULL, funcs);
#endif
	}

	return true;
}

static int slc_findConst(lua_State *L, const struct ConstTranscode *tbl){
	const char *arg = luaL_checkstring(L, 1);	/* Get the constant name to retreave */
	bool found;
	int i = selCore->findConst(arg,tbl,&found);

	if(!found){
		lua_pushnil(L);
		lua_pushstring(L, arg);
		lua_pushstring(L," : Unknown constant");
		lua_concat(L, 2);
		return 2;
	} else {
		lua_pushnumber(L, i);
		return 1;
	}
}

static int slc_rfindConst(lua_State *L, const struct ConstTranscode *tbl){
 	int arg = luaL_checkinteger(L, 1);	/* Get the integer to retrieve */

	const char *res = selCore->rfindConst(arg,tbl);

	if(!res){
		lua_pushnil(L);
		lua_pushinteger(L, arg);
		lua_tostring(L, -1);
		lua_pushstring(L," : Unknown constant");
		lua_concat(L, 2);

		return 2;
	} else {
		lua_pushstring(L, res);
		return 1;
	}
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
	selLua.libFuncs = slc_libFuncs;
	selLua.libAddFuncs = slc_libAddFuncs;
	selLua.objFuncs = slc_objFuncs;

	selLua.findConst = slc_findConst;
	selLua.rfindConst = slc_rfindConst;

	selLua.testudata = luaL_testudata;

	registerModule((struct SelModule *)&selLua);

		/* Initialize Lua */
	mainL = luaL_newstate();
	luaL_openlibs(mainL);

		/* Define globals functions */
	lua_pushnumber(mainL, SELENE_VERSION);	/* Expose version to lua side */
	lua_setglobal(mainL, "SELENE_VERSION");

	selLog->SelLuaInitialised(&selLua);

	return true;
}
