/* SelShared.h
 *
 * Everything related to shared variables
 *
 * 07/06/2015 LF : First version
 * 11/11/2015 LF : Add TaskOnce enum
 * 20/01/2016 LF : Rename as SelShared
 * 16/04/2016 LF : Add TTL for variables
 *
 * 05/04/2018 LF : Move to Selene v4
 */

#ifndef SHAREDOBJ_H
#define SHAREDOBJ_H

#include "libSelene.h"
#include "configuration.h"
#include "elastic_storage.h"

#include <pthread.h>

extern void init_sharedRepo( lua_State * );

	/****
	 * Shared variables
	 ****/

enum SharedObjType {
	SOT_UNKNOWN = 0,	/* Invalid variable */
	SOT_NUMBER,		/* Integers */
	SOT_STRING,		/* Dynamically allocated string (managed by sharedobj' functions) */
	SOT_XSTRING		/* Const char * managed externally (constant, allocated elsewhere ...) */
};

struct SharedVar {
	struct SharedVar *prev, *succ;	/* link list */
	const char *name;
	int H;
	enum SharedObjType type;
	time_t death;	/* when this variable become invalid ? */
	time_t mtime;	/* Time of the last modification */
	union {
		double num;
		const char *str;
	} val;
	pthread_mutex_t mutex;	/*AF* As long their is only 2 threads, a simple mutex is enough */
};

extern void soc_sets( const char *, const char * );

	/******
	 *  shared functions
	 ******/

extern int ssf_dumpwriter(lua_State *, const void *, size_t, void *);

enum TaskOnce {
	TO_MULTIPLE = 0,	/* Allow multiple run */
	TO_ONCE,			/* Push a task only if it isn't already queued */
	TO_LAST				/* Only one run but put at the end of the queue */
};

#ifdef NOT_YET
extern int pushtask( int, enum TaskOnce );	
#endif
	
	/******
	 * repo of shared objects
	 ******/

extern struct _SharedStuffs {
		/* Shared variables */
	struct SharedVar *first_shvar, *last_shvar;
	pthread_mutex_t mutex_shvar;	/*AF* As long there is only 2 threads, a simple mutex is enough */

	struct elastic_storage *shfunc;	/* shared functions list */
	pthread_mutex_t mutex_sfl;		/* shared functions protection */
#ifdef NOT_YET
		/* pending tasks */
	int todo[SO_TASKSSTACK_LEN];	/* pending tasks list */
	unsigned int ctask;			/* current task index */
	unsigned int maxtask;		/* top of the task stack */
	pthread_mutex_t mutex_tl;	/* tasklist protection */
	int tlfd;	/* Task list file descriptor for eventfd */
#endif
} SharedStuffs;

#endif
