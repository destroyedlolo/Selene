/***
 * LCD SubSurface
 *
 *	SubSurfaces are only restricted area of a main surface 
 *	(SelSurface or SelScreen)
 */
#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDSubSurface.h>

struct SGS_callbacks cb_subsurface;

static struct SelLCDSubSurfaceLua *checkSelLCDSubSurface(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCDSubSurface");
	luaL_argcheck(L, r != NULL, 1, "'SelLCDSubSurface' expected");

	return (struct SelLCDSubSurfaceLua *)r;
}

static int lcdssl_Refresh(lua_State *L){
	struct SelLCDSubSurfaceLua *lcd = checkSelLCDSubSurface(L);

	lcd->storage->shared.obj.cb->Refresh(&lcd->storage->shared.obj);

	return 0;
}

static int lcdssl_Dump(lua_State *L){
	struct SelLCDSubSurfaceLua *lcd = checkSelLCDSubSurface(L);

	lcd->storage->shared.obj.cb->Dump(&lcd->storage->shared.obj);

	return 0;
}

const struct luaL_Reg LCDSubSurfaceMethods[] = {
	{"Refresh", lcdssl_Refresh},
	{"Dump", lcdssl_Dump},
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

	for(j=0; j<lcd->shared.h; ++j){
		for(i=0; i<lcd->shared.w; ++i){
			struct SelCoordinate coord = {i,j};
			lcd->shared.obj.cb->bSet( &lcd->shared.obj, ' ', &coord);
		}
	}

	lcd->shared.obj.cb->Home((struct SelGenericSurface *)lcd);

	return true;
}

static bool lcdssc_WriteString(struct SelLCDSubSurface *srf, const char *txt){
	for(const char *c = txt; *c; ++c){
		srf->shared.obj.cb->bSet(&srf->shared.obj, *c, &srf->shared.cursor);
		++srf->shared.cursor.x;
	}
	return true;
}

static void lcdssc_bSet(struct SelLCDSubSurface *srf, const char c, struct SelCoordinate *crd){
	if(!srf->shared.obj.cb->inSurface(&srf->shared.obj, crd->x, crd->y))
		return;

	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);

	struct SelCoordinate pcrd;
	pcrd.x = crd->x + srf->shared.origine.x;
	pcrd.y = crd->y + srf->shared.origine.y;

	parent->obj.cb->bSet(&parent->obj, c, &pcrd);
}

static bool lcdssc_Refresh(struct SelLCDSubSurface *srf){
	/* SubSurface is not materialized independently, so we delegate the
	 * call to its parent.
	 * Although we could optimize by refreshing only our region, the LCD
	 * refresh is already efficient enough to avoid this.
	 */
	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);
	return(parent->obj.cb->Refresh(&parent->obj));
}

static bool lcdssc_Dump(struct SelLCDSubSurface *srf){
	/* SubSurface is not materialized independently, so we delegate the call
	 * to its parent.
	 */
	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);
	return(parent->obj.cb->Dump(&parent->obj));
}

void initSLLCDSubSurfaceCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_subsurface);

	cb_subsurface.LuaObjectName = LuaName;

	cb_subsurface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdssc_GetSize;
	/* A subsurface can only access to its parent, not the primary
	cb_subsurface.getPrimary = 
	*/
	cb_subsurface.getParent = (void *(*)(struct SelGenericSurface *))slss_getParent;
	cb_subsurface.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))slss_subSurface;

	cb_subsurface.Home = (bool (*)(struct SelGenericSurface *))lcdssc_Home;
	cb_subsurface.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))lcdssc_SetCursor;
	cb_subsurface.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdssc_inSurface;

	cb_subsurface.Clear = (bool (*)(struct SelGenericSurface *))lcdssc_Clear;
	cb_subsurface.WriteString = (bool (*)(struct SelGenericSurface *, const char *))lcdssc_WriteString;
	cb_subsurface.bSet = (void (*)(struct SelGenericSurface *, const char, struct SelCoordinate *))lcdssc_bSet;
	cb_subsurface.Refresh = (bool (*)(struct SelGenericSurface *))lcdssc_Refresh;
	cb_subsurface.Dump = (bool (*)(struct SelGenericSurface *))lcdssc_Dump;
}
