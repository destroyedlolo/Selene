/* SelDRMCairo.h
 *
 * 10/05/2020 LF : Creation
 * 26/04/2024 LF : Migrate to v7
 */

#include <Selene/SelPlug-in/SelDRMCairo.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

struct SelDRMCairo dc_selDRMCairo;

struct SeleneCore *dc_selCore;
struct SelLog *dc_selLog;
struct SelLua *dc_selLua;


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

/*
	registerSelCurses(NULL);
	dc_selLua->AddStartupFunc(registerSelCurses);
*/

	return true;
}
