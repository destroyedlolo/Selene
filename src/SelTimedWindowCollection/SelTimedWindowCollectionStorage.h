/* SelTimedWindowCollection.h
 *
 * Timed window values collection
 *
 * 17/03/2021 LF : emancipate to create shared collection
 */
#ifndef SELTIMEDWINDOWCOLLECTION_H
#define SELTIMEDWINDOWCOLLECTION_H

#include <Selene/SelTimedWindowCollection.h>

struct timedwdata {
	time_t t;				/* window segregator : all data stored in this timedwdata belong to it */
	lua_Number min_data;
	lua_Number max_data;

		/* To calcul average */
	lua_Number sum;	/* Sum of data for this window */
	size_t num;		/* Number of data in this window */
};

struct SelTimedWindowCollectionStorage {
	struct _SelNamedObject obj;	/* Object management */

	pthread_mutex_t mutex;	/* Prevent concurrent access */

	struct timedwdata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int last;	/* Last value pointer */
	bool full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
	unsigned long int group;	/* Number of second to group by */
};

#endif
