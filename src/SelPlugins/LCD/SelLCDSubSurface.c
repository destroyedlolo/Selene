/***
 * LCD SubSurface
 *
 *	SubSurfaces are only restricted area of a main surface 
 *	(SelSurface or SelScreen)
 */
#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDSubSurface.h>

struct SGS_callbacks cb_subsurface;

const struct luaL_Reg LCDSubSurfaceMethods[] = {
	{NULL, NULL}    /* End of definition */
};


static const char * const LuaName(){
	return "LCDSubSurface";
}

static bool lcdssc_GetSize(struct SelLCDSubSurface *lcd, uint32_t *w, uint32_t *h){
	if(w)
		*w = lcd->shared.w;
	if(h)
		*h = lcd->shared.h;

	return true;
}

static bool lcdssc_Home(struct SelLCDSubSurface *lcd){
	lcd->shared.cursor.x = lcd->shared.cursor.y = 0;

	return true;
}

static bool lcdssc_SetCursor(struct SelLCDSubSurface *lcd, uint32_t x, uint32_t y){
	lcd->shared.cursor.x = x;
	lcd->shared.cursor.y = y;

	return true;
}

static bool lcdssc_inSurface(struct SelLCDSubSurface *lcd, uint32_t x, uint32_t y){
	return( x < lcd->shared.w && y < lcd->shared.h );
}

static bool lcdssc_Clear(struct SelLCDSubSurface *lcd){
	uint8_t i,j;

puts("********* Clear 0");

	for(j=0; j<lcd->shared.h; ++j){
		struct SelCoordinate coord;
		coord.y = lcd->shared.origine.y+j;
		for(i=0; i<lcd->shared.w; ++i){
			coord.x = lcd->shared.origine.x+i;
printf("0.1 : %p\n", lcd->shared.obj.cb->bSet);
			lcd->shared.obj.cb->bSet(
				lcd->shared.obj.cb->getParent(&lcd->shared.obj),
				' ', &coord
			);
puts("0.2");
		}
	}

puts("**** Clear 1");
	lcd->shared.obj.cb->Home((struct SelGenericSurface *)lcd);
puts("**** Clear 2");

	return true;
}

void initSLLCDSubSurfaceCallBacks(){
	cb_subsurface.LuaObjectName = LuaName;

	cb_subsurface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdssc_GetSize;
	/* A subsurface can only access to its parent, not the primary
	cb_subsurface.getPrimary = 
	*/
	cb_subsurface.getParent = (void *(*)(struct SelGenericSurface *))slss_getParent;

	cb_subsurface.Home = (bool (*)(struct SelGenericSurface *))lcdssc_Home;
	cb_subsurface.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))lcdssc_SetCursor;
	cb_subsurface.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdssc_inSurface;

	cb_subsurface.Clear = (bool (*)(struct SelGenericSurface *))lcdssc_Clear;
printf("***** cb_subsurface.Clear : %p\n", cb_subsurface.Clear);
}
