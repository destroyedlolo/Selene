/**
 * all stuffs related to Cairo surfaces and especially graphics verbs
 *

@classmod SelDCSurface

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

	/* See https://www.cairographics.org/operators/ */
static const struct ConstTranscode _OpConst[] = {
	{ "CLEAR", CAIRO_OPERATOR_CLEAR },
	{ "SOURCE", CAIRO_OPERATOR_SOURCE },
	{ "OVER", CAIRO_OPERATOR_OVER },
	{ "IN", CAIRO_OPERATOR_IN },
	{ "OUT", CAIRO_OPERATOR_OUT },
	{ "ATOP", CAIRO_OPERATOR_ATOP },
	{ "DEST", CAIRO_OPERATOR_DEST },
	{ "DEST_OVER", CAIRO_OPERATOR_DEST_OVER },
	{ "DEST_IN", CAIRO_OPERATOR_DEST_IN },
	{ "DEST_OUT", CAIRO_OPERATOR_DEST_OUT },
	{ "DEST_ATOP", CAIRO_OPERATOR_DEST_ATOP },
	{ "XOR", CAIRO_OPERATOR_XOR },
	{ "ADD", CAIRO_OPERATOR_ADD },
	{ "SATURATE", CAIRO_OPERATOR_SATURATE },
	{ "MULTIPLY", CAIRO_OPERATOR_MULTIPLY },
	{ "SCREEN", CAIRO_OPERATOR_SCREEN },
	{ "OVERLAY", CAIRO_OPERATOR_OVERLAY },
	{ "DARKEN", CAIRO_OPERATOR_DARKEN },
	{ "LIGHTEN", CAIRO_OPERATOR_LIGHTEN },
#ifdef CAIRO_OPERATOR_DODGE		/* Not defined in my own cairo ???? */
	{ "DODGE", CAIRO_OPERATOR_DODGE }, 
	{ "BURN", CAIRO_OPERATOR_BURN },
#endif
	{ "HARD_LIGHT", CAIRO_OPERATOR_HARD_LIGHT },
	{ "SOFT_LIGHT", CAIRO_OPERATOR_SOFT_LIGHT },
	{ "DIFFERENCE", CAIRO_OPERATOR_DIFFERENCE },
	{ "EXCLUSION", CAIRO_OPERATOR_EXCLUSION },
	{ "HSL_HUE", CAIRO_OPERATOR_HSL_HUE },
	{ "HSL_COLOR", CAIRO_OPERATOR_HSL_COLOR },
	{ "HSL_LUMINOSITY", CAIRO_OPERATOR_HSL_LUMINOSITY },
	{ NULL, 0 }
};

int OperatorConst( lua_State *L ){
	/** transcode operators capabilities
	 *
	 * @function OperatorConst
	 * @tparam string operator
	 * @treturn int value
	 */
	return findConst(L, _OpConst);
}

static int create(lua_State *L){
	/** Create a surface for drawing
	 *
	 * @function create
	 * @tparam integer width
	 * @tparam integer height
	 */
	cairo_status_t err;
	lua_Number width = luaL_checknumber(L, 1);
	lua_Number height = luaL_checknumber(L, 2);
	struct SelDCSurface *srf = (struct SelDCSurface *)lua_newuserdata(L, sizeof(struct SelDCSurface));
	assert(srf);
	luaL_getmetatable(L, "SelDCSurface");
	lua_setmetatable(L, -2);

	srf->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	srf->cr = cairo_create(srf->surface);
	srf->type = DCSURFACE;

	if( (err=cairo_status(srf->cr)) != CAIRO_STATUS_SUCCESS){
		cairo_destroy(srf->cr);
		internal_release_surface(srf);
		lua_pop(L,1);	/* Remove the newly create surface object */
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, "create() failed");
#ifdef DEBUG
		puts("*E* create()\n");
#endif
		return 3;
	}

	srf->w = cairo_image_surface_get_width(srf->surface);
	srf->h = cairo_image_surface_get_height(srf->surface);

	return 1;
}

