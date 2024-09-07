/***
 * Display messages on an LCD textual screen (like 1602 one)

@classmod SelOLED

 * 06/09/2024 LF : First version
 */

#include <Selene/SelPlug-in/SelLCD.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

static struct SelLCD selLCD;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

struct LCDscreen {
	int bus;		/* I2C bus file descriptor */
	bool backlight;	/* is backlight enabled */
};

static const struct luaL_Reg LCDLib[] = {
	{NULL, NULL}    /* End of definition */
};

static void registerSelLCD(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelLCD", LCDLib);
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
	if(!initModule((struct SelModule *)&selLCD, "SelLCD", SELLCD_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selLCD);

	registerSelLCD(NULL);
	selLua->AddStartupFunc(registerSelLCD);

	return true;
}
