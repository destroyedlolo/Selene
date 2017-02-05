/* dfb_surface.c
 *
 * This file contains all stuffs related to DirectFB's surface.
 *
 * 24/04/2015 LF : First version (from a split of original directfb.c)
 */
#include "directfb.h"

#ifdef USE_DIRECTFB
#include <assert.h>

static const struct ConstTranscode _Capability[] = {
	{ "NONE", DSCAPS_NONE },
	{ "PRIMARY", DSCAPS_PRIMARY },
	{ "SYSTEMONLY", DSCAPS_SYSTEMONLY },
	{ "VIDEOONLY", DSCAPS_VIDEOONLY },
	{ "DOUBLE", DSCAPS_DOUBLE },
	{ "SUBSURFACE", DSCAPS_SUBSURFACE },
	{ "INTERLACED", DSCAPS_INTERLACED },
	{ "SEPARATED", DSCAPS_SEPARATED },
	{ "STATIC_ALLOC", DSCAPS_STATIC_ALLOC },
	{ "TRIPLE", DSCAPS_TRIPLE },
	{ "PREMULTIPLIED", DSCAPS_PREMULTIPLIED },
	{ "DEPTH", DSCAPS_DEPTH },
	{ "SHARED", DSCAPS_SHARED },
	{ "ROTATED", DSCAPS_ROTATED },
	{ "ALL", DSCAPS_ALL },
	{ "FLIPPING", DSCAPS_FLIPPING },
	{ NULL, 0 }
};

static int CapabilityConst( lua_State *L ){
	return findConst(L, _Capability);
}

static const struct ConstTranscode _TextLayout [] = {
	{ "LEFT", DSTF_LEFT },
	{ "CENTER", DSTF_CENTER },
	{ "RIGHT", DSTF_RIGHT },
	{ "TOP", DSTF_TOP },
	{ "BOTTOM", DSTF_BOTTOM },
	{ "OUTLINE", DSTF_OUTLINE },
	{ NULL, 0 }
};

static int TextLayoutConst( lua_State *L ){
	return findConst(L, _TextLayout);
}

static const struct ConstTranscode _DrawingFlags [] = {
	{"NOFX", DSDRAW_NOFX},
	{"BLEND", DSDRAW_BLEND},
	{"DST_COLORKEY", DSDRAW_DST_COLORKEY},
	{"SRC_PREMULTIPLY", DSDRAW_SRC_PREMULTIPLY},
	{"DST_PREMULTIPLY", DSDRAW_DST_PREMULTIPLY},
	{"DEMULTIPLY", DSDRAW_DEMULTIPLY},
	{"XOR", DSDRAW_XOR},
	{ NULL, 0 }
};

static int DrawingFlagsConst( lua_State *L ){
	return findConst(L, _DrawingFlags);
}

static const struct ConstTranscode _FlipFlags [] = {
	{"NONE", DSFLIP_NONE},
	{"WAIT", DSFLIP_WAIT},
	{"BLIT", DSFLIP_BLIT},
	{"ONSYNC", DSFLIP_ONSYNC},
	{"PIPELINE", DSFLIP_PIPELINE},
	{"ONCE", DSFLIP_ONCE},
	{"QUEUE", DSFLIP_QUEUE},
	{"FLUSH", DSFLIP_FLUSH},
	{"SWAP", DSFLIP_SWAP},
	{"UPDATE", DSFLIP_UPDATE},
	{"WAITFORSYNC", DSFLIP_WAITFORSYNC},
	{ NULL, 0 }
};

static int FlipFlagsConst( lua_State *L ){
	return findConst(L, _FlipFlags);
}

