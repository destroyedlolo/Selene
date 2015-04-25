/* Selene : DirectFB framework using Lua
 *
 * 12/04/2014 LF : First version
 * 25/04/2014 LF : Use loadfile() for script
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "selene.h"
#include "directfb.h"

#define VERSION 0.0001	/* major, minor, sub */

	/*
	 *  Lua stuffs
	 */
lua_State *L;

void clean_lua(void){
	lua_close(L);
}

void *luaL_checkuserdata(lua_State *L, int n){
	if(lua_isuserdata(L, n))
		return lua_touserdata(L, n);
	else {
		luaL_error(L, "parameter %d expected to be a userdata");
		return NULL;
	}
}

	/*
	 * Main loop
	 */
static const struct luaL_reg seleneLib[] = {
#ifdef USE_DIRECTFB
	{"CooperativeConst", CooperativeConst},
	{"SetCooperativeLevel", SetCooperativeLevel},
	{"init", SetCooperativeLevel},	/* Alias for SetCooperativeLevel */
#endif
	{NULL, NULL}    /* End of definition */
};

int main (int ac, char **av){
	char l[1024];

	L = lua_open();		/* opens Lua */
	luaL_openlibs(L);	/* and it's libraries */
	atexit(clean_lua);
	luaL_openlib(L,"Selene", seleneLib, 0);	/* Declare Selene's functions */

#ifdef USE_DIRECTFB
	init_directfb(L, &ac, &av);
#endif

	lua_pushnumber(L, VERSION);		/* Expose version to lua side */
	lua_setglobal(L, "SELENE_VERSION");

	if(ac > 1){
		int err = luaL_loadfile(L, av[1]) || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			exit(EXIT_FAILURE);
		}
	} else while(fgets(l, sizeof(l), stdin) != NULL){
		int err = luaL_loadbuffer(L, l, strlen(l), "line") || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
		}
	}

	exit(EXIT_SUCCESS);
}

