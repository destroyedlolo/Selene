/* tasklist.c
 *
 * Task list management
 *
 * 14/02/2024 First version
 */

#include "tasklist.h"

#include <pthread.h>
#include <unistd.h>	/* write() */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if LUA_VERSION_NUM <= 501
#define lua_rawlen lua_objlen
#endif

static int todo[TASKSSTACK_LEN];	/* pending tasks list */
static unsigned int ctask;			/* current task index */
static unsigned int maxtask;		/* top of the task stack */
static pthread_mutex_t mutex_tl;	/* tasklist protection */
int tlfd;	/* Task list file descriptor for eventfd */

int slc_pushtask(int funcref, enum TaskOnce once){
/**
 * @brief Push funcref in the stack
 * @tparam integer function reference
 * @return 0 : noerror, EUCLEAN = stack full
 */
	uint64_t v = 1;
	pthread_mutex_lock(&mutex_tl);

	if(once != TO_MULTIPLE){
		unsigned int i;
		for(i=ctask; i<maxtask; i++){
			if(todo[i % TASKSSTACK_LEN] == funcref){	/* Already in the stack */
				if(once == TO_LAST)	/* Put it at the end of the queue */
					todo[i % TASKSSTACK_LEN] = LUA_REFNIL;	/* Remove previous reference */
				else {	/* TO_ONCE : Don't push a new one */
					write(tlfd, &v, sizeof(v));
					pthread_mutex_unlock(&mutex_tl);

					return 0;
				}
			}
		}
	}

	if(maxtask - ctask >= TASKSSTACK_LEN){	/* Task is full */
		write(tlfd, &v, sizeof(v));	/* even if our task is not added, unlock others to try to resume this loosing condition */
		pthread_mutex_unlock(&mutex_tl);
		return(errno = EUCLEAN);
	}

	todo[maxtask++ % TASKSSTACK_LEN] = funcref;

	if(!(ctask % TASKSSTACK_LEN)){	/* Avoid counter to overflow */
		ctask %= TASKSSTACK_LEN;
		maxtask %= TASKSSTACK_LEN;
	}

	write(tlfd, &v, sizeof(v));
	pthread_mutex_unlock(&mutex_tl);

	return 0;
}

int slc_handleToDoList(lua_State *L){ /* Execute functions in the ToDo list */
	for(;;){
		int taskid;
		pthread_mutex_lock(&mutex_tl);
		if(ctask == maxtask){	/* No remaining waiting task */
			pthread_mutex_unlock(&mutex_tl);
			break;
		}
		taskid = todo[ctask++ % TASKSSTACK_LEN];
		pthread_mutex_unlock(&mutex_tl);

		if(taskid == LUA_REFNIL)	/* Deleted task */
			continue;

#ifdef DEBUG
printf("*D* todo : %d/%d, tid : %d, stack : %d ", ctask, maxtask, taskid, lua_gettop(L));
#endif
		lua_rawgeti( L, LUA_REGISTRYINDEX, taskid);
#ifdef DEBUG
printf("-> %d (%d : %d)\n", lua_gettop(L), taskid, lua_type(L, -1) );
#endif
		if(lua_pcall( L, 0, 0, 0 )){	/* Call the trigger without arg */
			sl_selLog->Log('E', "(ToDo) %s", lua_tostring(L, -1));
			lua_pop(L, 1); /* pop error message from the stack */
			lua_pop(L, 1); /* pop NIL from the stack */
		}
	}
	return 0;
}

int sll_registerfunc(lua_State *L){
/**
 * Register a function into lookup table
 *
 * @function RegisterFunction
 *
 * @tparam function function
 * @return reference ID
 */
	if(L != sl_selLua.getLuaState()){
		sl_selLog->Log('E', "Selene.RegisterFunction() can be called only by the main thread");
		luaL_error(L, "Selene.RegisterFunction() can be called only by the main thread");
	}

	lua_getglobal(L, FUNCREFLOOKTBL);	/* Check if this function is already referenced */
	if(!lua_istable(L, -1)){
		sl_selLog->Log('E', FUNCREFLOOKTBL " not defined as a table");
		luaL_error(L, FUNCREFLOOKTBL " not defined as a table");
	}
	lua_pop(L,1);

	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Task needed as 1st argument of Selene.RegisterFunction()");
		return 2;
	}

	lua_pushinteger(L, sl_selLua.findFuncRef(L,1));
	return 1;
}