static const struct ConstTranscode _BlitFlags [] = {
	{"NONE", DSBLIT_NOFX},
	{"BLEND_ALPHACHANNEL", DSBLIT_BLEND_ALPHACHANNEL},
	{"BLEND_COLORALPHA", DSBLIT_BLEND_COLORALPHA},
	{"COLORIZE", DSBLIT_COLORIZE},
	{"SRC_COLORKEY", DSBLIT_SRC_COLORKEY},
	{"DST_COLORKEY", DSBLIT_DST_COLORKEY},
	{"SRC_PREMULTIPLY", DSBLIT_SRC_PREMULTIPLY},
	{"DST_PREMULTIPLY", DSBLIT_DST_PREMULTIPLY},
	{"DEMULTIPLY", DSBLIT_DEMULTIPLY},
	{"DEINTERLACE", DSBLIT_DEINTERLACE},
	{"SRC_PREMULTCOLOR", DSBLIT_SRC_PREMULTCOLOR},
	{"XOR", DSBLIT_XOR},
	{"INDEX_TRANSLATION", DSBLIT_INDEX_TRANSLATION},
	{"ROTATE180", DSBLIT_ROTATE180},
	{"ROTATE270", DSBLIT_ROTATE270},
	{"COLORKEY_PROTECT", DSBLIT_COLORKEY_PROTECT},
	{"SRC_COLORKEY_EXTENDED", DSBLIT_SRC_COLORKEY_EXTENDED},
	{"DST_COLORKEY_EXTENDED", DSBLIT_DST_COLORKEY_EXTENDED},
	{"SRC_MASK_ALPHA", DSBLIT_SRC_MASK_ALPHA},
	{"SRC_MASK_COLOR", DSBLIT_SRC_MASK_COLOR},
	{"FLIP_HORIZONTAL", DSBLIT_FLIP_HORIZONTAL},
	{"FLIP_VERTICAL", DSBLIT_FLIP_VERTICAL},
	{"ROP", DSBLIT_ROP},
	{"SRC_COLORMATRIX", DSBLIT_ROP},
	{"SRC_CONVOLUTION", DSBLIT_SRC_CONVOLUTION},
	{ NULL, 0 }
};

static int BlittingFlagsConst( lua_State *L ){
	return findConst(L, _BlitFlags);
}

#define CQ1 1
#define CQ2 2
#define CQ3 4
#define CQ4 8
#define CQALL ( CQ1 | CQ2 | CQ3 | CQ4 )
static const struct ConstTranscode _CircleQuarter [] = {
	{"NONE", 0},
	{"Q1", CQ1},		/* 0-90° */
	{"Q2", CQ2},		/* 90-180° */
	{"Q3", CQ3},		/* 180-270° */
	{"Q4", CQ4},		/* 270-360° */
	{"ALL", CQALL},		/* 0-360° */
	{ NULL, 0 }
};

static int CircleQuarterConst( lua_State *L ){
	return findConst(L, _CircleQuarter);
}

