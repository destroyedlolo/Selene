/* SelDCPattern
 *
 * This file contains all stuffs related to Cairo patterns
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

static int Release(lua_State *L){
	/* Delete a pattern
	 * (i.e. remove all references to its objects)
	 */
	cairo_pattern_t *pat = checkSelDCPattern(L, 1);

	cairo_pattern_destroy(pat);

	return 0;
}

static int addFixPoint(lua_State *L){
	/* Add a new point to fix point
	 *	-> offset where to put this point (b/w 0.0 and 1.0)
	 *	-> r,g,b 
	 *	-> a opacity (optional, default 1)
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

static int createLinear(lua_State *L){
	/* Create linear gradient
	 *	-> x1,y1 : coordinate of the start point
	 *	-> x2,y2 : coordinate of the end point
	 *	<- new SelDCPattern object
	 */
	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);

	cairo_pattern_t **ppat = (cairo_pattern_t **)lua_newuserdata(L, sizeof(cairo_pattern_t *));
	assert(ppat);
	luaL_getmetatable(L, "SelDCPattern");
	lua_setmetatable(L, -2);

	*ppat = cairo_pattern_create_linear(x1,y1, x2,y2);
	if(cairo_pattern_status(*ppat) != CAIRO_STATUS_SUCCESS){
		cairo_pattern_destroy(*ppat);
		lua_pop(L,1);	/* Remove the newly create pattern object */
		lua_pushnil(L);
		lua_pushstring(L, "Unable to create Cairo's pattern");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's pattern\n");
#endif
		return 2;
	}

	return 1;
}

static int createCircle(lua_State *L){
	/* Create circular gradient
	 * cx0,cy0 : center of the 1st circle
	 * r0 : radius of the 1st circle
	 * cx1,cy1 : center of the 1st circle
	 * r1 : radius of the 1st circle
	 */
	lua_Number cx0 = luaL_checknumber(L, 1);
	lua_Number cy0 = luaL_checknumber(L, 2);
	lua_Number r0 = luaL_checknumber(L, 3);
	lua_Number cx1 = luaL_checknumber(L, 4);
	lua_Number cy1 = luaL_checknumber(L, 5);
	lua_Number r1 = luaL_checknumber(L, 6);

	cairo_pattern_t **ppat = (cairo_pattern_t **)lua_newuserdata(L, sizeof(cairo_pattern_t *));
	assert(ppat);
	luaL_getmetatable(L, "SelDCPattern");
	lua_setmetatable(L, -2);

	*ppat = cairo_pattern_create_radial(cx0,cy0,r0, cx1,cy1,r1);
	if(cairo_pattern_status(*ppat) != CAIRO_STATUS_SUCCESS){
		cairo_pattern_destroy(*ppat);
		lua_pop(L,1);	/* Remove the newly create pattern object */
		lua_pushnil(L);
		lua_pushstring(L, "Unable to create Cairo's pattern");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's pattern\n");
#endif
		return 2;
	}

	return 1;
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
