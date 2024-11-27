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

#include "SelSharedFunctionStorage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct SelSharedFunction selSharedFunction;


static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;
static struct SelElasticStorage *selElasticStorage;

static struct SelSharedFunctionStorage *checkSelSharedFunc(lua_State *L){
	struct SelSharedFunctionStorage **r = selLua->testudata(L, 1, "SelSharedFunction");
	luaL_argcheck(L, r != NULL, 1, "'SelSharedFunction' expected");
	return *r;
}

static struct SelSharedFunctionStorage *ssf_find(const char *name, unsigned int h){
/** 
 * Find a SelSharedFunction by its name.
 *
 * @function Find
 * @tparam string name Name of the SharedFunction
 * @param int hash code (recomputed if null)
 * @treturn ?SelSharedFunction|nil
 */
	return((struct SelSharedFunctionStorage *)selCore->findNamedObject((struct SelModule *)&selSharedFunction, name, h));
}

static int ssf_tostring(lua_State *L){
/**
 * Return function's bytecode as string
 *
 * @function tostring
 *
 * @treturn string
 */
	struct SelSharedFunctionStorage *s = checkSelSharedFunc(L);
	lua_pushstring(L, s->estorage.data);
	return 1;
}

static int ssf_getname(lua_State *L){
/**
 * Return function's Name
 *
 * @function getName
 *
 * @treturn string
 */
	struct SelSharedFunctionStorage *s = checkSelSharedFunc(L);
	lua_pushstring(L, s->estorage.name);
	return 1;
}

static int ssf_loadsharedfunc(lua_State *L){
/**
 * Load a shared function in a slave thread's stats.
 *
 * Used to execute a function from a slave thread : see Selenites/Detach2.sel
 *
 * @function LoadSharedFunction
 *
 * @tparam string name
 * @treturn bytecode
 */
	const char *name = luaL_checkstring(L, 1);

	struct SelSharedFunctionStorage *s = ssf_find(name, 0);
	if(!s){
		selLog->Log('D', "SelSharedFunction(%s) not found", name);
		return 0;
	}
	
	int err;
	if((err = selElasticStorage->loadsharedfunction(L, &s->estorage))){
		lua_pushnil(L);
		lua_pushstring(L, (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error");
		selLog->Log('E', (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error");
		return 2;
	}

	return 1;	/* The function is on the stack */
}

static int ssf_registersharedfunc(lua_State *L){
/**
 * Register a function
 *
 * @function RegisterSharedFunction
 *
 * @tparam function function
 * @tparam string name Name of created reference (optional)
 * @treturn SelSharedFunc
 */
	const char *name = NULL;
	struct SelSharedFunctionStorage *storage;

	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Function needed as 1st argument of SelSharedFunction.Register()");
		selLog->Log('E', "Function needed as 1st argument of SelSharedFunction.Register()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TSTRING ){	/* Named function */
		name = lua_tostring(L, 2);
		if((storage = ssf_find(name, 0))){ /* Already registered */
			struct SelSharedFunctionStorage **r = lua_newuserdata(L, sizeof(struct SelSharedFunctionStorage *));
			assert(r);
			luaL_getmetatable(L, "SelSharedFunction");
			lua_setmetatable(L, -2);
			*r = storage;			
			return 1;
		}
		lua_pop(L, 1);	/* Remove the string as the function must be at the top */
	}
	
		/* Allocate the new storage */
	storage = malloc(sizeof(struct SelSharedFunctionStorage));
	assert(storage);
	assert( selElasticStorage->init(&storage->estorage) );

		/* Dump the function in it */
	if(lua_dump(L, selElasticStorage->dumpwriter , &storage->estorage
#if LUA_VERSION_NUM > 501
		,1
#endif
	) != 0)
		return luaL_error(L, "unable to dump given function");
	lua_pop(L,1);	/* remove the function from the stack */

		/* Register this function */
	if(name){
		const char *dname = strdup(name);
		selCore->registerNamedObject((struct SelModule *)&selSharedFunction, (struct _SelNamedObject *)storage, dname);
		selElasticStorage->SetName(&storage->estorage, dname);
	} else
		selCore->initObject((struct SelModule *)&selSharedFunction, (struct SelObject *)storage);

		/* Push the result on stack */
	struct SelSharedFunctionStorage **r = lua_newuserdata(L, sizeof(struct SelSharedFunctionStorage *));
	assert(r);
	luaL_getmetatable(L, "SelSharedFunction");
	lua_setmetatable(L, -2);
	*r = storage;

	return(1);
}

static const struct luaL_Reg SelFuncSharedLib [] = {
	{"Register", ssf_registersharedfunc},
	{"LoadSharedFunction", ssf_loadsharedfunc},
	{NULL, NULL}
};

static const struct luaL_Reg SelFuncSharedM [] = {
	{"tostring", ssf_tostring},
	{"getName", ssf_getname},
	{NULL, NULL}
};

static void registerSelSharedFunction(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelSharedFunction", SelFuncSharedLib);
	selLua->objFuncs(L, "SelSharedFunction", SelFuncSharedM);
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

	uint16_t found;
	selElasticStorage = (struct SelElasticStorage *)selCore->loadModule("SelElasticStorage", SELELASTIC_STORAGE_VERSION, &found, 'F');
	if(!selElasticStorage)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selSharedFunction, "SelSharedFunction", SELSHAREDFUNCTION_VERSION, LIBSELENE_VERSION))
		return false;

	selSharedFunction.find = ssf_find;

	registerModule((struct SelModule *)&selSharedFunction);

	registerSelSharedFunction(NULL);
	selLua->AddStartupFunc(registerSelSharedFunction);

	return true;
}