static int SubSurface(lua_State *L){
	/** Create a SubSurface from this surface.
	 *
	 * It's only a portion of the mother surface on which drawing are clipped
	 * and origin is translated to its top-left corner.
	 *
	 * No addition buffer is allocated and all graphical manipulation are
	 * directly impacting the mother surface.
	 *
	 *  @function SubSurface
	 * @tparam Number x left top corner
	 * @tparam Number y left top corner
	 * @tparam Number width
	 * @tparam Number hight
	 * @treturn SelDCSurface surface
	 */

	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);
	cairo_status_t err;

	struct SelDCSurface *ssrf = (struct SelDCSurface *)lua_newuserdata(L, sizeof(struct SelDCSurface));
	assert(ssrf);
	luaL_getmetatable(L, "SelDCSurface");
	lua_setmetatable(L, -2);

	ssrf->surface = cairo_surface_create_for_rectangle(srf->surface, x,y, w,h);
	ssrf->cr = cairo_create(ssrf->surface);
	ssrf->w = w;
	ssrf->h = h;
	ssrf->type = DCSURFACE_SUBSURFACE;

	if( (err=cairo_status(ssrf->cr)) != CAIRO_STATUS_SUCCESS){
		internal_release_surface(ssrf);
		lua_pop(L,1);	/* Remove the newly create surface object */
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, "Unable to create Cairo's surface");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's surface\n");
#endif
		return 3;
	}

	return 1;
}

void internal_release_surface(struct SelDCSurface *srf){
	/* As it is needed also to cleanup if a new surface allocation failed
	 *	-> srf : the surface to delete
	 */
	cairo_destroy(srf->cr);
	cairo_surface_destroy(srf->surface);
}