static const struct ConstTranscode _TO[] = {
	{ "MULTIPLE", TO_MULTIPLE },
	{ "ONCE", TO_ONCE },
	{ "LAST", TO_LAST },
	{ NULL, 0 }
};

int sll_TaskOnceConst(lua_State *L ){
/**
 * Transcode "ONCE" code.
 *
 * **ONCE** : don't push if the task is already present in the list.
 * **MULTIPLE** : task will be pushed even if already present.
 * **LAST** : if already in the list, remove and push it as last entry of the list.
 *
 * @function TaskOnceConst
 *
 * @tparam string once
 * @return code
 */
	return sl_selLua.findConst(L, _TO);
}

int sll_PushTaskByRef(lua_State *L){
/**
 * Push a task by its reference
 *
 * **ONCE** : don't push if the task is already present in the list.
 * **MULTIPLE** : task will be pushed even if already present.
 * **LAST** : if already in the list, remove and push it as last entry of the list.
 *
 * @function PushTaskByRef
 *
 * @param reference function's reference
 * @param once **true** : ONCE (default), **false** : MULTIPLE or **Selene.TaskOnceConst("LAST")**
 */
	enum TaskOnce once = TO_ONCE;
	if(lua_type(L, 1) != LUA_TNUMBER){
		lua_pushnil(L);
		lua_pushstring(L, "Task reference needed as 1st argument of Selene.PushTaskByRef()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TBOOLEAN )
		once = lua_toboolean(L, 2) ? TO_ONCE : TO_MULTIPLE;
	else if( lua_type(L, 2) == LUA_TNUMBER )
		once = lua_tointeger(L, 2);

	int err = sl_selLua.pushtask( lua_tointeger(L, 1), once);
	if(err){
		lua_pushnil(L);
		lua_pushstring(L, strerror(err));
		return 2;
	}

	return 0;
}

int sll_PushTask(lua_State *L){
/**
 * Push a task to the waiting list
 *
 * **ONCE** : don't push if the task is already present in the list.
 * **MULTIPLE** : task will be pushed even if already present.
 * **LAST** : if already in the list, remove and push it as last entry of the list.
 *
 * @function PushTask
 *
 * @tparam function function
 * @param once **true** : ONCE (default), **false** : MULTIPLE or **SelShared.TaskOnceConst("LAST")**
 */
	enum TaskOnce once = TO_ONCE;
	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Task needed as 1st argument of SelShared.PushTask()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TBOOLEAN )
		once = lua_toboolean(L, 2) ? TO_ONCE : TO_MULTIPLE;
	else if( lua_type(L, 2) == LUA_TNUMBER )
		once = lua_tointeger(L, 2);

	int err = sl_selLua.pushtask(sl_selLua.findFuncRef(L,1), once);
	if(err){
		lua_pushnil(L);
		lua_pushstring(L, strerror(err));
		return 2;
	}

	return 0;
}

bool slc_isToDoListEmpty(){
	return(ctask == maxtask);
}

int sll_dumpToDoList(lua_State *L){
/**
 * List todo list content
 *
 * @function dumpToDoList
 */
	pthread_mutex_lock( &mutex_tl );
	printf("*D* Dumping pending tasks list : %d / %d\n", ctask, maxtask);
	if(maxtask)
		printf("\t");
	for(int i=ctask; i<maxtask; i++)
		printf("%x ", todo[i % TASKSSTACK_LEN]);
	pthread_mutex_unlock( &mutex_tl );
	if(maxtask)
		puts("");

	return 0;
}
