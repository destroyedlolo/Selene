/* multitasking.c
 *
 * Manage detached tasks
 *
 * 14/02/2024 First version
 */

#include "tasklist.h"

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

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
		sl_selLog->Log('E', "(launch) %s\n", lua_tostring(arg->L, -1));
	else {
		if( arg->triggerid != LUA_REFNIL){
			if(lua_toboolean(arg->L, -1))
				sl_selLua.pushtask( arg->triggerid, arg->trigger_once );
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

}
