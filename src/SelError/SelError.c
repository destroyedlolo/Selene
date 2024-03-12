/* SelError.h
 *
 * Error management
 *
 * 09/03/2024 First version
 */

#include <Selene/SelError.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>

struct SelError selError;

struct SeleneCore *selCore;
struct SelLog *selLog;
struct SelLua *selLua;

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

	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;

		/* Other mandatory modules */

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selError, "SelError", SELERROR_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selError);

	return true;
}
