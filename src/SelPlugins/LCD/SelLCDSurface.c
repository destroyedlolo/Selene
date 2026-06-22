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

static void RefreshParent(struct SelLCDSurface *srf){
	if(!srf->visible)
		return;

	for(uint8_t y = 0; y < srf->shared.h; ++y){
		for(uint8_t x = 0; x < srf->shared.w; ++x){
			struct SelCoordinate coord = {
				x + srf->shared.origine.x,
				y + srf->shared.origine.y
			};
			srf->shared.obj.cb->bSet(
				&srf->shared.obj,
				srf->buffer[x + y * srf->shared.w],
				&coord
			);
		}
		
	}
}

static bool lcdsc_setVisibility(struct SelLCDSurface *srf, bool v){
	bool ans = srf->visible;

	srf->visible = v;
	if(v && !ans){
		/* Becoming visible.
		 * Becoming invisible is not managed here : we let upstream
		 * to refresh the parent surface or display the new visible one.
		 */
		RefreshParent(srf);
	}

	return ans;
}

static bool lcdsc_getVisibility(struct SelLCDSurface *srf){
	return srf->visible;
}

void initSLLCDSurfaceCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_surface);

	cb_surface.LuaObjectName = LuaName;

	cb_surface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdsc_GetSize;

	cb_surface.getPrimary = (void *(*)(struct SelGenericSurface *))slss_getPrimary;
	cb_surface.getParent = (void *(*)(struct SelGenericSurface *))slss_getParent;

	cb_surface.setVisibility = (bool (*)(struct SelGenericSurface *, bool))lcdsc_setVisibility;
	cb_surface.getVisibility = (bool (*)(struct SelGenericSurface *))lcdsc_getVisibility;

	cb_surface.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))slss_subSurface;
}
