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

static const struct ConstTranscode _LayerCL[] = {
	{ "SHARE", DLSCL_SHARED },
	{ "EXCLUSIVE", DLSCL_EXCLUSIVE },
	{ "ADMINISTRATIVE", DLSCL_ADMINISTRATIVE },
	{ NULL, 0 }
};

static int LayerCoopLevel(lua_State *L ){
	return findConst(L, _LayerCL);
}

static DFBEnumerationResult layerenumcb( DFBDisplayLayerID id, DFBDisplayLayerDescription desc, void *L ){
	lua_pushinteger((lua_State *)L, id);
	return DFB_OK;
}

static int LayerEnumLayer(lua_State *L){
	DFBResult err;

	if((err = dfb->EnumDisplayLayers(dfb, layerenumcb, L)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return lua_gettop(L);
}

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

static int LayerGetSurface(lua_State *L){
	IDirectFBDisplayLayer *lyr = *checkSelLayer(L);
	IDirectFBSurface **srf;
	DFBResult err;

	if(!lyr){
		lua_pushnil(L);
		lua_pushstring(L, "GetSurface() on a dead Layer object");
		return 2;
	}

	srf = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = lyr->GetSurface(lyr, srf)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static int LayerGetLevel(lua_State *L){
	IDirectFBDisplayLayer *lyr = *checkSelLayer(L);
	DFBResult err;
	int lev;

	if(!lyr){
		lua_pushnil(L);
		lua_pushstring(L, "GetLevel() on a dead Layer object");
		return 2;
	}


	if((err = lyr->GetLevel(lyr, &lev)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	lua_pushinteger(L, lev);

	return 1;
}

static int LayerCreateWindow(lua_State *L){
	IDirectFBDisplayLayer *layer = *checkSelLayer(L);
	DFBResult err;
	IDirectFBWindow **wp;
	DFBWindowDescription dsc;
	assert(dfb);

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "SelLayer.CreateWindow() is expecting a table");
		return 2;
	}

	dsc.flags = 0;

	lua_pushstring(L, "caps");
	lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags = DWDESC_CAPS;
		dsc.caps = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "surface_caps");
	lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_SURFACE_CAPS;
		dsc.surface_caps = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "pixelformat");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_PIXELFORMAT;
		dsc.pixelformat = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "options");
	lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_OPTIONS;
		dsc.options = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "stacking");
	lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |=  DWDESC_STACKING ;
		dsc.stacking = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "x");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_POSX;
		dsc.posx = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "y");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_POSY;
		dsc.posy = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "pos");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TTABLE){
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			dsc.flags |= DWDESC_POSX;
			dsc.posx = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);
		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			dsc.flags |= DWDESC_POSY;
			dsc.posy = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "width");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_WIDTH;
		dsc.width = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "height");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DWDESC_HEIGHT;
		dsc.height = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "size");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TTABLE){
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			dsc.flags |= DWDESC_WIDTH;
			dsc.width = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);
		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			dsc.flags |= DWDESC_HEIGHT;
			dsc.height = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	wp = (IDirectFBWindow **)lua_newuserdata(L, sizeof(IDirectFBWindow *));
	luaL_getmetatable(L, "SelWindow");
	lua_setmetatable(L, -2);

	if((err = layer->CreateWindow(layer,&dsc, wp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static int LayerSetCooperativeLevel(lua_State *L){
	IDirectFBDisplayLayer *layer = *checkSelLayer(L);
	DFBResult err;

	if(!lua_isnumber(L, 1)){
		lua_pushnil(L);
		lua_pushstring(L, "SelLayer.SetCooperativeLevel() is expecting an integer");
		return 2;
	}
	int level = luaL_checkint(L, 1);

	if((err = layer->SetCooperativeLevel(layer, level)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int LayerEnableCursor(lua_State *L){
	IDirectFBDisplayLayer *layer = *checkSelLayer(L);
	DFBResult err;
	int flg;

	if(lua_isnumber(L, 2))
		flg = luaL_checkint(L, 2);
	else if(lua_isboolean(L, 2))
		flg = lua_toboolean(L, 2);
	else {
		lua_pushnil(L);
		lua_pushstring(L, "SelLayer.EnableCursor() is expecting an integer or a boolean");
		return 2;
	}

	if((err = layer->EnableCursor(layer, flg)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
puts("ok");
	return 0;
}

static int LayerRelease(lua_State *L){
	IDirectFBDisplayLayer **s = checkSelLayer(L);

	if(!*s){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	(*s)->Release(*s);
	*s = NULL;	/* Prevent double desallocation */

	return 0;
}


static const struct luaL_reg SelLayerLib [] = {
	{"EnumLayer", LayerEnumLayer},
	{"GetLayer", LayerGetLayer},
	{"CooperativeLevelConst", LayerCoopLevel},
	{"CoopLevelConst", LayerCoopLevel},	/* Alias */
	{NULL, NULL}
};

static const struct luaL_reg SelLayerM [] = {
	{"Release", LayerRelease},
	{"GetScreen", LayerGetScreen},
	{"GetSurface", LayerGetSurface},
	{"GetLevel", LayerGetLevel},
	{"CreateWindow", LayerCreateWindow},
	{"EnableCursor", LayerEnableCursor},
	{"SetCooperativeLevel", LayerSetCooperativeLevel},
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
