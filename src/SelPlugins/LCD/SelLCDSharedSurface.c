/***
 * Surface defintions shared by SelLCDSurface and SelLCDsubSurface.
 */

#include <Selene/SelPlug-in/SelLCD/SelLCDSharedSurface.h>
#include <Selene/SelPlug-in/SelLCD/SelLCDSubSurface.h>
#include "SelLCDShared.h"

#include <string.h>
#include <stdlib.h>

bool lcdss_getSize(struct SelLCDSharedSurface *lcd, uint32_t *w, uint32_t *h){
	if(w)
		*w = lcd->w;
	if(h)
		*h = lcd->h;

	return true;
}

void *lcdss_getPrimary(struct SelLCDSharedSurface *s){
	return s->screen;
}

void *lcdss_getParent(struct SelLCDSharedSurface *s){
	return s->parent;
}

bool lcdss_inSurface(struct SelLCDSharedSurface *s, uint32_t x, uint32_t y){
	return( x < s->w && y < s->h );
}

struct SelLCDSubSurface *lcdss_subSurface(struct SelLCDSharedSurface *p, uint32_t x, uint32_t y, uint32_t w, uint32_t h, struct SelLCDScreen *lcd){
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

	struct SelLCDSubSurface *srf = malloc(sizeof(struct SelLCDSubSurface));
	if(!srf)
		return NULL;

	initSharedSurface(&srf->shared, p, w,h, x,y, lcd);
	srf->shared.obj.cb = &cb_subsurface;

	return srf;
}

bool lcdss_Home(struct SelLCDSharedSurface *lcd){
	lcd->cursor.x = lcd->cursor.y = 0;

	return true;
}

bool lcdss_setCursor(struct SelLCDSharedSurface *lcd, uint32_t x, uint32_t y){
	lcd->cursor.x = x;
	lcd->cursor.y = y;

	return true;
}

void initSharedSurface(struct SelLCDSharedSurface *srf, struct SelLCDSharedSurface *parent, uint8_t width, uint8_t height, uint8_t left, uint8_t top, struct SelLCDScreen *lcd ){
	slcd_selCore->initGenericSurface((struct SelModule *)&slcd_selLCD, (struct SelGenericSurface *)srf);

	srf->parent = parent;
	srf->screen = lcd;
	srf->w = width;
	srf->h = height;
	srf->origine.x = left;
	srf->origine.y = top;

	if(!width || !height){	/* Nul : default value */
		if(!parent){	/* Primary surface */
			srf->w = 16;	/* Has there is no way to determine screen size */
			srf->h = 2;		/* we're guessing its a 1602 screen */
		} else {	/* Subsurface */
			srf->w = parent->w - left;
			srf->h = parent->h - top;
		}
	}
}


	/* *********
	 * Lua exposed methods shared by all LCD objects 
	 * *********/

static struct SelLCDSharedSurfaceLua *checkSelLCDderived(lua_State *L){
	const char *name = slcd_selLua->getMetaTableName(L, 1);
	bool ok = false;

	if(!strcmp(name, "SelLCDScreen"))
		ok = true;
	else if(!strcmp(name, "SelLCDSubSurface"))
		ok = true;

	lua_pop(L, 1);
	luaL_argcheck(L, ok, 1, "SelLCD's surface kind expected");

	return (struct SelLCDSharedSurfaceLua *)lua_touserdata(L,1);
}

static int lcdl_Clear(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);

	lcd->storage->obj.cb->Clear(&lcd->storage->obj);

	return 0;
}

static int lcdl_Home(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);

	lcd->storage->obj.cb->Home(&lcd->storage->obj);

	return 0;
}

static int lcdl_SetCursor(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);
	uint16_t x = lua_tonumber(L, 2);
	uint16_t y = lua_tonumber(L, 3);

	lcd->storage->obj.cb->setCursor(&lcd->storage->obj, x,y);

	return 0;
}

static int lcdl_WriteString(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);
	const char *txt = luaL_checkstring(L,2);

	lcd->storage->obj.cb->WriteString(&lcd->storage->obj, txt);

	return 0;
}

static int lcdl_GetSize(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);
	uint32_t w,h;

	lcd->storage->obj.cb->getSize(&lcd->storage->obj, &w,&h);

	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	return 2;
}

static int lcdl_subSurface(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);
	uint8_t x = lua_tonumber(L, 2);
	uint8_t y = lua_tonumber(L, 3);
	uint8_t w = lua_tonumber(L, 4);
	uint8_t h = lua_tonumber(L, 5);

	struct SelLCDSubSurface *srf = (struct SelLCDSubSurface *)lcd->storage->obj.cb->subSurface(&lcd->storage->obj, x,y, w,h, lcd->storage->obj.cb->getPrimary(&lcd->storage->obj));
	if(!srf)
		return 0;

	struct SelLCDSubSurfaceLua *srfl = (struct SelLCDSubSurfaceLua *)lua_newuserdata(L, sizeof(struct SelLCDSubSurfaceLua));
	srfl->storage = srf;

	luaL_getmetatable(L, "SelLCDSubSurface");
	lua_setmetatable(L, -2);

	return 1;
}

static int lcdl_getVisibility(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);

	bool res = lcd->storage->obj.cb->getVisibility(&lcd->storage->obj);
	lua_pushboolean(L, res);
	return 1;
}

static int lcdl_setVisibility(lua_State *L){
	struct SelLCDSharedSurfaceLua *lcd = checkSelLCDderived(L);
	bool v = lua_toboolean(L, 2);

	bool res = lcd->storage->obj.cb->setVisibility(&lcd->storage->obj, v);
	lua_pushboolean(L, res);
	return 1;
}

		/* here, only the methods managed the same way whatever the
		 * LCD object's kind.
		 * If a method applies to a subset of object or if is implemented
		 * in a different way (like refresh), it's local to the said object.
		 */
const struct luaL_Reg LCDShared[] = {
	{"Clear", lcdl_Clear},
	{"Home", lcdl_Home},
	{"SetCursor", lcdl_SetCursor},
	{"WriteString", lcdl_WriteString},
	{"GetSize", lcdl_GetSize},
	{"SubSurface", lcdl_subSurface},
	{"GetVisibility", lcdl_getVisibility},
	{"SetVisibility", lcdl_setVisibility},
#if 0
	{"Refresh", lcdl_Refresh},
	{"Dump", lcdl_dump},
#endif
	{NULL, NULL}    /* End of definition */
};
