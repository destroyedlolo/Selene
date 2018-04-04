/* Selene object */

#include <unistd.h>             /* gethostname(), ... */

#include "libSelene.h"

static int SelHostname( lua_State *L ){
        char n[HOST_NAME_MAX];
        gethostname(n, HOST_NAME_MAX);

        lua_pushstring(L, n);
        return 1;
}

/* Selene own functions */
static const struct luaL_Reg seleneLib[] = {
        {"Hostname", SelHostname},
        {NULL, NULL} /* End of definition */
};


int initSelene( lua_State *L ){
	libSel_openlib( L, "Selene", seleneLib );

	return 1;
}

