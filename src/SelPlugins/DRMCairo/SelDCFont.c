/***
 * Direct Rendering Manager font related
 *

@classmod SelDCFont

 *
 * 17/05/2020 LF : Creation
 */

#include <assert.h>
#include <cairo-ft.h>

#include "DRMCairo.h"

static struct selDCFont *checkSelDCFont(lua_State *L, int idx){
	struct selDCFont *r = luaL_checkudata(L, idx, "SelDCFont");
	luaL_argcheck(L, r != NULL, 1, "'SelDCFont' expected");
	return r;
}

static const struct ConstTranscode _Slant[] = {
	{ "NORMAL", CAIRO_FONT_SLANT_NORMAL },
	{ "ITALIC", CAIRO_FONT_SLANT_ITALIC },
	{ "OBLIQUE", CAIRO_FONT_SLANT_OBLIQUE },
	{ NULL, 0 }
};

static int SlantConst(lua_State *L){
/** transcode slant font capabilities
 *
 * @function SlantConst
 * @tparam string slant "NORMAL", "ITALIC", "OBLIQUE"
 * @treturn int value
 */
	return dc_selDRMCairo.selLua->findConst(L, _Slant);
}

static const struct ConstTranscode _Weight[] = {
	{ "NORMAL", CAIRO_FONT_WEIGHT_NORMAL },
	{ "BOLD", CAIRO_FONT_WEIGHT_BOLD },
	{ NULL, 0 }
};

static int WeightConst(lua_State *L){
/** transcode Weight font capabilities
 *
 * @function WeightConst
 * @tparam string Weight "NORMAL", "BOLD"
 * @treturn int value
 */
	return dc_selDRMCairo.selLua->findConst(L, _Weight);
}

static int createInternal(lua_State *L){
	/**
	 * Use internal font using a simplified interface
	 *
	 * @function createInternal
	 * @tparam strig fontname "*serif*", "*sans-serif*", "*cursive*", "*fantasy*", "*monospace*"
	 * @tparam table options "**slant**" = One of SlantConst, "**weight**" = One of WeightConst
	 * @treturn SelDcFont font
	 * @see SlantConst, WeightConst
	 */
	struct selDCFont *pfont;
	const char *fontname = luaL_checkstring(L, 1);
	cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
	cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
	cairo_status_t err;

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

	pfont = (struct selDCFont *)lua_newuserdata(L, sizeof(struct selDCFont));
	luaL_getmetatable(L, "SelDCFont");
	lua_setmetatable(L, -2);

	pfont->ft = NULL;
	pfont->cairo = cairo_toy_font_face_create(fontname, slant, weight);
	if( (err=cairo_font_face_status(pfont->cairo)) != CAIRO_STATUS_SUCCESS){
		lua_pop(L,1);	/* Remove pfont */
		dc_selDRMCairo.selLog->Log('E',"Can't create font : %s", cairo_status_to_string(err));
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, "Can't create font");
		return 3;
	}
	return 1;
}

static int createFreeType(lua_State *L){
	/** Load a Freetype font
	 *
	 * @function createFreeType
	 * @tparam string fontname full path to the TTF file
	 * @treturn SelDcFont font
	 */
	struct selDCFont *pfont;
	const char *fontname = luaL_checkstring(L, 1);
	FT_Error status;
	cairo_status_t err;

	pfont = (struct selDCFont *)lua_newuserdata(L, sizeof(struct selDCFont));
	luaL_getmetatable(L, "SelDCFont");
	lua_setmetatable(L, -2);

	pthread_mutex_lock(&DMCContext.FT_mutex);
	if( (status = FT_New_Face(DMCContext.FT, fontname, 0, &pfont->ft)) != FT_Err_Ok ){
		pthread_mutex_unlock(&DMCContext.FT_mutex);
		lua_pop(L,1);	/* Remove pfont */
		dc_selDRMCairo.selLog->Log('E',"Can't load font");
		lua_pushnil(L);
		lua_pushstring(L, "Can't load font");
		return 2;
	}
	pthread_mutex_unlock(&DMCContext.FT_mutex);

	pfont->cairo = cairo_ft_font_face_create_for_ft_face(pfont->ft, 0);
	if( (err=cairo_font_face_status(pfont->cairo)) != CAIRO_STATUS_SUCCESS){
		lua_pop(L,1);	/* Remove pfont */
		dc_selDRMCairo.selLog->Log('E',"Can't create font : %s", cairo_status_to_string(err));
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));		
		lua_pushstring(L, "Can't create font");
		return 3;
	}
	
	return 1;
}

static int FontRelease(lua_State *L){
	/**
	 * Release font - free all ressources
	 *
	 * @function Release
	 */
	/**
	 * Release font - free all ressources
	 *
	 * @function destroy
	 */
	struct selDCFont *font = checkSelDCFont(L, 1);

	cairo_font_face_destroy(font->cairo);

	if(font->ft){
		pthread_mutex_lock(&DMCContext.FT_mutex);
		FT_Done_Face(font->ft);
		pthread_mutex_unlock(&DMCContext.FT_mutex);
		font->ft = NULL;
	}

	return 0;
}

static const struct luaL_Reg SelDCFontLib [] = {
	{"createInternal", createInternal},
	{"createFreeType", createFreeType},
	{"SlantConst", SlantConst},
	{"WeightConst", WeightConst},
	{NULL, NULL}
};

static const struct luaL_Reg SelDCFontM [] = {
	{"Release", FontRelease},
	{"destroy", FontRelease},	/* Alias */
/*	{"GetHeight", FontGetHeight},
	{"EnumEncodings", EnumEncodings},
	{"FindEncoding", FindEncoding},
	{"SetEncoding", SetEncoding},
	{"StringWidth", FontStringWidth}, */
	{NULL, NULL}
};

void _include_SelDCFont( lua_State *L ){
	dc_selDRMCairo.selLua->objFuncs( L, "SelDCFont", SelDCFontM );
	dc_selDRMCairo.selLua->libFuncs( L, "SelDCFont", SelDCFontLib );

		/* late building to avoid to export the symbol */
	if(!dc_selDRMCairo.checkSelDCFont)
		dc_selDRMCairo.checkSelDCFont = checkSelDCFont;
}
