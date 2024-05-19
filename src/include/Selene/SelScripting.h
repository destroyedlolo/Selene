/* SelScripting.h
 *
 * Expose Lua scripting functions.
 * Because not all application using Selene framework need scripting related
 * methods.
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELSCRIPTING_VERSION
#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELSCRIPTING_VERSION 2

#ifdef __cplusplus
extern "C"
{
#endif

struct SelScripting {
	struct SelModule module;

		/* Call backs */
	int (*pushtask)(int, enum TaskOnce);
	int (*findFuncRef)(lua_State *L, int num);
/*
	int (*getToDoListFD)(void);
	int (*handleToDoList)(lua_State *L);

	int (*registerfunc)(lua_State *L);
	int (*TaskOnceConst)(lua_State *L);
	int (*PushTaskByRef)(lua_State *L);
	int (*PushTask)(lua_State *L);
	bool (*isToDoListEmpty)();
	int (*dumpToDoList)(lua_State *L);
*/
};

#ifdef __cplusplus
}
#endif

#endif
