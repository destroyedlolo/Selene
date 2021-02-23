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
#include <stdbool.h>

#define FUNCREFLOOKTBL "__SELENE_FUNCREF"	/* Function reference lookup table */
extern void initG_SelShared( lua_State * );

	/****
	 * store names of objects
	 ****/

struct NameH {
	const char *name;
	int H;
};

	/****
	 * Shared variables
	 ****/

struct SharedVar {
	struct NameH name;	/* Identifier */
	struct SharedVar *prev, *succ;	/* link list */
	enum SharedObjType type;
	time_t death;	/* when this variable become invalid ? */
	time_t mtime;	/* Time of the last modification */
	union {
		double num;
		const char *str;
	} val;
	pthread_mutex_t mutex;	/*AF* As long their is only 2 threads, a simple mutex is enough */
};


	/******
	 *  shared functions
	 ******/

enum TaskOnce {
	TO_MULTIPLE = 0,	/* Allow multiple run */
	TO_ONCE,			/* Push a task only if it isn't already queued */
	TO_LAST				/* Only one run but put at the end of the queue */
};

extern bool loadandlaunch( lua_State *L, lua_State *newL, struct elastic_storage *, int, int, int, enum TaskOnce );
extern lua_State *createslavethread( void );


	/******
	 *  Tasks
	 ******/

struct SharedFuncRef {
		/* Keep only reference to function */
	struct SharedFuncRef *next;
	const char *name;
	int H;
	int ref;
};

extern int pushtask( int, enum TaskOnce );


	/******
	 * Shared timed collection
	 ******/
#include "SelTimedCollection.h"

struct SharedTimedCollection {
	struct NameH name;	/* Identifier */
	struct SharedTimedCollection *next;
	struct SelTimedCollection *collection;
};

	/******
	 * repo of shared objects
	 ******/

extern struct _SharedStuffs {
		/* Shared variables */
	struct SharedVar *first_shvar, *last_shvar;
	pthread_mutex_t mutex_shvar;	/*AF* As long there is only 2 threads, a simple mutex is enough */

	struct elastic_storage *shfunc;	/* shared functions list */
	pthread_mutex_t mutex_sfl;		/* list protection */

	struct SharedFuncRef *shfuncref;	/* shared functions references */
	pthread_mutex_t mutex_sfr;			/* list protection */

		/* pending tasks */
	int todo[SO_TASKSSTACK_LEN];	/* pending tasks list */
	unsigned int ctask;			/* current task index */
	unsigned int maxtask;		/* top of the task stack */
	pthread_mutex_t mutex_tl;	/* tasklist protection */
	int tlfd;	/* Task list file descriptor for eventfd */

		/* Collections */
	struct SharedTimedCollection *timed;
	pthread_mutex_t mutex_timed;

} SharedStuffs;

#endif
