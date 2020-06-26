/* SelFIFO.h
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 *	07/04/2018	LF : Migrate to Selene v4
 *	26/06/2020	LF : CAUTION userdt changed from int to lua_Number
 */

#ifndef SELFIFO_H
#define SELFIFO_H

#include "libSelene.h"
#include <pthread.h>

struct SelFIFO {
	struct SelFIFOCItem {
		struct SelFIFOCItem *next;
		int type;
		union {
			char *s;
			lua_Number n;
		} data;			/* payload */
		lua_Number userdt;		/* additional (and optional) user data */
	} *first, *last;

	pthread_mutex_t mutex;	/* prevent concurrent access */

		/* Linked list function */
	struct SelFIFO *next;
	const char *name;
	int h;
};

#endif
