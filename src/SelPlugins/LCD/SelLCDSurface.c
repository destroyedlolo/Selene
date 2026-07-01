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
#include <ctype.h>

struct SGS_callbacks cb_surface;

static struct SelLCDSurfaceLua *checkSelLCDSurface(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCDSurface");
	luaL_argcheck(L, r != NULL, 1, "'SelLCDSurface' expected");

	return (struct SelLCDSurfaceLua *)r;
}

static int lcdsl_Refresh(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);

	lcd->storage->shared.obj.cb->Refresh(&lcd->storage->shared.obj);

	return 0;
}

static int lcdsl_Dump(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);

	lcd->storage->shared.obj.cb->Dump(&lcd->storage->shared.obj);

	return 0;
}

const struct luaL_Reg LCDSurfaceMethods[] = {
	{"Refresh", lcdsl_Refresh},
	{"Dump", lcdsl_Dump},
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

	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);
	for(uint8_t y = 0; y < srf->shared.h; ++y){
		for(uint8_t x = 0; x < srf->shared.w; ++x){
			struct SelCoordinate coord = {
				x + srf->shared.origine.x,
				y + srf->shared.origine.y
			};
			parent->obj.cb->bSet(
				&parent->obj,
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

	return(!!srf->buffer);
}

static bool lcdsc_Refresh(struct SelLCDSurface *srf){
	/* Delegate the  call to its parent ... only if we are visible.
	 * Although we could optimize by refreshing only our region, the LCD
	 * refresh is already efficient enough to avoid this.
	 */
	
	if(!srf->shared.obj.cb->getVisibility(&srf->shared.obj))
		return false;

	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);
	return(parent->obj.cb->Refresh(&parent->obj));
}

static void lcdsc_DumpBuffers(struct SelLCDSurface *srf){
	if(srf->buffer){
		puts("Surface buffer :");
		for(int j = 0; j < srf->shared.h; ++j){
			printf("'");
			for(int i = 0; i < srf->shared.w; ++i){
				char c = srf->buffer[i + j*srf->shared.w];
				if(isprint(c))
					printf("%c  ", c);
				else
					printf("%02x ", (unsigned char)c);
			}
			printf("'\n");
		}
	}
}

void initSeLLCDSurfaceCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_surface);

	cb_surface.LuaObjectName = LuaName;

	cb_surface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdsc_GetSize;

	cb_surface.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdss_subSurface;
	cb_surface.getPrimary = (void *(*)(struct SelGenericSurface *))lcdss_getPrimary;
	cb_surface.getParent = (void *(*)(struct SelGenericSurface *))lcdss_getParent;

	cb_surface.setVisibility = (bool (*)(struct SelGenericSurface *, bool))lcdsc_setVisibility;
	cb_surface.getVisibility = (bool (*)(struct SelGenericSurface *))lcdsc_getVisibility;

	cb_surface.Home = (bool (*)(struct SelGenericSurface *))lcdss_Home;
	cb_surface.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))lcdss_setCursor;
	cb_surface.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdss_inSurface;
	cb_surface.Clear = (bool (*)(struct SelGenericSurface *))lcdss_Clear;
	cb_surface.WriteString = (bool (*)(struct SelGenericSurface *, const char *))lcdss_WriteString;

	cb_surface.AllocateBuffer = (bool (*)(struct SelGenericSurface *))lcdsc_AllocBuff;
	cb_surface.Dump = (bool (*)(struct SelGenericSurface *))lcdsc_DumpBuffers;
	cb_surface.bSet = (void (*)(struct SelGenericSurface *, const char, struct SelCoordinate *))lcdsc_bSet;
	cb_surface.Refresh = (bool (*)(struct SelGenericSurface *))lcdsc_Refresh;
}

void initSelLCDSurface(struct SelLCDSurface *srf, uint8_t width, uint8_t height, uint8_t left, uint8_t top, struct SelLCDSharedSurface *parent){
	srf->shared.obj.cb = &cb_surface;
	srf->buffer = NULL;
	srf->visible = true;

	initSharedSurface(
		&srf->shared,	/* ourself */
		parent,			/* the parent */
		width, height,	/* size */
		left, top,		/* origine */
		parent->obj.cb->getPrimary(&parent->obj)
	);

	srf->shared.obj.cb->AllocateBuffer(&srf->shared.obj);
}
