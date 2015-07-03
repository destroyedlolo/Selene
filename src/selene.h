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

#include "configuration.h"

#define strdup(s) mystrdup(s)
extern char *mystrdup(const char *);

#ifdef DEBUG
extern void dumpstack(lua_State *);
#endif

struct ConstTranscode {
	const char *name;
	const int value;
};
extern int findConst( lua_State *, const struct ConstTranscode * );
extern int rfindConst( lua_State *, const struct ConstTranscode * );

extern void *luaL_checkuserdata(lua_State *, int );

	/* init of objects */
void init_SelTimer( lua_State * );
#endif

