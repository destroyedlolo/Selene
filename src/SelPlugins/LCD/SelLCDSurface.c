/***
 * Manage sub surface on textual LCD screen.
 *

@classmod SelLCDSurface

 * 30/11/2024 LF : First version
 */

#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDScreen.h>

#include <stdlib.h>

struct SGS_callbacks sLCD_cb;		/* Primary surface callbacks */
struct SGS_callbacks sLCDsub_cb;	/* Sub surface callbacks */

static struct SelLCDSurfaceLua *checkSelLCDSurface(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCDSurface");
	luaL_argcheck(L, r != NULL, 1, "'SelLCDSurface' expected");

	return (struct SelLCDSurfaceLua *)r;
}

static bool lcdsc_inSurface(struct SelLCDSurface *lcd, uint32_t x, uint32_t y){
	return( x < lcd->w && y < lcd->h );
}

static bool lcdsc_Home(struct SelLCDSurface *lcd){
	lcd->cursor.x = lcd->cursor.y = 0;

	return true;
}

static bool lcdsc_Lock(struct SelLCDSurface *srf){
	pthread_mutex_lock(&srf->screen->mutex);
	return true;
}

static bool lcdsc_Unlock(struct SelLCDSurface *srf){
	pthread_mutex_unlock(&srf->screen->mutex);
	return true;
}

static int lcdsl_Home(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);

	lcd->storage->obj.cb->Home((struct SelGenericSurface *)lcd->storage);

	return 0;
}

static bool lcdsc_GetSize(struct SelLCDSurface *lcd, uint32_t *w, uint32_t *h){
	if(w)
		*w = lcd->w;
	if(h)
		*h = lcd->h;

	return true;
}

static int lcdsl_GetSize(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);
	uint32_t w,h;

	lcd->storage->obj.cb->getSize((struct SelGenericSurface *)lcd->storage, &w,&h);

	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	return 2;
}

static bool lcdsc_Clear(struct SelLCDSurface *lcd){
	uint8_t i,j;

	lcd->obj.cb->Lock((struct SelGenericSurface *)lcd);

	for(j=0; j<lcd->h; ++j){
		slcd_selLCD.SetCursor(lcd->screen, lcd->origine.x, lcd->origine.y+j);
		for(i=0; i<lcd->w; ++i)
			slcd_selLCD.SendData(lcd->screen, ' ');
	}
	lcd->obj.cb->Home((struct SelGenericSurface *)lcd);

	lcd->obj.cb->Unlock((struct SelGenericSurface *)lcd);
	return true;
}

static int lcdsl_Clear(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);

	lcd->storage->obj.cb->Clear((struct SelGenericSurface *)lcd->storage);

	return 0;
}

static bool lcdsc_WriteString(struct SelLCDSurface *lcd, const char *txt){
	if(!lcdsc_inSurface(lcd, lcd->cursor.x, lcd->cursor.y))
		return true;	/* Supported even not displayed */

	lcd->obj.cb->Lock((struct SelGenericSurface *)lcd);

		/* Move to the beginning of the string */
	slcd_selLCD.SetCursor(lcd->screen, 
		lcd->origine.x + lcd->cursor.x,
		lcd->origine.y + lcd->cursor.y
	);

	while(*txt){
		slcd_selLCD.SendData(lcd->screen, *txt);
		if(!lcdsc_inSurface(lcd, ++(lcd->cursor.x), lcd->cursor.y))
			break;
		++txt;
	}

	lcd->obj.cb->Unlock((struct SelGenericSurface *)lcd);
	return true;
}

static int lcdsl_WriteString(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);
	const char *txt = luaL_checkstring(L,2);

	lcd->storage->obj.cb->WriteString((struct SelGenericSurface *)lcd->storage, txt);

	return 0;
}

static struct SelLCDSurface *lcdsc_subSurface(struct SelLCDSurface *p, uint32_t x, uint32_t y, uint32_t w, uint32_t h, struct SelLCDScreen *lcd){
	/*** Create a subSurface
	 *
	 * @cfunction subSurface
	 * @tparam lua_State * Lua context (if NULL, allocated using malloc() )
	 * @tparam struct SelLCDSurface * Parent surface
	 * @tparam uint32_t x,y origine
	 * @tparam uint32_t w,h size
	 * @tparam struct SelLCDScreen physical driver
	 * @return pointer to the new subSurface (NULL if error)
	 */

	if(!p->obj.cb->inSurface((struct SelGenericSurface *)p, x,y))	/* Outsize parent surface */
		return NULL;

	if(x+w > p->w){
		if(x > p->w)
			return NULL;
		w = p->w - x;
	}

	if(y+h > p->h){
		if(y > p->h)
			return NULL;
		h = p->h - y;
	}

	struct SelLCDSurface *srf = malloc(sizeof(struct SelLCDSurface));
	if(!srf)
		return NULL;

	initExportedSurface(srf, p, w,h, x,y, lcd);

	return srf;
}

