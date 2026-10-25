/***
 * Framebuffer graphics using DRM/KMS and Cairo.
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X or Wayland.
 *

@classmod SelDRMCairo

 * 25/10/2026 LF : version 2
 */

#include <Selene/SelPlug-in/SelDRMCairo/SelDRMCairo.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

struct SelDRMCairo sdrmc_selDRMCairo;

struct SeleneCore *sdrmc_selCore;
struct SelLog *sdrmc_selLog;
struct SelLua *sdrmc_selLua;

static void registerSelDRMCairo(lua_State *L){
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/

bool InitModule( void ){
		/* Core modules */
	sdrmc_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!sdrmc_selCore)
		return false;

	sdrmc_selLog = (struct SelLog *)sdrmc_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!sdrmc_selLog)
		return false;

		/* Other mandatory modules */
	sdrmc_selLua = (struct SelLua *)sdrmc_selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&sdrmc_selDRMCairo, "SelDRMCairo", SELDRMCAIRO_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&sdrmc_selDRMCairo);

	if(sdrmc_selLua){	/* Only if Lua is used */
		registerSelDRMCairo(NULL);
		sdrmc_selLua->AddStartupFunc(registerSelDRMCairo);
	}
#ifdef DEBUG
	else
		sdrmc_selLog->Log('D', "SelLua not loaded");
#endif

}
