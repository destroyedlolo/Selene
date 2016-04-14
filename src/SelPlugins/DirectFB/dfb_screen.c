/* dfb_screen.c
 *
 * This file contains all stuffs related to DirectFB's screen.
 *
 * 24/04/2015 LF : First version
 */
#include "directfb.h"

#ifdef USE_DIRECTFB
#include <assert.h>

static const struct ConstTranscode _ScnCaps[] = {
	{ "NONE", DSCCAPS_NONE },
	{ "VSYNC", DSCCAPS_VSYNC },
	{ "POWER_MANAGEMENT", DSCCAPS_POWER_MANAGEMENT },
	{ NULL, 0 }
};

static int ScreenCapsConst( lua_State *L ){
	return findConst(L, _ScnCaps);
}

static const struct ConstTranscode _ScnPowerMode[] = {
	{ "ON", DSPM_ON },
	{ "STANDBY", DSPM_STANDBY },
	{ "SUSPEND", DSPM_SUSPEND },
	{ "OFF", DSPM_OFF },
	{ NULL, 0 }
};

static int ScreenPowerModeConst( lua_State *L ){
	return findConst(L, _ScnPowerMode);
}

	/*
	 * Enumeration
	 */

static DFBEnumerationResult scrcallback( DFBScreenID id, DFBScreenDescription desc, void *data ){
	lua_pushinteger( ((struct callbackContext *)data)->L, ++((struct callbackContext *)data)->index );	/* Push new entry index */

	lua_newtable(((struct callbackContext *)data)->L);

		/* res['id'] = id */
	lua_pushstring( ((struct callbackContext *)data)->L, "id");
	lua_pushinteger( ((struct callbackContext *)data)->L, id );
	lua_settable( ((struct callbackContext *)data)->L, 3 );

	lua_pushstring( ((struct callbackContext *)data)->L, "name");
	lua_pushstring( ((struct callbackContext *)data)->L, desc.name);
	lua_settable( ((struct callbackContext *)data)->L, 3 );

	lua_pushstring( ((struct callbackContext *)data)->L, "caps");
	lua_pushinteger( ((struct callbackContext *)data)->L, (int)desc.caps );
	lua_settable( ((struct callbackContext *)data)->L, 3 );

	lua_settable( ((struct callbackContext *)data)->L, 1 );

	return DFENUM_OK;
}

static int EnumScreens(lua_State *L){
	struct callbackContext dt;
	dt.L = L;
	dt.index = 0;
	assert(dfb);

	lua_newtable(L);	/* Result table */

	dfb->EnumScreens(dfb, scrcallback, &dt);

	return 1;
}

static int GetScreen(lua_State *L){
	DFBResult err;
	IDirectFBScreen **scn;
	int id;
	assert(dfb);

	if(!lua_isnumber(L, -1)){
		lua_pushnil(L);
		lua_pushstring(L, "SelScreen.GetScreen() is expecting a screen id");
		return 2;
	}
	id = luaL_checkint(L, -1);

	scn = (IDirectFBScreen **)lua_newuserdata(L, sizeof(IDirectFBScreen *));
	luaL_getmetatable(L, "SelScreen");
	lua_setmetatable(L, -2);

	if((err = dfb->GetScreen(dfb, id, scn)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static IDirectFBScreen **checkSelScreen(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelScreen");
	luaL_argcheck(L, r != NULL, 1, "'SelScreen' expected");
	return (IDirectFBScreen **)r;
}

static int ScreenGetSize(lua_State *L){
	DFBResult err;
	IDirectFBScreen *s = *checkSelScreen(L);
	int w,h;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "GetSize() on a dead object");
		return 2;
	}

	if((err = s->GetSize(s, &w, &h)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	lua_pushinteger(L, w);
	lua_pushinteger(L, h);
	return 2;
}

static int ScreenSetPowerMode(lua_State *L){
	DFBResult err;
	IDirectFBScreen *s = *checkSelScreen(L);
	int mode = luaL_checkint(L, 2);

	if((err = s->SetPowerMode( s, mode )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int ScreenWaitForSync(lua_State *L){
	DFBResult err;
	IDirectFBScreen *s = *checkSelScreen(L);

	if((err = s->WaitForSync(s)) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static const struct luaL_reg SelScreenLib [] = {
	{"CapsConst", ScreenCapsConst},
	{"PowerModeConst", ScreenPowerModeConst},
	{"Enumerate", EnumScreens},
	{"GetScreen", GetScreen},
	{NULL, NULL}
};

static const struct luaL_reg SelScreenM [] = {
	{"GetSize", ScreenGetSize},
	{"SetPowerMode", ScreenSetPowerMode},
	{"WaitForSync", ScreenWaitForSync},
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
#endif
