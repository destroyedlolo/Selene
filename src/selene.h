/* selene.h
 *
 * Shared definition among modules
 *
 * 13/04/2015 LF : First version
 */

#ifndef SELENE_H
#define SELENE_H

#include <lua.h>		/* Lua's Basic */
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */

#ifdef DEBUG
extern void dumpstack(lua_State *);
#endif

extern void *luaL_checkuserdata(lua_State *, int );
#endif
