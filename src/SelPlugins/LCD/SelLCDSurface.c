/***
 * Manage sub surface on textual LCD screen.
 *

@classmod SelLCDSurface

 * 30/11/2024 LF : First version
 */

#include "SelLCDShared.h"

static bool lcdsc_GetSize(struct SelLCDSurface *lcd, uint8_t *w, uint8_t *h){
	if(w)
		*w = lcd->w;
	if(h)
		*h = lcd->h;

	return true;
}

const struct luaL_Reg LCDSM[] = {
/*
	{"Clear", lcdsl_Clear},
	{"Home", lcdsl_Home},
	{"SetCursor", lcdsl_SetCursor},
	{"WriteString", lcdsl_WriteString},
	{"GetSize", lcdsl_GetSize},
*/
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
	slcd_selCore->initExportedSurface((struct SelModule *)&slcd_selLCD, (struct ExportedSurface *)srf);

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

	if(!parent){	/* Primary surface */
		srf->obj.LuaObjectName = LuaName;
		srf->obj.getSize = (bool (*)(struct ExportedSurface *, uint16_t *, uint16_t *))slcd_selLCD.GetSize;
	} else {		/* Sub surface */
		srf->obj.LuaObjectName = LuaSName;
		srf->obj.getSize = (bool (*)(struct ExportedSurface *, uint16_t *, uint16_t *))lcdsc_GetSize;
	}
}
