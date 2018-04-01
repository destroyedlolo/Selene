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
