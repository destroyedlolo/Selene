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
#include <assert.h>

#include "DRMCairo.h"

static struct SelDCSurface *checkSelDCSurface(lua_State *L, int where){
	void *r = luaL_checkudata(L, where, "SelDCSurface");
	luaL_argcheck(L, r != NULL, where, "'SelDCSurface' expected");
	return (struct SelDCSurface *)r;
}

void internal_release_surface(struct SelDCSurface *srf){
	/* As it is needed also to cleanup if a new surface allocation failed
	 *	-> srf : the surface to delete
	 */
	cairo_destroy(srf->cr);
	cairo_surface_destroy(srf->surface);
}

static int Release(lua_State *L){
	/* Delete a surface
	 * (i.e. remove all references to its objects)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	internal_release_surface(srf);

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
	return 1;
}

static int GetHight(lua_State *L){
	/* Return surface's size
	 * <- hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->h);
	return 1;
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

static int SetSourcePattern(lua_State *L){
	/* Set source the given pattern
	 * <- pattern
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	cairo_pattern_t *pat = checkSelDCPattern(L, 2);

	cairo_set_source(srf->cr, pat);

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

static int SetPenWidth(lua_State *L){
	/* Set line width when drawing
	 * -> width
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number w = luaL_checknumber(L, 2);

	cairo_set_line_width(srf->cr, w);

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
	lua_Number w = -1;

	if(lua_gettop(L) > 5)
		w = luaL_checknumber(L, 6);

	cairo_move_to(srf->cr, x1, y1);
	cairo_line_to(srf->cr, x2, y2);
	if( w > 0 )
		cairo_set_line_width(srf->cr, w);
	cairo_stroke(srf->cr);

	return 0;
}

static int DrawRectangle(lua_State *L){
	/* Draw a rectangle
	 * -> x1,y1 : left top corner
	 * -> w,h :	size
	 * 	-> width (optional)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x1 = luaL_checknumber(L, 2);
	lua_Number y1 = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);
	lua_Number wdt = -1;

	if(lua_gettop(L) > 5)
		wdt = luaL_checknumber(L, 6);

	cairo_rectangle(srf->cr, x1, y1, w, h);
	if( wdt > 0 )
		cairo_set_line_width(srf->cr, wdt);
	cairo_stroke(srf->cr);

	return 0;
}

static int FillRectangle(lua_State *L){
	/* Draw a filled rectangle
	 * -> x1,y1 : left top corner
	 * -> w,h :	size
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x1 = luaL_checknumber(L, 2);
	lua_Number y1 = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);

	cairo_rectangle(srf->cr, x1, y1, w, h);
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

static int Blit(lua_State *L){
	/* Blit another surface to the current one
	 * -> source : source surface
	 * -> x,y : target position
	 * -> xs,ys : position in the source (optional, 0,0 by default)
	 * -> w,h : size to blit (optional, source size by default)
	 *
	 *  Notez-bien :
	 *  - Alpha channel is considered during bliting
	 */
	
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	struct SelDCSurface *src = checkSelDCSurface(L, 2);
	lua_Number x = luaL_checknumber(L, 3);
	lua_Number y = luaL_checknumber(L, 4);
	lua_Number x_src = 0, y_src = 0;
	lua_Number w = src->w, h = src->h;

	if(lua_gettop(L) > 4){
		x_src = luaL_checknumber(L, 5);
		y_src = luaL_checknumber(L, 6);
	}

	if(lua_gettop(L) > 6){
		w = luaL_checknumber(L, 7);
		h = luaL_checknumber(L, 8);
	}

	cairo_save(srf->cr);
	cairo_set_source_surface(srf->cr, src->surface, x - x_src, y - y_src);
	cairo_rectangle (srf->cr, x, y, w, h);
	cairo_fill (srf->cr);
	cairo_restore(srf->cr);

	return 0;
}

static int SaveContext(lua_State *L){
	/* Push cairo for latter use
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_save(srf->cr);

	return 0;
}

static int RestoreContext(lua_State *L){
	/* Restore previously saved cairo context
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_restore(srf->cr);

	return 0;
}

static int SubSurface(lua_State *L){
	/* Create a SubSurface from this surface.
	 * It's only a portion of the mother surface on which drawing are clipped
	 * and origin is translated to its top-left corner.
	 * No addition buffer is allocated and all graphical manipulation are
	 * directly impacting the mother surface.
	 * 	-> x,y : place
	 *  -> width, height : geometry of the rectangular area
	 */

	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);

	struct SelDCSurface *ssrf = (struct SelDCSurface *)lua_newuserdata(L, sizeof(struct SelDCSurface));
	assert(ssrf);
	luaL_getmetatable(L, "SelDCSurface");
	lua_setmetatable(L, -2);

	ssrf->surface = cairo_surface_create_for_rectangle(srf->surface, x,y, w,h);
	ssrf->cr = cairo_create(ssrf->surface);
	ssrf->w = w;
	ssrf->h = h;
	ssrf->type = DCSURFACE_SUBSURFACE;

	if(cairo_status(ssrf->cr) != CAIRO_STATUS_SUCCESS){
		internal_release_surface(ssrf);
		lua_pop(L,1);	/* Remove the newly create surface object */
		lua_pushnil(L);
		lua_pushstring(L, "Unable to create Cairo's surface");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's surface\n");
#endif
		return 2;
	}

	return 1;
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

static int SetClip(lua_State *L){
	/* Restrict drawing to a rectangular area
	 * -> x,y : top left corner
	 * -> x2,y2 : bottom right corner
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number x2 = luaL_checknumber(L, 4);
	lua_Number y2 = luaL_checknumber(L, 5);

	cairo_rectangle(srf->cr, x,y, x2-x, y2-y);
	cairo_clip(srf->cr);

	return 0;
}

static int SetClipS(lua_State *L){
	/* Restrict drawing to a rectangular area
	 * -> x,y : left top corner
	 * -> w,h :	size
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);

	cairo_rectangle(srf->cr, x,y, w, h);
	cairo_clip(srf->cr);

	return 0;
}

static int ResetClip(lua_State *L){
	/* Remove clipping restriction : drawing can be done everywhere on
	 * the surface.
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_reset_clip(srf->cr);

	return 0;
}

#ifdef CAIRO_HAS_PNG_FUNCTIONS
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
#endif

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
	{"SetSourcePattern", SetSourcePattern},
	{"SetColor", SetColor},
	{"SetPenWidth", SetPenWidth},
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
	{"SetRenderOptions", SurfaceSetRenderOptions}, */
	{"Blit", Blit},
/*	{"TileBlit", SurfaceTileBlit},
	{"TileBlitClip", SurfaceTileBlitClip},
	{"StretchBlit", SurfaceStretchBlit}, */
	{"SetClip", SetClip},
	{"SetClipS", SetClipS},
	{"ResetClip", ResetClip},
	{"SetFont", SetFont},
/*	{"GetFont", SurfaceGetFont}, */
	{"SubSurface", SubSurface},
	{"SaveContext", SaveContext},
	{"RestoreContext", RestoreContext},
/*	{"GetPixelFormat", SurfaceGetPixelFormat},
	{"Flip", SurfaceFlip}, */
#ifdef CAIRO_HAS_PNG_FUNCTIONS
	{"Dump", Dump},
#endif
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
