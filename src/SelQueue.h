/* SelQueue.h
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 */

#ifndef SELQUEUE_H
#define SELQUEUE_H

#include "selene.h"
#include <pthread.h>

void init_SelQueue( lua_State * );

struct SelQueue {
	struct SelQCItem {
		struct SelQCItem *next;
		int type;
		union {
			const char *s;
			lua_Number n;
		} data;
	} *first, *last;

	pthread_mutex_t mutex;
};

#endif
