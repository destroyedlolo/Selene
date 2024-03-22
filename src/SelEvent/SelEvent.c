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

#include "SelEventStorage.h"

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>


static struct SelEvent selEvent;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SelEventStorage *checkSelEvent(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelEvent");
	luaL_argcheck(L, r != NULL, 1, "'SelEvent' expected");
	return (struct SelEventStorage *)r;
}

static int sel_EventCreate(lua_State *L){
/** 
 * Create a new SelEvent
 *
 * @function Create
 * @tparam string /dev/input/event's file
 * @tparam function Function to be called (must be as fast as possible)
 */
	struct SelEventStorage *event;
	const char *fn = luaL_checkstring(L, 1);	/* Event's file */
	int t,f;

	if( lua_type(L, 2) != LUA_TFUNCTION ){
		lua_pushstring(L, "Expecting function for argument #2 of SelEvent.create()");
		lua_error(L);
		exit(EXIT_FAILURE);
	} else
		f = selLua->findFuncRef(L,2);

	if((t = open( fn, O_RDONLY | O_NOCTTY )) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	event = (struct SelEventStorage *)lua_newuserdata(L, sizeof( struct SelEventStorage ));
	luaL_getmetatable(L, "SelEvent");
	lua_setmetatable(L, -2);
	event->fd = t;
	event->func = f;

	return 1;
}

static const struct luaL_Reg SelEventLib [] = {
	{"create", sel_EventCreate},
#if 0
	{"KeyConst", EventKeyConst},
	{"KeyName", EventKeyName},
	{"TypeConst", EventTypeConst},
	{"TypeName", EventTypeName},
#endif
	{NULL, NULL}
};

static const struct luaL_Reg SelEventM [] = {
#if 0
	{"read", EventRead},
	{"Read", EventRead},
#endif
	{NULL, NULL}
};

static void registerSelEvent(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelEvent", SelEventLib);
	selLua->objFuncs(L, "SelEvent", SelEventM);
}

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
