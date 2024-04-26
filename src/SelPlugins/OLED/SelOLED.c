/***
 *
 * Graphical framework for small displays
 * like SSD1306 I2C driven OLED display.

@classmod SelOLED

 *
 * 26/12/2018 LF : First version
 * 18/04/2024 LF : migrate to v7
 */

#include <Selene/SelPlug-in/SelOLED.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <ArduiPi_OLED_C.h>

static struct SelOLED selOLED;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static int OLEDot(lua_State *L){
/** "list" all known screens
 *
 * @function oled_type
 * @treturn string Type
 * @treturn string ...
 */
	int i;
	for(i=0; i<OLED_LAST_OLED; i++)
		lua_pushstring( L, oled_type_str[i] );

	return OLED_LAST_OLED;
}

static const struct ConstTranscode _Colors[] = {
	{ "BLACK", BLACK },
	{ "WHITE", WHITE },
	{ NULL, 0 }
};

static int OLEColorsConst( lua_State *L ){
/** Color transcodification
 *
 * @function ColorsConst
 * @tparam string name color name in CAPITAL
 * @treturn number numeric value
 */
	return selLua->findConst(L, _Colors);
}

static int OLEDinit(lua_State *L){
/** Initialize a display
 *
 * @function Init
 * @tparam integer screen ID
 * @tparam string i2c device
 * @treturn boolean does the initialisation succeeded ?
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
/** Free all resources used for OLED
 *
 * @function Close
 */
	PiOLED_Close();
	return 0;
}

static int OLEDdisplay(lua_State *L){
/** refresh the screen
 *
 * @function Display
 */
/** refresh the screen
 *
 * @function Refresh
 */
	PiOLED_Display();
	return 0;
}

static int OLEDclear(lua_State *L){
/** Clear the screen
 *
 * @function Clear
 */
	PiOLED_ClearDisplay();
	return 0;
}

static int OLEDsave(lua_State *L){
/** Save the screen as a PBM
 *
 * @function SaveToPBM
 * @tparam string file target file
 * @treturn boolean does it succeeded ?
 * @usage
SelOLED.SaveToPBM("/tmp/tst.pbm")
 */
	const char *fch = luaL_checkstring(L, 1);
	lua_pushboolean(L, PiOLED_SaveToPBM(fch));
	return 1;
}

static int OLEDtextsize(lua_State *L){
/** Set the text size
 *
 * @function SetTextSize
 * @tparam integer size size of letters
 */
	int v = luaL_checkinteger(L, 1);
	PiOLED_SetTextSize(v);
	return 0;
}

static int OLEDtextcolor(lua_State *L){
/** Set the text color
 *
 * @function SetTextColor
 * @param front
 * @param background (optional)
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
/** Set the text cursor position
 *
 * @function SetCursor
 * @tparam integer x
 * @tparam integer y
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	
	PiOLED_SetCursor(x,y);

	return 0;
}

static int OLEDinvert(lua_State *L){
/** Invert the screen
 *
 * @function Invert
 * @tparam boolean inverted is screen inverted ?
 */
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

static int OLEDflip(lua_State *L){
/** Flip the screen upside-down
 *
 * @function Flip
 * @tparam boolean flipped is screen flipped ?
 */
	boolean v;
	if(!lua_isboolean(L,1)){
		lua_pushnil(L);
		lua_pushstring(L, "Boolean expected");
		return 2;
	}

	v = lua_toboolean(L, 1);
	PiOLED_Flip(!!v);
	return 0;
}

static int OLEDOnOff(lua_State *L){
/** Turn the screen On or Off
 *
 * @function OnOff
 * @tparam boolean power
 */
	boolean v;
	if(!lua_isboolean(L,1)){
		lua_pushnil(L);
		lua_pushstring(L, "Boolean expected");
		return 2;
	}

	v = lua_toboolean(L, 1);
	PiOLED_OnOff(!!v);
	return 0;
}

static int OLEDdrawpixel(lua_State *L){
/** Draw a pixel
 *
 * @function DrawPixel
 * @tparam integer x
 * @tparam integer y
 * @tparam ?integer|nil color
 */
/** Draw a pixel
 *
 * @function Pset
 * @tparam integer x
 * @tparam integer y
 * @tparam ?integer|nil color
 */	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);

	int color = WHITE;
	if( lua_isnumber(L, 3) )
		color = lua_tointeger(L, 3);
	
	PiOLED_DrawPixel(x,y, color);
	return 0;
}

