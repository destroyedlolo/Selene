/***
This collection stores **immediate** and **average** values sub collection.


  - **Immediate** values : a collection of up to *isize* values;
When a additional one is pushed, the oldest one is pushed out.

  - **Average** values : when *group* **new** data are available in the immediate collection,
their average value is calculated and then pushed into this list.


----------------------
Typical usage : to store frequent metrics (like energy counter) and display both the recent values
and a long term trend to avoid too large curve.

@classmod SelAverageCollection

* History :
* 11/06/2022 LF : creation
* 27/03/2024 LF : migrate to v7

@usage
local col = SelAverageCollection.Create(5,7,3)

for i=1,4 do
	col:Push(i)
end
print( "Size : ", col:GetSize() )
print( "HowMany : ", col:HowMany() )
col:dump() 
 */

#include <Selene/SelAverageCollection.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelAverageCollectionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct SelAverageCollection selAverageCollection;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SelAverageCollectionStorage *checkSelAverageCollection(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelAverageCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelAverageCollection' expected");
	return (struct SelAverageCollectionStorage *)r;
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

		/* optional modules */
	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'E');

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selAverageCollection, "SelAverageCollection", SELAVERAGECOLLECTION_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selAverageCollection);

#if 0
	if(selLua){	/* Only if Lua is used */
		registerSelCollection(NULL);
		selLua->AddStartupFunc(registerSelCollection);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif
#endif

	return true;
}
