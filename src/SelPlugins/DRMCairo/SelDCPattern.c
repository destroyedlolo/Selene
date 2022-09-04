/***
 * Cairo patterns

@classmod SelDCPattern

 *
 * 23/05/2020 LF : Creation
 */

#ifdef USE_DRMCAIRO

#include <assert.h>

#include "DRMCairo.h"

cairo_pattern_t *checkSelDCPattern(lua_State *L, int where){
	void **r = luaL_checkudata(L, where, "SelDCPattern");
	luaL_argcheck(L, r != NULL, where, "'SelDCPattern' expected");
	return (cairo_pattern_t *)*r;
}

static int createLinear(lua_State *L){
	/** Create linear gradient
	 *
	 * @function createLinear
	 * @tparam Number x1 coordinate of the start point
	 * @tparam Number y1 coordinate of the start point
	 * @tparam Number x2 coordinate of the end point
	 * @tparam Number y2 coordinate of the end point
	 *
	 * @treturn SelDCPattern pattern
	 */
	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	cairo_status_t err;

	cairo_pattern_t **ppat = (cairo_pattern_t **)lua_newuserdata(L, sizeof(cairo_pattern_t *));
	assert(ppat);
	luaL_getmetatable(L, "SelDCPattern");
	lua_setmetatable(L, -2);

	*ppat = cairo_pattern_create_linear(x1,y1, x2,y2);
	if( (err=cairo_pattern_status(*ppat)) != CAIRO_STATUS_SUCCESS){
		cairo_pattern_destroy(*ppat);
		lua_pop(L,1);	/* Remove the newly create pattern object */
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, "Unable to create Cairo's pattern");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's pattern\n");
#endif
		return 3;
	}

	return 1;
}

static int createCircle(lua_State *L){
	/** Create radial gradient
	 *
	 * @function createCircle
	 * @tparam Number cx0 center of the 1st circle
	 * @tparam Number cy0 center of the 1st circle
	 * @tparam Number r0 radius of the 1st circle
	 * @tparam Number cx1 center of the 2nd circle
	 * @tparam Number cy1 center of the 2nd circle
	 * @tparam Number r1 radius of the 2nd circle
	 */
	lua_Number cx0 = luaL_checknumber(L, 1);
	lua_Number cy0 = luaL_checknumber(L, 2);
	lua_Number r0 = luaL_checknumber(L, 3);
	lua_Number cx1 = luaL_checknumber(L, 4);
	lua_Number cy1 = luaL_checknumber(L, 5);
	lua_Number r1 = luaL_checknumber(L, 6);
	cairo_status_t err;

	cairo_pattern_t **ppat = (cairo_pattern_t **)lua_newuserdata(L, sizeof(cairo_pattern_t *));
	assert(ppat);
	luaL_getmetatable(L, "SelDCPattern");
	lua_setmetatable(L, -2);

	*ppat = cairo_pattern_create_radial(cx0,cy0,r0, cx1,cy1,r1);
	if( (err=cairo_pattern_status(*ppat)) != CAIRO_STATUS_SUCCESS){
		cairo_pattern_destroy(*ppat);
		lua_pop(L,1);	/* Remove the newly create pattern object */
		lua_pushnil(L);
		lua_pushstring(L, cairo_status_to_string(err));
		lua_pushstring(L, "Unable to create Cairo's pattern");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's pattern\n");
#endif
		return 3;
	}

	return 1;
}

static int Release(lua_State *L){
	/** Delete a pattern
	 *
	 * Remove all references to its objects.
	 *
	 * @function Release
	 */
	/** Delete a pattern
	 *
	 * Remove all references to its objects.
	 *
	 * @function destroy
	 */
	cairo_pattern_t *pat = checkSelDCPattern(L, 1);

	cairo_pattern_destroy(pat);

	return 0;
}

static int addFixPoint(lua_State *L){
	/** Adds a translucent color stop to a gradient pattern
	 *
	 * @function addFixPoint
	 * @tparam Number offset where to put this point (b/w 0.0 and 1.0)
	 * @tparam Number r colors component
	 * @tparam Number g colors component
	 * @tparam Number b colors component
	 * @tparam Number opacity optional, default 1
	 */
	cairo_pattern_t *pat = checkSelDCPattern(L, 1);
	lua_Number offset = luaL_checknumber(L, 2);
	lua_Number r = luaL_checknumber(L, 3);
	lua_Number g = luaL_checknumber(L, 4);
	lua_Number b = luaL_checknumber(L, 5);
	lua_Number a = 1.0;
	if(lua_gettop(L) > 5)
		a = luaL_checknumber(L, 6);

	cairo_pattern_add_color_stop_rgba(pat, offset, r,g,b,a);

	return 0;
}

/* Object's own functions */
static const struct luaL_Reg SelLib [] = {
	{"createLinear", createLinear},
	{"createCircle", createCircle},
	{NULL, NULL}
};

	/* Type's functions */
static const struct luaL_Reg SelM [] = {
	{"Release", Release},
	{"destroy", Release},	/* Alias */
	{"addFixPoint", addFixPoint},
	{NULL, NULL}
};

void _include_SelDCPattern( lua_State *L ){
	libSel_objFuncs( L, "SelDCPattern", SelM );
	libSel_libFuncs( L, "SelDCPattern", SelLib );
}

#endif
