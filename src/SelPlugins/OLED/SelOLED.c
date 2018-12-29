/* SelOLED
 *
 * This file contains all stuffs related to SSD1306 I2C driven OLED display
 *
 * 26/12/2018 LF : First version
 */

#include "SelOLED.h"

#ifdef USE_OLED
static int OLEDot(lua_State *L){
/* "list" all known screens
 * <- known screen
 */
	int i;
	for(i=0; i<OLED_LAST_OLED; i++){
		lua_pushstring( L, oled_type_str[i] );
	}

	return OLED_LAST_OLED;
}

static int OLEDinit(lua_State *L){
/* Open the display
 * -> 1: screen ID
 * -> 2: i2c device
 * <- success or not (boolean)
 */
	int v = luaL_checkinteger(L, 1);
	const char *arg = luaL_checkstring(L, 2);

	if(v < 1 || v > OLED_LAST_OLED){
		lua_pushnil(L);
		lua_pushstring(L, "Screen ident our of range");
		return 2;
	}

	lua_pushboolean(L, PiOLED_Init(--v, arg));
	return 1;
}

static int OLEDclose(lua_State *L){
/* Free all ressources used for OLED
 */
	PiOLED_Close();
	return 0;
}

static int OLEDdisplay(lua_State *L){
/* refresh the screen
 */
	PiOLED_Display();
	return 0;
}

static int OLEDclear(lua_State *L){
/* Clear the screen
 */
	PiOLED_ClearDisplay();
	return 0;
}

static int OLEDtextsize(lua_State *L){
/* Set the text size
 * -> 1: size of letters
 */
	int v = luaL_checkinteger(L, 1);
	PiOLED_SetTextSize(v);
	return 0;
}

static int OLEDtextcolor(lua_State *L){
/* Set the text color
 * -> 1: front
 * -> 2: background (optional)
 */
	int f = luaL_checkinteger(L, 1);
	if( lua_isnumber(L, 2) ){
		int b = lua_tointeger(L, 2);
		PiOLED_SetTextColor2(f,b);
	} else
		PiOLED_SetTextColor(f);
	return 0;
}

static int OLEDcursor(lua_State *L){
/* Set the text cursor position
 * -> 1 : x
 * -> 2 : y
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	
	PiOLED_SetCursor(x,y);

	return 0;
}

static int OLEDinvert(lua_State *L){
	boolean v;
	if(!lua_isboolean(L,1)){
		lua_pushnil(L);
		lua_pushstring(L, "Boolean expected");
		return 2;
	}

	v = lua_toboolean(L, 1);
	PiOLED_Invert(!!v);
	return 0;
}

static int OLEDprint(lua_State *L){
/* Print a message to the screen
 * -> 1: message to print (string)
 */
	const char *arg = luaL_checkstring(L, 1);

	PiOLED_Print(arg);
	return 0;
}

static const struct luaL_Reg OLEDLib[] = {
	{"oled_type", OLEDot},
	{"Init", OLEDinit},
	{"Display", OLEDdisplay},
	{"Refresh", OLEDdisplay},	/* Alias */
	{"Clear", OLEDclear},
	{"SetTextSize", OLEDtextsize},
	{"SetTextColor", OLEDtextcolor},
	{"SetTextColor", OLEDtextcolor},
	{"SetCursor", OLEDcursor},
	{"Print", OLEDprint},
	{"Invert", OLEDinvert},
	{"Close", OLEDclose},
	{NULL, NULL}    /* End of definition */
};

void initSelOLED(lua_State *L){
	libSel_libFuncs( L, "SelOLED", OLEDLib );
}
#endif
