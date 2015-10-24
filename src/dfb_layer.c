/* dfb_layer.c
 *
 * This file contains all stuffs related to DirectFB's layers
 *
 * 24/10/2015 LF : First version.
 * 			Focussing only on getting the currently displayed surface
 */
#include "directfb.h"

#ifdef USE_DIRECTFB
#include <assert.h>

static const struct luaL_reg SelLayerLib [] = {
	{NULL, NULL}
};

static const struct luaL_reg SelLayerM [] = {
	{NULL, NULL}
};

void _include_SelLayer( lua_State *L ){
	luaL_newmetatable(L, "SelLayer");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelLayerM);
	luaL_register(L,"SelScreen", SelLayerLib);
}

#endif
