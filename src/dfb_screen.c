/* dfb_screen.c
 *
 * This file contains all stuffs related to DirectFB's screen.
 *
 * 24/04/2015 LF : First version
 */
#include "directfb.h"

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

	/*
	 * Enumeration
	 */

struct scb_data {
	lua_State *L;
	int index;
};

static DFBEnumerationResult scrcallback( DFBScreenID id, DFBScreenDescription desc, void *data ){
	lua_pushinteger( ((struct scb_data *)data)->L, ++((struct scb_data *)data)->index );	/* Push new entry index */

	lua_newtable(((struct scb_data *)data)->L);

		/* res['id'] = id */
	lua_pushstring( ((struct scb_data *)data)->L, "id");
	lua_pushinteger( ((struct scb_data *)data)->L, id );
	lua_settable( ((struct scb_data *)data)->L, 3 );

	lua_pushstring( ((struct scb_data *)data)->L, "name");
	lua_pushstring( ((struct scb_data *)data)->L, desc.name);
	lua_settable( ((struct scb_data *)data)->L, 3 );

	lua_pushstring( ((struct scb_data *)data)->L, "caps");
	lua_pushinteger( ((struct scb_data *)data)->L, (int)desc.caps );
	lua_settable( ((struct scb_data *)data)->L, 3 );

	lua_settable( ((struct scb_data *)data)->L, 1 );

	return DFENUM_OK;
}

static int EnumScreens(lua_State *L){
	struct scb_data dt;
	dt.L = L;
	dt.index = 0;
	assert(dfb);

	lua_newtable(L);	/* Result table */

	dfb->EnumScreens( dfb, scrcallback, &dt );

	return 1;
}

static const struct luaL_reg SelScreenLib [] = {
	{"ScreenCapsConst", ScreenCapsConst},
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
