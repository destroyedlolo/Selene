/* SeleneLua.c
 *
 * Selene's Lua language support.
 *
 * SelLua provides reduced APIs to Séléné's based applications that
 * use Lua as helper.
 *
 * SelScripting module provides full APIs and targets application based on
 * Séléné, where Séléné acts as a core component and manages all the aspects.
 *
 * 06/02/2024 First version
 */

#include <Selene/SelLua.h>
#include "tasklist.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>

struct SelLua sl_selLua;

lua_State *sl_mainL;	/* Main thread Lua's state (to make the initialisation easier */
struct SeleneCore *sl_selCore;
struct SelLog *sl_selLog;

#if LUA_VERSION_NUM <= 501
void *luaL_testudata(lua_State *L, int ud, const char *tname){
/* Like luaL_checkudata() but w/o crashing if doesn't march
 * From luaL_checkudata() source code
 * This function appeared with 5.2 so it's a workaround for 5.1
 */
	void *p = lua_touserdata(L, ud);
	if(p){
		if(lua_getmetatable(L, ud)){  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			if(!lua_rawequal(L, -1, -2))  /* does it have the correct mt ? */
				p = NULL;	/* No */
			lua_pop(L, 2);  /* remove both metatables */
			return p;
		}
	}
	return NULL;	/* Not an user data */
}
#endif

static lua_State *slc_getLuaState(){
/**
 * @brief Returns main thread Lua state
 *
 * @function getLuaState
 * @return LuaState
 */
	return sl_mainL;
}

static bool slc_libFuncs(lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = sl_mainL;

#if LUA_VERSION_NUM > 501
	lua_newtable(L);
	luaL_setfuncs (L, funcs, 0);
	lua_pushvalue(L, -1);	// pluck these lines out if they offend you
	lua_setglobal(L, name); // for they clobber the Holy _G
#else
	luaL_register(L, name, funcs);
#endif
	lua_pop(L, 1);
	return true;
}

static bool slc_libAddFuncs(lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = sl_mainL;

	lua_getglobal(L, name);
	if(!lua_istable(L, -1)){
		lua_pop(L, 1);
		sl_selLog->Log('E', "Can't add functions to unknown library \"%s\"", name);
		return false;
	}
#if LUA_VERSION_NUM > 501
	luaL_setfuncs (L, funcs, 0);
#else
	luaL_register(L, NULL, funcs);
#endif
	lua_pop(L, 1);
	return true;
}

static bool slc_libCreateOrAddFuncs(lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = sl_mainL;

	lua_getglobal(L, name);
	if(lua_isnil(L,-1)){
		lua_pop(L, 1);
		return slc_libFuncs(L, name, funcs);
	} else {
		lua_pop(L, 1);
		return slc_libAddFuncs(L, name, funcs);
	}
}

static bool slc_objFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs){
	if(!L)
		L = sl_mainL;

	luaL_newmetatable(L, name);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */

#if LUA_VERSION_NUM < 503
	/* Insert __name field if Lua < 5.3
	 * on 5.3+, it's provided out of the box
	 */
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__name");
#endif

	if(funcs){	/* May be NULL if we're creating an empty metatable */
#if LUA_VERSION_NUM > 501
		luaL_setfuncs( L, funcs, 0);
#else
		luaL_register(L, NULL, funcs);
#endif
	}

	return true;
}

static int slc_findConst(lua_State *L, const struct ConstTranscode *tbl){
	const char *arg = luaL_checkstring(L, 1);	/* Get the constant name to retreave */
	bool found;
	int i = sl_selCore->findConst(arg,tbl,&found);

	if(!found){
		lua_pushnil(L);
		lua_pushstring(L, arg);
		lua_pushstring(L," : Unknown constant");
		lua_concat(L, 2);
		return 2;
	} else {
		lua_pushnumber(L, i);
		return 1;
	}
}

