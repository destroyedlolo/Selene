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
#define SELLUA_VERSION 8

#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>	/* Functions to open libraries */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TASKSSTACK_LEN
#	define TASKSSTACK_LEN 256	/* Maximum number pending tasks */
#endif

#ifndef WAITMAXFD
#	define WAITMAXFD	256		/* Maximum number of FD to wait for */
#endif


struct ConstTranscode;

enum TaskOnce {
	TO_MULTIPLE = 0,	/* Allow multiple run */
	TO_ONCE,			/* Push a task only if it isn't already queued */
	TO_LAST				/* Only one run but put at the end of the queue */
};

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
	int (*pushtask)(int, enum TaskOnce);
	int (*getToDoListFD)(void);
	int (*handleToDoList)(lua_State *L);

	int (*registerfunc)(lua_State *L);
	void (*dumpstack)(lua_State *L);
	int (*TaskOnceConst)(lua_State *L);
	int (*PushTaskByRef)(lua_State *L);
	int (*PushTask)(lua_State *L);
	bool (*isToDoListEmpty)();
	int (*dumpToDoList)(lua_State *L);

	void (*AddStartupFunc)(void (*)(lua_State *));
	void (*ApplyStartupFunc)(lua_State *);
};

#ifdef __cplusplus
}
#endif

#endif