static int OLEDgetpixel(lua_State *L){
/** Get a pixel value
 *
 * @function GetPixel
 * @tparam integer x
 * @tparam integer y
 * @treturn integer the pixel value
 */
/** Get a pixel value
 *
 * @function Point
 * @tparam integer x
 * @tparam integer y
 * @treturn integer the pixel value
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);

	lua_pushinteger(L, PiOLED_getPixel(x,y));
	return 1;
}

static int OLEDdrawline(lua_State *L){
/** Draw a line
 *
 * @function DrawLine
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer bot_x bottom right x
 * @tparam integer bot_y bottom right y
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a line
 *
 * @function Line
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer bot_x bottom right x
 * @tparam integer bot_y bottom right y
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x0 = luaL_checkinteger(L, 1);
	int y0 = luaL_checkinteger(L, 2);
	int x1 = luaL_checkinteger(L, 3);
	int y1 = luaL_checkinteger(L, 4);

	int color = WHITE;
	if( lua_isnumber(L, 5) )
		color = lua_tointeger(L, 5);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 6) )
		pat = lua_tointeger(L, 6);

	PiOLED_DrawLinePat(x0,y0, x1,y1, color, pat);
	return 0;
}

static int OLEDrect(lua_State *L){
/** Draw a rectangle
 *
 * @function DrawRect
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a rectangle
 *
 * @function Box
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);

	int color = WHITE;
	if( lua_isnumber(L, 5) )
		color = lua_tointeger(L, 5);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 6) )
		pat = lua_tointeger(L, 6);

	PiOLED_DrawRectPat(x,y, w,h, color, pat);
	return 0;
}

static int OLEDrectf(lua_State *L){
/** Draw a filled rectangle
 *
 * @function FillRect
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a filled rectangle
 *
 * @function BoxF
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);

	int color = WHITE;
	if( lua_isnumber(L, 5) )
		color = lua_tointeger(L, 5);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 6) )
		pat = lua_tointeger(L, 6);

	PiOLED_FillRectPat(x,y, w,h, color, pat);
	return 0;
}

static int OLEDrrect(lua_State *L){
/** Draw a rounded rectangle
 *
 * @function DrawRoundRect
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a rounded rectangle
 *
 * @function BoxR
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	int r = luaL_checkinteger(L, 5);

	int color = WHITE;
	if( lua_isnumber(L, 6) )
		color = lua_tointeger(L, 6);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 7) )
		pat = lua_tointeger(L, 7);

	PiOLED_DrawRoundRectPat(x,y, w,h, r, color, pat);
	return 0;
}

static int OLEDrfrect(lua_State *L){
/** Draw a rounded filled rectangle
 *
 * @function FillRoundRect
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a rounded filled rectangle
 *
 * @function BoxRF
 * @tparam integer top_x left upper x
 * @tparam integer top_y left upper y
 * @tparam integer width
 * @tparam integer height
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	int r = luaL_checkinteger(L, 5);

	int color = WHITE;
	if( lua_isnumber(L, 6) )
		color = lua_tointeger(L, 6);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 7) )
		pat = lua_tointeger(L, 7);

	PiOLED_FillRoundRectPat(x,y, w,h, r, color, pat);
	return 0;
}

static int OLEDcircle(lua_State *L){
/** Draw a circle
 *
 * @function DrawCircle
 * @tparam integer x center x
 * @tparam integer y center y
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a circle
 *
 * @function Circle
 * @tparam integer x center x
 * @tparam integer y center y
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int r = luaL_checkinteger(L, 3);

	int color = WHITE;
	if( lua_isnumber(L, 4) )
		color = lua_tointeger(L, 4);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 5) )
		pat = lua_tointeger(L, 5);

	PiOLED_DrawCirclePat(x,y, r, color, pat);
	return 0;
}

static int OLEDcirclef(lua_State *L){
/** Draw a filled circle
 *
 * @function FillCircle
 * @tparam integer x center x
 * @tparam integer y center y
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a filled circle
 *
 * @function CircleF
 * @tparam integer x center x
 * @tparam integer y center y
 * @tparam integer radius
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int r = luaL_checkinteger(L, 3);

	int color = WHITE;
	if( lua_isnumber(L, 4) )
		color = lua_tointeger(L, 4);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 5) )
		pat = lua_tointeger(L, 5);

	PiOLED_FillCirclePat(x,y, r, color, pat);
	return 0;
}

static int OLEDtriangle(lua_State *L){
/** Draw a triangle
 *
 * @function DrawTriangle
 * @tparam integer x0
 * @tparam integer y0
 * @tparam integer x1
 * @tparam integer y1
 * @tparam integer x2
 * @tparam integer y2
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a triangle
 *
 * @function Triangle
 * @tparam integer x0
 * @tparam integer y0
 * @tparam integer x1
 * @tparam integer y1
 * @tparam integer x2
 * @tparam integer y2
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x0 = luaL_checkinteger(L, 1);
	int y0 = luaL_checkinteger(L, 2);
	int x1 = luaL_checkinteger(L, 3);
	int y1 = luaL_checkinteger(L, 4);
	int x2 = luaL_checkinteger(L, 5);
	int y2 = luaL_checkinteger(L, 6);

	int color = WHITE;
	if( lua_isnumber(L, 7) )
		color = lua_tointeger(L, 7);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 8) )
		pat = lua_tointeger(L, 8);

	PiOLED_DrawTrianglePat(x0,y0, x1,y1, x2,y2, color, pat);
	return 0;
}

static int OLEDftriangle(lua_State *L){
/** Draw a filled triangle
 *
 * @function FillCircle
 * @tparam integer x0
 * @tparam integer y0
 * @tparam integer x1
 * @tparam integer y1
 * @tparam integer x2
 * @tparam integer y2
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
/** Draw a filled triangle
 *
 * @function TriangleF
 * @tparam integer x0
 * @tparam integer y0
 * @tparam integer x1
 * @tparam integer y1
 * @tparam integer x2
 * @tparam integer y2
 * @tparam ?integer|nil color
 * @tparam ?integer|nil pattern
 */
	int x0 = luaL_checkinteger(L, 1);
	int y0 = luaL_checkinteger(L, 2);
	int x1 = luaL_checkinteger(L, 3);
	int y1 = luaL_checkinteger(L, 4);
	int x2 = luaL_checkinteger(L, 5);
	int y2 = luaL_checkinteger(L, 6);

	int color = WHITE;
	if( lua_isnumber(L, 7) )
		color = lua_tointeger(L, 7);

	uint16_t pat = 0xffff;
	if( lua_isnumber(L, 8) )
		pat = lua_tointeger(L, 8);

	PiOLED_FillTrianglePat(x0,y0, x1,y1, x2,y2, color, pat);
	return 0;
}

