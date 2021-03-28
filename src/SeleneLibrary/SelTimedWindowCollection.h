/* SelTimedWindowCollection.h
 *
 * Timed window values collection
 *
 * 17/03/2021 LF : emancipate to create shared collection
 */
#ifndef SELTIMEDWINDOWCOLLECTION_H
#define SELTIMEDWINDOWCOLLECTION_H

#include "libSelene.h"
#include "sel_Shareable.h"

struct timedwdata {
	time_t t;				/* window segregator : all data stored in this timedwdata belong to it */
	lua_Number min_data;
	lua_Number max_data;
};

struct SelTimedWindowCollection {
	struct sel_Shareable shareme;
	struct timedwdata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
	unsigned long int group;	/* Number of second to group by */
};

extern struct SelTimedWindowCollection **checkSelTimedWindowCollection(lua_State *);
#endif
