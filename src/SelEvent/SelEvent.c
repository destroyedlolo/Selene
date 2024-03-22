/***
Linux' input events interface

[Input events](https://www.kernel.org/doc/html/v4.17/input/event-codes.html) is the way to interact with
input devices (keyboard, touchscreen, mice, ...)

@classmod SelEvent

 * History :
 * 24/04/2017 LF : First version
 * 07/04/2018 LF : Migrate to Selene v4
 * 22/03/2024 LF : Migrate to Selene v7
 */

#include <Selene/SelEvent.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>

static struct SelEvent selEvent;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

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
	if(!selLua)
		return false;

		/* optional modules */
	
		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selEvent, "selEvent", SELEVENT_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selEvent);

	registerSelEvent(NULL);
	selLua->AddStartupFunc(registerSelEvent);

	return true;
}
