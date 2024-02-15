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


