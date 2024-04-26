/* SelDRMCairo.h
 *
 * 10/05/2020 LF : Creation
 * 26/04/2024 LF : Migrate to v7
 */

#include "DRMCairo.h"

struct SelDRMCairo dc_selDRMCairo;

struct SeleneCore *dc_selCore;
struct SelLog *dc_selLog;
struct SelLua *dc_selLua;

struct DRMCairoContext DMCContext;

static void cleanDRMC(){
	FT_Done_FreeType(DMCContext.FT);
}

static void registerSelDRMCairo(lua_State *L){
	_include_SelDCCard(L);

#ifdef DRMC_WITH_FB
	if(!L){
		lua_pushboolean(dc_selLua->getLuaState(), true);       /* Expose plugin to lua side */
		lua_setglobal(dc_selLua->getLuaState(), "SELPLUG_DRMCairo_FBEXTENSION");
	}
#endif
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	dc_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!dc_selCore)
		return false;

	dc_selLog = (struct SelLog *)dc_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!dc_selLog)
		return false;

		/* Other mandatory modules */
	dc_selLua =  (struct SelLua *)dc_selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&dc_selDRMCairo, "SelCurses", SELCAIRO_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&dc_selDRMCairo);

	FT_Error err;
	if((err = FT_Init_FreeType(&DMCContext.FT)) != FT_Err_Ok){
		dc_selLog->Log('F',"Error %d opening FreeType library.", err);
		exit(EXIT_FAILURE);
	}
	atexit(cleanDRMC);
	pthread_mutex_init( &DMCContext.FT_mutex, NULL);

	registerSelDRMCairo(NULL);
	dc_selLua->AddStartupFunc(registerSelDRMCairo);

	return true;
}