static int OLEDvbar(lua_State *L){
/** Draw a vertical bar
 *
 * @function DrawVerticalBargraph
 * @tparam integer top_x top left
 * @tparam integer top_y top left
 * @tparam integer width
 * @tparam integer hight
 * @tparam integer percent fuel value
 * @tparam ?integer|nil color
 */
/** Draw a vertical bar
 *
 * @function VerticalGauge
 * @tparam integer top_x top left
 * @tparam integer top_y top left
 * @tparam integer width
 * @tparam integer hight
 * @tparam integer percent fuel value
 * @tparam ?integer|nil color
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	int p = luaL_checkinteger(L, 5);

	int color = WHITE;
	if( lua_isnumber(L, 6) )
		color = lua_tointeger(L, 6);

	PiOLED_DrawVerticalBargraph(x,y, w,h, color, p);
	return 0;
}

static int OLEDhbar(lua_State *L){
/** Draw a horizontal bar
 *
 * @function DrawHorizontalBargraph
 * @tparam integer top_x top left
 * @tparam integer top_y top left
 * @tparam integer width
 * @tparam integer hight
 * @tparam integer percent fuel value
 * @tparam ?integer|nil color
 */
/** Draw a horizontal bar
 *
 * @function HorizontalGauge
 * @tparam integer top_x top left
 * @tparam integer top_y top left
 * @tparam integer width
 * @tparam integer hight
 * @tparam integer percent fuel value
 * @tparam ?integer|nil color
 */
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	int p = luaL_checkinteger(L, 5);

	int color = WHITE;
	if( lua_isnumber(L, 6) )
		color = lua_tointeger(L, 6);

	PiOLED_DrawHorizontalBargraph(x,y, w,h, color, p);
	return 0;
}

