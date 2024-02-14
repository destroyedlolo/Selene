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
