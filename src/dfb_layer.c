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

static int LayerGetLayer(lua_State *L){
	DFBResult err;
	IDirectFBDisplayLayer **lyr;
	int id;
	assert(dfb);

	if(!lua_isnumber(L, -1)){
		lua_pushnil(L);
		lua_pushstring(L, "SelLayer.GetLayer() is expecting a Layer id");
		return 2;
	}
	id = luaL_checkint(L, -1);

	lyr = (IDirectFBDisplayLayer **)lua_newuserdata(L, sizeof(IDirectFBDisplayLayer *));
	luaL_getmetatable(L, "SelLayer");
	lua_setmetatable(L, -2);

	if((err = dfb->GetDisplayLayer(dfb, id, lyr)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static IDirectFBDisplayLayer **checkSelLayer(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelLayer");
	luaL_argcheck(L, r != NULL, 1, "'SelLayer' expected");
	return (IDirectFBDisplayLayer **)r;
}

static int LayerGetScreen(lua_State *L){
	IDirectFBDisplayLayer *lyr = *checkSelLayer(L);
	IDirectFBScreen **scn;
	DFBResult err;

	if(!lyr){
		lua_pushnil(L);
		lua_pushstring(L, "GetScreen() on a dead Layer object");
		return 2;
	}

	scn = (IDirectFBScreen **)lua_newuserdata(L, sizeof(IDirectFBScreen *));
	luaL_getmetatable(L, "SelScreen");
	lua_setmetatable(L, -2);

	if((err = lyr->GetScreen(lyr, scn)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static const struct luaL_reg SelLayerLib [] = {
	{"GetLayer", LayerGetLayer},
	{NULL, NULL}
};

static const struct luaL_reg SelLayerM [] = {
	{"GetScreen", LayerGetScreen},
	{NULL, NULL}
};

void _include_SelLayer( lua_State *L ){
	luaL_newmetatable(L, "SelLayer");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelLayerM);
	luaL_register(L,"SelLayer", SelLayerLib);
}

#endif