static int OLEDprint(lua_State *L){
/** Print a message to the screen
 *
 * @function Print
 * @tparam string message
 */
	const char *arg = luaL_checkstring(L, 1);

	PiOLED_Print(arg);
	return 0;
}

static int OLEDwidth(lua_State *L){
/** Return the width of the screen
 *
 * @function Width
 * @treturn integer width
 */
	lua_pushinteger(L, PiOLED_DisplayWidth());
	return 1;
}

static int OLEDHeight(lua_State *L){
/** Return the height of the screen
 *
 * @function Height
 * @treturn integer height
 */
	lua_pushinteger(L, PiOLED_DisplayHeight());
	return 1;
}

static const struct luaL_Reg SelOLEDLib[] = {
	{"oled_type", OLEDot},
	{"ColorsConst", OLEColorsConst},
	{"Init", OLEDinit},
	{"Close", OLEDclose},
	{"Display", OLEDdisplay},
	{"Refresh", OLEDdisplay},	/* Alias */
	{"Clear", OLEDclear},
	{"SaveToPBM", OLEDsave},
	{"SetTextSize", OLEDtextsize},
	{"SetTextColor", OLEDtextcolor},
	{"SetCursor", OLEDcursor},
	{"Invert", OLEDinvert},
	{"OnOff", OLEDOnOff},
	{"Flip", OLEDflip},
	{"DrawPixel", OLEDdrawpixel},
	{"Pset", OLEDdrawpixel},	/* Alias */
	{"GetPixel", OLEDgetpixel},
	{"Point", OLEDgetpixel},
	{"DrawLine", OLEDdrawline},
	{"Line", OLEDdrawline},		/* Alias */
	{"DrawRect", OLEDrect},
	{"Box", OLEDrect},			/* Alias */
	{"FillRect", OLEDrectf},
	{"BoxF", OLEDrectf},		/* Alias */
	{"DrawRoundRect", OLEDrrect},
	{"BoxR", OLEDrrect},		/* Alias */
	{"FillRoundRect", OLEDrfrect},
	{"BoxRF", OLEDrfrect},		/* Alias */
	{"DrawCircle", OLEDcircle},
	{"Circle", OLEDcircle},		/* Alias */
	{"FillCircle", OLEDcirclef},
	{"CircleF", OLEDcirclef},		/* Alias */
	{"DrawTriangle", OLEDtriangle},
	{"Triangle", OLEDtriangle},	/* Alias */
	{"FillTriangle", OLEDftriangle},
	{"TriangleF", OLEDftriangle},	/* Alias */
	{"DrawHorizontalBargraph", OLEDvbar},
	{"HorizontalGauge", OLEDhbar},	/* Alias */
	{"DrawVerticalBargraph", OLEDvbar},
	{"VerticalGauge", OLEDvbar},	/* Alias */
	{"Print", OLEDprint},
	{"Width", OLEDwidth},
	{"Height", OLEDHeight},
	{NULL, NULL}    /* End of definition */
};

static void registerSelOLED(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelOLED", SelOLEDLib);
/*	No object's methods
	selLua->objFuncs(L, "SelOLED", SelOLEDM);
*/
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Other mandatory modules */
	selLua =  (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selOLED, "SelOLED", SELOLED_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selOLED);

	registerSelOLED(NULL);
	selLua->AddStartupFunc(registerSelOLED);

	return true;
}
