/* directfb.c
 *
 * This file contains all stuffs related to DirectFB.
 *
 * 13/04/2015 LF : First version
 */
#include "directfb.h"

#include <assert.h>

static IDirectFB *dfb = NULL;	/* DirectDB's "super interface" */

	/*****
	 * Transcodification
	 *****/

struct ConstTranscode {
	const char *name;
	const int value;
};

static int findConst( lua_State *L, const struct ConstTranscode *tbl ){
	const char *arg = luaL_checkstring(L, 1);	/* Get the constant name to retreave */

	for(int i=0; tbl[i].name; i++){
		if(!strcmp(arg, tbl[i].name)){
			lua_pushnumber(L, tbl[i].value);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushstring(L, arg);
	lua_pushstring(L," : Unknown constant");
	lua_concat(L, 2);
	return 2;
}


static const struct ConstTranscode _CooperativeLevel[] = {
	{ "NORMAL", DFSCL_NORMAL },
	{ "FULLSCREEN", DFSCL_FULLSCREEN },
	{ "EXCLUSIVE", DFSCL_EXCLUSIVE },
	{ NULL, 0 }
};

int CooperativeConst( lua_State *L ){
	return findConst(L, _CooperativeLevel);
}


static const struct ConstTranscode _Capability[] = {
	{ "NONE", DSCAPS_NONE },
	{ "PRIMARY", DSCAPS_PRIMARY },
	{ "SYSTEMONLY", DSCAPS_SYSTEMONLY },
	{ "VIDEOONLY", DSCAPS_VIDEOONLY },
	{ "DOUBLE", DSCAPS_DOUBLE },
	{ "SUBSURFACE", DSCAPS_SUBSURFACE },
	{ "INTERLACED", DSCAPS_INTERLACED },
	{ "SEPARATED", DSCAPS_SEPARATED },
	{ "STATIC_ALLOC", DSCAPS_STATIC_ALLOC },
	{ "TRIPLE", DSCAPS_TRIPLE },
	{ "PREMULTIPLIED", DSCAPS_PREMULTIPLIED },
	{ "DEPTH", DSCAPS_DEPTH },
	{ "SHARED", DSCAPS_SHARED },
	{ "ROTATED", DSCAPS_ROTATED },
	{ "ALL", DSCAPS_ALL },
	{ "FLIPPING", DSCAPS_FLIPPING },
	{ NULL, 0 }
};

int CapabilityConst( lua_State *L ){
	return findConst(L, _Capability);
}

int SetCooperativeLevel( lua_State *L ){
	int arg = luaL_checkint(L, 1);	/* Get the constant name to retreave */
	DFBResult err;
	assert(dfb);

	if((err = dfb->SetCooperativeLevel(dfb, arg)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static const struct ConstTranscode _TextLayout [] = {
	{ "LEFT", DSTF_LEFT },
	{ "CENTER", DSTF_CENTER },
	{ "RIGHT", DSTF_RIGHT },
	{ "TOP", DSTF_TOP },
	{ "BOTTOM", DSTF_BOTTOM },
	{ "OUTLINE", DSTF_OUTLINE },
	{ NULL, 0 }
};

int TextLayoutConst( lua_State *L ){
	return findConst(L, _TextLayout);
}

	/*****
	 * Surface
	 *****/

static int createsurface(lua_State *L){
	DFBResult err;
	IDirectFBSurface **sp;
	DFBSurfaceDescription dsc;
	assert(dfb);

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "SelSurface.create() is expecting a table");
		return 2;
	}

	dsc.flags = 0;
	lua_pushstring(L, "caps");
	lua_gettable(L, -2);	/* Retreave caps parameter if it exists */
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags = DSDESC_CAPS;
		dsc.caps = luaL_checkint(L, -1);
	} else
		lua_pop(L, -1);	/* Remove the result we don't need */
/* tbd : other fields */

	sp = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = dfb->CreateSurface(dfb, &dsc, sp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static IDirectFBSurface **checkSelSurface(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelSurface");
	luaL_argcheck(L, r != NULL, 1, "'SelSurface' expected");
	return (IDirectFBSurface **)r;
}

static int Release(lua_State *L){
	IDirectFBSurface **s = checkSelSurface(L);

	if(!*s){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	(*s)->Release(*s);
	*s = NULL;	/* Prevent double desallocation */

	return 0;
}

static int GetSize(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface(L);
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


static int FillRectangle(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	int w = luaL_checkint(L, 4);
	int h = luaL_checkint(L, 5);

	if((err = s->FillRectangle( s, x,y,w,h )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SetColor(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface(L);
	int r = luaL_checkint(L, 2);
	int g = luaL_checkint(L, 3);
	int b = luaL_checkint(L, 4);
	int a = luaL_checkint(L, 5);

	if((err = s->SetColor( s, r,g,b,a )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int DrawLine(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface(L);
	int sx = luaL_checkint(L, 2);
	int sy = luaL_checkint(L, 3);
	int dx = luaL_checkint(L, 4);
	int dy = luaL_checkint(L, 5);

	if((err = s->DrawLine( s, sx,sy,dx,dy )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static const struct luaL_reg SelSurfaceLib [] = {
	{"create", createsurface},
	{NULL, NULL}
};

static const struct luaL_reg SelSurfaceM [] = {
	{"Release", Release},
	{"destroy", Release},	/* Alias */
	{"GetSize", GetSize},
	{"SetColor", SetColor},
	{"FillRectangle", FillRectangle},
	{"DrawLine", DrawLine},
	{NULL, NULL}
};

	/****
	 * C access functions
	 ****/
static void clean_directFB(void){
	dfb->Release( dfb );
}

void init_directfb(lua_State *L, int *ac, char ***av ){
	DFBResult err = DirectFBInit(ac, av);
	if(err != DFB_OK){
		DirectFBError("DirectFBInit()", err);
		exit(EXIT_FAILURE);
	}

	if((err = DirectFBCreate (&dfb)) != DFB_OK){
		DirectFBError("DirectFBCreate()", err);
		exit(EXIT_FAILURE);
	}

	atexit(clean_directFB);

		/* Transforms SelSurface to object
		 * From http://www.lua.org/pil/28.3.html
		 */
	luaL_newmetatable(L, "SelSurface");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_openlib(L, NULL, SelSurfaceM, 0);

	luaL_openlib(L,"SelSurface", SelSurfaceLib, 0);
}
