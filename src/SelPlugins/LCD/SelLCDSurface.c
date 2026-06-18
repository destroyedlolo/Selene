/***
 * LCD Surface
 *
 *	Surfaces is like a subSurface but with its dedicated buffer
 */

#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDSurface.h>

struct SGS_callbacks cb_surface;

const struct luaL_Reg LCDSurfaceMethods[] = {
	{NULL, NULL}    /* End of definition */
};

static const char * const LuaName(){
	return "SelLCDSurface";
}

static bool lcdsc_GetSize(struct SelLCDSurface *lcd, uint32_t *w, uint32_t *h){
	if(w)
		*w = lcd->shared.w;
	if(h)
		*h = lcd->shared.h;

	return true;
}

void initSLLCDSurfaceCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_surface);

	cb_surface.LuaObjectName = LuaName;

	cb_surface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdsc_GetSize;

	cb_surface.getParent = (void *(*)(struct SelGenericSurface *))slss_getParent;
	cb_subsurface.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))slss_subSurface;
}
