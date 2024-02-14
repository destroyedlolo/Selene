/* SelLua.h
 *
 * Lua support
 *
 * This module provides only basic mechanisms to expose objects to Lua side.
 * As such, it is mandatory for all application using Lua. Selene's are
 * implemented in SelScripting one.
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLUA_VERSION
#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLUA_VERSION 4

#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>	/* Functions to open libraries */

#ifdef __cplusplus
extern "C"
{
#endif

struct ConstTranscode;

struct SelLua {
	struct SelModule module;

		/* Call backs */
	lua_State *(*getLuaState)();
	bool (*libFuncs)(lua_State *L, const char *name, const struct luaL_Reg *funcs);
	bool (*libAddFuncs)(lua_State *L, const char *name, const struct luaL_Reg *funcs);
	bool (*objFuncs)(lua_State *L, const char *name, const struct luaL_Reg *funcs);

	int (*findConst)(lua_State *L, const struct ConstTranscode *tbl);
	int (*rfindConst)(lua_State *L, const struct ConstTranscode *tbl);

	void *(*testudata)(lua_State *L, int ud, const char *tname);

	int (*findFuncRef)(lua_State *L, int num);
};

#ifdef __cplusplus
}
#endif

#endif
