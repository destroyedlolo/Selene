/* SelLua.h
 *
 * Lua support
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLUA_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLUA_VERSION 1

#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>	/* Functions to open libraries */

#ifdef __cplusplus
extern "C"
{
#endif

struct SelLua {
	struct SelModule module;

		/* Call backs */
	lua_State *(*getLuaState)();
	bool (*libFuncs)(lua_State *L, const char *name, const struct luaL_Reg *funcs);
	bool (*libAddFuncs)(lua_State *L, const char *name, const struct luaL_Reg *funcs);
	bool (*objFuncs)(lua_State *L, const char *name, const struct luaL_Reg *funcs);

};

#ifdef __cplusplus
}
#endif

#endif
