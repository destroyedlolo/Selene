/* SelDCSurface
 *
 * This file contains all stuffs related to Cairo surfaces
 *
 * 17/05/2020 LF : Creation
 */

#ifdef USE_DRMCAIRO

#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "DRMCairo.h"

static struct SelDCSurface *checkSelDCSurface(lua_State *L, int where){
	void *r = luaL_checkudata(L, where, "SelDCSurface");
	luaL_argcheck(L, r != NULL, where, "'SelDCSurface' expected");
	return (struct SelDCSurface *)r;
}

static int DCSurfaceRelease(lua_State *L){
	/* Delete a surface
	 * (i.e. remove all references to its objects 
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_destroy(srf->cr);
	cairo_surface_destroy(srf->surface);
	
	return 0;
}

static int DCSurfaceGetSize(lua_State *L){
	/* Return surface's size
	 * <- width, hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->w);
	lua_pushnumber(L, srf->h);
	return 2;
}

static int DCSurfaceGetWidth(lua_State *L){
	/* Return surface's size
	 * <- Width
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->w);
	return 2;
}

static int DCSurfaceGetHight(lua_State *L){
	/* Return surface's size
	 * <- hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->h);
	return 2;
}

static int DCSurfaceClear(lua_State *L){
	/* Fill a surface with given color
	 * -> r, g, b	(component saturation from 0 to 1)
	 * -> a 		(opacity from 0 to 1)
	 * -> width		(line width, optional, 1 by default)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number r = luaL_checknumber(L, 2);
	lua_Number g = luaL_checknumber(L, 3);
	lua_Number b = luaL_checknumber(L, 4);
	lua_Number a = luaL_checknumber(L, 5);

	cairo_save(srf->cr);
	cairo_set_source_rgba(srf->cr, r, g, b, a);
	cairo_set_operator(srf->cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(srf->cr);
	cairo_restore(srf->cr);

	return 0;
}

static int DCSurfaceSetColor(lua_State *L){
	/* Set foreground color
	 * -> r, g, b	(component saturation from 0 to 1)
	 * -> a 		(opacity from 0 to 1)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number r = luaL_checknumber(L, 2);
	lua_Number g = luaL_checknumber(L, 3);
	lua_Number b = luaL_checknumber(L, 4);
	lua_Number a = luaL_checknumber(L, 5);

	cairo_set_source_rgba (srf->cr, r, g, b, a);
	
	return 0;
}

static int DCSurfaceDrawLine(lua_State *L){
	/* Draw a line
	 * 	-> x1,y1 : start point
	 * 	-> x2,y2 : end point
	 * 	-> width (optional)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x1 = luaL_checknumber(L, 2);
	lua_Number y1 = luaL_checknumber(L, 3);
	lua_Number x2 = luaL_checknumber(L, 4);
	lua_Number y2 = luaL_checknumber(L, 5);
	lua_Number w = 1;

	if(lua_gettop(L) > 5)
		w = luaL_checknumber(L, 6);

	cairo_move_to(srf->cr, x1, y1);
	cairo_line_to(srf->cr, x2, y2);
	cairo_set_line_width(srf->cr, w);
	cairo_stroke(srf->cr);

	return 0;
}

static int DCSurfaceDrawRectangle(lua_State *L){
	/* Draw a rectangle
	 * -> x1,y1 : upper top
	 * -> w,h :	size
	 * 	-> width (optional)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x1 = luaL_checknumber(L, 2);
	lua_Number y1 = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);
	lua_Number wdt = 1;

	if(lua_gettop(L) > 5)
		wdt = luaL_checknumber(L, 6);

	cairo_rectangle(srf->cr, x1, y1, w, h);
	cairo_set_line_width(srf->cr, wdt);
	cairo_stroke(srf->cr);

	return 0;
}

static int DCSurfaceFillRectangle(lua_State *L){
	/* Draw a filled rectangle
	 * -> x1,y1 : upper top
	 * -> w,h :	size
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x1 = luaL_checknumber(L, 2);
	lua_Number y1 = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);

	cairo_rectangle(srf->cr, x1, y1, w, h);
	cairo_set_line_width(srf->cr, 1);
	cairo_fill(srf->cr);

	return 0;
}

static int DCSurfaceDump(lua_State *L){
	/* Save the surface as a PNG file 
	 *	2 : Directory where to save the file
	 *	3 : file's prefix
	 *
	 *	-> 1 : file written (nil in case of error)
	 *	-> 2 : error string
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	const char *dir= luaL_checkstring(L, 2);
	const char *prf= luaL_checkstring(L, 3);

	/* to be compatible with DirectFB's version and for
	 * convenience, the file name is incremented
	 */
	char tmp[ strlen(dir) + strlen(prf) + 20 ];
	strcpy(tmp,dir);
	strcat(tmp,"/");
	strcat(tmp,prf);
	char *tfch = tmp + strlen(tmp);
	
	for(short unsigned int i=0; i < 0xffff; i++){
		struct stat st;
		sprintf(tfch, "%04x.png", i);
		if(stat(tmp,&st))
			break;
	}

	if(cairo_surface_write_to_png(srf->surface, tmp) != CAIRO_STATUS_SUCCESS){
		lua_pushnil(L);
		lua_pushstring(L,strerror(errno));
		return 2;
	}

	lua_pushstring(L,tmp);
	return 1;
}

/* Object's own functions */
static const struct luaL_Reg SelDCSurfaceLib [] = {
/*	{"create", createsurface}, */
	{NULL, NULL}
};

	/* Type's functions */
static const struct luaL_Reg SelDCSurfaceM [] = {
	{"Release", DCSurfaceRelease},
	{"destroy", DCSurfaceRelease},	/* Alias */
/*	{"GetPosition", SurfaceGetPosition}, */
	{"GetSize", DCSurfaceGetSize},
	{"GetHight", DCSurfaceGetHight},
	{"GetWidth", DCSurfaceGetWidth},
/*	{"GetAfter", SurfaceGetAfter},
	{"GetBelow", SurfaceGetBelow}, */
	{"Clear", DCSurfaceClear},
	{"SetColor", DCSurfaceSetColor},
/*	{"SetDrawingFlags", SurfaceSetDrawingFlags}, */
	{"DrawRectangle", DCSurfaceDrawRectangle},
	{"FillRectangle", DCSurfaceFillRectangle},
/*	{"FillGrandient", SurfaceFillGrandient},
	{"FillTriangle", SurfaceFillTriangle}, */
	{"DrawLine", DCSurfaceDrawLine},
/*	{"DrawCircle", SurfaceDrawCircle},
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
	{"Dump", DCSurfaceDump},
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
