/***
 * 

@classmod SelDCSurfaceImages

 *
 * All stuff related to images handling in surfaces
 *
 * 23/05/2020 LF : Creation
 * 29/04/2024 LF : Migrate to v7
 */

#include <assert.h>
#include <cairo-svg.h>

#include "DRMCairo.h"

#ifdef CAIRO_HAS_PNG_FUNCTIONS
static int createFromPNG(lua_State *L){
/** Create an image surface from a PNG file.
 *
 * Due to inheritances problem with Lua's metatables, those surface
 * are exposed as **SelDCSurface**. Dedicated methods have to rely on field 
 * '*type*' to check if there are applicable or not.
 *
 * @function createFromPNG
 * @tparam string filename
 * @treturn SelDCSurface surface.
 */

	cairo_status_t err;
	const char *filename= luaL_checkstring(L, 1);
	struct SelDCSurface *srf = (struct SelDCSurface *)lua_newuserdata(L, sizeof(struct SelDCSurface));
	assert(srf);
	luaL_getmetatable(L, "SelDCSurface");
	lua_setmetatable(L, -2);

	srf->surface = cairo_image_surface_create_from_png(filename);
	srf->cr = cairo_create(srf->surface);
	srf->type = DCSURFACE_IMAGE;

	if( (err=cairo_status(srf->cr)) != CAIRO_STATUS_SUCCESS){
		cairo_destroy(srf->cr);
		dc_selDRMCairo.internal_release_surface(srf);
		lua_pop(L,1);	/* Remove the newly create surface object */
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, "createFromPNG() failed");
		dc_selDRMCairo.selLog->Log('E',"createFromPNG(%s) failed", filename);
		return 3;
	}

	srf->w = cairo_image_surface_get_width(srf->surface);
	srf->h = cairo_image_surface_get_height(srf->surface);

	return 1;
}
#endif

static const struct luaL_Reg SelLib [] = {
#ifdef CAIRO_HAS_PNG_FUNCTIONS
	{"createFromPNG", createFromPNG},
#endif
	{NULL, NULL}
};

	/* Type's functions */
static const struct luaL_Reg SelM [] = {
	{NULL, NULL}
};

void _include_SelDCSurfaceImage( lua_State *L ){
	dc_selDRMCairo.selLua->objFuncs( L, "SelDCSurface", SelM );
	dc_selDRMCairo.selLua->libFuncs( L, "SelDCSurfaceImage", SelLib );
}
