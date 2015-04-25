/* dfb_screen.c
 *
 * This file contains all stuffs related to DirectFB's screen.
 *
 * 24/04/2015 LF : First version
 */
#include "directfb.h"

#include <assert.h>

static DFBEnumerationResult scrcallback( DFBScreenID id, DFBScreenDescription desc, void *data ){
printf("screen_id : %d\n", id);
printf("screen_name : '%s'\n", desc.name);

(*(int *)data)++;
	return DFENUM_OK;
}

static int EnumScreens(lua_State *L){
	int nbre = 0;
	dfb->EnumScreens( dfb, scrcallback, &nbre );
printf("=> %d", nbre);

	return 0;
}

static const struct luaL_reg SelScreenLib [] = {
	{"Enumerate", EnumScreens},
	{NULL, NULL}
};

static const struct luaL_reg SelScreenM [] = {
	{NULL, NULL}
};

void _include_SelScreen( lua_State *L ){
	luaL_newmetatable(L, "SelScreen");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelScreenM);
	luaL_register(L,"SelScreen", SelScreenLib);
}
