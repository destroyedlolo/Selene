/* SelSharedFunction.c
 *
 * Share function among threads
 *
 * 03/03/2024 First version
 */

#include <Selene/SelSharedFunction.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelElasticStorage.h>

static struct SelSharedFunction selSharedFunction;


static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;
static struct SelElasticStorage *selElasticStorage;

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

	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;

	selElasticStorage = (struct SelElasticStorage *)selCore->findModuleByName("SelElasticStorage", SELELASTIC_STORAGE_VERSION,'F');
	if(!selLua)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selSharedFunction, "SelSharedFunction", SELSHAREDFUNCTION_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selSharedFunction);

	return true;
}
