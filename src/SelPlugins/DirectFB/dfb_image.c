/* dfb_image.c
 *
 * This file contains all stuffs related to DirectFB's image.
 *
 * 12/05/2015 LF : First version
 * 04/05/2018 LF : Move to v4
 */
#ifdef USE_DIRECTFB
#include "directfb.h"

#include <assert.h>

static int createimage( lua_State *L ){
	DFBResult err;
	IDirectFBImageProvider **sp;
	const char *file = luaL_checkstring(L, 1);
	assert(dfb);

	sp = (IDirectFBImageProvider **)lua_newuserdata(L, sizeof(IDirectFBImageProvider *));
	luaL_getmetatable(L, "SelImage");
	lua_setmetatable(L, -2);

	if((err = dfb->CreateImageProvider(dfb, file, sp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static IDirectFBImageProvider **checkSelImage(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelImage");
	luaL_argcheck(L, r != NULL, 1, "'SelImage' expected");
	return (IDirectFBImageProvider **)r;
}

static int ImageRelease(lua_State *L){
	IDirectFBImageProvider **s = checkSelImage(L);

	if(!*s){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	(*s)->Release(*s);
	*s = NULL;	/* Prevent double desallocation */

	return 0;
}

static int ImageGetSize(lua_State *L){
	DFBResult err;
	IDirectFBImageProvider *img = *checkSelImage(L);
	DFBSurfaceDescription dsc;

	if(!img){
		lua_pushnil(L);
		lua_pushstring(L, "GetSize() on a dead object");
		return 2;
	}

	if((err = img->GetSurfaceDescription( img, &dsc )) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	lua_pushinteger(L, dsc.width);
	lua_pushinteger(L, dsc.height);
	return 2;
}

static int ImageRenderTo(lua_State *L){
/* arguments :
 * 1 :image (self)
 * 2: img = blit destination
 * 3: table for the definition of the rectangle where to blit the image (x,y,w,h)
 *
 * Caution : remaining args are discarded
 */
	DFBResult err;
	IDirectFBImageProvider *img = *checkSelImage(L);
	IDirectFBSurface *s = *checkSelSurface(L,2);
	DFBRectangle *drec = NULL, drec_dt;

	if(!img || !s){
		lua_pushnil(L);
		lua_pushstring(L, "RenderTo() on a dead object");
		return 2;
	}

	if(lua_istable(L,3)){
		lua_settop(L, 3);
		drec = &drec_dt;

		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER)
			drec->x = luaL_checkinteger(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER)
			drec->y = luaL_checkinteger(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER)
			drec->w = luaL_checkinteger(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 4);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER)
			drec->h = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
	}

	if((err = img->RenderTo( img, s, drec ))){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int ImagetoSurface(lua_State *L){
/* Creates a surface exactly holding the image
 * (useful to blit)
 */
	DFBResult err;
	IDirectFBImageProvider *img = *checkSelImage(L);
	DFBSurfaceDescription   dsc;
	IDirectFBSurface **sp;

	if(( err = img->GetSurfaceDescription (img, &dsc)) ){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	sp = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = dfb->CreateSurface(dfb, &dsc, sp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = img->RenderTo (img, *sp, NULL)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static const struct luaL_Reg SelImageLib [] = {
	{"create", createimage},
	{NULL, NULL}
};

static const struct luaL_Reg SelImageM [] = {
	{"Release", ImageRelease},
	{"destroy", ImageRelease},	/* Alias */
	{"GetSize", ImageGetSize},
	{"RenderTo", ImageRenderTo},
	{"toSurface", ImagetoSurface},
	{NULL, NULL}
};

void _include_SelImage( lua_State *L ){
	libSel_objFuncs( L, "SelImage", SelImageM );
	libSel_libFuncs( L, "SelImage", SelImageLib );
}
#endif
