/*	SelTimedCollection.h
 *
 *	Values collection associated to a time window
 *
 * 	05/06/2017	LF : First version
 */

#ifndef SELTWCOLLECTION_H
#define SELTWCOLLECTION_H

#include "selene.h"

#include <time.h>

extern void init_SelTimedWindowCollection( lua_State * );

struct timedwdata {
	time_t t;
	float min_data;
	float max_data;
};

struct SelTimedWindowCollection {
	struct timedwdata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
	unsigned long int group;	/* Number of second to group by */
};

#endif
