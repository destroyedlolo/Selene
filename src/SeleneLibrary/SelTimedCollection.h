/* SelTimedCollection.h
 *
 * Collection of timed values
 *
 * 15/02/2021 LF : emancipate to create shared collection
 */

#ifndef SELTIMEDCOLLECTION_H
#define SELTIMEDCOLLECTION_H

#include "libSelene.h"
#include "sel_Shareable.h"

struct timeddata {
	time_t t;
	lua_Number *data;
};

struct SelTimedCollection {
	struct sel_Shareable shareme;
	struct timeddata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int ndata;	/* how many data per sample */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};

extern struct SelTimedCollection **checkSelTimedCollection(lua_State *);

#endif
