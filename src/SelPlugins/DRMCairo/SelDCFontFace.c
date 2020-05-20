/* SelDCFontFace
 *
 *	This file contains all stuffs related to fontFaces
 *	Fontface defines the font family to use that will be implemented in 
 *	Font objects.
 *
 * 17/05/2020 LF : Creation
 */

#ifdef USE_DRMCAIRO

#include <assert.h>

#include "DRMCairo.h"

static cairo_font_face_t *checkSelDCFontFace(lua_State *L){
	cairo_font_face_t **r = luaL_checkudata(L, 1, "SelDCFontFace");
	luaL_argcheck(L, r != NULL, 1, "'SelDCFontFace' expected");
	return *r;
}

static const struct ConstTranscode _Slant[] = {
	{ "NORMAL", CAIRO_FONT_SLANT_NORMAL },
	{ "ITALIC", CAIRO_FONT_SLANT_ITALIC },
	{ "OBLIQUE", CAIRO_FONT_SLANT_OBLIQUE },
	{ NULL, 0 }
};

static int SlantConst(lua_State *L){
	return findConst(L, _Slant);
}

static const struct ConstTranscode _Weight[] = {
	{ "NORMAL", CAIRO_FONT_WEIGHT_NORMAL },
	{ "BOLD", CAIRO_FONT_WEIGHT_BOLD },
	{ NULL, 0 }
};

static int WeightConst(lua_State *L){
	return findConst(L, _Weight);
}

static int createInternal(lua_State *L){
/* Select internal font using a simplified interface
 *	-> fontname ("serif", "sans-serif", "cursive", "fantasy", "monospace")
 *	-> options as a table
 *		"slant" = One of SlantConst
 *		"weight" = One of WeightConst
 */
	cairo_font_face_t **pfont;
	const char *fontname = luaL_checkstring(L, 1);
	cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
	cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;

	if(lua_istable(L, 2)){
		lua_pushstring(L, "slant");
		lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
		if(lua_type(L, -1) == LUA_TNUMBER)
			slant = luaL_checkinteger(L, -1);
		lua_pop(L, 1);	/* Remove the result we don't need */

		lua_pushstring(L, "weight");
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER)
			weight = luaL_checkinteger(L, -1);
		lua_pop(L, 1);	/* Remove the result we don't need */
	} else if(!lua_isnoneornil(L, 2))
		return luaL_error(L, "createSimplified() : Third optional argument has to be a table");

	assert( (pfont = (cairo_font_face_t **)lua_newuserdata(L, sizeof(cairo_font_face_t *))) );
	luaL_getmetatable(L, "SelDCFontFace");
	lua_setmetatable(L, -2);

	pfont = (cairo_font_face_t **)lua_newuserdata(L, sizeof(cairo_font_face_t *));
	luaL_getmetatable(L, "SelDCFontFace");
	lua_setmetatable(L, -2);

	*pfont = cairo_toy_font_face_create(fontname, slant, weight);
	if(cairo_font_face_status(*pfont) != CAIRO_STATUS_SUCCESS){
		lua_pushnil(L);
		lua_pushstring(L, "Can't create font");
		return 2;
	}
	return 1;
}

static int Release(lua_State *L){
/* Free resources used by font */
	cairo_font_face_t *font = checkSelDCFontFace(L);
	cairo_font_face_destroy(font);

	return 0;
}

static const struct luaL_Reg SelDCFontFaceLib [] = {
	{"createInternal", createInternal},
	{"SlantConst", SlantConst},
	{"WeightConst", WeightConst},
	{NULL, NULL}
};

static const struct luaL_Reg SelDCFontFaceM [] = {
	{"Release", Release},
	{"destroy", Release},	/* Alias */
/*	{"GetHeight", FontFaceGetHeight},
	{"EnumEncodings", EnumEncodings},
	{"FindEncoding", FindEncoding},
	{"SetEncoding", SetEncoding},
	{"StringWidth", FontFaceStringWidth}, */
	{NULL, NULL}
};

void _include_SelFontFace( lua_State *L ){
	libSel_objFuncs( L, "SelDCFontFace", SelDCFontFaceM );
	libSel_libFuncs( L, "SelDCFontFace", SelDCFontFaceLib );
}

#endif