void *lcdc_getPrimary(struct SelLCDSurface *s){
	/* Returns the primary */
	return s->screen;
}

static int lcdsl_subSurface(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);
	uint8_t x = lua_tonumber(L, 2);
	uint8_t y = lua_tonumber(L, 3);
	uint8_t w = lua_tonumber(L, 4);
	uint8_t h = lua_tonumber(L, 5);

	struct SelLCDSurface *srf = lcdsc_subSurface(lcd->storage, x,y, w,h, lcd->storage->screen);
	if(!srf)
		return 0;

	struct SelLCDSurfaceLua *srfl = (struct SelLCDSurfaceLua *)lua_newuserdata(L, sizeof(struct SelLCDSurfaceLua));
	srfl->storage = srf;

	luaL_getmetatable(L, "SelLCDSurface");
	lua_setmetatable(L, -2);

	return 1;
}

bool lcdsc_SetCursor(struct SelLCDSurface *lcd, uint32_t x, uint32_t y){
	lcd->cursor.x = x;
	lcd->cursor.y = y;

	return true;
}

static int lcdsl_SetCursor(lua_State *L){
	struct SelLCDSurfaceLua *lcd = checkSelLCDSurface(L);
	uint32_t x = lua_tonumber(L, 2);
	uint32_t y = lua_tonumber(L, 3);

	lcd->storage->obj.cb->setCursor((struct SelGenericSurface *)lcd->storage, x,y);

	return 0;
}

const struct luaL_Reg LCDSM[] = {
	{"Home", lcdsl_Home},
	{"SubSurface", lcdsl_subSurface},
	{"Clear", lcdsl_Clear},
	{"SetCursor", lcdsl_SetCursor},
	{"WriteString", lcdsl_WriteString},
	{"GetSize", lcdsl_GetSize},
	{NULL, NULL}    /* End of definition */
};

	/* ***
	 * Toile's exported functions
	 * ***/

static const char * const LuaName(){
	return "SelLCD";
}

static const char * const LuaSName(){
	return "SelLCDSurface";
}

void initExportedSurface(struct SelLCDSurface *srf, struct SelLCDSurface *parent, uint8_t width, uint8_t height, uint8_t left, uint8_t top, struct SelLCDScreen *lcd ){
	slcd_selCore->initGenericSurface((struct SelModule *)&slcd_selLCD, (struct SelGenericSurface *)srf);

	srf->parent = parent;
	srf->screen = lcd;
	srf->w = width;
	srf->h = height;
	srf->origine.x = left;
	srf->origine.y = top;

		/* CAUTION : if the geometry is provided, no boundary check is done */
	if(parent){	/* offset to physical positioning */
		srf->origine.x += parent->origine.x;
		srf->origine.y += parent->origine.y;
	}

	if(!width || !height){	/* Nul : default value */
		if(!parent){	/* Primary surface */
			srf->w = 16;	/* Has there is no way to determine screen size */
			srf->h = 2;		/* we're guessing its a 1602 screen */
		} else {	/* Subsurface */
			srf->w = parent->w - left;
			srf->h = parent->h - top;
		}
	}

	if(!parent)	/* Primary surface */
		srf->obj.cb = &sLCD_cb;
	else 		/* Sub surface */
		srf->obj.cb = &sLCDsub_cb;
}

void initSLSCallBacks(){
	sLCD_cb.LuaObjectName = LuaName;
	sLCD_cb.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))slcd_selLCD.GetSize;
	sLCD_cb.Home = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Home;
	sLCD_cb.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdsc_subSurface;
	sLCD_cb.getPrimary = (void *(*)(struct SelGenericSurface *))lcdc_getPrimary;

	sLCD_cb.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))slcd_selLCD.SetCursor;
	sLCD_cb.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdsc_inSurface;
	sLCD_cb.Clear = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Clear;
	sLCD_cb.WriteString = (bool (*)(struct SelGenericSurface *, const char *))slcd_selLCD.WriteString;

	sLCD_cb.Lock = (bool (*)(struct SelGenericSurface *))lcdsc_Lock;
	sLCD_cb.Unlock = (bool (*)(struct SelGenericSurface *))lcdsc_Unlock;

	sLCDsub_cb.LuaObjectName = LuaSName;
	sLCDsub_cb.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))lcdsc_GetSize;
	sLCDsub_cb.Home = (bool (*)(struct SelGenericSurface *))lcdsc_Home;
	sLCDsub_cb.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdsc_subSurface;
	sLCDsub_cb.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))lcdsc_SetCursor;
	sLCDsub_cb.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdsc_inSurface;
	sLCDsub_cb.Clear = (bool (*)(struct SelGenericSurface *))lcdsc_Clear;
	sLCDsub_cb.WriteString = (bool (*)(struct SelGenericSurface *, const char *))lcdsc_WriteString;

	sLCDsub_cb.Lock = (bool (*)(struct SelGenericSurface *))lcdsc_Lock;
	sLCDsub_cb.Unlock = (bool (*)(struct SelGenericSurface *))lcdsc_Unlock;
}
