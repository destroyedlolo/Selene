/*	libSelene.h
 *
 * 	Definitions shared outside library code itself
 */

#ifndef SEL_LIBRARY_H
#define SEL_LIBRARY_H	4.0003	/* libSelene version (major, minor, sub) */

#include "elastic_storage.h"

/*
 *	Options agnostics libraries contents
 */

extern char *SelL_strdup( const char * );	/* May missing on some system */
extern int hash( const char * );	/* calculates hash code of a string */

/*
 * Lua startup functions management
 *
 *	These function will be called at every thread creation
 *	(Generally, for libraries initialisation)
 */
#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>	/* Functions to open libraries */

	/* This function **MUST** be called before any call to shared object
	 * stuffs.
	 * It has to be called only once, with the main Lua State
	 */
extern void initSeleneLibrary( lua_State *L );

	/* Add a function to a startup list
	 * -> func : startup function to call
	 * -> list : current list (returned from a previous libSel_AddStartupFunc()
	 *  call. Null for the 1st one.
	 * <- return an opaque list object (NULL in case of fatal error)
	 *
	 * Notez-bien : they will be called in reverse order
	 */
extern void *libSel_AddStartupFunc( void (*func)( lua_State * ), void *list );

	/* Apply startup function to a Lua's State
	 * -> L : Lua state
	 * -> list : pointer returned by last libSel_AddStartupFunc() call
	 */
extern void libSel_ApplyStartupFunc( lua_State *L, void *list );

	/* Add library's functions
	 * -> L : Lua State
	 * -> name : library name
	 * -> funcs : functions array
	 */
extern int libSel_libFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs);

	/* Add object's functions
	 * -> L : Lua State
	 * -> name : library name
	 * -> funcs : functions array
	 */
extern int libSel_objFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs);

	/* Trans codification stuffs */
struct ConstTranscode {
	const char *name;
	const int value;
};

extern int findConst( lua_State *, const struct ConstTranscode * );
extern int rfindConst( lua_State *, const struct ConstTranscode * );

	/* Find a function reference
	 * -> L : Lua State
	 * -> id : function identifier
	 */
extern int findFuncRef(lua_State *L, int id);

	/* Load a shared function from an elastic storage
	 * -> L : Lua State
	 * -> func : stored function to load
	 * <- same as lua_load()'s return
	 */
extern int loadsharedfunction(lua_State *L, struct elastic_storage *func);

	/* Creates Selene objects
	 * -> L : Lua State
	 */
extern int initSelene( lua_State *L );
extern int initSelLog( lua_State *L );
extern int initSelCollection( lua_State *L );
extern int initSelTimedCollection( lua_State *L );
extern int initSelTimedWindowCollection( lua_State *L );
extern int initSelTimer( lua_State *L );
extern int initSelShared( lua_State *L );
extern int initSelSharedFunc( lua_State *L );
extern int initSelFIFO( lua_State *L );
extern int initSelEvent( lua_State * );
#ifdef USE_MQTT
extern int initSelMQTT(lua_State *);
#endif

#endif
