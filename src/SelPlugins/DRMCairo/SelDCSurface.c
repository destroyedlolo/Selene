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
/*	{"GetPosition", SurfaceGetPosition},
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
