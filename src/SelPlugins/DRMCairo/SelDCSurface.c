/* SelDCSurface
 *
 * This file contains all stuffs related to Cairo surfaces
 *
 * 17/05/2020 LF : Creation
 */

#ifdef USE_DRMCAIRO

#include "DRMCairo.h"

static struct SelDCSurface **checkSelDCSurface(lua_State *L, int where){
	void *r = luaL_checkudata(L, where, "SelDCSurface");
	luaL_argcheck(L, r != NULL, where, "'SelDCSurface' expected");
	return (struct SelDCSurface **)r;
}

/* Object's own functions */
static const struct luaL_Reg SelDCSurfaceLib [] = {
/*	{"create", createsurface}, */
	{NULL, NULL}
};

	/* Type's functions */
static const struct luaL_Reg SelDCSurfaceM [] = {
/*	{"Release", SurfaceRelease},
	{"destroy", SurfaceRelease},	// Alias
	{"GetPosition", SurfaceGetPosition},
	{"GetSize", SurfaceGetSize},
	{"GetHight", SurfaceGetHight},
	{"GetWidth", SurfaceGetWidth},
	{"GetAfter", SurfaceGetAfter},
	{"GetBelow", SurfaceGetBelow},
	{"Clear", SurfaceClear},
	{"SetColor", SurfaceSetColor},
	{"SetDrawingFlags", SurfaceSetDrawingFlags},
	{"DrawRectangle", SurfaceDrawRectangle},
	{"FillGrandient", SurfaceFillGrandient},
	{"FillRectangle", SurfaceFillRectangle},
	{"FillTriangle", SurfaceFillTriangle},
	{"DrawLine", SurfaceDrawLine},
	{"DrawCircle", SurfaceDrawCircle},
	{"FillCircle", SurfaceFillCircle},
	{"DrawString", SurfaceDrawString},
	{"SetBlittingFlags", SurfaceSetBlittingFlags},
	{"SetRenderOptions", SurfaceSetRenderOptions},
	{"Blit", SurfaceBlit},
	{"TileBlit", SurfaceTileBlit},
	{"TileBlitClip", SurfaceTileBlitClip},
	{"StretchBlit", SurfaceStretchBlit},
	{"SetClip", SurfaceSetClip},
	{"SetClipS", SurfaceSetClipS},
	{"SetFont", SurfaceSetFont},
	{"GetFont", SurfaceGetFont},
	{"SubSurface", SurfaceSubSurface},
	{"GetSubSurface", SurfaceSubSurface},
	{"GetPixelFormat", SurfaceGetPixelFormat},
	{"Flip", SurfaceFlip}, */
/*	{"Dump", SurfaceDump}, */
/*	{"clone", SurfaceClone},
	{"restore", SurfaceRestore},
*/
	{NULL, NULL}
};

void _include_SelDCSurface( lua_State *L ){
	libSel_objFuncs( L, "SelDCSurface", SelDCSurfaceM );
	libSel_libFuncs( L, "SelDCSurface", SelDCSurfaceLib );
}


#endif
