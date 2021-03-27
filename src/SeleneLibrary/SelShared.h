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
#include "sel_Shareable.h"

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
	struct sel_Shareable mutex;	/*AF* As long their is only 2 threads, a simple mutex is enough */
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
#include "SelTimedWindowCollection.h"

enum CollectionType {
	COLTYPE_TIMED,
	COLTYPE_TIMEDWINDOW,
};

	/* Notez-bien : all structures MUST starts by a sel_Shareable */
union Collections {
	struct sel_Shareable *shareable;
	struct SelTimedCollection *timed;
	struct SelTimedWindowCollection *timedwindow;
};

struct SharedCollection {
	struct NameH name;	/* Identifier */
	struct SharedCollection *next;
	enum CollectionType type;
	union Collections collection;
};

	/******
	 * repo of shared objects
	 ******/

extern struct _SharedStuffs {
		/* Shared variables */
	struct SharedVar *first_shvar, *last_shvar;
	struct sel_Shareable mutex_shvar;	/*AF* As long there is only 2 threads, a simple mutex is enough */

	struct elastic_storage *shfunc;	/* shared functions list */
	struct sel_Shareable mutex_sfl;		/* list protection */

	struct SharedFuncRef *shfuncref;	/* shared functions references */
	struct sel_Shareable mutex_sfr;			/* list protection */

		/* pending tasks */
	int todo[SO_TASKSSTACK_LEN];	/* pending tasks list */
	unsigned int ctask;			/* current task index */
	unsigned int maxtask;		/* top of the task stack */
	struct sel_Shareable mutex_tl;	/* tasklist protection */
	int tlfd;	/* Task list file descriptor for eventfd */

		/* Collections */
	struct SharedCollection *collections;
	struct sel_Shareable mutex_collection;

} SharedStuffs;

#endif
