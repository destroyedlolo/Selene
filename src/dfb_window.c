/* dfb_window.c
 *
 * This file contains all stuffs related to DirectFB's windows
 *
 * 26/10/2015 LF : First version.
 * 		Very small subset of what DirectFB can do, focussing on my dashboard needs
 */
#include "directfb.h"

#ifdef USE_DIRECTFB
#include <assert.h>

static const struct ConstTranscode _WndCaps[] = {
	{ "NONE", DWCAPS_NONE },
	{ "ALPHACHANNEL", DWCAPS_ALPHACHANNEL },
	{ "DOUBLEBUFFER", DWCAPS_DOUBLEBUFFER },
	{ "NODECORATION", DWCAPS_NODECORATION },
	{ "NOFOCUS", DWCAPS_NOFOCUS },
	{ NULL, 0 }
};

static int WindowsCapsConst(lua_State *L ){
	return findConst(L, _WndCaps);
}

static const struct ConstTranscode _WndStk[] = {
	{ "MIDDLE", DWSC_MIDDLE },
	{ "UPPER", DWSC_UPPER },
	{ "LOWER", DWSC_LOWER },
	{ NULL, 0 }
};

static int WindowsStackingConst(lua_State *L ){
	return findConst(L, _WndStk);
}

static IDirectFBWindow **checkSelWindow(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelWindow");
	luaL_argcheck(L, r != NULL, 1, "'SelWindow' expected");
	return (IDirectFBWindow **)r;
}

static int WindowGetSurface(lua_State *L){
	IDirectFBWindow *wnd = *checkSelWindow(L);
	IDirectFBSurface **srf;
	DFBResult err;

	if(!wnd){
		lua_pushnil(L);
		lua_pushstring(L, "GetSurface() on a dead Window object");
		return 2;
	}

	srf = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = wnd->GetSurface(wnd, srf)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static int WindowSetOpacity(lua_State *L){
	IDirectFBWindow *wnd = *checkSelWindow(L);
	int val = luaL_checkint(L, 2);
	DFBResult err;

	if((err = wnd->SetOpacity(wnd, val)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	
	return 0;
}

static int WindowRelease(lua_State *L){
	IDirectFBWindow **s = checkSelWindow(L);

	if(!*s){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	(*s)->Release(*s);
	*s = NULL;	/* Prevent double desallocation */

	return 0;
}

static const struct luaL_reg SelWndLib [] = {
	{"CapsConst", WindowsCapsConst},
	{"StackingConst", WindowsStackingConst},
	{NULL, NULL}
};

static const struct luaL_reg SelWndM [] = {
	{"Release", WindowRelease},
	{"GetSurface", WindowGetSurface},
	{"SetOpacity", WindowSetOpacity},
	{NULL, NULL}
};

void _include_SelWindow( lua_State *L ){
	luaL_newmetatable(L, "SelWindow");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelWndM);
	luaL_register(L,"SelWindow", SelWndLib);
}

#endif
