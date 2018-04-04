/* LuaSupportFunc
 *
 *	General Lua functions
 */

#include "libSelene.h"

#include <stdlib.h>

struct startupFunc {
	struct startupFunc *next;			/* Next entry */
	void (*func)( lua_State * );	/* Function to launch */
};

void *libSel_AddStartupFunc( void (*func)( lua_State * ), void *lst ){
	struct startupFunc *new = malloc( sizeof(struct startupFunc) );
	if(!new)
		return NULL;

	new->func = func;
	new->next = lst;

	return new;
}

void libSel_ApplyStartupFunc( lua_State *L, void *list ){
	struct startupFunc *lst = (struct startupFunc *)list;	/* just to avoid zillion of casts */

	for(;lst; lst = lst->next)
		lst->func( L );
}

int libSel_openlib( lua_State *L, const char *name, const struct luaL_Reg *funcs){
#if LUA_VERSION_NUM > 501
        lua_newtable(L);
        luaL_setfuncs (L, funcs, 0);
        lua_pushvalue(L, -1);        // pluck these lines out if they offend you
        lua_setglobal(L, name); // for they clobber the Holy _G
#else
        luaL_register(L, name, funcs);
#endif
        return 1;
}

