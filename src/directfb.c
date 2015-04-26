/* directfb.c
 *
 * This file contains all stuffs related to DirectFB.
 *
 * 13/04/2015 LF : First version
 */
#include "directfb.h"

#include <assert.h>

IDirectFB *dfb = NULL;	/* DirectDB's "super interface" */

	/*****
	 * Transcodification
	 *****/

int findConst( lua_State *L, const struct ConstTranscode *tbl ){
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

int SetCooperativeLevel( lua_State *L ){
	int arg = luaL_checkint(L, 1);	/* Get the constant name to retrieve */
	DFBResult err;
	assert(dfb);

	if((err = dfb->SetCooperativeLevel(dfb, arg)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

	/****
	 * Font
	 ****/

static int createfont(lua_State *L){
	DFBResult err;
	IDirectFBFont **pfont;
	DFBFontDescription desc;
	const char *fontname = luaL_checkstring(L, 1);
	assert(dfb);
	desc.flags = 0;

	if(lua_istable(L, 2)){
		lua_pushstring(L, "height");
		lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
		if(lua_type(L, -1) == LUA_TNUMBER){
			desc.flags = DFDESC_HEIGHT;
			desc.height = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);	/* Remove the result we don't need */

		lua_pushstring(L, "width");
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			desc.flags = DFDESC_WIDTH;
			desc.width = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);	/* Remove the result we don't need */

/* tbd : other fields */
	} else if(!lua_isnoneornil(L, 2))
		return luaL_error(L, "createfont() : Second optional argument has to be a table");

	pfont = (IDirectFBFont **)lua_newuserdata(L, sizeof(IDirectFBFont *));
	luaL_getmetatable(L, "SelFont");
	lua_setmetatable(L, -2);

	if((err = dfb->CreateFont( dfb, fontname, &desc, pfont)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static IDirectFBFont **checkSelFont(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelFont");
	luaL_argcheck(L, r != NULL, 1, "'SelFont' expected");
	return (IDirectFBFont **)r;
}

static int FontRelease(lua_State *L){
	IDirectFBFont **font = checkSelFont(L);

	if(!*font){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	(*font)->Release(*font);
	*font = NULL;	/* Prevent double desallocation */

	return 0;
}

static int FontGetHeight(lua_State *L){
	IDirectFBFont *font = *checkSelFont(L);
	DFBResult err;
	int h;

	if(!font){
		lua_pushnil(L);
		lua_pushstring(L, "GetHeight() on a dead object");
		return 2;
	}

	if((err = font->GetHeight(font, &h)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	lua_pushinteger(L, h);

	return 1;
}

static int FontStringWidth(lua_State *L){
	IDirectFBFont *font = *checkSelFont(L);
	const char *msg = luaL_checkstring(L, 2);
	DFBResult err;
	int w;

	if(!font){
		lua_pushnil(L);
		lua_pushstring(L, "GetStringWidth() with a dead font");
		return 2;
	}

	if((err = font->GetStringWidth(font, msg, -1, &w)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	lua_pushinteger(L, w);

	return 1;
}

static DFBEnumerationResult enccallback(DFBTextEncodingID id, const char *n, void *data){
	lua_pushinteger( ((struct callbackContext *)data)->L, id );
	lua_newtable(((struct callbackContext *)data)->L);

	lua_pushstring( ((struct callbackContext *)data)->L, "id");
	lua_pushinteger( ((struct callbackContext *)data)->L, id );
	lua_settable( ((struct callbackContext *)data)->L, 4 );

	lua_pushstring( ((struct callbackContext *)data)->L, "name");
	lua_pushstring( ((struct callbackContext *)data)->L, n );
	lua_settable( ((struct callbackContext *)data)->L, 4 );

	lua_settable( ((struct callbackContext *)data)->L, 2 );

	return DFENUM_OK;
}

static int EnumEncodings(lua_State *L){
	struct callbackContext dt;
	IDirectFBFont *font = *checkSelFont(L);
	dt.L = L;
	dt.index = 0;

	if(!font){
		lua_pushnil(L);
		lua_pushstring(L, "EnumEncodings() with a dead font");
		return 2;
	}

	lua_newtable(L);	/* Result table */

	font->EnumEncodings(font, enccallback, &dt);

	return 1;
}

static int FindEncoding(lua_State *L){
	IDirectFBFont *font = *checkSelFont(L);
	const char *encname = luaL_checkstring(L, 2);
	DFBResult err;
	DFBTextEncodingID id;

	if(!font){
		lua_pushnil(L);
		lua_pushstring(L, "FindEncoding() with a dead font");
		return 2;
	}

	if((err = font->FindEncoding(font, encname, &id)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	lua_pushinteger(L, id);

	return 1;
}

static int SetEncoding(lua_State *L){
	IDirectFBFont *font = *checkSelFont(L);
	DFBTextEncodingID eid;
	DFBResult err;

	if(!font){
		lua_pushnil(L);
		lua_pushstring(L, "FindEncoding() with a dead font");
		return 2;
	}

	if(lua_isstring(L, 2)){
		const char *encname = lua_tostring(L, 2);
		if((err = font->FindEncoding(font, encname, &eid)) != DFB_OK){
			lua_pushnil(L);
			lua_pushstring(L, DirectFBErrorString(err));
			return 2;
		}
	} else if(lua_isnumber(L, 2))
		eid = lua_tointeger(L,2);
	else {
		lua_pushnil(L);
		lua_pushstring(L, "SetEncoding() requires a number or a string");
		return 2;
	}

	if((err = font->SetEncoding(font, eid)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static const struct luaL_reg SelFontLib [] = {
	{"create", createfont},
	{NULL, NULL}
};

static const struct luaL_reg SelFontM [] = {
	{"Release", FontRelease},
	{"destroy", FontRelease},	/* Alias */
	{"GetHeight", FontGetHeight},
	{"EnumEncodings", EnumEncodings},
	{"FindEncoding", FindEncoding},
	{"SetEncoding", SetEncoding},
	{"StringWidth", FontStringWidth},
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

	_include_SelSurface(L);
	_include_SelScreen(L);


		/* Transforms SelFont to object
		 * From http://www.lua.org/pil/28.3.html
		 */

	luaL_newmetatable(L, "SelFont");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelFontM);
	luaL_register(L,"SelFont", SelFontLib);
}
