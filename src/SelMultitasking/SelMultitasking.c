/* SelMultitasking.h
 *
 * Detached tasks
 *
 * 23/02/2024 First version
 */
#include <Selene/SelMultitasking.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

struct SelMultitasking selMultitasking;

struct SeleneCore *selCore;
struct SelLog *selLog;
struct SelLua *selLua;

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

pthread_attr_t thread_attr;

	/* Arguments to be passed to the function to be launched */

struct launchargs {
	lua_State *L;	/* New thread Lua state */
	int nargs;		/* Number of arguments for the function */
	int nresults;	/* Number of results */
	int triggerid;	/* Trigger to add in todo list if return true */
	enum TaskOnce trigger_once;
};

static void *launchfunc(void *a){
/* Low level code the launch the detached function in a new thread 
 */
	struct launchargs *arg = a;	/* To avoid further casting */

	if(lua_pcall( arg->L, arg->nargs, arg->nresults, 0))
		selLog->Log('E', "(launch) %s\n", lua_tostring(arg->L, -1));
	else {
		if(arg->triggerid != LUA_REFNIL){
			if(lua_toboolean(arg->L, -1))
				selLua->pushtask(arg->triggerid, arg->trigger_once);
		}
	}

	lua_close(arg->L);	/* Remove the new state */
	free(arg);			/* free arguments */
	return NULL;
}

lua_State *slc_createSlaveState(void){
/**
 * Create and initialize a new state for slave threads
 *
 * @function createSlaveState
 *
 * @return new Lua state
 */
	lua_State *tstate = luaL_newstate();
	assert(tstate);
	luaL_openlibs( tstate );

#if 0 /*AF */
	libSel_ApplyStartupFunc( SalveInitFunctionsList, tstate );
	initSelTimedCollection( tstate );
#endif

	return tstate;
}

	/* *****
	 *  shared functions
	 *
	 *  RegisterSharedFunction / LoadSharedFunction : 
	 *  	share functions across threads (run the func in another thread)
	 *  RegisterFunction / PushTask(ByRef) :
	 *  	task list
	 * *****/

struct readerdt {
	int somethingtoread;
	struct elastic_storage *func;
};

static const char *reader( lua_State *L, void *ud, size_t *size ){
	struct readerdt *tracking = (struct readerdt *)ud;

	if( !tracking->somethingtoread )	/* It's over */
		return NULL;

	*size = tracking->func->data_sz; /* Read everything at once */
	tracking->somethingtoread = 0;

	return tracking->func->data;
}

static int loadsharedfunction(lua_State *L, struct elastic_storage *func){
	struct readerdt dt;
	dt.somethingtoread = 1;
	dt.func = func;

	return lua_load( L, reader, &dt, func->name ? func->name : "unnamed"
#if LUA_VERSION_NUM > 501
		, NULL
#endif
	);
}

bool slc_loadandlaunch( lua_State *L, lua_State *newL, struct elastic_storage *storage, int nargs, int nresults, int trigger, enum TaskOnce trigger_once){
/**
 * load and then launch a stored function in a slave thread
 *
 * @function loadandlaunch
 *
 * @tparam state L master thread (for error reporting, may be NULL)
 * @tparam state newL slave thread
 * @tparam elastic_storage storage the function stored
 * @tparam integer nargs number of arguments to the functions
 * @tparam integer trigger if not LUA_REFNIL, push this trigger_id in the todo list
 * @tparam TaskOnce trigger_once
 *
 * @treturn boolean succeeded or not
 */

		/* It's needed because this structure has to survive until
		 * slave function is over.
		 * It will be cleared in launchfunc()
		 */
	struct launchargs *arg = malloc( sizeof(struct launchargs) );
	assert(arg);
	arg->L = newL;
	arg->nargs = nargs;
	arg->nresults = nresults;
	arg->triggerid = trigger;

	int err;
	if((err = loadsharedfunction(newL, storage))){
		if(L){
			lua_pushnil(L);
			lua_pushstring(L, (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error");
		} else
			selLog->Log('E', "Can't create a new thread : %s", (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error" );
		return false;
	}

	if(nargs)	/* Move the function before its arguments */
		lua_insert(newL, -1 - nargs);

	pthread_t tid;	/* No need to be kept */
	if(pthread_create( &tid, &thread_attr, launchfunc,  arg) < 0){
		selLog->Log('E', "Can't create a new thread : %s", strerror(errno));
		if(L){
			lua_pushnil(L);
			lua_pushstring(L, strerror(errno));
		}
		return false;
	}
	return true;
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

	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selMultitasking, "SelMultitasking", SELMULTITASKING_VERSION, LIBSELENE_VERSION))
		return false;

	selMultitasking.createSlaveState = slc_createSlaveState;
	selMultitasking.loadandlaunch = slc_loadandlaunch;

	registerModule((struct SelModule *)&selMultitasking);

	return true;
}
