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

void init_SelFIFO( lua_State * );

struct SelFIFO {
	struct SelFIFOCItem {
		struct SelFIFOCItem *next;
		int type;
		union {
			const char *s;
			lua_Number n;
		} data;			/* payload */
		int userdt;		/* additional (and optional) user data */
	} *first, *last;

	pthread_mutex_t mutex;	/* prevent concurrent access */
};

#endif