static int Release(lua_State *L){
	/** Delete a surface
	 *
	 * Remove all references to its objects.
	 *
	 * @function Release
	 */
	/** Delete a surface
	 *
	 * Remove all references to its objects.
	 *
	 * @function destroy
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	internal_release_surface(srf);

	return 0;
}

static int GetSize(lua_State *L){
	/** Return surface's size
	 *
	 * @function GetSize
	 * @treturn integer width
	 * @treturn integer hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->w);
	lua_pushnumber(L, srf->h);
	return 2;
}

static int GetWidth(lua_State *L){
	/** Return surface's width
	 *
	 * @function GetWidth
	 * @treturn integer width
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->w);
	return 1;
}

static int GetHight(lua_State *L){
	/** Return surface's hight
	 *
	 * @function GetHight
	 * @treturn integer hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushnumber(L, srf->h);
	return 1;
}

static int SaveContext(lua_State *L){
	/** Push cairo context for latter use.
	 *
	 * Useful to modify temporary drawing context
	 *
	 *  @function SaveContext
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_save(srf->cr);

	return 0;
}

static int RestoreContext(lua_State *L){
	/** Restore previously saved cairo context
	 *
	 *  @function RestoreContext
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_restore(srf->cr);

	return 0;
}

static int SetSourcePattern(lua_State *L){
	/** Set source the given pattern
	 *
	 * @function SetSourcePattern
	 * @tparam SelDCPattern pattern
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	cairo_pattern_t *pat = checkSelDCPattern(L, 2);

	cairo_set_source(srf->cr, pat);

	return 0;
}

static int SetSourceSurface(lua_State *L){
	/** Set source the given surface
	 *
	 * @function SetSourceSurface
	 * @tparam SelDCSurface surface
	 * @tparam Number x surface origin (default 0)
	 * @tparam Number y surface origin (default 0)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	struct SelDCSurface *ssrf = checkSelDCSurface(L, 2);
	lua_Number x = 0;
	lua_Number y = 0;
	if(lua_gettop(L) > 2){
		x = luaL_checknumber(L, 3);
		y = luaL_checknumber(L, 4);
	}

	cairo_set_source_surface(srf->cr, ssrf->surface, x,y);

	return 0;
}

static int SetColor(lua_State *L){
	/** Set foreground color
	 *
	 * @function SetColor
	 * @tparam Number r component saturation from 0 to 1
	 * @tparam Number g component saturation from 0 to 1
	 * @tparam Number b component saturation from 0 to 1
	 * @tparam Number opacity component saturation from 0 to 1
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
	/** Set line width when drawing
	 *
	 * @function SetPenWidth
	 * @tparam Number width
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number w = luaL_checknumber(L, 2);

	cairo_set_line_width(srf->cr, w);

	return 0;
}

static int SetOperator(lua_State *L){
	/** Sets the compositing operator to be used for all drawing operations
	 *
	 * @function SetOperator
	 * @tparam integer mode
	 * @see OperatorConst
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	 cairo_operator_t op = luaL_checkinteger(L, 2);

	cairo_set_operator (srf->cr, op);
	
	return 0;
}

static int Scale(lua_State *L){
	/** Scale the surface
	 *
	 *  @function Scale
	 * @tparam Number x scale factor
	 * @tparam Number y scale factor
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number sx = luaL_checknumber(L, 2);
	lua_Number sy = luaL_checknumber(L, 3);

	cairo_scale(srf->cr, sx, sy);

	return 0;
}

static int SetClip(lua_State *L){
	/** Restrict drawing to a rectangular area
	 * 
	 * @function SetClip
	 * @tparam Number x1 left top corner
	 * @tparam Number y1 left top corner
	 * @tparam Number x2 bottom right corner
	 * @tparam Number y2 bottom right corner
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
	/** Restrict drawing to a rectangular area
	 * 
	 * @function SetClipS
	 * @tparam Number x left top corner
	 * @tparam Number y left top corner
	 * @tparam Number width
	 * @tparam Number hight
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
	/** Remove clipping restriction : drawing can be done everywhere on
	 * the surface.
	 *
	 * @function ResetClip
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_reset_clip(srf->cr);

	return 0;
}

static int Clear(lua_State *L){
	/** Fill a surface with given color
	 *
	 * @function Clear
	 * @tparam Number r component saturation from 0 to 1
	 * @tparam Number g component saturation from 0 to 1
	 * @tparam Number b component saturation from 0 to 1
	 * @tparam Number opacity component saturation from 0 to 1
	 * @tparam Number width line width (optional, 1 by default)
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

static int DrawLine(lua_State *L){
	/** Draw a line
	 *
	 * @function DrawLine
	 * @tparam Number x1 start point
	 * @tparam Number y1 start point
	 * @tparam Number x2 end point
	 * @tparam Number y2 end point
	 * @tparam Number width line width (optional, 1 by default)
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
	/** Draw a rectangle
	 *
	 * @function DrawRectangle
	 * @tparam Number x left top corner
	 * @tparam Number y left top corner
	 * @tparam Number width
	 * @tparam Number hight
	 * @tparam Number width line width (optional, 1 by default)
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
	/** Draw a filled rectangle
	 *
	 * @function FillRectangle
	 * @tparam Number x1 left top corner
	 * @tparam Number y1 left top corner
	 * @tparam Number width
	 * @tparam Number hight
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

static int SurfaceFillTriangle(lua_State *L){
	/** Draw a filled triangle
	 *
	 * @function FillTriangle
	 * @tparam Number x1 top #1
	 * @tparam Number y1 top #1
	 * @tparam Number x2 top #2
	 * @tparam Number y2 top #2
	 * @tparam Number x3 top #3
	 * @tparam Number y3 top #3
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x1 = luaL_checknumber(L, 2);
	lua_Number y1 = luaL_checknumber(L, 3);
	lua_Number x2 = luaL_checknumber(L, 4);
	lua_Number y2 = luaL_checknumber(L, 5);
	lua_Number x3 = luaL_checknumber(L, 6);
	lua_Number y3 = luaL_checknumber(L, 7);

	cairo_move_to(srf->cr, x1,y1);
	cairo_line_to(srf->cr, x2,y2);
	cairo_line_to(srf->cr, x3,y3);
	cairo_close_path(srf->cr);
	cairo_fill(srf->cr);

	return 0;
}

static int DrawArc(lua_State *L){
	/** Draw an arc.
	 *
	 *  *Notez-bien :* angles are in **radian**
	 *
	 * @function DrawArc
	 * @tparam Number x center of the circle
	 * @tparam Number y center of the circle
	 * @tparam Number radius
	 * @tparam Number start_angle
	 * @tparam Number end_angle
	 * @tparam Number width line width (optional, 1 by default)
	 * @tparam boolean safe if true, ensure end > start (not for development)
	 *
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number r = luaL_checknumber(L, 4);
	lua_Number start = luaL_checknumber(L, 5);
	lua_Number end = luaL_checknumber(L, 6);

	lua_Number wdt = 1;

	if(lua_gettop(L) > 6)
		wdt = luaL_checknumber(L, 7);

	if(lua_isboolean(L, 8))
		if(lua_toboolean(L, 8) && start > end)
			end = start;

	cairo_set_line_width(srf->cr, wdt);
	cairo_arc(srf->cr, x,y, r, start, end);
	cairo_stroke(srf->cr);

	return 0;
}

static int FillArc(lua_State *L){
	/** Draw a filled arc.
	 *
	 *  *Notez-bien :* angles are in **radian**
	 *
	 * @function FillArc
	 * @tparam Number x center of the circle
	 * @tparam Number y center of the circle
	 * @tparam Number radius
	 * @tparam Number start_angle
	 * @tparam Number end_angle
	 * @tparam Number width line width (optional, 1 by default)
	 * @tparam boolean safe if true, ensure end > start (not for development)
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

static int Paint(lua_State *L){
	/** drawing operator that paints the current source everywhere within
	 * the current clip region
	 *
	 * @function Paint
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	cairo_paint(srf->cr);

	return 0;
}

static int Blit(lua_State *L){
	/** Blit another surface to the current one.
	 *
	 *  *Notez-bien :*  Alpha channel is considered during bliting
	 *
	 * @function Blit
	 * @tparam SelDCSurface source source surface
	 * @tparam Number x target position
	 * @tparam Number y target position
	 * @tparam Number xs source position (default 0)
	 * @tparam Number ys source position (default 0)
	 * @tparam integer width
	 * @tparam integer height
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

static int SetFont(lua_State *L){
	/** Set Font to use
	 *
	 *  @function SetFont
	 *  @tparam SelDCFont font
	 *  @tparam Number size
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	struct selDCFont *font = checkSelDCFont(L, 2);
	lua_Number sz = luaL_checknumber(L, 3);

	cairo_set_font_face(srf->cr, font->cairo);
	cairo_set_font_size(srf->cr, sz);

	return 0;
}

static int DrawString(lua_State *L){
	/** Draw a string with the current font.
	 *
	 * The position is based on it's baseline.
	 *
	 * If the position is not provided, the string is placed at the
	 * current drawing position which may be just after the current line.
	 *
	 * @function DrawString
	 * @tparam string text
	 * @tparam Number x string position (optional)
	 * @tparam Number y string position (optional)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	const char *str = luaL_checkstring(L, 2);
	if(lua_gettop(L) > 2){
		lua_Number x = luaL_checknumber(L, 3);
		lua_Number y = luaL_checknumber(L, 4);

		cairo_move_to(srf->cr, x, y);
	}

	cairo_show_text(srf->cr, str);

	return 0;
}

static int DrawStringTop(lua_State *L){
	/** Draw a string with the current font.
	 *
	 * The position is the top-left of the resulting drawing.
	 *
	 * @function DrawStringTop
	 * @tparam string text
	 * @tparam Number x string position
	 * @tparam Number y string position
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	const char *str = luaL_checkstring(L, 2);
	lua_Number x = luaL_checknumber(L, 3);
	lua_Number y = luaL_checknumber(L, 4);
	cairo_font_extents_t extents;

	cairo_font_extents(srf->cr, &extents);
	cairo_move_to(srf->cr, x, y + extents.ascent);
	cairo_show_text(srf->cr, str);

	return 0;
}

static int GetFontExtents(lua_State *L){
	/** Return current font extents
	 *
	 * @function GetFontExtents
	 * @treturn Number height recommended vertical distance between baselines
	 * @treturn Number ascent the distance that the font extends above the baseline
	 * @treturn Number descent the distance that the font extends below the baseline
	 * @treturn Number max_x_advance the maximum space advance in X direction
	 * @treturn Number max_y_advance the maximum space advance in Y direction (for vertical one)
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	cairo_font_extents_t ext;

	cairo_font_extents(srf->cr, &ext);

	lua_pushnumber(L, ext.height);
	lua_pushnumber(L, ext.ascent);
	lua_pushnumber(L, ext.descent);
	lua_pushnumber(L, ext.max_x_advance);
	lua_pushnumber(L, ext.max_y_advance);

	return 5;
}

static int GetStringExtents(lua_State *L){
	/** Gets the extents for a string of text
	 *
	 * @function GetStringExtents
	 * @tparam string text
	 * @tparam Number width
	 * @tparam Number hight
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);
	const char *str = luaL_checkstring(L, 2);
	cairo_text_extents_t ext;

	cairo_text_extents(srf->cr, str, &ext);

	lua_pushnumber(L, ext.width);
	lua_pushnumber(L, ext.height);

	return 2;
}

static int getStatus(lua_State *L){
	/** return Cairo's status.
	 *
	 * *Notez-bien :* **0**, the surface is valid
	 *
	 * @function getStatus
	 * @treturn string surface_status surface own status
	 * @treturn string cairo_status cairo's context status
	 *
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	lua_pushstring(L, cairo_status_to_string(cairo_surface_status(srf->surface)) );
	lua_pushstring(L, cairo_status_to_string(cairo_status(srf->cr)) );
	
	return 2;
}

#ifdef CAIRO_HAS_PNG_FUNCTIONS
static int Dump(lua_State *L){
	/** Save the surface as a PNG file 
	 *
	 * @function Dump
	 * @tparam string directory where to save the file
	 * @tparam string prefix file's prefix
	 * @treturn ?string|nil result file written (nil in case of error)
	 * @treturn ?nil|string error error message if any
	 */
	struct SelDCSurface *srf = checkSelDCSurface(L, 1);

	const char *dir= luaL_checkstring(L, 2);
	const char *prf= luaL_checkstring(L, 3);
	cairo_status_t err;

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

	if((err = cairo_surface_write_to_png(srf->surface, tmp)) != CAIRO_STATUS_SUCCESS){
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, strerror(errno));
		return 3;
	}

	lua_pushstring(L,tmp);
	return 1;
}
#endif

