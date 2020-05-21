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
#include <math.h>

#include "DRMCairo.h"

static struct SelDCSurface *checkSelDCSurface(lua_State *L, int where){
	void *r = luaL_checkudata(L, where, "SelDCSurface");
	luaL_argcheck(L, r != NULL, where, "'SelDCSurface' expected");
	return (struct SelDCSurface *)r;
}

static int Release(lua_State *L){
	/* Delete a surface
	 * (i.e. remove all references to its objects 
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_destroy(srf->cr);
	cairo_surface_destroy(srf->surface);
	
	return 0;
}

static int GetSize(lua_State *L){
	/* Return surface's size
	 * <- width, hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->w);
	lua_pushnumber(L, srf->h);
	return 2;
}

static int GetWidth(lua_State *L){
	/* Return surface's size
	 * <- Width
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->w);
	return 2;
}

static int GetHight(lua_State *L){
	/* Return surface's size
	 * <- hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->h);
	return 2;
}

static int Clear(lua_State *L){
	/* Fill a surface with given color
	 * -> r, g, b	(component saturation from 0 to 1)
	 * -> a 		(opacity from 0 to 1)
	 * -> width		(line width, optional, 1 by default)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number r = luaL_checknumber(L, 2);
	lua_Number g = luaL_checknumber(L, 3);
	lua_Number b = luaL_checknumber(L, 4);
	lua_Number a = 1.0;
	if(lua_gettop(L) > 4)
		a = luaL_checknumber(L, 5);

	cairo_save(srf->cr);
	cairo_set_source_rgba(srf->cr, r, g, b, a);
	cairo_set_operator(srf->cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(srf->cr);
	cairo_restore(srf->cr);

	return 0;
}

static int SetColor(lua_State *L){
	/* Set foreground color
	 * -> r, g, b	(component saturation from 0 to 1)
	 * -> a 		(opacity from 0 to 1)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number r = luaL_checknumber(L, 2);
	lua_Number g = luaL_checknumber(L, 3);
	lua_Number b = luaL_checknumber(L, 4);
	lua_Number a = 1.0;
	if(lua_gettop(L) > 4)
		a = luaL_checknumber(L, 5);

	cairo_set_source_rgba (srf->cr, r, g, b, a);
	
	return 0;
}

static int DrawLine(lua_State *L){
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

static int DrawRectangle(lua_State *L){
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

static int FillRectangle(lua_State *L){
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

static int DrawArc(lua_State *L){
	/* Draw an arc
	 * -> x,y : center of the circle
	 * -> radius
	 * -> start angle
	 * -> end angle
	 * -> line width
	 *
	 *  Notez-bien : angles are in radian
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number r = luaL_checknumber(L, 4);
	lua_Number start = luaL_checknumber(L, 5);
	lua_Number end = luaL_checknumber(L, 6);
	lua_Number wdt = 1;

	if(lua_gettop(L) > 7)
		wdt = luaL_checknumber(L, 7);

	cairo_set_line_width(srf->cr, wdt);
	cairo_arc(srf->cr, x,y, r, start, end);
	cairo_stroke(srf->cr);

	return 0;
}

static int FillArc(lua_State *L){
	/* Draw a filled arc
	 * -> x,y : center of the circle
	 * -> radius
	 * -> start angle
	 * -> end angle
	 *
	 *  Notez-bien : angles are in radian
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number r = luaL_checknumber(L, 4);
	lua_Number start = luaL_checknumber(L, 5);
	lua_Number end = luaL_checknumber(L, 6);

	cairo_set_line_width(srf->cr, 1);
	cairo_arc(srf->cr, x,y, r, start, end);
	cairo_move_to(srf->cr, x+ r*cos(start), y+r*sin(start));
	cairo_line_to(srf->cr, x, y);
	cairo_line_to(srf->cr, x+ r*cos(end), y+r*sin(end)); 
	cairo_fill(srf->cr);

	return 0;
}

static int DrawString(lua_State *L){
	/* Draw a string with the current font
	 * -> String
	 * -> x,y : position
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	const char *str = luaL_checkstring(L, 2);
	lua_Number x = luaL_checknumber(L, 3);
	lua_Number y = luaL_checknumber(L, 4);

	cairo_move_to(srf->cr, x, y);
	cairo_show_text(srf->cr, str);

	return 0;
}

static int GetStringExtents(lua_State *L){
	/* Gets the extents for a string of text
	 *	-> string
	 *	<- width, height
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	const char *str = luaL_checkstring(L, 2);
	cairo_text_extents_t ext;

	cairo_text_extents(srf->cr, str, &ext);

	lua_pushnumber(L, ext.width);
	lua_pushnumber(L, ext.height);

	return 2;
}

static int SetFont(lua_State *L){
	/* Set Font to use
	 * -> font : SelDCFont to use
	 * -> size
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	struct selDCFont *font = checkSelDCFont(L, 2);
	lua_Number sz = luaL_checknumber(L, 3);

	cairo_set_font_face(srf->cr, font->cairo);
	cairo_set_font_size(srf->cr, sz);

	return 0;
}

static int Dump(lua_State *L){
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
static const struct luaL_Reg SelLib [] = {
/*	{"create", createsurface}, */
	{NULL, NULL}
};

	/* Type's functions */
static const struct luaL_Reg SelM [] = {
	{"Release", Release},
	{"destroy", Release},	/* Alias */
/*	{"GetPosition", SurfaceGetPosition}, */
	{"GetSize", GetSize},
	{"GetHight", GetHight},
	{"GetWidth", GetWidth},
/*	{"GetAfter", SurfaceGetAfter},
	{"GetBelow", SurfaceGetBelow}, */
	{"Clear", Clear},
	{"SetColor", SetColor},
/*	{"SetDrawingFlags", SurfaceSetDrawingFlags}, */
	{"DrawRectangle", DrawRectangle},
	{"FillRectangle", FillRectangle},
/*	{"FillGrandient", SurfaceFillGrandient},
	{"FillTriangle", SurfaceFillTriangle}, */
	{"DrawLine", DrawLine},
	{"DrawArc", DrawArc},
	{"FillArc", FillArc},
	{"DrawString", DrawString},
	{"GetStringExtents", GetStringExtents},
/*	{"SetBlittingFlags", SurfaceSetBlittingFlags},
	{"SetRenderOptions", SurfaceSetRenderOptions},
	{"Blit", SurfaceBlit},
	{"TileBlit", SurfaceTileBlit},
	{"TileBlitClip", SurfaceTileBlitClip},
	{"StretchBlit", SurfaceStretchBlit},
	{"SetClip", SurfaceSetClip},
	{"SetClipS", SurfaceSetClipS}, */
	{"SetFont", SetFont},
/*	{"GetFont", SurfaceGetFont},
	{"SubSurface", SurfaceSubSurface},
	{"GetSubSurface", SurfaceSubSurface},
	{"GetPixelFormat", SurfaceGetPixelFormat},
	{"Flip", SurfaceFlip}, */
	{"Dump", Dump},
/*	{"clone", SurfaceClone},
	{"restore", SurfaceRestore},
*/
	{NULL, NULL}
};

void _include_SelDCSurface( lua_State *L ){
	libSel_objFuncs( L, "SelDCSurface", SelM );
	libSel_libFuncs( L, "SelDCSurface", SelLib );
}


#endif
