/* SeleneScripting.c
 *
 * Expose Lua scripting functions.
 *
 * 08/02/2024 First version
 */
#include "Selene/SelScripting.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"

static struct SelScripting selScripting;

static struct SeleneCore *selCore;
static struct SelLog *selLog;

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selScripting, "SelScripting", SELSCRIPTING_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selScripting);

	return true;
}
