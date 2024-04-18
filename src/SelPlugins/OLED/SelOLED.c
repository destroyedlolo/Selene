/***
 *
 * Graphical framework for small displays
 * like SSD1306 I2C driven OLED display.

@classmod SelOLED

 *
 * 26/12/2018 LF : First version
 * 18/04/2024 LF : migrate to v7
 */

#include <Selene/SelOLED/SelOLED.h>
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

static const struct luaL_Reg SelOLEDLib[] = {
	{"oled_type", OLEDot},
#if 0
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
#endif
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
