/***
 * LCD SubSurface
 *
 *	SubSurfaces are only restricted area of a main surface 
 *	(SelSurface or SelScreen)
 */
#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDSubSurface.h>
#include <Selene/SelPlug-in/SelLCD/SelLCDSurface.h>

struct SGS_callbacks cb_subsurface;

static struct SelLCDSubSurfaceLua *checkSelLCDSubSurface(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCDSubSurface");
	luaL_argcheck(L, r != NULL, 1, "'SelLCDSubSurface' expected");

	return (struct SelLCDSubSurfaceLua *)r;
}

static int lcdssl_Surface(lua_State *L){
	struct SelLCDSubSurfaceLua *lcd = checkSelLCDSubSurface(L);
	uint8_t x = lua_tonumber(L, 2);
	uint8_t y = lua_tonumber(L, 3);
	uint8_t w = lua_tonumber(L, 4);
	uint8_t h = lua_tonumber(L, 5);

	struct SelLCDSurface *srf = (struct SelLCDSurface *)lcd->storage->shared.obj.cb->Surface(&lcd->storage->shared.obj, x,y, w,h, lcd->storage->shared.obj.cb->getPrimary(&lcd->storage->shared.obj));
	if(!srf)
		return 0;

	struct SelLCDSurfaceLua *srfl = (struct SelLCDSurfaceLua *)lua_newuserdata(L, sizeof(struct SelLCDSurfaceLua));
	srfl->storage = srf;

	luaL_getmetatable(L, "SelLCDSurface");
	lua_setmetatable(L, -2);

	return 1;
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
	{"Surface", lcdssl_Surface},
	{"Refresh", lcdssl_Refresh},
	{"Dump", lcdssl_Dump},
	{NULL, NULL}    /* End of definition */
};


static const char * const LuaName(){
	return "SelLCDSubSurface";
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

static bool lcdssc_setVisibility(struct SelLCDSubSurface *srf, bool){
	/* It's not possible to set the visibility of a subSurface.
	 * So we send a message an return the parent's visibility.
	 */
	
	slcd_selLog->Log('D', "Can't set the visibility of a subSurface");

	return(srf->shared.obj.cb->getVisibility(&srf->shared.obj));
}

static bool lcdssc_getVisibility(struct SelLCDSubSurface *srf){
	/* Propagating parent visibility since SubSurface has no independent
	 * visibility.
	 */
	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);
	return(parent->obj.cb->getVisibility(&parent->obj));
}

static bool lcdssc_Dump(struct SelLCDSubSurface *srf){
	/* SubSurface is not materialized independently, so we delegate the call
	 * to its parent.
	 */
	struct SelLCDSharedSurface *parent = srf->shared.obj.cb->getParent(&srf->shared.obj);
	return(parent->obj.cb->Dump(&parent->obj));
}

void initSelLCDSubSurfaceCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_subsurface);

	cb_subsurface.LuaObjectName = LuaName;

	cb_subsurface.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdss_getSize;
	/* A subsurface can only access to its parent, not the primary
	cb_subsurface.getPrimary = 
	*/
	cb_subsurface.getParent = (void *(*)(struct SelGenericSurface *))lcdss_getParent;
	cb_subsurface.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdss_subSurface;
	cb_subsurface.Surface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdss_Surface;

	cb_subsurface.setVisibility = (bool (*)(struct SelGenericSurface *, bool))lcdssc_setVisibility;
	cb_subsurface.getVisibility = (bool (*)(struct SelGenericSurface *))lcdssc_getVisibility;

	cb_subsurface.Home = (bool (*)(struct SelGenericSurface *))lcdss_Home;
	cb_subsurface.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))lcdss_setCursor;
	cb_subsurface.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdss_inSurface;

	cb_subsurface.Clear = (bool (*)(struct SelGenericSurface *))lcdss_Clear;
	cb_subsurface.WriteString = (bool (*)(struct SelGenericSurface *, const char *))lcdss_WriteString;
	cb_subsurface.bSet = (void (*)(struct SelGenericSurface *, const char, struct SelCoordinate *))lcdssc_bSet;
	cb_subsurface.Refresh = (bool (*)(struct SelGenericSurface *))lcdssc_Refresh;
	cb_subsurface.Dump = (bool (*)(struct SelGenericSurface *))lcdssc_Dump;
}
