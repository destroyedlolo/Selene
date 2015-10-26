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

static int WindowsCapConst(lua_State *L ){
	return findConst(L, _WndCaps);
}

static const struct luaL_reg SelWndLib [] = {
	{"CapsConst", WindowsCapConst},
	{NULL, NULL}
};

static const struct luaL_reg SelWndM [] = {
	{NULL, NULL}
};

void _include_SelWindow( lua_State *L ){
	luaL_newmetatable(L, "SelWindows");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelWndM);
	luaL_register(L,"SelLayer", SelWndLib);
}

#endif
