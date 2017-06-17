/*	SelTimedCollection.h
 *
 *	Values collection associated to a timestamp
 *
 * 	10/04/2017	LF : First version
 */

#ifndef SELTCOLLECTION_H
#define SELTCOLLECTION_H

#include "selene.h"

#include <time.h>

void init_SelTimedCollection( lua_State * );

struct timeddata {
	time_t t;
	lua_Number data;
};

struct SelTimedCollection {
	struct timeddata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};

#endif