static int createsurface(lua_State *L){
	DFBResult err;
	IDirectFBSurface **sp;
	DFBSurfaceDescription dsc;
	assert(dfb);

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "SelSurface.create() is expecting a table");
		return 2;
	}

	dsc.flags = 0;

	lua_pushstring(L, "caps");
	lua_gettable(L, -2);	/* Retrieve caps parameter if it exists */
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags = DSDESC_CAPS;
		dsc.caps = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "width");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DSDESC_WIDTH;
		dsc.width = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "height");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		dsc.flags |= DSDESC_HEIGHT;
		dsc.height = luaL_checkint(L, -1);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "size");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TTABLE){
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			dsc.flags |= DSDESC_WIDTH;
			dsc.width = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);
		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TNUMBER){
			dsc.flags |= DSDESC_HEIGHT;
			dsc.height = luaL_checkint(L, -1);
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);	/* cleaning ... */
/* tbd : other fields */

	sp = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = dfb->CreateSurface(dfb, &dsc, sp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

IDirectFBSurface **checkSelSurface(lua_State *L, int where){
	void *r = luaL_checkudata(L, where, "SelSurface");
	luaL_argcheck(L, r != NULL, where, "'SelSurface' expected");
	return (IDirectFBSurface **)r;
}

#define checkSelSurface1(L) checkSelSurface(L, 1)	/* Shortcut to ckeck the 1st argument */

static int SurfaceRelease(lua_State *L){
	IDirectFBSurface **s = checkSelSurface1(L);

	if(!*s){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	(*s)->Release(*s);
	*s = NULL;	/* Prevent double desallocation */

	return 0;
}

static int SurfaceGetPosition(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int x,y;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "GetPosition() on a dead object");
		return 2;
	}

	if((err = s->GetPosition(s, &x, &y)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	return 2;
}

static int SurfaceGetSize(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int w,h;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "GetSize() on a dead object");
		return 2;
	}

	if((err = s->GetSize(s, &w, &h)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	lua_pushinteger(L, w);
	lua_pushinteger(L, h);
	return 2;
}

static int SurfaceGetHeight(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int w,h;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "GetHeight() on a dead object");
		return 2;
	}

	if((err = s->GetSize(s, &w, &h)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	lua_pushinteger(L, h);
	return 1;
}

static int SurfaceGetWidth(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int w,h;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "GetWidth() on a dead object");
		return 2;
	}

	if((err = s->GetSize(s, &w, &h)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	lua_pushinteger(L, w);
	return 1;
}

static int SurfaceDrawRectangle(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	int w = luaL_checkint(L, 4);
	int h = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "DrawRectangle() on a dead surface");
		return 2;
	}

	if((err = s->DrawRectangle( s, x,y,w,h )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceFillRectangle(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	int w = luaL_checkint(L, 4);
	int h = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "FillRectangle() on a dead surface");
		return 2;
	}

	if((err = s->FillRectangle( s, x,y,w,h )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceClear(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int r = luaL_checkint(L, 2);
	int g = luaL_checkint(L, 3);
	int b = luaL_checkint(L, 4);
	int a = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "Clear() on a dead surface");
		return 2;
	}

	if((err = s->Clear( s, r,g,b,a )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceSetColor(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int r = luaL_checkint(L, 2);
	int g = luaL_checkint(L, 3);
	int b = luaL_checkint(L, 4);
	int a = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SetColor() on a dead surface");
		return 2;
	}

	if((err = s->SetColor( s, r,g,b,a )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceDrawLine(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int sx = luaL_checkint(L, 2);
	int sy = luaL_checkint(L, 3);
	int dx = luaL_checkint(L, 4);
	int dy = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "DrawLine() on a dead surface");
		return 2;
	}

	if((err = s->DrawLine( s, sx,sy,dx,dy )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

/* Based on "midpoint circle algorithm" and especially
 * BetterOS' code : http://betteros.org/tut/graphics1.php
 */
static int internalDrawCircle(lua_State *L, bool filled){
	IDirectFBSurface *s = *checkSelSurface1(L);
	int cx = luaL_checkint(L, 2);
	int cy = luaL_checkint(L, 3);
	int radius = luaL_checkint(L, 4);
	int quarter = CQALL;
	if( lua_gettop(L) == 5 )
		quarter = luaL_checkint(L, 5);

	int error = -radius;
	float x = radius;
	float y = 0;

	inline void plot4points(IDirectFBSurface *s, float cx, float cy, float x, float y, int Q){
/*		DFBResult err; */
		if( Q & CQ1 ) s->FillRectangle( s, cx+x, cy+y, 1,1 );
		if( Q & CQ2 ) s->FillRectangle( s, cx-x, cy+y, 1,1 );
		if( Q & CQ3 ) s->FillRectangle( s, cx-x, cy-y, 1,1 );
		if( Q & CQ4 ) s->FillRectangle( s, cx+x, cy-y, 1,1 );
	}

	inline void line4points(IDirectFBSurface *s, float cx, float cy, float x, float y, int Q){
		s->DrawLine( s, cx+x, cy + y, cx-x, cy + y );
		s->DrawLine( s, cx+x, cy-y, cx-x, cy-y );
	}

	inline void plot8points(IDirectFBSurface *s, int cx, int cy, float x, float y, bool filled, int Q){
		if(filled){
			line4points(s, cx, cy, x, y, Q);
			line4points(s, cx, cy, y, x, Q);
		} else {
			plot4points(s, cx, cy, x, y, Q);
			plot4points(s, cx, cy, y, x, Q);
		}
	}

	while(x >= y){
		plot8points(s, cx, cy, x, y, filled, quarter);

		error += y;
		y++;
		error += y;

		if(error >= 0){
			error += -x;
			x--;
			error += -x;
		}
	}
	return 0;
}

static int SurfaceDrawCircle(lua_State *L){
	return internalDrawCircle( L, false );
}

static int SurfaceFillCircle(lua_State *L){
	return internalDrawCircle( L, true );
}

static int SurfaceFillTriangle(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int x1 = luaL_checkint(L, 2);
	int y1 = luaL_checkint(L, 3);
	int x2 = luaL_checkint(L, 4);
	int y2 = luaL_checkint(L, 5);
	int x3 = luaL_checkint(L, 6);
	int y3 = luaL_checkint(L, 7);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "FillTriangle() on a dead surface");
		return 2;
	}

	if((err = s->FillTriangle( s, x1,y1,x2,y2,x3,y3 )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceSetFont(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBFont **font = luaL_checkudata(L, 2, "SelFont");

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SetFont() on a dead surface");
		return 2;
	}

	if(!font){
		lua_pushnil(L);
		lua_pushstring(L, "SelFont expected");
		return 2;
	}

	if((err = s->SetFont(s, *font)) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceGetFont(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBFont **pfont;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "GetFont() on a dead surface");
		return 2;
	}

	pfont = (IDirectFBFont **)lua_newuserdata(L, sizeof(IDirectFBFont *));
	luaL_getmetatable(L, "SelFont");
	lua_setmetatable(L, -2);

	if((err = s->GetFont( s, pfont)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static int SurfaceDrawString(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	const char *msg = luaL_checkstring(L, 2);	/* Message to display */
	int x = luaL_checkint(L, 3);
	int y = luaL_checkint(L, 4);
	int alignment;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "DrawString() on a dead surface");
		return 2;
	}

	if(lua_isnoneornil(L, 5))
		alignment = DSTF_TOPLEFT;
	else
		alignment = luaL_checkint(L, 5);

	if((err = s->DrawString(s, msg,-1,x,y,alignment)) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceSetDrawingFlags(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int flg = luaL_checkint(L, 2);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SetDrawingFlags() on a dead surface");
		return 2;
	}

	if((err = s->SetDrawingFlags( s, flg )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceSetBlittingFlags(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int flg = luaL_checkint(L, 2);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SetBlittingFlags() on a dead surface");
		return 2;
	}

	if((err = s->SetBlittingFlags( s, flg )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static DFBRectangle *readRectangle(lua_State *L, int idx, DFBRectangle *rec){
	/* Read an optionnal Rectangle at idx position */
	if(lua_type(L, idx) == LUA_TTABLE){
		lua_pushinteger(L, 1);
		lua_gettable(L, idx);
		rec->x = luaL_checkint(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, idx);
		rec->y = luaL_checkint(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, idx);
		rec->w = luaL_checkint(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 4);
		lua_gettable(L, idx);
		rec->h = luaL_checkint(L, -1);
		lua_pop(L, 1);

		return rec;
	} else
		return NULL;
}

static int SurfaceBlit(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBSurface *src = *checkSelSurface(L,2);	/* Source surface */
	DFBRectangle trec;
	DFBRectangle *rec = readRectangle(L, 3, &trec);	/* 3: source rect */
	int x = luaL_checkint(L, 4);
	int y = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "Blit() on a dead surface");
		return 2;
	}

	if((err = s->Blit (s, src, rec, x,y)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceTileBlit(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBSurface *src = *checkSelSurface(L,2);	/* Source surface */
	DFBRectangle trec;
	DFBRectangle *rec = readRectangle(L, 3, &trec);	/* 3: source rect */
	int x = luaL_checkint(L, 4);
	int y = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "TileBlit() on a dead surface");
		return 2;
	}

	if((err = s->TileBlit (s, src, rec, x,y)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceTileBlitClip(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBSurface *src = *checkSelSurface(L,2);	/* Source surface */
	DFBRectangle trec;
	DFBRectangle *rec = readRectangle(L, 3, &trec);	/* 3: source rect */
	DFBRectangle clip;
	DFBRegion pclip;

	if(!readRectangle(L, 4, &clip)){
		lua_pushnil(L);
		lua_pushstring(L, "TileBlitClip() without clipping zone");
		return 2;
	}

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "TileBlitClip() on a dead surface");
		return 2;
	}

	if((err = s->GetClip(s, &pclip)) != DFB_OK){	/* Save the original clipping */
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

		/* Convert Rectangle to Region */
	clip.w += clip.x;
	clip.h += clip.y;
	if((err = s->SetClip(s, (DFBRegion *)&clip)) != DFB_OK){	/* Set the new clipping */
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = s->TileBlit (s, src, rec, clip.x, clip.y)) != DFB_OK){	/* Blitting */
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = s->SetClip(s, &pclip)) != DFB_OK){	/* Restore the original clipping */
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceStretchBlit(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBSurface *src = *checkSelSurface(L,2);	/* Source surface */
	DFBRectangle trec,tdrec;
	DFBRectangle *rec = readRectangle(L, 3, &trec);	/* 3: source rect */
	DFBRectangle *rdst = readRectangle(L, 4, &tdrec);	/* 4: destrination rect */
	
	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "StretchBlit() on a dead surface");
		return 2;
	}

	if((err = s->StretchBlit (s, src, rec, rdst)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 0;
}

static int SurfaceSetClip(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	DFBRegion trec;
	DFBRegion *rec = (DFBRegion *)readRectangle(L, 2, (DFBRectangle *)&trec);
	
	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SetClip() on a dead surface");
		return 2;
	}

	if((err = s->SetClip(s, rec)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceSetClipS(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	DFBRegion rec;

	rec.x1 = luaL_checkint(L, 2);
	rec.y1 = luaL_checkint(L, 3);
	rec.x2 = rec.x1 + luaL_checkint(L, 4);
	rec.y2 = rec.y1 + luaL_checkint(L, 5);
	
	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SetClipS() on a dead surface");
		return 2;
	}

	if((err = s->SetClip(s, &rec)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceSubSurface(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBSurface **sp;
	DFBRectangle geo;

	geo.x = luaL_checkint(L, 2);
	geo.y = luaL_checkint(L, 3);
	geo.w = luaL_checkint(L, 4);
	geo.h = luaL_checkint(L, 5);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "SubSurface() on a dead surface");
		return 2;
	}

	sp = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = s->GetSubSurface(s, &geo, sp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	
	return 1;
}

static int SurfaceDump(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	const char *dir= luaL_checkstring(L, 2);	/* Directory where to put the grab */
	const char *prf= luaL_checkstring(L, 3);	/* prefix for the file */

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "Dump() on a dead surface");
		return 2;
	}

	if((err = s->Dump( s, dir, prf )) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceFlip(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	int flg = luaL_checkint(L, 2);

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "Flip() on a dead surface");
		return 2;
	}

	if((err = s->Flip( s, NULL, flg)) !=  DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

static int SurfaceClone(lua_State *L){
	/* Clone to an identical surface */
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	DFBSurfaceDescription   dsc;
	IDirectFBSurface **sp;

	if(!s){
		lua_pushnil(L);
		lua_pushstring(L, "clone() on a dead surface");
		return 2;
	}

	dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
	s->GetSize( s, &dsc.width, &dsc.height );

	sp = (IDirectFBSurface **)lua_newuserdata(L, sizeof(IDirectFBSurface *));
	luaL_getmetatable(L, "SelSurface");
	lua_setmetatable(L, -2);

	if((err = dfb->CreateSurface(dfb, &dsc, sp)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = (*sp)->Blit (*sp, s, NULL, 0,0)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;
}

static int SurfaceRestore(lua_State *L){
	DFBResult err;
	IDirectFBSurface *s = *checkSelSurface1(L);
	IDirectFBSurface *back = *checkSelSurface(L,2);
	DFBRegion pclip;

	if(!s || !back){
		lua_pushnil(L);
		lua_pushstring(L, "restore() on a dead surface");
		return 2;
	}

	if((err = s->GetClip(s, &pclip)) != DFB_OK){	/* Save the original clipping */
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = s->SetBlittingFlags(s, DSBLIT_NOFX)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = s->SetClip(s, NULL)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = s->Blit (s, back, NULL, 0,0)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	if((err = s->SetClip(s, &pclip)) != DFB_OK){	/* Restore the original clipping */
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	return 1;	
}

static const struct luaL_reg SelSurfaceLib [] = {
	{"CapabilityConst", CapabilityConst},
	{"TextLayoutConst", TextLayoutConst},
	{"DrawingFlagsConst", DrawingFlagsConst},
	{"BlittingFlagsConst", BlittingFlagsConst},
	{"FlipFlagsConst", FlipFlagsConst},
	{"CircleQuarterConst", CircleQuarterConst},
	{"create", createsurface},
	{NULL, NULL}
};

static const struct luaL_reg SelSurfaceM [] = {
	{"Release", SurfaceRelease},
	{"destroy", SurfaceRelease},	/* Alias */
	{"GetPosition", SurfaceGetPosition},
	{"GetSize", SurfaceGetSize},
	{"GetHeight", SurfaceGetHeight},
	{"GetWidth", SurfaceGetWidth},
	{"Clear", SurfaceClear},
	{"SetColor", SurfaceSetColor},
	{"SetDrawingFlags", SurfaceSetDrawingFlags},
	{"DrawRectangle", SurfaceDrawRectangle},
	{"FillRectangle", SurfaceFillRectangle},
	{"FillTriangle", SurfaceFillTriangle},
	{"DrawLine", SurfaceDrawLine},
	{"DrawCircle", SurfaceDrawCircle},
	{"FillCircle", SurfaceFillCircle},
	{"DrawString", SurfaceDrawString},
	{"SetBlittingFlags", SurfaceSetBlittingFlags},
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
	{"Flip", SurfaceFlip},
	{"Dump", SurfaceDump},
	{"clone", SurfaceClone},
	{"restore", SurfaceRestore},
	{NULL, NULL}
};

void _include_SelSurface( lua_State *L ){
	luaL_newmetatable(L, "SelSurface");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelSurfaceM);
	luaL_register(L,"SelSurface", SelSurfaceLib);
}
#endif
