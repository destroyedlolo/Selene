/***
 * Surface defintions shared by SelLCDSurface and SelLCDsubSurface.
 */

#include <Selene/SelPlug-in/SelLCD/SelLCDSharedSurface.h>
#include "SelLCDShared.h"

#include <string.h>

void *slss_getPrimary(struct SelLCDSharedSurface *s){
	return s->screen;
}

bool slss_inSurface(struct SelLCDSharedSurface *s, uint32_t x, uint32_t y){
	return( x < s->w && y < s->h );
}

	/* Lua exposed methods shared by all LCD objects */

static struct SelLCDSharedSurfaceLua *checkSelLCDderived(lua_State *L){
	const char *name = slcd_selLua->getMetaTableName(L, 1);
	bool ok = false;

	if(!strcmp(name, "SelLCDScreen"))
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
#if 0
	{"SubSurface", lcdl_subSurface},
	{"Refresh", lcdl_Refresh},
	{"DumpBuffer", lcdl_dump},
#endif
	{NULL, NULL}    /* End of definition */
};
