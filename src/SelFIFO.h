/* SelFIFO.h
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 */

#ifndef SELFIFO_H
#define SELFIFO_H

#include "selene.h"
#include <pthread.h>

extern void init_SelFIFO( lua_State * );
extern int sff_push(lua_State *);

struct SelFIFO {
	struct SelFIFOCItem {
		struct SelFIFOCItem *next;
		int type;
		union {
			char *s;
			lua_Number n;
		} data;			/* payload */
		int userdt;		/* additional (and optional) user data */
	} *first, *last;

	pthread_mutex_t mutex;	/* prevent concurrent access */

		/* Linked list function */
	struct SelFIFO *next;
	const char *name;
	int h;
};

#endif
