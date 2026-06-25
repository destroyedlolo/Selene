/***
 * LCD Surface
 *
 *	Surfaces is like a subSurface but with its dedicated buffer
 */

#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDSurface.h>

#include <stdlib.h>	/* malloc() */
#include <assert.h>
#include <string.h>

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

static void ApplyParent(struct SelLCDSurface *srf){
	/* update the parent with our buffer's content */
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
		ApplyParent(srf);
	}

	return ans;
}

static bool lcdsc_getVisibility(struct SelLCDSurface *srf){
	return srf->visible;
}

static void lcdsc_bSet(struct SelLCDSurface *srf, const char c, struct SelCoordinate *crd){
	if(!srf->shared.obj.cb->inSurface(&srf->shared.obj, crd->x, crd->y))
		return;

		/* update local buffer */
	srf->buffer[crd->x + crd->y * srf->shared.w] = c;

		/* update parent if we're visible */
	if(srf->visible){
		struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);

		struct SelCoordinate pcrd;
		pcrd.x = crd->x + srf->shared.origine.x;
		pcrd.y = crd->y + srf->shared.origine.y;

		parent->obj.cb->bSet(&parent->obj, c, &pcrd);
	}
}

bool lcdsc_AllocBuff(struct SelLCDSurface *srf){
#ifdef DEBUG
	slcd_selLog->Log('D', "Surface Buffers allocation");
#endif

		/* Free existing buffers */
	if(srf->buffer){
		free(srf->buffer);
		srf->buffer = NULL;	/* Only if there is a failure afterward */
	}

	srf->buffer = malloc(srf->shared.w * srf->shared.h);
	assert(srf->buffer);

		/* Only to avoid garbages in this buffer.
		 * As it's very unlikely to feed a screen with 0 (clear() is writing
		 * spaces), it will force a full refresh at first ... unlike
		 * we are Clear()ing first.
		 */
	memset(srf->buffer, 0, srf->shared.w * srf->shared.h);

	return(!!srf->buffer);
}

void initSLLCDSurfaceCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_surface);

	cb_surface.LuaObjectName = LuaName;

	cb_surface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdsc_GetSize;

	cb_surface.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdss_subSurface;
	cb_surface.getPrimary = (void *(*)(struct SelGenericSurface *))lcdss_getPrimary;
	cb_surface.getParent = (void *(*)(struct SelGenericSurface *))lcdss_getParent;
	cb_surface.AllocateBuffer = (bool (*)(struct SelGenericSurface *))lcdsc_AllocBuff;

	cb_surface.setVisibility = (bool (*)(struct SelGenericSurface *, bool))lcdsc_setVisibility;
	cb_surface.getVisibility = (bool (*)(struct SelGenericSurface *))lcdsc_getVisibility;

	cb_surface.Home = (bool (*)(struct SelGenericSurface *))lcdss_Home;
	cb_surface.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))lcdss_Home;
	cb_surface.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdss_inSurface;

	cb_subsurface.bSet = (void (*)(struct SelGenericSurface *, const char, struct SelCoordinate *))lcdsc_bSet;
	
}
