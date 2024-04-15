/***
Timed window values collection.

The current implementation rely on :

  - time_t is an integer kind of,
  - it represents the number of seconds since era

@classmod SelTimedWindowCollection

 * History :
 *	10/04/2017	LF : First version
 *	17/03/2021	LF : storing in userdata prevents sharing b/w thread
 *		so only a pointer in now stored in the state
 */

#include <Selene/SelTimedWindowCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelTimedWindowCollectionStorage.h"


#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

static struct SelTimedWindowCollection selTimedWindowCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

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

		/* optional modules */
	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'E');

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selTimedWindowCollection, "SelTimedWindowCollection", SELTIMEDWINDOWCOLLECTION_VERSION, LIBSELENE_VERSION))
		return false;

/*
	selTimedCollection.module.dump = stcc_dump;

	selTimedCollection.create = sctc_create;
	selTimedCollection.find = sctc_find;
	selTimedCollection.clear = sctc_clear;
	selTimedCollection.push= sctc_push;
	selTimedCollection.minmaxs= sctc_minmaxs;
	selTimedCollection.minmax= sctc_minmax;
	selTimedCollection.getsize = sctc_getsize;
	selTimedCollection.howmany = sctc_howmany;
	selTimedCollection.getn = sctc_getn;
	selTimedCollection.gets = sctc_gets;
	selTimedCollection.get = sctc_get;
	selTimedCollection.getat = sctc_getat;
	selTimedCollection.save = sctc_save;
	selTimedCollection.load = sctc_load;
*/

	registerModule((struct SelModule *)&selTimedWindowCollection);

#if 0
	if(selLua){	/* Only if Lua is used */
		registerSelTimedCollection(NULL);
		selLua->AddStartupFunc(registerSelTimedCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif
#endif

	return true;
}