/* Object's own functions */
static const struct luaL_Reg SelLib [] = {
	{"create", create},
	{"OperatorConst", OperatorConst},
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
	{"SetSourceSurface", SetSourceSurface},
	{"SetColor", SetColor},
	{"SetPenWidth", SetPenWidth},
/*	{"SetDrawingFlags", SurfaceSetDrawingFlags}, */
	{"DrawRectangle", DrawRectangle},
	{"FillRectangle", FillRectangle},
/*	{"FillGrandient", SurfaceFillGrandient}, */
	{"FillTriangle", SurfaceFillTriangle},
	{"DrawLine", DrawLine},
	{"DrawArc", DrawArc},
	{"FillArc", FillArc},
/*	{"SetBlittingFlags", SurfaceSetBlittingFlags}, */
	{"SetOperator", SetOperator}, 
	{"SetRenderOptions", SetOperator}, /* Alias */
	{"Blit", Blit},
/*	{"TileBlit", SurfaceTileBlit},
	{"TileBlitClip", SurfaceTileBlitClip},
	{"StretchBlit", SurfaceStretchBlit}, */
	{"SetClip", SetClip},
	{"SetClipS", SetClipS},
	{"ResetClip", ResetClip},
	{"SetFont", SetFont},
/*	{"GetFont", SurfaceGetFont}, */
	{"DrawString", DrawString},
	{"DrawStringTop", DrawStringTop},
	{"GetFontExtents", GetFontExtents},
	{"GetStringExtents", GetStringExtents},
	{"SubSurface", SubSurface},
	{"Scale", Scale},
	{"Paint", Paint},
	{"SaveContext", SaveContext},
	{"RestoreContext", RestoreContext},
/*	{"GetPixelFormat", SurfaceGetPixelFormat},
	{"Flip", SurfaceFlip}, */
#ifdef CAIRO_HAS_PNG_FUNCTIONS
	{"Dump", Dump},
#endif
	{"getStatus", getStatus},
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
