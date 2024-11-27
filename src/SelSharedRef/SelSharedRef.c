/* SelSharedRef.c
 *
 * Store Lua reference.
 *
 * 07/05/2024 First version
 */

#include <Selene/SelSharedRef.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include "SelSharedRefStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct SelSharedRef selSharedRef;


static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

#if 0
static struct SelSharedRefStorage *checkSelSharedFunc(lua_State *L){
	struct SelSharedRefStorage **r = selLua->testudata(L, 1, "SelSharedRef");
	luaL_argcheck(L, r != NULL, 1, "'SelSharedRef' expected");
	return *r;
}
#endif

static struct SelSharedRefStorage *ssr_find(const char *name, unsigned int h){
/** 
 * Find a SelSharedRef by its name.
 *
 * @function Find
 * @tparam string name Name of the SelSharefRef
 * @param int hash code (recomputed if null)
 * @treturn ?SelSharefRef|nil
 */
	return((struct SelSharedRefStorage *)selCore->findNamedObject((struct SelModule *)&selSharedRef, name, h));
}

static int ssrl_find(lua_State *L){
/**
 * Find a reference by its name
 *
 * @function Find
 *
 * @tparam string name
 * @treturn ?number|nil if not found
 */
	const char *name = luaL_checkstring(L, 1);
	struct SelSharedRefStorage *st = ssr_find(name, 0);

	if(st){
		lua_pushnumber(L, st->ref);
		return 1;
	}
	return 0;
}

static int ssr_registersharedref(lua_State *L){
/**
 * Register a reference
 *
 * @function RegisterRef
 *
 * @tparam number Reference to be registered
 * @tparam string name
 * @treturn SelSharedRef
 */
	const char *name = luaL_checkstring(L, 2);
	struct SelSharedRefStorage *storage;

	if(lua_type(L, 1) != LUA_TNUMBER){
		selLog->Log('E', "Number needed as 1st argument of SelSharedRef.Register(), got %s", lua_typename(L,1));
		lua_pushnil(L);
		lua_pushstring(L, "Number needed as 1st argument of SelSharedRef.Register()");
		return 2;
	}

	if((storage = ssr_find(name, 0))){ /* Already registered */
		struct SelSharedRefStorage **r = lua_newuserdata(L, sizeof(struct SelSharedRefStorage *));
		assert(r);
		luaL_getmetatable(L, "SelSharedRef");
		lua_setmetatable(L, -2);
		*r = storage;			
		return 1;
	}
	lua_pop(L, 1);	/* Remove the string as the function must be at the top */

		/* Allocate the new storage */
	storage = malloc(sizeof(struct SelSharedRefStorage));
	assert(storage);
	storage->ref = luaL_checknumber(L,1);

		/* Register this Ref */
	selCore->registerNamedObject((struct SelModule *)&selSharedRef, (struct _SelNamedObject *)storage, name);

		/* Push the result on stack */
	struct SelSharedRefStorage **r = lua_newuserdata(L, sizeof(struct SelSharedRefStorage *));
	assert(r);
	luaL_getmetatable(L, "SelSharedRef");
	lua_setmetatable(L, -2);
	*r = storage;

	return(1);
}

static const struct luaL_Reg SelRefSharedLib [] = {
	{"Register", ssr_registersharedref},
	{"Find", ssrl_find},
	{NULL, NULL}
};

#if 0
static const struct luaL_Reg SelRefSharedM [] = {
	{"tostring", ssf_tostring},
	{"getName", ssf_getname},
	{NULL, NULL}
};
#endif

static void registerSelSharedRef(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelSharedRef", SelRefSharedLib);
#if 0
	selLua->objFuncs(L, "SelSharedRef", SelRefSharedM);
#endif
}

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

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selSharedRef, "SelSharedRef", SELSHAREDREF_VERSION, LIBSELENE_VERSION))
		return false;

	selSharedRef.find = ssr_find;

	registerModule((struct SelModule *)&selSharedRef);

	registerSelSharedRef(NULL);
	selLua->AddStartupFunc(registerSelSharedRef);

	return true;
}
