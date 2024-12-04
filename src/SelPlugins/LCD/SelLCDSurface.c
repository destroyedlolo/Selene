/***
 * Manage sub surface on textual LCD screen.
 *

@classmod SelLCDSurface

 * 30/11/2024 LF : First version
 */

#include "SelLCDShared.h"

struct SGS_callbacks sLCD_cb;		/* Primary surface callbacks */
struct SGS_callbacks sLCDsub_cb;	/* Sub surface callbacks */

static struct SelLCDSurface *checkSelLCDSurface(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCDSurface");
	luaL_argcheck(L, r != NULL, 1, "'SelLCDSurface' expected");

	return (struct SelLCDSurface *)r;
}

static bool lcdsc_Home(struct SelLCDSurface *lcd){
	lcd->cursor.x = lcd->cursor.y = 0;

	return true;
}

static int lcdsl_Home(lua_State *L){
	struct SelLCDSurface *lcd = checkSelLCDSurface(L);

	lcd->obj.cb->Home((struct SelGenericSurface *)lcd);

	return 0;
}

static bool lcdsc_GetSize(struct SelLCDSurface *lcd, uint8_t *w, uint8_t *h){
	if(w)
		*w = lcd->w;
	if(h)
		*h = lcd->h;

	return true;
}

static int lcdsl_GetSize(lua_State *L){
	struct SelLCDSurface *lcd = checkSelLCDSurface(L);
	uint8_t w,h;

	lcdsc_GetSize(lcd, &w,&h);

	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	return 2;
}


const struct luaL_Reg LCDSM[] = {
	{"Home", lcdsl_Home},
/*
	{"Clear", lcdsl_Clear},
	{"SetCursor", lcdsl_SetCursor},
	{"WriteString", lcdsl_WriteString},
*/
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

void initExportedSurface(struct SelLCDSurface *srf, struct SelLCDSurface *parent, uint8_t width, uint8_t height, uint8_t left, uint8_t top ){
	slcd_selCore->initGenericSurface((struct SelModule *)&slcd_selLCD, (struct SelGenericSurface *)srf);

	srf->parent = parent;
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
	sLCD_cb.getSize = (bool (*)(struct SelGenericSurface *, uint16_t *, uint16_t *))slcd_selLCD.GetSize;
	sLCD_cb.Home = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Home;

	sLCDsub_cb.LuaObjectName = LuaSName;
	sLCDsub_cb.getSize = (bool (*)(struct SelGenericSurface *, uint16_t *, uint16_t *))lcdsc_GetSize;
	sLCDsub_cb.Home = (bool (*)(struct SelGenericSurface *))lcdsc_Home;
}