static int slc_rfindConst(lua_State *L, const struct ConstTranscode *tbl){
 	int arg = luaL_checkinteger(L, 1);	/* Get the integer to retrieve */

	const char *res = sl_selCore->rfindConst(arg,tbl);

	if(!res){
		lua_pushnil(L);
		lua_pushinteger(L, arg);
		lua_tostring(L, -1);
		lua_pushstring(L," : Unknown constant");
		lua_concat(L, 2);

		return 2;
	} else {
		lua_pushstring(L, res);
		return 1;
	}
}

static int slc_findFuncRef(lua_State *L, int num){
/**
 * @brief Find a function reference
 * @tparam integer id fonction identifier
 * @treturn integer function identifier
 */
	if(L != sl_selLua.getLuaState()){
		sl_selLog->Log('E', "findFuncRef() called outside the main thread");
		luaL_error(L, "findFuncRef() called outside the main thread");
	}

	lua_getglobal(L, FUNCREFLOOKTBL);	/* Check if this function is already referenced */
	if(!lua_istable(L, -1)){
		sl_selLog->Log('E', FUNCREFLOOKTBL " not defined as a table");
		luaL_error(L, FUNCREFLOOKTBL " not defined as a table");
	}

	lua_pushvalue(L, num);	/* The function is the key */
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)){	/* Doesn't exist yet */
		lua_pop(L, 1);	/* Remove nil */

		lua_pushvalue(L, num); /* Get its reference */
		int func = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_pushvalue(L, num); 		/* Push the function as key */
		lua_pushinteger(L, func);	/* Push it's reference */
		lua_settable(L, -3);

		lua_pop(L, 1);	/* Remove the table */
		return func;
	} else {	/* Reference already exists */
		lua_remove(L, -2);	/* Remove the table */
		int func = luaL_checkinteger(L, -1);
		lua_pop(L, 1);	/* Pop the reference */
		return func;
	}
}

static int slc_getToDoListFD(void){
	return tlfd;
}

static void slc_dumpstack(lua_State *L){
	int i;
	int top = lua_gettop(L);

	sl_selLog->Log('D', "Stack trace");
	sl_selLog->Log('D', "===========");

	for(i = 1; i <= top; i++){  /* repeat for each level */
		int t = lua_type(L, i);
		switch(t){
          case LUA_TSTRING:  /* strings */
		  	sl_selLog->Log('D', "String : \"%s\"", lua_tostring(L, i));
            break;
          case LUA_TBOOLEAN:  /* booleans */
		  	sl_selLog->Log('D', "Bool : \"%s\"", lua_toboolean(L, i) ? "true" : "false");
            break;
          case LUA_TNUMBER:  /* numbers */
		  	sl_selLog->Log('D', "Number : %g", lua_tonumber(L, i));
            break;
          default:  /* other values */
		  	sl_selLog->Log('D', lua_typename(L, t));
            break;
		}
	}
}

static int ssl_Hostname( lua_State *L ){
/** 
 * @brief Get the host's name.
 *
 * @function getHostname
 * @treturn string the host's name
 */
	char n[HOST_NAME_MAX];
	gethostname(n, HOST_NAME_MAX);

	lua_pushstring(L, n);
	return 1;
}

static int ssl_getPID( lua_State *L ){
/** 
 * @brief Get the current process ID
 *
 * @function getPid
 * @treturn num PID of the current process
 */
	lua_pushinteger(L, getpid());
	return 1;
}

static int ssl_Use(lua_State *L){
/** 
 * @brief Load a module
 *
 * @function Use
 * @tparam module_name Load a module
 * @treturn boolean does it succeed
 */
	uint16_t verfound;
	const char *name = luaL_checkstring(L, 1);

		/* No need to check for version as it only meaningful at C level */
	struct SelModule *m = sl_selCore->findModuleByName(name, 0, 0);

	if(m)	/* Already found */
		return 1;

	m = sl_selCore->loadModule(name, 0, &verfound, 'E');	/* load it */

	if(m){
		if(m->initLua)
			m->initLua(&sl_selLua);
		lua_pushboolean(L, 1);
	} else
		lua_pushboolean(L, 0);
	return 1;
}

