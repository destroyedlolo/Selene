/* sharedvar.h
 *
 * SharedVar definition
 *
 * 05/03/2024 First version
 */

#ifndef SHAREDVAR_H
#define SHAREDVAR_H

#include <Selene/SelSharedVar.h>

#include <pthread.h>
#include <time.h>

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

#endif
