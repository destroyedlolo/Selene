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

void init_shared( lua_State * );
void init_shared_Lua( lua_State * );	/* Init only Lua's object */
#endif
