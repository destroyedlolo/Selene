/* sharedobj.h
 *
 * Everything related to shared variables
 *
 * 07/06/2015 LF : First version
 */

#ifndef SHAREDOBJ_H
#define SHAREDOBJ_H

#include "selene.h"
#include <pthread.h>

extern pthread_mutex_t lua_mutex;

void init_shared( lua_State * );
void init_shared_Lua( lua_State * );	/* Init only Lua's object */

enum SharedObjType {
	SOT_UNKNOWN = 0,
	SOT_NUMBER,		/* Integers */
	SOT_STRING,		/* Dynamically allocated string (managed by sharedobj' functions) */
	SOT_XSTRING		/* Const char * managed externally (constant, allocated elsewhere ... */
};

struct SharedVar {
	struct SharedVar *prev, *succ;	/* link list */
	const char *name;
	int H;
	enum SharedObjType type;
	union {
		double num;
		const char *str;
	} val;
	pthread_mutex_t mutex;	/*AF* As long their is only 2 threads, a simple mutex is enough */
};

extern struct _SharedStuffs {
		/* Shared variables */
	struct SharedVar *first_shvar, *last_shvar;
	pthread_mutex_t mutex_shvar;	/*AF* As long there is only 2 threads, a simple mutex is enough */

		/* pending tasks */
	int todo[SO_TASKSSTACK_LEN];	/* pending tasks list */
	unsigned int ctask;			/* current task index */
	unsigned int maxtask;		/* top of the task stack */
	pthread_mutex_t mutex_tl;	/* tasklist protection */
	int tlfd;			/* Task list file descriptor for eventfd */
} SharedStuffs;


	/* exposed API */
extern int pushtask( int, int );
extern void soc_sets( const char *, const char * );

#endif
