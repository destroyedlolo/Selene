/* Selene object */

#include <unistd.h>	     /* gethostname(), ... */
#include <time.h>

#include "libSelene.h"

static int SelSleep( lua_State *L ){
	struct timespec ts;
	lua_Number lenght = luaL_checknumber(L, 1);
	ts.tv_sec = (time_t)lenght;
	ts.tv_nsec = (unsigned long int)((lenght - (time_t)lenght) * 1e9);

	nanosleep( &ts, NULL );
	return 0;
}

static int SelHostname( lua_State *L ){
	char n[HOST_NAME_MAX];
	gethostname(n, HOST_NAME_MAX);

	lua_pushstring(L, n);
	return 1;
}

/* Selene own functions */
static const struct luaL_Reg seleneLib[] = {
	{"Sleep", SelSleep},
	{"Hostname", SelHostname},
	{NULL, NULL} /* End of definition */
};


int initSelene( lua_State *L ){
	libSel_openlib( L, "Selene", seleneLib );

	return 1;
}