static int ssl_LetsGo(lua_State *L){
/** 
 * @brief Do all late operation before running our application
 *
 * @function LetsGo
 */
	sl_selLog->Log('D', "Late dependencies building");

	for(struct SelModule *m = modules; m; m=m->next){	/* Ensure all dependencies are met */
		if(!m->checkdependencies()){
			if(m->laterebuilddependancies)
				m->laterebuilddependancies();
		}
	}

	sl_selLog->Log('D', "Let's go ...");
	return 0;
}

static const struct luaL_Reg seleneAdminLib[] = {
	{"Use", ssl_Use},
	{"LetsGo", ssl_LetsGo},
	{NULL, NULL} /* End of definition */
};

static int slc_exposeAdminAPI(lua_State *L){
/**
 * @Brief expose restricted/admin API to Lua state
 *
 * Some functions are only for Lua main thread but can be useful also
 * to enhance application where Lua is only an helper. As example, to load
 * additional Séléné modules.
 *
 * Notez-bien : same function is used both in Lua and C side.
 */
	sl_selLua.libCreateOrAddFuncs(L, "Selene", seleneAdminLib);

	return 0;
}

static const struct luaL_Reg seleneLib[] = {
	{"Hostname", ssl_Hostname},
	{"getHostname", ssl_Hostname},
	{"getPid", ssl_getPID},
	{"exposeAdminAPI", slc_exposeAdminAPI},
	{NULL, NULL} /* End of definition */
};

static void registerSelene(lua_State *L){
	sl_selLua.libCreateOrAddFuncs(L, "Selene", seleneLib);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	sl_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!sl_selCore)
		return false;

	sl_selLog = (struct SelLog *)sl_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!sl_selLog)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&sl_selLua, "SelLua", SELLUA_VERSION, LIBSELENE_VERSION))
		return false;

	sl_selLua.getLuaState = slc_getLuaState;
	sl_selLua.libFuncs = slc_libFuncs;
	sl_selLua.libAddFuncs = slc_libAddFuncs;
	sl_selLua.libCreateOrAddFuncs = slc_libCreateOrAddFuncs;
	sl_selLua.objFuncs = slc_objFuncs;

	sl_selLua.findConst = slc_findConst;
	sl_selLua.rfindConst = slc_rfindConst;

	sl_selLua.testudata = luaL_testudata;
	sl_selLua.exposeAdminAPI = slc_exposeAdminAPI;

	sl_selLua.findFuncRef = slc_findFuncRef;
	sl_selLua.pushtask = slc_pushtask;
	sl_selLua.getToDoListFD = slc_getToDoListFD;
	sl_selLua.handleToDoList = slc_handleToDoList;

	sl_selLua.registerfunc = sll_registerfunc;
	sl_selLua.dumpstack = slc_dumpstack;
	sl_selLua.TaskOnceConst = sll_TaskOnceConst;
	sl_selLua.PushTaskByRef = sll_PushTaskByRef;
	sl_selLua.PushTask= sll_PushTask;
	sl_selLua.isToDoListEmpty = slc_isToDoListEmpty;
	sl_selLua.dumpToDoList = sll_dumpToDoList;

	sl_selLua.AddStartupFunc = slc_AddStartupFunc;
	sl_selLua.ApplyStartupFunc = slc_ApplyStartupFunc;

	registerModule((struct SelModule *)&sl_selLua);

		/* Initialize Lua */
	sl_mainL = luaL_newstate();
	luaL_openlibs(sl_mainL);

		/* Define globals variables*/
	lua_pushnumber(sl_mainL, SELENE_VERSION);	/* Expose version to lua side */
	lua_setglobal(sl_mainL, "SELENE_VERSION");

		/* Functions lookup table */
	lua_newtable(sl_mainL);
	lua_setglobal(sl_mainL, FUNCREFLOOKTBL);

		/* initialize evenfd */
	if((tlfd = eventfd( 0, 0 )) == -1){
		sl_selLog->Log('E', "SelLua's eventfd() : %s", strerror(errno));
		return false;
	}

		/* Link with already loaded module */
	for(struct SelModule *m = modules; m; m = m->next){
		if(m->initLua)
			m->initLua(&sl_selLua);
	}
	
	registerSelene(NULL);
	sl_selLua.AddStartupFunc(registerSelene);

	return true;
}
